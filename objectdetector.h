#ifndef OBJECTDETECTOR_H
#define OBJECTDETECTOR_H

#include <QObject>
#include <QImage>

#include <iostream>
#include <fstream>
#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

#include "cuteopencvbase.h"

struct DetectedObject
{
    int id;
    int centerX;
    int centerY;
    int width;
    int height;
    int left;
    int top;

    float cxf;
    float cyf;
    float xf;
    float yx;
    float wf;
    float hf;

    double confidence;
};

class ObjectDetector : public CuteOpenCVBase
{
    Q_OBJECT

    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QString config READ config WRITE setConfig NOTIFY configChanged)

public:
    explicit ObjectDetector(QObject *parent = nullptr);
    ~ObjectDetector();

    Q_INVOKABLE bool loadModel();

    QString model() const
    {
        return m_model;
    }

    QString config() const
    {
        return m_config;
    }

signals:

    void modelChanged(QString model);

    void configChanged(QString config);

    void noObjectDetected();

    void objectDetected(int cid, double confidence, float x, float y, float w, float h);

public slots:

void setModel(QString model)
{
    if (m_model == model)
        return;

    m_model = model;
    emit modelChanged(m_model);
}

void setConfig(QString config)
{
    if (m_config == config)
        return;

    m_config = config;
    emit configChanged(m_config);
}

protected:
    bool processOpenCVFrame(cv::Mat &frame);
private:
    QString m_model;
    QString m_config;
    QString m_class;

    double m_confidence;

    const double m_darknet_scale;

    int m_width;
    int m_height;

    bool m_crop;

    cv::dnn::Net m_net;
    std::vector<cv::String> getOutputsNames(const cv::dnn::Net &net);

    QList<DetectedObject> m_objects;
};

#endif // OBJECTDETECTOR_H
