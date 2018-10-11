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

class ObjectDetectorWorker : public QObject
{
    Q_OBJECT
public:
    explicit ObjectDetectorWorker(QObject *parent = nullptr);

signals:
    void error();
    void detectionStarted();
    void detectionEnded();

public slots:

    void loadModel(QString config, QString model, QString classes);

    void processOpenCVFrame(cv::Mat frame);

private:
    QString m_model;
    QString m_config;
    QString m_class;

    const double m_darknet_scale;

    int m_width;
    int m_height;

    bool m_crop;
    cv::dnn::Net m_net;
    std::vector<cv::String> getOutputsNames(const cv::dnn::Net &net);
};

#endif // OBJECTDETECTORWORKER_H
