
#include<math.h>
#include "preprocess.h"
#include "../utils/caffe2_net.h"
#include "../utils/utils.h"
#include "../seeta/aligner.h"
#include "newface.h"


static caffe2net::Net face_predictor;
static caffe2net::Net modify_predictor;


static CPreProcess pre;
static std::shared_ptr<seeta::Aligner> small_aligner_;
static std::shared_ptr<seeta::Aligner> big_aligner_;

static int rect_limit = 64;
static float seeta_ext_x = 0.05;
static float seeta_ext_y = 0.05;
static int start_chn = 1;
static int start_width = 0;
static int start_height = 0;
static int ext_width = 0;
static int ext_height = 0;

static const float mean[3] = { 127.5,127.5,127.5 };
static const float norms[3] = { 1.0f/127.5,1.0f/127.5, 1.0f/127.5 };

static const float default_mean[3] = { 0,0,0 };
static const float default_norms[3] = { 1.0f / 255,1.0f / 255, 1.0f / 255 };



void set_distance_threshold(int width)
{
	rect_limit = width;
}

int init_face(const char* init_net, const char*  predict_net)
{
	if (!face_predictor.initNet(init_net, predict_net)) {
		std::cout << "init net error" << std::endl;
		return 0;
	}
	std::vector<int64_t> dim;
	face_predictor.initInput("input", &dim);

	start_chn = dim[1];
	start_height = dim[2];
	start_width = dim[3];

	pre.set_img_size(start_width,start_height);
	ext_width = start_width + 2*round(seeta_ext_x * start_width);
	ext_height = start_height + 2*round(seeta_ext_y * start_height);
	big_aligner_.reset(new seeta::Aligner(ext_height, ext_width,"linear"));
	small_aligner_.reset(new seeta::Aligner(start_height, start_width , "linear"));
	//big_aligner_.reset(new seeta::Aligner(ext_height, ext_width));
	//small_aligner_.reset(new seeta::Aligner(start_height, start_width));

	return 1;
}

int init_modify_face(const char* init_net, const char*  predict_net)
{
	if (!modify_predictor.initNet(init_net, predict_net)) {
		std::cout << "init net error" << std::endl;
		return 0;
	}
	std::vector<int64_t> dim;
	modify_predictor.initInput("input", &dim);
	modify_predictor.initInput("up", &dim);
	return 1;
}

void get_modify_face(cv::Mat src,cv::Mat &dst)
{
	caffe2net::Mat cm;
	caffe2net::Utils util;

	unsigned char *data = new unsigned char[src.channels()*src.cols*src.rows];
	util.Mat2array(src, data);
	cm.setMat(data, src.channels(), src.cols, src.rows);
	delete[] data;
	cm.normalize(default_mean, default_norms);
	modify_predictor.setInput("input", cm);
	modify_predictor.setInput("up", cm);

	if (!modify_predictor.run()) {
		std::cout << "predict error" << std::endl;
		return;
	}

	std::vector<float> vec;
	modify_predictor.setOutput("output", vec);
	util.array2Mat(vec, src.channels(), src.cols, src.rows, dst);
}

static cv::Mat  crop(cv::Mat img,
	             cv::Rect rect,
	             std::vector<cv::Point2f> five,
	             int crop_mode)
{
	cv::Mat src;
	if (crop_mode == NO_CROP)
	{
		img.copyTo(src);
		pre.crop_img(src);
		return src;
	}
	else if (crop_mode == RECT_CROP)
	{
		src = pre.get_face_with_center(rect, img, five);
		pre.crop_img(src);
		return src;
	}
	else if (crop_mode == SEETA_CROP)
	{
		float point_data[10];
		for (int i = 0; i < 5; ++i)
		{
			point_data[i * 2] = five[i].x;
			point_data[i * 2 + 1] = five[i].y;
		}
		seeta::ImageData src_img_data(img.cols, img.rows, img.channels());
		src_img_data.data = img.data;
		if (rect.width >= rect_limit)
		{
			cv::Mat dst_img(ext_height, ext_width, CV_8UC(img.channels()));
			seeta::ImageData dst_img_data(ext_width, ext_height, img.channels());
			dst_img_data.data = dst_img.data;
			big_aligner_->Alignment(src_img_data, point_data, dst_img_data);
			cv::Rect real;
			real.x = (ext_width - start_width) / 2;
			real.y = (ext_height - start_height ) / 2;
			real.width = start_width;
			real.height = start_height;
			dst_img = dst_img(real);
			return dst_img;
		}
		else
		{
			cv::Mat dst_img(start_height, start_width, CV_8UC(img.channels()));
			seeta::ImageData dst_img_data(start_width, start_height, img.channels());
			dst_img_data.data = dst_img.data;
			small_aligner_->Alignment(src_img_data, point_data, dst_img_data);
			return dst_img;
		}
	}
	else if (crop_mode == FACE_CROP)
	{
		src = pre.get_face(rect, img, five);
		pre.crop_img(src);
		return src;
	}
	else if (crop_mode == CCL_CROP)
	{
		src = pre.get_face_with_ccl(rect, img, five);
		pre.crop_img(src);
		return src;
	}
	else;
	return src;
}

