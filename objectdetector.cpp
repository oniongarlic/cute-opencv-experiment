#include "objectdetector.h"

#include <string>

#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>

ObjectDetector::ObjectDetector(QObject *parent) :
    CuteOpenCVBase(parent),    
    m_confidence(0.75),
    m_darknet_scale(0.00392),
    m_width(32),
    m_height(320),
    m_crop(false)
{
    m_scaledown=false;
    m_scaleheight=480;
    m_scalewidth=480;
}

ObjectDetector::~ObjectDetector()
{
    m_thread.quit();
    m_thread.wait();
}

bool ObjectDetector::start()
{
    if (m_thread.isRunning())
        return false;

    return startWorkerThread();
}

bool ObjectDetector::stop()
{
    if (m_thread.isFinished())
        return false;

    m_thread.quit();
    m_thread.wait();

    return true;
}

void ObjectDetector::dataLoaded(const QByteArray &data)
{

}

void ObjectDetector::objectDetectedByWorker(int cid, double confidence, QPointF center, QRectF rect, QString rgb, QString color)
{
    qDebug() << "Worker Reports: " << cid << confidence << center << rect << rgb << color;

    DetectedObject o;
    o.id=cid;
    o.center=center;
    o.relative=rect;
    o.confidence=confidence;
    o.rgb=rgb;
    o.color=color;
    m_objects.append(o);

    emit objectDetected(cid, confidence, center, rect, rgb, color);
}

void ObjectDetector::workerDetectionStarted()
{
    qDebug() << "Worker workerDetectionStarted";

    m_objects.clear();

    emit detectionStarted();
}

void ObjectDetector::workerDetectionEnded()
{
    qDebug() << "Worker workerDetectionEnded" << m_objects.size();

    emit detectionEnded(m_objects.size());

    if (m_objects.size()==0)
        emit noObjectDetected();
}

bool ObjectDetector::startWorkerThread()
{
    w=new ObjectDetectorWorker();
    w->setSize(m_width, m_height);
    w->loadModel(m_config, m_model);
    w->moveToThread(&m_thread);
    connect(&m_thread, &QThread::finished, w, &QObject::deleteLater);
    connect(this, &ObjectDetector::processFrameInThread, w, &ObjectDetectorWorker::processOpenCVFrame);
    connect(w, &ObjectDetectorWorker::objectDetected, this, &ObjectDetector::objectDetectedByWorker);
    connect(w, &ObjectDetectorWorker::detectionEnded, this, &ObjectDetector::workerDetectionEnded);
    connect(w, &ObjectDetectorWorker::detectionStarted, this, &ObjectDetector::workerDetectionStarted);
    m_thread.setObjectName("WorkerThread");
    m_thread.start();

    return true;
}

bool ObjectDetector::loadClasses()
{
    if (m_class!="" && !QFile::exists(m_class)) {
        qWarning() << "Class ID to name mapping configuration set but not found.";
        return false;
    }

    if (m_class!="") {
        QFile classes(m_class);
        classes.open(QIODevice::ReadOnly);
        QTextStream text(&classes);
        int l=0;
        m_classes.clear();
        while (!text.atEnd()) {
            QString line=text.readLine();
            m_classes.insert(l, line);
            l++;
        }
        return true;
    }

    return false;
}

QString ObjectDetector::getClassName(int i) const
{
    return m_classes.value(i, "");
}

bool ObjectDetector::processOpenCVFrame(cv::Mat &frame)
{
    cv::Mat blob, f;

    if (!w->setFrame(frame))
        return false;

    w->setConfidence(m_confidence);

    emit processFrameInThread();

    return true;
}


