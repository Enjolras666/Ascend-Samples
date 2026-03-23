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

* File sample_process.h
* Description: handle acl resource
*/
#ifndef PARAMS_H
#define PARAMS_H
#pragma once

#include <iostream>
#include <mutex>
#include <memory>
#include <vector>
#include <unistd.h>
#include <sys/timeb.h>
#include "AclLiteType.h"
#include "AclLiteModel.h"
#include "AclLiteImageProc.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/highgui/highgui.hpp"
#include "X11/Xlib.h"

namespace {
const int MSG_APP_START = 1;
const int MSG_READ_FRAME = 2;
const int MSG_PREPROC_DETECTDATA = 3;
const int MSG_DO_DETECT_INFER = 4;
const int MSG_POSTPROC_DETECTDATA = 5;  //推理发给后处理

const int MSG_PREPROC_CLAASSIFYDATA = 6;  //后处理发给预处理
const int MSG_DO_CLASSIFY_INFER = 7;  //预处理发给推理
const int MSG_POSTPROC_CLAASSIFYDATA = 8;  //推理发给后处理

const int MSG_OUTPUT_FRAME = 9;
const int MSG_ENCODE_FINISH = 10;
const int MSG_RTSP_DISPLAY = 11;
const int MSG_APP_EXIT = 12;

const std::string kDataInputName = "dataInput";
const std::string kPreName = "pre";
const std::string kInferName = "infer";
const std::string kPostName = "post";
const std::string kClassifyPreName = "Classifypre";
const std::string kClassifyInferName = "Classifyinfer";
const std::string kClassifyPostName = "Classifypost";
const std::string kDataOutputName = "dataOutput";
const std::string kRtspDisplayName = "rtspDisplay";
}

struct ClassifyDataMsg {
    ImageData cropedImgs;           // cropped car image from original image
    ImageData resizedImgs;          // resized image for classify inference
    cv::Point lt;                   // left top
    cv::Point rb;                   // right bottom
    std::string detect_result = "";      // yolo detect output
    std::string classify_result = "";    // inceptionv3 classify output
};

struct BatchData {
    ImageData decodedImg;
    cv::Mat frame;
    std::string textPrint = "";
    std::vector<ClassifyDataMsg> classifyDataMsg;
    bool hasDetected;
};

struct DetectDataMsg {
    int detectPreThreadId;
    int detectInferThreadId;
    int detectPostThreadId;

    int classifyPreThreadId;
    int classifyInferThreadId;
    int classifyPostThreadId;

    int dataOutputThreadId;
    int rtspDisplayThreadId;
    int postId;
    uint32_t deviceId;
    uint32_t channelId;  // record msg belongs to which rtsp/video channel
    bool isLastFrame;  // whether the last frame of rtsp/video of this channel has been decoded
    int msgNum;  // record frameID in rtsp/video of this channel
    ImageData modelInputImg;  // image after detect preprocess
    ImageData classifyModelInputImg;
    std::vector<InferenceOutput> inferenceOutput;  // yolo detect output
    std::vector<InferenceOutput> classifyInferenceOutput;  // yolo detect output
    // ImageData decodedImg;
    // cv::Mat frame;
    // std::string textPrint = "";
    // std::vector<ClassifyDataMsg> classifyDataMsg;
    // bool hasDetected;
    std::vector<BatchData> batchData;
};

#endif