#define USE_OPENCV

#include "../utils/caffe2_net.h"
#include "../utils/utils.h"
#include "detect.h"
#include "ssd.h"

static caffe2net::Net predictor;
static std::vector<float>  prior_data;
static std::vector<int64_t> inputDim;

//320
//static float mean[3] = { 0,0,0 };
//static float norms[3] = { 1.0f,1.0f, 1.0f };

//256
static float mean[3] = { 127.5,127.5,127.5 };
static float norms[3] = { 1.0f / 127.5,1.0f / 127.5, 1.0f / 127.5 };

static int img_chn = 3;
static int img_width = 256;
static int img_height = 256;
static const float threshold = 0.5;


void  setNormalize(const float* mean_, const float* norm_) {
	memcpy(mean, mean_, sizeof(float) * 3);
	memcpy(norms, norm_, sizeof(float) * 3);
}

void  getInputDim(std::vector<int64_t> &dim) {
	dim = inputDim;
}

std::string getClass(int index) {
	Detect dect;
	return dect.getClass(index);
}

int  initSsd(const char* init_net, const char*  predict_net) {
	if (!predictor.initNet(init_net, predict_net)) {
		std::cout << "init net error" << std::endl;
		return 0;
	}
	predictor.initInput("input", &inputDim);
	img_chn = inputDim[1];
	img_width = inputDim[2];
	img_height = inputDim[3];

	PriorBox box;
	box.createPriorBox(prior_data);
	return 1;
}

int  getSsdVec(cv::Mat img, std::vector<float> &vec, cv::Mat *dst) {

	caffe2net::Mat cm;
	caffe2net::Utils util;

	cv::Mat src;
	cv::resize(img, src,cv::Size(img_width, img_height));
	if (img_chn == 1 && src.channels() != 1) cv::cvtColor(src, src, CV_BGR2GRAY);
	
	unsigned char *data = new unsigned char[src.channels()*src.cols*src.rows];
	util.Mat2array(src, data);
	cm.setMat(data, src.channels(), src.cols, src.rows);
	delete[] data;
	cm.normalize(mean, norms);
	predictor.setInput("input", cm);
	if (!predictor.run()) {
		std::cout << "predict error" << std::endl;
		return 0;
	}

	std::vector<float>  loc;
	std::vector<float>  conf;

	predictor.setOutput("loc_output", loc);
	predictor.setOutput("conf_output", conf);

	Detect dect;
	dect.getAnswer(loc.data(),conf.data(), prior_data, vec);
	if (dst == NULL) return 1;	
	cv::resize(img, *dst, cv::Size(dst->cols, dst->rows));

	int len = vec.size() / 6;
	for (int i = 0; i < len; i++) {
		if (vec[i * 6 + 5] < threshold) continue;

		cv::Point point1(vec[i * 6 + 0] * dst->cols, vec[i * 6 + 1] * dst->rows);
		cv::Point point2(vec[i * 6 + 2] * dst->cols, vec[i * 6 + 3] * dst->rows);
		cv::rectangle(*dst, cv::Rect(point1, point2), cv::Scalar(0, 225, 255), 1);
		char ch[100];
		sprintf(ch, "%s %.2f", dect.getClass(int(vec[i * 6 + 4])).c_str(), vec[i * 6 + 5]);
		std::string temp(ch);
		cv::putText(*dst, temp, point1, CV_FONT_HERSHEY_COMPLEX, 0.4, cv::Scalar(255, 255, 255));
		break;
	}
	return 1;
}