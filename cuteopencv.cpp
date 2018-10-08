#include "cuteopencv.h"

#include <QDebug>

CuteOpenCv::CuteOpenCv()
{

}

cv::Mat CuteOpenCv::CuteImageToOpenCVMat(QImage &frame)
{
    qDebug() << frame.format();
    switch(frame.format()) {
    case QImage::Format_Invalid:
        return cv::Mat();
    case QImage::Format_RGB32:
        return cv::Mat(frame.height(), frame.width(), CV_8UC4,(void *)frame.constBits(), frame.bytesPerLine());
    case QImage::Format_RGB888:
    {
        cv::Mat view(frame.height(), frame.width(), CV_8UC3,(void *)frame.constBits(), frame.bytesPerLine());
        cvtColor(view, view, cv::COLOR_RGB2BGR);
        return view;
    }
    default:
    {
        QImage conv = frame.convertToFormat(QImage::Format_ARGB32);
        cv::Mat view(conv.height(),conv.width(),CV_8UC4,(void *)conv.constBits(),conv.bytesPerLine());
        return view;
    }
    }
}
