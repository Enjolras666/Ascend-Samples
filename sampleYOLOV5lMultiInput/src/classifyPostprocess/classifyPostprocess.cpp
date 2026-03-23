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
#include "acl/acl.h"
#include "Params.h"
#include "classifyPostprocess.h"
#include "AclLiteUtils.h"
#include "AclLiteApp.h"
#include "imageNetClasses.h"
#include "label.h"

using namespace std;

namespace {
    const uint32_t kSleepTime = 500;
    const double kFountScale = 0.5;
    const cv::Scalar kFountColor(0, 0, 255);
    const uint32_t kLabelOffset = 11;
    const uint32_t kLineSolid = 2;
    const vector <cv::Scalar> kColors{
        cv::Scalar(237, 149, 100), cv::Scalar(0, 215, 255),
        cv::Scalar(50, 205, 50), cv::Scalar(139, 85, 26)};
    typedef struct BoundBox {
        float x;
        float y;
        float width;
        float height;
        float score;
        size_t classIndex;
        size_t index;
    } BoundBox;

    bool sortScore(BoundBox box1, BoundBox box2)
    {
        return box1.score > box2.score;
    }
}

ClassifyPostprocessThread::ClassifyPostprocessThread(uint32_t modelWidth, uint32_t modelHeight,
    aclrtRunMode& runMode, uint32_t batch)
    :modelWidth_(modelWidth), modelHeight_(modelHeight), runMode_(runMode),
    sendLastBatch_(false), batch_(batch)
{
}

ClassifyPostprocessThread::~ClassifyPostprocessThread() {
}

AclLiteError ClassifyPostprocessThread::Init()
{
    return ACLLITE_OK;
}

AclLiteError ClassifyPostprocessThread::Process(int msgId, shared_ptr<void> data)
{
    AclLiteError ret = ACLLITE_OK;
    switch (msgId) {
        case MSG_POSTPROC_CLAASSIFYDATA:
            InferOutputProcess(static_pointer_cast<DetectDataMsg>(data));
            MsgSend(static_pointer_cast<DetectDataMsg>(data));
            break;
        default:
            ACLLITE_LOG_INFO("Detect PostprocessThread thread ignore msg %d", msgId);
            break;
    }

    return ret;
}

AclLiteError ClassifyPostprocessThread::InferOutputProcess(shared_ptr<DetectDataMsg> detectDataMsg)
{
    int hasData = detectDataMsg->batchData.size();
    if (hasData == 0) {
        return ACLLITE_OK;
    }
    if (detectDataMsg->batchData[0].hasDetected == false) {
        return ACLLITE_OK;
    }
    if (detectDataMsg->classifyInferenceOutput.empty()) {
        return ACLLITE_OK;
    }

    void* outHostData = CopyDataToHost(detectDataMsg->classifyInferenceOutput[0].data.get(),
        detectDataMsg->classifyInferenceOutput[0].size, runMode_, MEMORY_NORMAL);
    if (outHostData == nullptr) {
        ACLLITE_LOG_ERROR("Copy inference output to host failed");
        return ACLLITE_ERROR_COPY_DATA;
    }
    float* outData = reinterpret_cast<float*>(outHostData);
    
    uint32_t totalSize = detectDataMsg->classifyInferenceOutput[0].size;
    totalSize = totalSize/8;
    for (int i=0;i<detectDataMsg->batchData[0].classifyDataMsg.size();i++){
        map<float, unsigned int, greater<float> > resultMap;
        for (unsigned int j = 1000*i; j < 1000*(i+1); ++j) {
            resultMap[*outData] = j-i*1000;
            outData++;
        }
        auto it = resultMap.begin();
        // ACLLITE_LOG_INFO(" index[%d] value[%f] frame[%d] class[%s]", it->second,
        //             it->first, detectDataMsg->msgNum, classifyLabel[it->second].c_str());
        detectDataMsg->batchData[0].classifyDataMsg[i].classify_result = classifyLabel[it->second].c_str();
    }
    aclrtFreeHost(outHostData);
    outHostData = nullptr;
    outData = nullptr;

    return ACLLITE_OK;
}

AclLiteError ClassifyPostprocessThread::MsgSend(shared_ptr<DetectDataMsg> detectDataMsg)
{

    if (!detectDataMsg->isLastFrame) {
        while (1) {
            AclLiteError ret = SendMessage(detectDataMsg->dataOutputThreadId, MSG_OUTPUT_FRAME, detectDataMsg);
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
    }
    if (detectDataMsg->isLastFrame) {
        while (1) {
            AclLiteError ret = SendMessage(detectDataMsg->dataOutputThreadId, MSG_ENCODE_FINISH, detectDataMsg);
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
    }

    return ACLLITE_OK;
}