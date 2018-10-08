#ifndef CUTEOPENCV_H
#define CUTEOPENCV_H

#include <QImage>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

class CuteOpenCv
{
public:
    CuteOpenCv();
    static cv::Mat CuteImageToOpenCVMat(QImage &frame);
};

#endif // CUTEOPENCV_H
