#ifndef OVVIDEOFILTERRUNNABLE_H
#define OVVIDEOFILTERRUNNABLE_H

#include <QObject>
#include <QVideoFilterRunnable>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "ocvobjectcolordetector.h"

class OvVideoFilter;

class OvVideoFilterRunnable : public QVideoFilterRunnable
{
public:
    OvVideoFilterRunnable(OvVideoFilter *parent=nullptr);

    // QVideoFilterRunnable interface
public:
    QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags);
private:
    OvVideoFilter *m_parent;
    bool frameToImage(const QVideoFrame &input);
    cv::Mat m_frame;

    OCVObjectColorDetector m_cd;
};

#endif // OVVIDEOFILTERRUNNABLE_H
