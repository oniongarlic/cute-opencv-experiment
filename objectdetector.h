#ifndef OBJECTDETECTOR_H
#define OBJECTDETECTOR_H

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

#include "cuteopencvbase.h"
#include "objectdetectorworker.h"

struct DetectedObject
{
    int id;
    double confidence;
    QPointF center;    
    QRectF relative;  
};

class ObjectDetector : public CuteOpenCVBase
{
    Q_OBJECT

    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QString config READ config WRITE setConfig NOTIFY configChanged)

public:
    explicit ObjectDetector(QObject *parent = nullptr);
    ~ObjectDetector();

    QString model() const
    {
        return m_model;
    }

    QString config() const
    {
        return m_config;
    }

    Q_INVOKABLE QString getClassName(int i) const;
    Q_INVOKABLE int getObjectCount() const {
        return m_objects.size();
    }
    bool startWorkerThread();
    Q_INVOKABLE bool loadClasses();
signals:

    void modelChanged(QString model);

    void configChanged(QString config);

    void noObjectDetected();

    void objectDetected(int cid, double confidence, QPointF center, QRectF rect);

    void detectionStarted();

    void detectionEnded();

    void processFrameInThread();

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

    void dataLoaded(const QByteArray &data);

    void objectDetectedByWorker(int cid, double confidence, QPointF center, QRectF rect);

    void workerDetectionStarted();
    void workerDetectionEnded();

protected:
    bool processOpenCVFrame(cv::Mat &frame);
    bool loadModelAsync();
private:
    QString m_model;
    QString m_config;
    QString m_class;

    QThread m_thread;

    ObjectDetectorWorker *w;

    double m_confidence;

    const double m_darknet_scale;

    int m_width;
    int m_height;

    bool m_crop;

    cv::dnn::Net m_net;
    std::vector<cv::String> getOutputsNames(const cv::dnn::Net &net);

    QList<DetectedObject> m_objects;
    QMap<int, QString>m_classes;
};

#endif // OBJECTDETECTOR_H
