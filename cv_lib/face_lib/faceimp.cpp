#include "face/newface.h"
//#include "mtcnn/mtcnn.h"
#include "ssd/ssd.h"
#include"faceimp.h"


int    face_init(const char* init_net, const char*  predict_net) {
	return init_face(init_net, predict_net);
}

int    face_get_vec(cv::Mat img, cv::Rect rect, std::vector<cv::Point2f> five, std::vector<float> &vec, cv::Mat *dst) {
	return get_vec(img,rect,five,vec, FACE_IMG_DEFAULT, SEETA_CROP, NORM_NEGA,dst);
}

float  face_cal_cos(std::vector<float> a, std::vector<float> b) {
	return cal_cos(a, b);
}

int    modify_init(const char* init_net, const char*  predict_net) {
	return init_modify_face(init_net, predict_net);
}

void   modify_face(cv::Mat src, cv::Mat &dst) {
	 get_modify_face(src, dst);	
}


/*int   mtcnn_init(const char* pnet, const char*  rnet, const char* onet) {
	set_Mtcnn_Net(pnet, rnet, onet);
}

int   mtcnn_dextect(cv::Mat img, cv::Rect &rect, std::vector<cv::Point2f> &five) {
	mtcnn *find;

	find = new mtcnn(img.rows, img.cols);
	int ret = find->findFace(img, rect, five);
	delete find;
	return ret;
}*/


int  ssd_init(const char* init_net, const char*  predict_net) {
	return initSsd(init_net, predict_net);
}

int  ssd_get_vec(cv::Mat img, std::vector<float> &vec, cv::Mat *dst) {
	return getSsdVec(img, vec, dst);
}

std::string ssd_get_class(int index) {
	return getClass(index);
}
