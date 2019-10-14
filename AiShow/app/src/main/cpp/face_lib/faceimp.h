#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

int    face_init(const char* init_net, const char*  predict_net);
int    face_get_vec(cv::Mat img, cv::Rect rect, std::vector<cv::Point2f> five, std::vector<float> &vec, cv::Mat *dst = NULL);
float  face_cal_cos(std::vector<float> a, std::vector<float> b);

int    modify_init(const char* init_net, const char*  predict_net);
void   modify_face(cv::Mat src, cv::Mat &dst);


//int   mtcnn_init(const char* pnet, const char*  rnet, const char* onet);
//int   mtcnn_dextect(cv::Mat img, cv::Rect rect, std::vector<cv::Point2f> five);


int  ssd_init(const char* init_net, const char*  predict_net);
int  ssd_get_vec(cv::Mat img, std::vector<float> &vec, cv::Mat *dst = NULL);
std::string ssd_get_class(int index);



