#include <QElapsedTimer>
#include <QFile>
#include <QDebug>
#include <QMutexLocker>

#include "objectdetectorworker.h"

ObjectDetectorWorker::ObjectDetectorWorker(QObject *parent) :
    QObject(parent),
    m_mutex(),
    m_darknet_scale(1.0/255.0),
    m_width(480),
    m_height(480),
    m_confidence(0.75),
    m_crop(false),
    m_scale(true),
    m_nms(true),
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

    if (config.isEmpty() && model.endsWith("onnx")) {
        try {
            m_net = cv::dnn::readNet(m_model.toStdString());
            if (m_net.empty())
                throw std::invalid_argument("Net is empty");
        } catch (const std::exception& e) {
            qWarning() << "Loading model failed with error" << e.what();
        }
    } else {
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

#if 1 \
            //m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU); \
            //m_net.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL);
#else \
            // Make sure OpenCV is built with CUDA
            m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
            m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
#endif

            qDebug() << "WorkerThread: Model loaded in " << timer.elapsed()/1000.0 << "s" << (m_net.empty() ? "Empty net" : "Net OK");

            emit modelLoaded();
        } catch (const std::exception& e) {
            qWarning() << "Loading model failed with error" << e.what();
        }
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
        for (size_t i = 0; i < outLayers.size(); ++i) {
            names[i] = layersNames[outLayers[i] - 1];
        }
    }
    qDebug() << names;
    return names;
}

void ObjectDetectorWorker::processOpenCVFrame()
{
    cv::Mat blob, f, frame;

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

    if (m_frame.empty()) {
        qWarning() << "Empty frame";
        emit error(3);
        return;
    }

    if (m_frame.channels()!=3) {
        qWarning() << "Image must have 3 channels, got " << frame.channels();
        emit error(4);
        return;
    }

    m_processing=true;
    emit detectionStarted();

    qDebug() << "Starting processing in thread, required confidence " << m_confidence;

    QElapsedTimer timer;
    timer.start();

    if (m_scale && m_width!=m_frame.rows && m_height!=m_frame.cols) {
        qDebug() << "Scaling frame to match network size from " << m_frame.rows << m_frame.cols;
        cv::Size netsize(m_width, m_height);
        cv::resize(m_frame, frame, netsize, cv::INTER_NEAREST);
        qDebug() << "resize" << timer.elapsed()/1000.0 << "s";
    } else {
        qDebug() << "Using frame as-is: " << m_frame.rows << m_frame.cols;
        frame=m_frame;
    }    

    cv::dnn::blobFromImage(frame, blob, m_darknet_scale, cv::Size(m_width, m_height), cv::Scalar(), true, false);    
    m_net.setInput(blob);

    m_net.enableWinograd(false);

    std::vector<cv::Mat> outs;
    std::vector<std::vector<cv::Mat>> outs2;

    try {
        const cv::String a;
        if (m_width==640) {
            m_net.forward(outs2, getOutputsNames(m_net));
        } else {
            m_net.forward(outs, m_net.getUnconnectedOutLayersNames());

            qDebug() << outs[0].size[1];
            qDebug() << outs[0].size[2];
            qDebug() << outs[0].size[3];

            qDebug() << outs[1].size[1];
            qDebug() << outs[1].size[2];
            qDebug() << outs[1].size[3];

            qDebug() << outs[2].size[1];
            qDebug() << outs[2].size[2];
            qDebug() << outs[2].size[3];

        }
    } catch (const std::exception& e) {
        qWarning() << "OpenCV DNN failed: " << e.what();
        emit detectionEnded();
        m_processing=false;
        return;
    }

    qDebug() << "Network processed in " << timer.elapsed()/1000.0 << "s" << outs2.size();

    std::vector<int> outLayers = m_net.getUnconnectedOutLayers();
    std::string outLayerType = m_net.getLayer(outLayers[0])->type;

    std::vector<float> confidences;
    std::vector<int> classes;
    std::vector<cv::Rect2d> boxes;
    std::vector<cv::Point2f> centers;

    bool found=false;
    int outside=0;

    // Loop over the yolo layer vectors, 3 for yolov7

    for (size_t i = 0; i < outs2.size(); i++) {
        // Network produces output blob with a shape NxC where N is a number of
        // detected objects and C is a number of classes + 4 where the first 4
        // numbers are [center_x, center_y, width, height]

        // Get the output matrix for current layer
        cv::Mat & output = outs2[i][0];
        int nclasses=output.cols-5;

        qDebug() << "Classes" << nclasses;

        for (int j=0; j<output.rows; j++) {
            cv::Mat scores = output.row(j).colRange(5, output.cols);
            cv::Point classIdPoint;
            double confidence;

            const float * const data = output.ptr<float>(j);

            // Skip if
            if (data[4]<0.01f) {
                continue;
            }

            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            qDebug() << "cv::minMaxLoc" << i << j << confidence << scores.depth() << scores.channels();

            // Skip if outside confidence
            if (confidence < m_confidence) {
                outside++;
                continue;
            }

            found=true;
            int id=classIdPoint.x;

            float cxf=data[0];
            float cyf=data[1];
            float wf=data[2];
            float hf=data[3];
            float ob=data[4];

            cv::Rect2d r(cxf-wf/2.0f,cyf-hf/2.0f,wf,hf);

            const cv::Point2f c(cxf, cyf);

            boxes.push_back(r);
            centers.push_back(c);
            classes.push_back(id);
            confidences.push_back(confidence);

            if (!m_nms) {
                QPointF center;
                QRectF relative;
                QString rgb;
                QString color;

                center.setX(cxf);
                center.setY(cyf);

                relative.setRect(center.x()-wf/2.0, center.y()-hf/2.0, wf, hf);

                m_colordetector.setROI(cxf, cyf);
                if (m_colordetector.processOpenCVFrame(m_frame)==true) {
                    rgb=m_colordetector.getColorRGB();
                    color=m_colordetector.getColorGroup();
                }

                emit objectDetected(id, confidence, center, relative, rgb, color);
            }
        }
    }

    qDebug() << "Result processed in " << timer.elapsed()/1000.0;

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, 0.0, 0.4, indices);
    qDebug() << "indices" << indices.size();

    for (const auto & ind : indices) {
        QPointF center;
        QRectF relative;
        QString rgb;
        QString color;

        float cxf=centers[ind].x;
        float cyf=centers[ind].y;

        float x,y,w,h;

        x=boxes[ind].x;
        y=boxes[ind].y;
        w=boxes[ind].width;
        h=boxes[ind].height;

        center.setX(cxf);
        center.setY(cyf);

        relative.setRect(qBound(0.0f, x, 1.0f),
                         qBound(0.0f, y, 1.0f),
                         qBound(0.0f, w, 1.0f),
                         qBound(0.0f, h, 1.0f));

        m_colordetector.setROI(cxf, cyf);
        if (m_colordetector.processOpenCVFrame(m_frame)==true) {
            rgb=m_colordetector.getColorRGB();
            color=m_colordetector.getColorGroup();
        }

        emit objectDetected(classes[ind], confidences[ind], center, relative, rgb, color);
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

