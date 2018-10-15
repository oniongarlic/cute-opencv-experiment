#include "ovvideofilterrunnable.h"

#include <QDebug>
#include <opencv2/imgproc/types_c.h>

OvVideoFilterRunnable::OvVideoFilterRunnable(OvVideoFilter *parent) :
    m_parent(parent)
{
    //cv::namedWindow("debug");
}

QVideoFrame OvVideoFilterRunnable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, QVideoFilterRunnable::RunFlags flags)
{
    Q_UNUSED(surfaceFormat)
    Q_UNUSED(flags)

    if (!input->isValid()) {
        qWarning("Frame is not valid.");
        return *input;
    }

    if (surfaceFormat.handleType()!=QAbstractVideoBuffer::NoHandle) {
        qWarning("Surface format not supported.");
        return *input;
    }

    if (!input->map(QAbstractVideoBuffer::ReadOnly)) {
        qWarning("Failed to map video frame for reading.");
        return *input;
    }

    qDebug() << "Frame format: " << input->pixelFormat() << surfaceFormat.frameRate();

#if 0
    QImage image(input->bits(),input->width(), input->height(), QVideoFrame::imageFormatFromPixelFormat(input->pixelFormat()));
    image = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(image.height(), image.width(), CV_8UC3, image.bits(), image.bytesPerLine());
    m_frame=cv::Mat(image.height(), image.width(), CV_8UC3);
    cvtColor(mat, m_frame, CV_RGB2BGR);
#else
    if (frameToImage(*input)) {
        //cv::imshow("debug", m_frame);
        m_cd.processOpenCVFrame(m_frame);
    }
#endif
    input->unmap();

    return *input;
}

bool OvVideoFilterRunnable::frameToImage(const QVideoFrame &input)
{
    QVideoFrame::PixelFormat m_format=input.pixelFormat();

    int w=input.width();
    int h=input.height();

    switch (m_format) {
    case QVideoFrame::Format_UYVY:
        qWarning() << "Unhandled VideoFrame format";
        break;
    case QVideoFrame::Format_RGB32:
        m_frame=cv::Mat(h, w, CV_8UC3, (void*) input.bits());
        return true;
    case QVideoFrame::Format_ARGB32:
    case QVideoFrame::Format_ARGB32_Premultiplied:
        m_frame=cv::Mat(h, w, CV_8UC4, (void*) input.bits());
        return true;
    case QVideoFrame::Format_RGB565:
    case QVideoFrame::Format_RGB555: {
        m_frame=cv::Mat(h, w, CV_8UC3, (void*) input.bits());
        //cvtColor(m_frame, m_frame, CV_RGB2BGR555, 3);
        return true;
    }
    case QVideoFrame::Format_BGR32: {
        m_frame=cv::Mat(h, w, CV_8UC3, (void*) input.bits());
        return true;
    }
    case QVideoFrame::Format_NV12:
    case QVideoFrame::Format_NV21:
    case QVideoFrame::Format_YUV420P: {
        cv::Mat mYUV(h + h/2, w, CV_8UC1, (void*) input.bits());
        //cv::Mat mRGB(h, w, CV_8UC3);
        cvtColor(mYUV, m_frame, CV_YUV2RGB_YV12, 3);
        //m_frame=cv::Mat(h, w, CV_8UC3);
        //cvtColor(mYUV, m_frame, CV_YUV2BGR);
        return true;
    }
    default:;
        qWarning() << "Unhandled VideoFrame format";
    }
    return false;
}
