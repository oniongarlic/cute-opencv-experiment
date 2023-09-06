#include "ovvideofiltersink.h"

#include <QDebug>
#include <opencv2/imgproc/types_c.h>

OvVideoFilterSink::OvVideoFilterSink(QObject *parent)
    : QObject{parent}
{

}

void OvVideoFilterSink::setVideoSink(QObject *videosink)
{
    if (m_videosink) {
        disconnect(m_videosink, nullptr, this, nullptr);
        m_videosink=nullptr;
    }
    m_videosink = qobject_cast<QVideoSink*>(videosink);

    connect(m_videosink, &QVideoSink::videoFrameChanged, this, &OvVideoFilterSink::processFrame);
}

void OvVideoFilterSink::processFrame(const QVideoFrame &frame)
{
    QImage img=frame.toImage();

    switch (img.format()) {
    case QImage::Format_RGB32:
        m_frame=cv::Mat(img.height(), img.width(), CV_8UC4, (void*)img.constBits(), img.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        m_frame=cv::Mat(img.height(), img.width(), CV_8UC3, (void*)img.constBits(), img.bytesPerLine());
        break;
    case QImage::Format_Invalid:
        return;
    default: {
        QImage conv = img.convertToFormat(QImage::Format_RGB32);
        m_frame=cv::Mat(conv.height(), conv.width(), CV_8UC4, (void*)conv.constBits(), conv.bytesPerLine());
    }
    }

    m_cd.processOpenCVFrame(m_frame);

    if (m_cd.isValid()) {
        int i=m_cd.colorIndex();
        if (i!=m_ci) {
            emit colorFound(m_cd.getColorGroup(), m_cd.getColorName(), m_cd.getColorRGB());
            m_videosink->setSubtitleText(m_cd.getColorName());
            m_ci=i;
        }
    }
}

