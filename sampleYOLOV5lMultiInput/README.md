## 目录

  - [样例介绍](#样例介绍)
  - [样例流程图](#样例流程图)
  - [目录结构](#目录结构)
  - [获取源码包](#获取源码包) 
  - [第三方依赖安装](#第三方依赖安装)
  - [样例运行](#样例运行)
  - [其他资源](#其他资源)
  - [更新说明](#更新说明)
  - [已知issue](#已知issue)
    
## 样例介绍
功能：使用yolov5l模型对输入数据进行预测推理，推理检测出图片/视频中所有可检测物体，并将推理结果打印到输出上，是一个是基于多路、多线程方案实现的高性能案例，通过多卡并行处理多路数的数据并输出，支持多种输入输出。    
样例输入：视频mp4文件/视频h26X文件/rtsp视频流。   
样例输出：打屏显示。 

## 样例流程图
通用目标检测与识别一站式方案是一个是基于多路、多线程方案实现的高性能案例，通过多卡并行处理多路数的数据并输出。
其中整体流程图如下所示：
![流程图](https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/003_Atc_Models/AE/ATC%20Model/sampleYolov7MultiInput/%E6%B5%81%E7%A8%8B%E5%9B%BE.png)

- 管理线程：将线程和队列打包在一起，并完成进程创建、消息队列创建、消息发送和消息接收守护。
- 数据输入线程：对输入图片或视频进行解码。
- 数据预处理线程：对数据输入线程传过来的YUV图片进行处理（resize等操作）。
- 推理线程：使用YOLOV5l模型进行推理。
- 数据后处理线程：分析推理结果，输出框点及标签信息。
- 数据输出线程：将框点及标签信息标识到输出数据上。


## 目录结构

```
├── model                      //模型文件夹，存放样例运行需要的模型文件
│   └── xxxx.onnx                 
├── data                       //数据文件夹
│   └── xxxx                   //测试数据,输入视频 
├── pic                        //数据文件夹
│   └── xxxx                   //测试数据,输入图片
├── inc                        //头文件文件夹
│   ├── Params.h               //声明样例使用的数据结构的头文件 
│   └── label.h                //声明样例模型使用的类别标签的头文件 
├── out                        //编译输出文件夹，存放编译生成的可执行文件
│   ├── xxxx                   //可执行文件 
│   └── xxxx                   //样例运行的输出结果图片/视频
├── scripts                    //配置文件+脚本文件夹
│   ├── test.json              //样例运行使用的参数配置文件 
│   ├── sample_build.sh        //快速编译脚本
│   ├── sample_run.sh          //快速运行脚本
│   ├── writingMethodForEachInputOutput.json        //不同输入输出数据类型参数配置参考文件
│   └── multiInputWithMultiModelPerDevice.json      //多device多推理线程多输入数据配置参考文件
├── src 
│   ├── acl.json               //系统初始化的配置文件 
│   ├── CMakeLists.txt         //Cmake编译文件
│   ├── dataInput              //数据输入解码处理线程文件夹，存放该业务线程的头文件及源码
│   ├── dataOutput             //数据输出处理线程文件夹，存放该业务线程的头文件及源码
│   ├── detectInference        //检测模型推理线程文件夹，存放该业务线程的头文件及源码
│   ├── detectPreprocess       //检测模型预处理线程文件夹，存放该业务线程的头文件及源码
│   ├── detectPostprocess      //检测模型后处理线程文件夹，存放该业务线程的头文件及源码
│   ├── pushrtsp               //rtsp展示线程文件夹，存放该业务线程的头文件及源码
│   └── main.cpp               //主函数，yolo检测功能的实现文件  
└── CMakeLists.txt             //编译脚本入口，调用src目录下的CMakeLists文件
```

## 获取源码包

命令行方式下载

```    
# 开发环境，非root用户命令行中执行以下命令下载源码仓。    
cd ${HOME}     
git clone https://github.com/Enjolras666/Ascend-Samples.git 
```
## 设置环境变量

```
# 全部操作在宿主机执行
export DDK_PATH=/usr/local/Ascend/ascend-toolkit/latest
export NPU_HOST_LIB=$DDK_PATH/runtime/lib64/stub
export THIRDPART_PATH=${DDK_PATH}/thirdpart
export LD_LIBRARY_PATH=${THIRDPART_PATH}/lib:$LD_LIBRARY_PATH

mkdir -p ${THIRDPART_PATH}
```

## 第三方依赖安装

- git、gcc、cmake

  执行以下命令安装

  ```
  vim /etc/yum.conf
  (最后一行添加sslverify=False，保存退出)
  yum install -y git
  yum install -y gcc-g++ cmake
  git config --global http.sslVerify False
  cd ${HOME}
  ```

- x264

    执行以下命令安装x264
   ```
   git clone https://code.videolan.org/videolan/x264.git
   cd x264
   ./configure --enable-shared --disable-asm
   make
   make install
   ls /usr/local/lib/libx264.so.165
   cp /usr/local/lib/libx264.so.165 /lib
   ```
   
- ffmpeg

  执行以下命令安装ffmpeg
   ```
  cd $HOME
  wget http://www.ffmpeg.org/releases/ffmpeg-4.1.3.tar.gz --no-check-certificate
  tar -zxvf ffmpeg-4.1.3.tar.gz
  cd ffmpeg-4.1.3
  ./configure --enable-shared --enable-pic --enable-static --disable-x86asm --enable-libx264 --enable-gpl --prefix=${THIRDPART_PATH}
  make -j8
  make install
   ```
  
   </details> 
  
- opencv

  命令行yum安装opencv:

  ```
  yum install -y opencv
  ln -s /usr/include/opencv4/opencv2 /usr/include/opencv2
  ```

- X11

  命令行yum安装X11:

  ```
  yum install -y libX11-devel
  ```
  
- jsoncpp

  命令行yum安装jsoncpp：

   ```
  yum install -y jsoncpp-devel
   ```

## 模型准备

```
cd sampleYOLOV5lMultiInput/model/
```

- InceptionV3

```
# onnx
wget https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/003_Atc_Models/AE/ATC%20Model/InceptionV3/inceptionv3.onnx

# aipp
wget https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/003_Atc_Models/AE/ATC%20Model/InceptionV3/aipp_inceptionv3_pth.config

# atc转换
atc --model=./inceptionv3.onnx --framework=5 --output=InceptionV3_bs8 --soc_version=Ascend310P3 --insert_op_conf=./aipp_inceptionv3_pth.config --input_shape="actual_input_1:8,3,300,300" --input_format=NCHW --precision_mode=allow_fp32_to_fp16 --enable_small_channel=1 --op_select_implmode=high_performance
```

- Yolov5l

```
# yolo代码
git clone https://github.com/ultralytics/yolov5.git
cd yolov5
pip3 install -r requirements.txt
pip3 install onnx==1.14.1 protobuf==3.20.3

# 下载权重
wget https://github.com/ultralytics/yolov5/releases/download/v7.0/yolov5l.pt --no-check-certificate

# 导出onnx文件
python3 export.py --weights yolov5l.pt --include onnx --opset 12

# aipp
vim aipp.cfg
​```
aipp_op {
    aipp_mode: static
    input_format: YUV420SP_U8
    csc_switch: true
    # 如果输入的是YVU420SP_U8（NV21）图像，则需要将rbuv_swap_switch参数设置为true
    rbuv_swap_switch: false
    related_input_rank: 0
    src_image_size_w: 640
    src_image_size_h: 640
    crop: false
    matrix_r0c0: 256
    matrix_r0c1: 0
    matrix_r0c2: 359
    matrix_r1c0: 256
    matrix_r1c1: -88
    matrix_r1c2: -183
    matrix_r2c0: 256
    matrix_r2c1: 454
    matrix_r2c2: 0
    input_bias_0: 0
    input_bias_1: 128
    input_bias_2: 128
    # 归一化系数需要根据用户模型实际需求配置，如下所列常见值仅作为示例
    # 归一化系数应用于色域转换和通道交换之后的通道
    mean_chn_0: 0
    mean_chn_1: 0
    mean_chn_2: 0
    min_chn_0: 0.0
    min_chn_1: 0.0
    min_chn_2: 0.0
    var_reci_chn_0: 0.003921568859368563
    var_reci_chn_1: 0.003921568859368563
    var_reci_chn_2: 0.003921568859368563
    var_reci_chn_3: 1
}
​```

# op
vim fusion.cfg
​```
TbeConvDequantSigmoidMulAddFusionPass:on
​```

# atc转换
atc --model=yolov5l.onnx --framework=5 --output=yolov5l --input_shape="images:1,3,640,640"  --soc_version=Ascend310P3  --insert_op_conf=aipp.cfg --fusion_switch_file=fusion.cfg --enable_small_channel=1 --optypelist_for_implmode="Sigmoid" --op_select_implmode=high_performance
```

## 程序编译

```
cd ../scripts
# 编译
bash sample_build.sh

# 测试,正常退出即程序可用。
bash sample_run.sh
```

## Live555推流

- 下载

```
wget https://download.live555.com/live555-latest.tar.gz --no-check-certificate

tar -zxvf live555-latest.tar.gz

cd live/liveMedia
```

- 改视频循环

```
chmod +w ByteStreamFileSource.cpp

vim ByteStreamFileSource.cpp +95

# 改ByteStreamFileSource::doGetNextFrame()函数
​```
fseek(fFid, 0, SEEK_SET);
​```

cd ..
```

![image-20260323150513305](data\readme\image-20260323150513305.png)

- 编译

```
./genMakefiles linux
make
# 启动服务
cd mediaServer/
./live555MediaServer
```

- 设置264文件

另起个终端

```
cd ~
```

安装ffmpeg-plugin

```
wget https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n4.4.4.zip --no-check-certificate -O FFmpeg-n4.4.4.zip
unzip FFmpeg-n4.4.4.zip
cd FFmpeg-n4.4.4

# patch
wget https://raw.gitcode.com/Ascend/mindsdk-referenceapps/blobs/a4a6809c55f12cf650113d2ab47ade31592bd707/ascend_ffmpeg.patch
yum install -y patch
patch -p1 -f < ascend_ffmpeg.patch

# 重新编译
export ASCEND_HOME=/usr/local/Ascend
. /usr/local/Ascend/ascend-toolkit/set_env.sh

./configure \
    --prefix=./ascend \
    --enable-shared \
    --extra-cflags="-I${ASCEND_HOME}/ascend-toolkit/latest/acllib/include" \
    --extra-ldflags="-L${ASCEND_HOME}/ascend-toolkit/latest/acllib/lib64" \
    --extra-libs="-lacl_dvpp_mpi -lascendcl" \
    --enable-ascend \
    && make -j && make install

# 添加FFmpeg环境变量
示例: /root为FFmpeg-n4.4.4所在目录
export FFMPEG_LIB_PATH=/root/FFmpeg-n4.4.4/ascend/lib
export LD_LIBRARY_PATH=${FFMPEG_LIB_PATH}:$LD_LIBRARY_PATH
```

mp4转264

```
wget https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/003_Atc_Models/yolov5s/test.mp4 --no-check-certificate

./ffmpeg -hwaccel ascend -c:v h264_ascend -i test.mp4 -s:v 1920x1080 -rc_mode 1 -r 25 -g 250 -b:v 8m -c:v h264_ascend test.264

将转换好的test.264，放到推理的live555服务路径文件夹下，即可完成推流。
mv ./test.264 /home/kz/live/mediaServer/
```

复制一个转换好的test.264，给其他device使用

```
cd /home/kz/live/mediaServer/
cp test.264 test1.264
```

在另一台服务器再起一个live555服务

按照上面复制test.264把test2.264，test3.264设置推流给device2，device3使用。

（测试发现live555无法同时承载240+路数，代码打开rtsp会报错，所以配置两个）

第一个

![image-20260323151236261](data\readme\image-20260323151236261.png)

第二个

![image-20260323151257056](data\readme\image-20260323151257056.png)

## 配置修改

另起一个终端

```
cd sampleYOLOV5lMultiInput/scripts

vim test_device0.json
```

批量修改地址可使用

```
sed -i "s@rtsp://192.168.1.214:8554/h264ESVideoTest@rtsp://141.61.21.215:554/test.264@g" test_device0.json

sed -i "s@rtsp://192.168.1.214:8554/h264ESVideoTest@rtsp://141.61.21.215:554/test1.264@g" test_device1.json

sed -i "s@rtsp://192.168.1.214:8554/h264ESVideoTest@rtsp://141.61.21.213:554/test2.264@g" test_device2.json

sed -i "s@rtsp://192.168.1.214:8554/h264ESVideoTest@rtsp://141.61.21.213:554/test3.264@g" test_device3.json
```

按照图示修改对应配置

![image-20260323151805703](data\readme\image-20260323151805703.png)

修改启动脚本

```
cp sample_run.sh sample_run1.sh
cp sample_run.sh sample_run2.sh
cp sample_run.sh sample_run3.sh

vim sample_run.sh
```

![image-20260323151915512](data\readme\image-20260323151915512.png)

sample_run1.sh、sample_run2.sh、sample_run3.sh修改为对应的test_devicex.json

## 测试启动

再启动3个终端，进入启动脚本路径

```
cd sampleYOLOV5lMultiInput/scripts
```

依次运行

```
bash sample_run.sh
```

等到出现一连串的帧获取时

![image-20260323152252924](data\readme\image-20260323152252924.png)

下一个及其他终端再同理启动程序1,2,3

```
bash sample_run1.sh 
bash sample_run2.sh 
bash sample_run3.sh 
```

## 测试结果

两张300I duo卡（4个device），各打开80 channel的视频流，总共320路视频流。

查看日志及截图如下：

```
vim sampleYOLOV5lMultiInput/out/device0.log
```

![image-20260323152526307](data\readme\image-20260323152526307.png)

Aicore利用率

![image-20260323152557593](data\readme\image-20260323152557593.png)

Cpu使用率

![image-20260323152643739](data\readme\image-20260323152643739.png)

显存使用

![image-20260323152731235](data\readme\image-20260323152731235.png)



## 更新说明

| 时间 | 更新事项 |
|----|------|
| 2026/03/23 | 新增sampleYOLOV5lMultiInput/README.md |


## 已知issue

1.   正常使用时，plog也会打印ERROR的日志信息（Free host memory failed，get index by name failed, cannot find tensor name[ascend_mbatch_shape_data]），会影响报错定位，暂无解决方法；