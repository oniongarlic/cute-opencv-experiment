#ifndef OBJECTDETECTORWORKER_H
#define OBJECTDETECTORWORKER_H

#include <QObject>
#include <QImage>
#include <QThread>
#include <QMap>

#include <iostream>
#include <fstream>
#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

#include <QThread>
#include <QMutex>

#include "ocvobjectcolordetector.h"

class ObjectDetectorWorker : public QObject
{
    Q_OBJECT
public:
    explicit ObjectDetectorWorker(QObject *parent = nullptr);

signals:
    void error(int code);
    void detectionStarted();
    void detectionEnded();
    void noObjectDetected();
    void objectDetected(int cid, double confidence, QPointF center, QRectF rect, QString rgb, QString color);
    void modelLoaded();    

public slots:
    void setSize(int width, int height);
    void loadModel(const QString config, const QString model);
    void processOpenCVFrame();
    bool setFrame(cv::Mat &frame);    

    void setConfidence(qreal confidence);

private:
    QMutex m_mutex;

    QString m_model;
    QString m_config;

    const double m_darknet_scale;    
    int m_width;
    int m_height;
    double m_confidence;
    bool m_crop;
    bool m_processing;

    double m_frametime;

    cv::dnn::Net m_net;
    cv::Mat m_frame;
    std::vector<cv::String> getOutputsNames(const cv::dnn::Net &net);

    OCVObjectColorDetector m_colordetector;
};

#endif // OBJECTDETECTORWORKER_H
