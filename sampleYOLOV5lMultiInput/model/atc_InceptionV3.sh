wget https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/003_Atc_Models/AE/ATC%20Model/InceptionV3/inceptionv3.onnx
wget https://obs-9be7.obs.cn-east-2.myhuaweicloud.com/003_Atc_Models/AE/ATC%20Model/InceptionV3/aipp_inceptionv3_pth.config
atc --model=./inceptionv3.onnx --framework=5 --output=InceptionV3 --soc_version=Ascend310P3 --insert_op_conf=./aipp_inceptionv3_pth.config --input_shape="actual_input_1:1,3,300,300" --input_format=NCHW
