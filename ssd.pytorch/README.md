# I modified the SSD (300) to 256 * 256 and 320 * 320 to use onnx to convert to caffe2 model. Some notes and thanks:

* [Single Shot MultiBox Detector](http://arxiv.org/abs/1512.02325) from the 2016 paper by Wei Liu, Dragomir Anguelov, Dumitru Erhan, Christian Szegedy, Scott Reed, Cheng-Yang, and Alexander C. Berg.  The official and original Caffe code can be found [here](https://github.com/weiliu89/caffe/tree/ssd).
* pytorch code refer to https://github.com/amdegroot/ssd.pytorch


# Create onnx model
 python eval.py
