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
#include <cmath>
#include <algorithm>
#include "acl/acl.h"
#include "AclLiteApp.h"
#include "AclLiteModel.h"
#include "Params.h"
#include "classifyInference.h"

using namespace std;

namespace {
const uint32_t kSleepTime = 500;
}

ClassifyInferenceThread::ClassifyInferenceThread(string modelPath)
    :model_(modelPath), isReleased(false)
{
}

ClassifyInferenceThread::~ClassifyInferenceThread()
{
    if(!isReleased) {
        model_.DestroyResource();
    }
    isReleased = true;
}

AclLiteError ClassifyInferenceThread::Init()
{
    AclLiteError ret = model_.Init();
    if (ret != ACLLITE_OK) {
        ACLLITE_LOG_ERROR("Model init failed, error:%d", ret);
        return ret;
    }
    return ACLLITE_OK;
}

AclLiteError ClassifyInferenceThread::ModelExecute(shared_ptr<DetectDataMsg> detectDataMsg)
{
    int hasData = detectDataMsg->batchData.size();
    if (hasData == 0) {
        return ACLLITE_OK;
    }
    if (detectDataMsg->batchData[0].hasDetected == false) {
        return ACLLITE_OK;
    }
    AclLiteError ret = model_.CreateInput(detectDataMsg->classifyModelInputImg.data.get(),
                                          detectDataMsg->classifyModelInputImg.size);
    if (ret != ACLLITE_OK) {
        ACLLITE_LOG_ERROR("Create model input dataset failed");
        return ACLLITE_ERROR;
    }

    ret = model_.Execute(detectDataMsg->classifyInferenceOutput);
    if (ret != ACLLITE_OK) {
        ACLLITE_LOG_ERROR("Execute detect model inference failed, error: %d", ret);
        return ACLLITE_ERROR;
    }
    model_.DestroyInput();
    return ACLLITE_OK;
}

AclLiteError ClassifyInferenceThread::MsgSend(shared_ptr<DetectDataMsg> detectDataMsg)
{
    while (1) {
        AclLiteError ret = SendMessage(detectDataMsg->classifyPostThreadId, MSG_POSTPROC_CLAASSIFYDATA, detectDataMsg);
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

AclLiteError ClassifyInferenceThread::Process(int msgId, shared_ptr<void> data)
{
    switch (msgId) {
        case MSG_DO_CLASSIFY_INFER:
            ModelExecute(static_pointer_cast<DetectDataMsg>(data));
            MsgSend(static_pointer_cast<DetectDataMsg>(data));
            break;
        default:
            ACLLITE_LOG_INFO("Inference thread ignore msg %d", msgId);
            break;
    }

    return ACLLITE_OK;
}
