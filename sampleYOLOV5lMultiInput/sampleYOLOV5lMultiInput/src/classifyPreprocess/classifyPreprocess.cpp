/**
* Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.apache.org/licenses/LICENSE-2.0

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.

* File sample_process.cpp
* Description: handle acl resource
*/
#include <iostream>
#include <sys/timeb.h>
#include "Params.h"
#include "classifyPreprocess.h"
#include "AclLiteApp.h"

using namespace std;

namespace {
const uint32_t kSleepTime = 500;
const int kInvalidSize = -1;
}

ClassifyPreprocessThread::ClassifyPreprocessThread(uint32_t modelWidth, uint32_t modelHeight,
    uint32_t batch)
    :modelWidth_(modelWidth), modelHeight_(modelHeight), isReleased(false), batch_(batch)
{
}

ClassifyPreprocessThread::~ClassifyPreprocessThread()
{
    if(!isReleased) {
        dvpp_.DestroyResource();
    }
    isReleased = true;
}

AclLiteError ClassifyPreprocessThread::Init()
{
    AclLiteError aclRet = dvpp_.Init("DVPP_CHNMODE_VPC");
    if (aclRet) {
        ACLLITE_LOG_ERROR("Dvpp init failed, error %d", aclRet);
        return ACLLITE_ERROR;
    }

    return ACLLITE_OK;
}

AclLiteError ClassifyPreprocessThread::Process(int msgId, shared_ptr<void> data)
{
    switch (msgId) {
        case MSG_PREPROC_CLAASSIFYDATA:
            MsgProcess(static_pointer_cast<DetectDataMsg>(data));
            MsgSend(static_pointer_cast<DetectDataMsg>(data));
            break;
        default:
            ACLLITE_LOG_INFO("Detect Preprocess thread ignore msg %d", msgId);
            break;
    }

    return ACLLITE_OK;
}

AclLiteError ClassifyPreprocessThread::Crop(vector<ClassifyDataMsg> &detectResultImg, ImageData &orgImg)
{
    static int cnt = 0;
    AclLiteError ret = ACLLITE_OK;
    for (int i = 0; i < detectResultImg.size(); i++) {
        if (detectResultImg[i].lt.x < 0) {
            detectResultImg[i].lt.x = 0;
        }
        if (detectResultImg[i].lt.y < 0) {
            detectResultImg[i].lt.y = 0;
        }
        if (detectResultImg[i].rb.x > orgImg.width) {
            detectResultImg[i].rb.x = orgImg.width;
        }
        if (detectResultImg[i].rb.y > orgImg.height) {
            detectResultImg[i].rb.y = orgImg.height;
        }
        ret = dvpp_.Crop(detectResultImg[i].cropedImgs, orgImg,
                         detectResultImg[i].lt.x, detectResultImg[i].lt.y,
                         detectResultImg[i].rb.x, detectResultImg[i].rb.y);
        if (ret) {
            ACLLITE_LOG_ERROR("Crop image failed, error: %d, orgImg width %d, "
                              "height %d, size %d, crop area (%d, %d) (%d, %d)",
                              ret, orgImg.width, orgImg.height,
                              detectResultImg[i].cropedImgs.size, detectResultImg[i].lt.x,
                              detectResultImg[i].lt.y, detectResultImg[i].rb.x,
                              detectResultImg[i].rb.y);
            return ACLLITE_ERROR;
        }
    }
    return ret;
}

AclLiteError ClassifyPreprocessThread::Resize(vector<ClassifyDataMsg> &detectResultImg)
{
    AclLiteError ret = ACLLITE_OK;
    for (size_t i = 0; i < detectResultImg.size(); i++) {
        AclLiteError ret = dvpp_.Resize(detectResultImg[i].resizedImgs, detectResultImg[i].cropedImgs,
                                        modelWidth_, modelHeight_);
        if (ret) {
            ACLLITE_LOG_ERROR("ColorRecognition Resize image failed");
            return ACLLITE_ERROR;
        }
    }
    return ret;
}

int ClassifyPreprocessThread::CopyImageData(uint8_t *buffer,
                                   uint32_t bufferSize, ImageData& image)
{
    aclrtRunMode runMode;
    aclrtGetRunMode(&runMode);
    uint32_t dataSize = YUV420SP_SIZE(modelWidth_, modelHeight_);
    AclLiteError ret = CopyDataToDeviceEx(buffer, bufferSize, image.data.get(),
                                          dataSize, runMode);
    if (ret) {
        ACLLITE_LOG_ERROR("Copy face data to device failed");
        return kInvalidSize;
    }

    return dataSize;
}

