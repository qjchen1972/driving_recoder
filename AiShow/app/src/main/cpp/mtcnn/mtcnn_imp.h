//
// Created by 陈黔江 on 2018/11/13.
//

#ifndef MTCNN_MTCNN_IMP_H
#define MTCNN_MTCNN_IMP_H

#include <string>
#include <vector>



#ifdef __cplusplus
extern "C" {
#endif

namespace mtcnn_imp
{
    enum imageType
    {
        eBGR888,    /**< The image is stored using a 24-bit BGR format (8-8-8). */
        eRGB888,    /**< The image is stored using a 24-bit RGB format (8-8-8). */
        eRGBA
    };

    struct SMtcnnFace
    {
        float score;
        int boundingBox[4];    // x1, y1, x2, y2
        int landmark[10];    // x1, x2, x3, x4, x5, y1, y2, y3, y4, y5
    };

    void init(const std::string path);
    void set_MinFace(int size);
    void set_NumThreads(int threads);
    void detect(const unsigned char* src, int height, int width,imageType type,std::vector<SMtcnnFace>& finalBbox);
    void detectMaxFace(const unsigned char* src, int height, int width,imageType type,std::vector<SMtcnnFace>& finalBbox);

}
#ifdef __cplusplus
}
#endif /* end of __cplusplus */


#endif //MTCNN_MTCNN_IMP_H
