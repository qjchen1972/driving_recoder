//
// Created by 陈黔江 on 2018/11/13.
//

#include <type_traits>
#include <memory>

#include "mtcnn.h"
#include "mtcnn_imp.h"


static std::shared_ptr<MTCNN> mtcnn;


static void ConvertToSMtcnnFace(const std::vector<Bbox>& src, std::vector<mtcnn_imp::SMtcnnFace>& dst)
{
    mtcnn_imp::SMtcnnFace tmpFace;
    for (auto it = src.begin(); it != src.end(); it++) {

        tmpFace.score = it->score;
        tmpFace.boundingBox[0] = it->x1;
        tmpFace.boundingBox[1] = it->y1;
        tmpFace.boundingBox[2] = it->x2;
        tmpFace.boundingBox[3] = it->y2;

        for (int i = 0; i < 10; ++i)
            tmpFace.landmark[i] = (int) (it->ppoint[i]);

        dst.push_back(tmpFace);
    }
}

static int GetNcnnImageConvertType(mtcnn_imp::imageType type)
{
    switch (type)
    {
        case mtcnn_imp::eRGB888:
            return ncnn::Mat::PIXEL_RGB;
        case mtcnn_imp::eBGR888:
            return ncnn::Mat::PIXEL_BGR2RGB;
        case mtcnn_imp::eRGBA:
            return ncnn::Mat::PIXEL_RGBA2RGB;
        default:
            return ncnn::Mat::PIXEL_RGBA2RGB;
    }
}


void mtcnn_imp::init(const std::string path) {
	mtcnn.reset(new MTCNN(path));
}
    	

void mtcnn_imp::set_MinFace(int size) {
	mtcnn->SetMinFace(size);
}
    
void mtcnn_imp::set_NumThreads(int threads) {
	mtcnn->SetNumThreads(threads);
}
    

void mtcnn_imp::detect(const unsigned char* src, int height, int width, mtcnn_imp::imageType type, std::vector<mtcnn_imp::SMtcnnFace>& finalBbox) {
	ncnn::Mat ncnnImg = ncnn::Mat::from_pixels(src, GetNcnnImageConvertType(type), width, height);
	std::vector<Bbox> box;
	mtcnn->detect(ncnnImg, box);
	ConvertToSMtcnnFace(box, finalBbox);
}
    
void mtcnn_imp::detectMaxFace(const unsigned char* src, int height, int width, mtcnn_imp::imageType type, std::vector<mtcnn_imp::SMtcnnFace>& finalBbox) {
	ncnn::Mat ncnnImg = ncnn::Mat::from_pixels(src, GetNcnnImageConvertType(type), width, height);
	std::vector<Bbox> box;
	mtcnn->detectMaxFace(ncnnImg, box);
	ConvertToSMtcnnFace(box, finalBbox);
}
	
