#include "cuteopencvbase.h"

#include <QDebug>

CuteOpenCVBase::CuteOpenCVBase(QObject *parent) :
    QObject(parent),
    m_scaledown(false),
    m_scalewidth(0),
    m_scaleheight(0)
{

}

bool CuteOpenCVBase::processImageFile(QString path)
{
    QImage i;

    qDebug() << path;
    if (path.startsWith("file://")) {        
        path=path.remove(0,7);
        qDebug() << path;
    }

    i.load(path);

    if (i.isNull()) {
        qWarning() << "Failed to load image file " << path;
        return false;
    }

    return processFrame(i);
}

bool CuteOpenCVBase::processOpenCVFrame(cv::Mat &frame)
{
    Q_UNUSED(frame)

    qDebug() << "Implement your own processOpenCVFrame!";

    return false;
}

bool CuteOpenCVBase::processFrame(QImage &frame)
{
    qDebug() << frame.format() << frame.width() << frame.height();

    if (m_scaledown && m_scalewidth>0 && m_scaleheight>0) {
        frame=frame.scaled(m_scalewidth, m_scaleheight, Qt::KeepAspectRatioByExpanding);
        qDebug() << "ScaledDownTo" << frame.format() << frame.width() << frame.height();
    }

    switch(frame.format()) {
    case QImage::Format_Invalid:
    {
        qDebug() << "Invalid image";
        return false;
    }
    case QImage::Format_RGB32:
    {
        qDebug() << "Format_RGB32";
        cv::Mat view(frame.height(), frame.width(), CV_8UC4,(void *)frame.constBits(), frame.bytesPerLine());
        cvtColor(view, view, cv::COLOR_RGB2BGR);
        return processOpenCVFrame(view);
    }
    case QImage::Format_RGB888:
    {
        qDebug() << "Format_RGB888";
        cv::Mat view(frame.height(), frame.width(), CV_8UC3,(void *)frame.constBits(), frame.bytesPerLine());
        cvtColor(view, view, cv::COLOR_RGB2BGR);
        return processOpenCVFrame(view);
    }
    default:
    {
        qDebug() << "Format_ARGB32";
        QImage conv = frame.convertToFormat(QImage::Format_ARGB32);
        cv::Mat view(conv.height(),conv.width(),CV_8UC4,(void *)conv.constBits(),conv.bytesPerLine());
        return processOpenCVFrame(view);
    }
    }

    return false;
}