int ClassifyPreprocessThread::CopyOneBatchImages(uint8_t* buffer, uint32_t bufferSize,
                                        vector<ClassifyDataMsg> &detectResultImg, int batchIdx)
{
    uint32_t j = 0;
    int dataLen = 0;
    int totalSize = 0;

    for (uint32_t i = batchIdx * batch_;
         i < detectResultImg.size() && j < batch_ && bufferSize > totalSize;
         i++, j++) {
        dataLen = CopyImageData(buffer + totalSize,
                                bufferSize - totalSize, detectResultImg[i].resizedImgs);
        if (dataLen == kInvalidSize) {
            return kInvalidSize;
        }
        totalSize += dataLen;
    }
    
    if (j < batch_) {
        for (uint32_t k = 0;
             k < batch_ - j && bufferSize > totalSize;
             k++) {
            dataLen = CopyImageData(buffer + totalSize,
                                    bufferSize - totalSize,
                                    detectResultImg[detectResultImg.size() - 1].resizedImgs);
            if (dataLen == kInvalidSize) {
                return kInvalidSize;
            }
            totalSize += dataLen;
        }
    }

    return j;
}

AclLiteError ClassifyPreprocessThread::MsgProcess(shared_ptr<DetectDataMsg> detectDataMsg)
{
    int hasData = detectDataMsg->batchData.size();
    if (hasData == 0) {
        return ACLLITE_OK;
    }

    AclLiteError ret;
    uint32_t  modelInputSize = YUV420SP_SIZE(modelWidth_, modelHeight_) * batch_;
    void* buf = nullptr;
    ret = aclrtMalloc(&buf, modelInputSize, ACL_MEM_MALLOC_HUGE_FIRST);
    if ((buf == nullptr) || (ret != ACL_ERROR_NONE)) {
        ACLLITE_LOG_ERROR("Malloc classify inference input buffer failed, "
                          "error %d", ret);
        return ACLLITE_ERROR;
    }
    uint8_t* batchBuffer = (uint8_t *)buf;
    int32_t setValue = 0;
    aclrtMemset(batchBuffer, modelInputSize, setValue, modelInputSize);

    // 对每一张图片
    // 判断是否检测出有效检测结果
    detectDataMsg->batchData[0].hasDetected = true;
    if (detectDataMsg->batchData[0].classifyDataMsg.size() == 0) {
        detectDataMsg->batchData[0].hasDetected = false;
        return ACLLITE_OK;
    }
    // crop
    ret = Crop(detectDataMsg->batchData[0].classifyDataMsg, detectDataMsg->batchData[0].decodedImg);
    if (ret) {
        ACLLITE_LOG_ERROR("Crop all the data failed, all the data failed");
        return ACLLITE_ERROR;
    }
    // resize
    ret = Resize(detectDataMsg->batchData[0].classifyDataMsg);
    if (ret) {
        ACLLITE_LOG_ERROR("Resize all the data failed, all the data failed");
        return ACLLITE_ERROR;
    }
    // Copy one batch preprocessed image data to device
    int detectedNum = CopyOneBatchImages(batchBuffer, modelInputSize,
                                    detectDataMsg->batchData[0].classifyDataMsg, 0);
    if (detectedNum < 0) {
        ACLLITE_LOG_ERROR("Copy the 0th batch images failed");
        return ACLLITE_ERROR;
    }
    detectDataMsg->classifyModelInputImg.data = SHARED_PTR_DEV_BUF(batchBuffer);
    detectDataMsg->classifyModelInputImg.size = modelInputSize;
    return ACLLITE_OK;
}

AclLiteError ClassifyPreprocessThread::MsgSend(shared_ptr<DetectDataMsg> detectDataMsg)
{
    while (1) {
        AclLiteError ret = SendMessage(detectDataMsg->classifyInferThreadId, MSG_DO_CLASSIFY_INFER, detectDataMsg);
        if (ret == ACLLITE_ERROR_ENQUEUE) {
            usleep(kSleepTime);
            continue;
        } else if(ret == ACLLITE_OK) {
            break;
        } else {
            ACLLITE_LOG_ERROR("Send read frame message failed, error %d", ret);
            return ret;
        }
    }
    return ACLLITE_OK;
}
