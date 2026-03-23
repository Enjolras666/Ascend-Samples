atc --model=yolov5l.onnx --framework=5 --output=yolov5l --input_shape="images:1,3,640,640"  --soc_version=Ascend310P3  --insert_op_conf=aipp.cfg
