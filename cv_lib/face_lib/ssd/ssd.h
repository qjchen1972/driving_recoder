#pragma once

#include <opencv2/opencv.hpp>
#include <vector>


int  initSsd(const char* init_net, const char*  predict_net);
int  getSsdVec(cv::Mat img, std::vector<float> &vec, cv::Mat *dst = NULL);
void  setNormalize(const float* mean_, const float* norm_);
void  getInputDim(std::vector<int64_t> &dim);
std::string getClass(int index);






