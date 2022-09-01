#include <QElapsedTimer>
#include <QFile>
#include <QDebug>
#include <QMutexLocker>

#include "objectdetectorworker.h"

ObjectDetectorWorker::ObjectDetectorWorker(QObject *parent) :
    QObject(parent),
    m_mutex(QMutex::NonRecursive),
    m_darknet_scale(0.00392),
    m_width(480),
    m_height(480),
    m_confidence(0.75),
    m_crop(false),    
    m_processing(false),
    m_colordetector(parent)
{

}

void ObjectDetectorWorker::setSize(int width, int height)
{
    m_width=width;
    m_height=height;
}

void ObjectDetectorWorker::loadModel(const QString config, const QString model)
{
    m_config=config;
    m_model=model;

    QElapsedTimer timer;
    timer.start();

    QMutexLocker locker(&m_mutex);

    qDebug() << "Configuration & Weights are" << m_config << m_model;

    try {
        if (QFile::exists(m_config)==false)
            throw std::invalid_argument("Model configuration not found");

        if (QFile::exists(m_model)==false)
            throw std::invalid_argument("Model weights not found");

        if (m_config.startsWith(":///")) {
            qDebug() << "Loading model from resources / filesystem " << m_config << m_model;

            QFile config(m_config);
            if (config.open(QIODevice::ReadOnly)==false)
                throw std::invalid_argument("Invalid model configuration");

            const QByteArray cdata=config.readAll();           

            QFile model(m_model);
            if (model.open(QIODevice::ReadOnly)==false)
                throw std::invalid_argument("Invalid model data");

            const QByteArray mdata=model.readAll();
            // From memory
            m_net = cv::dnn::readNetFromDarknet(cdata, cdata.size(), mdata, mdata.size());
        } else {
            qDebug() << "Loading model from filesystem " << m_config << m_model;

            // From files
            m_net = cv::dnn::readNetFromDarknet(m_config.toStdString(), m_model.toStdString());
            if (m_net.empty())
                throw std::invalid_argument("Net is empty");
        }

        //m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        //m_net.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL);

        qDebug() << "WorkerThread: Model loaded in " << timer.elapsed()/1000.0 << "s" << (m_net.empty() ? "Empty net" : "Net OK");

        emit modelLoaded();
    } catch (const std::exception& e) {
        qWarning() << "Loading model failed with error" << e.what();
    }
    emit error(1);
}

std::vector<cv::String> ObjectDetectorWorker::getOutputsNames(const cv::dnn::Net& net)
{
    static std::vector<cv::String> names;
    if (names.empty())
    {
        std::vector<int> outLayers = net.getUnconnectedOutLayers();
        std::vector<cv::String> layersNames = net.getLayerNames();
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i)
            names[i] = layersNames[outLayers[i] - 1];
    }
    return names;
}

void ObjectDetectorWorker::processOpenCVFrame()
{
    cv::Mat blob, f;

    QMutexLocker locker(&m_mutex);

    if (m_net.empty()) {
        qWarning() << "Net is not loaded";
        emit error(2);
        return;
    }

    if (m_processing==true) {
        qWarning() << "Processing is already running...";
        return;
    }

    m_processing=true;
    const cv::Mat frame=m_frame;

    if (frame.empty()) {
        qWarning() << "Empty frame";
        emit error(3);
        return;
    }

    if (frame.channels()!=3) {
        qWarning() << "Image must have 3 channels, got " << frame.channels();
        emit error(4);
        return;
    }

    emit detectionStarted();

    qDebug() << "Starting processing in thread, required confidence " << m_confidence;

    QElapsedTimer timer;
    timer.start();

    cv::dnn::blobFromImage(frame, blob, m_darknet_scale, cv::Size(m_width, m_height), cv::Scalar(), true, m_crop);
    qDebug() << "blobFromImage" << timer.elapsed()/1000.0 << "s";

    m_net.setInput(blob);
    std::vector<cv::Mat> outs;

    try {
        const cv::String a;
        m_net.forward(outs, getOutputsNames(m_net));
    } catch (const std::exception& e) {
        qWarning() << e.what();
        return;
    }

    qDebug() << "Frame processed in " << timer.elapsed()/1000.0 << "s";

    std::vector<int> outLayers = m_net.getUnconnectedOutLayers();
    std::string outLayerType = m_net.getLayer(outLayers[0])->type;

    bool found=false;
    int outside=0;

    for (size_t i = 0; i < outs.size(); ++i) {
        // Network produces output blob with a shape NxC where N is a number of
        // detected objects and C is a number of classes + 4 where the first 4
        // numbers are [center_x, center_y, width, height]
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) {
            cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            cv::Point classIdPoint;
            double confidence;

            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);

            if (confidence < m_confidence) {
                outside++;
                continue;
            }

            found=true;
            QPointF center;
            QRectF relative;
            int id=classIdPoint.x;

            float cxf=data[0];
            float cyf=data[1];
            float wf=data[2];
            float hf=data[3];

            center.setX(cxf);
            center.setY(cyf);

            relative.setRect(center.x()-wf/2.0, center.y()-hf/2.0, wf, hf);

            QString rgb;
            QString color;

            m_colordetector.setROI(cxf, cyf);
            if (m_colordetector.processOpenCVFrame(m_frame)==true) {
                rgb=m_colordetector.getColorRGB();
                color=m_colordetector.getColorGroup();
            }

            emit objectDetected(id, confidence, center, relative, rgb, color);
        }
    }

    m_frametime=timer.elapsed()/1000.0;
    qDebug() << "Detection done in " << m_frametime << "s" << "Outside confidence" << outside;

    if (!found)
        emit noObjectDetected();

    emit detectionEnded();  

    m_processing=false;
}

bool ObjectDetectorWorker::setFrame(cv::Mat &frame)
{
    QMutexLocker locker(&m_mutex);
    if (m_processing) {
        qDebug("Frame is active, can not set at this time.");
        return false;
    }

    // Make a copy of it for our use
    m_frame=frame.clone();

    return true;
}

void ObjectDetectorWorker::setConfidence(qreal confidence)
{
    m_confidence=confidence;
}

