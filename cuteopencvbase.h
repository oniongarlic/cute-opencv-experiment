#ifndef CUTEOPENCVBASE_H
#define CUTEOPENCVBASE_H

#include <QObject>
#include <QImage>

#include <opencv2/core/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

class CuteOpenCVBase : public QObject
{
    Q_OBJECT
public:
    explicit CuteOpenCVBase(QObject *parent = nullptr);

    Q_INVOKABLE bool processFrame(QImage &frame);
    Q_INVOKABLE bool processImageFile(QString path);
    QImage copy(const QRect &rectangle = QRect()) const;

signals:

protected:
    virtual bool processOpenCVFrame(cv::Mat &frame);

    bool m_scaledown;
    int m_scalewidth;
    int m_scaleheight;

    QImage m_image;

public slots:
};

#endif // CUTEOPENCVBASE_H
