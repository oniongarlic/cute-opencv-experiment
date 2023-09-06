#ifndef OVVIDEOFILTERSINK_H
#define OVVIDEOFILTERSINK_H

#include "ocvobjectcolordetector.h"
#include <QObject>
#include <QQmlEngine>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>

class OvVideoFilterSink : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QObject* videoSink WRITE setVideoSink)
public:
    explicit OvVideoFilterSink(QObject *parent = nullptr);

signals:
    void colorFound(const QString cgroup, const QString cname, const QString chex);

protected slots:
    void processFrame(const QVideoFrame &frame);

private:
    void setVideoSink(QObject *videosink);
    QVideoSink *m_videosink=nullptr;
    cv::Mat m_frame;
    OCVObjectColorDetector m_cd;
    int m_ci;
};

#endif // OVVIDEOFILTERSINK_H