//3Í¨µÀ
int  get_vec(cv::Mat img,
	cv::Rect rect,
	std::vector<cv::Point2f> five,
	std::vector<float> &vec,
	int img_type, //= FACE_IMG_LMCP
	int crop_mode, //= SEETA_CROP
	int norm_mode,
	cv::Mat *dst ) //= NULL
{
	
	cv::Mat src;

	if (img_type == FACE_IMG_GRAY)
	{
		/*if (start_chn != 1)
		{
			printf("ERROR:net input need channel 3\n");
			return 0;
		}*/
		if (img.channels() != 1)	cv::cvtColor(img, img, CV_BGR2GRAY);		
		src = crop(img, rect, five, crop_mode);
		if (start_chn != 1)
		{
			cv::Mat rgb_channel[3];
			rgb_channel[0] = src;
			rgb_channel[1] = src;
			rgb_channel[2] = src;
			cv::merge(rgb_channel, 3, src);
		}
	}
	else if (img_type == FACE_IMG_LMCP)
	{
		/*if (start_chn != 1)
		{
			printf("ERROR:net input need channel 3\n");
			return 0;
		}*/
		//if (start_chn == 1 && img.channels() != 1)	cvtColor(img, img, CV_BGR2GRAY);
		if (img.channels() != 1)	cv::cvtColor(img, img, CV_BGR2GRAY);
		src = crop(img, rect, five, crop_mode);
		src = pre.preprocess(src);
		if (start_chn != 1)
		{
			cv::Mat rgb_channel[3];
			rgb_channel[0] = src;
			rgb_channel[1] = src;
			rgb_channel[2] = src;
			cv::merge(rgb_channel, 3, src);
		}
	}
	else 
	{
		if(start_chn == 1 && img.channels() != 1) cv::cvtColor(img, img, CV_BGR2GRAY);
		src = crop(img, rect, five, crop_mode);
	}


	if(dst != NULL ) *dst= src;
	caffe2net::Mat cm;
	caffe2net::Utils util;

	unsigned char *data = new unsigned char[src.channels()*src.cols*src.rows];
	util.Mat2array(src, data);
	cm.setMat(data, src.channels(),src.cols, src.rows);
	if(norm_mode == NORM_NEGA)	cm.normalize(mean, norms);
	else cm.normalize(default_mean, default_norms);
	delete[] data;
	face_predictor.setInput("input", cm);
	if (!face_predictor.run()) {
		std::cout << "predict error" << std::endl;
		return 0;
	}
	face_predictor.setOutput("output", vec);
	return 1;
}

void set_face_ext(float top_x, float top_y)
{
	seeta_ext_x = top_x;
	seeta_ext_y = top_y;
	//pre.set_face_ext(top_x, top_y);
}

float  cal_cos(std::vector<float> a, std::vector<float> b)
{
	float a_dis = 0;
	float b_dis = 0;
	float sum = 0;
	for (int i = 0; i < a.size(); i++)
	{
		sum += a[i] * b[i];
		a_dis += a[i] * a[i];
		b_dis += b[i] * b[i];
	}
	return sum / (sqrt(a_dis) * sqrt(b_dis));
}

