#include "objectdetector.h"

#include <QDebug>
#include <QFile>

ObjectDetector::ObjectDetector(QObject *parent) :
    CuteOpenCVBase(parent),
    m_model("/home/milang/qt/qt-openvc-helloworld/yolo/test.weights"),
    m_config(":///yolo/obj-detect.cfg"),
    m_confidence(0.75),
    m_darknet_scale(0.00392),
    m_width(480),
    m_height(480),
    m_crop(false)
{

}

ObjectDetector::~ObjectDetector()
{
    //m_net.
}

bool ObjectDetector::loadModel()
{
    if (!QFile::exists(m_model)) {
        qWarning() << "Model network file not found.";
        return false;
    }

    if (!QFile::exists(m_config)) {
        qWarning() << "Model configuration not found.";
        return false;
    }

    try {
        if (m_config.startsWith(":///")) {
            QFile config(m_config);
            config.open(QIODevice::ReadOnly);
            QByteArray cdata=config.readAll();

            QFile model(m_model);
            model.open(QIODevice::ReadOnly);
            QByteArray mdata=model.readAll();

            m_net = cv::dnn::readNetFromDarknet(cdata, cdata.size(), mdata, mdata.size());
        } else {
            m_net = cv::dnn::readNetFromDarknet(m_config.toStdString(), m_model.toStdString());
        }


        //        m_net = cv::dnn::readNetFromDarknet(m_config.toStdString(), m_model.toStdString());
        //m_net=cv::dnn::readNet(m_model.toStdString(), m_config.toStdString());
        //m_net.setPreferableBackend(0);
        //m_net.setPreferableTarget(0);

        qDebug() << (m_net.empty() ? "Empty net" : "Net OK");

        return true;
    } catch (const std::exception& e) {
        qWarning() << e.what();
    }
    return false;
}

std::vector<cv::String> ObjectDetector::getOutputsNames(const cv::dnn::Net& net)
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

bool ObjectDetector::processOpenCVFrame(cv::Mat &frame)
{
    cv::Mat blob, f;

    if (m_net.empty()) {
        qWarning() << "Net is not loaded";
        return false;
    }

    if (frame.empty()) {
        qWarning() << "Empty frame";
        return false;
    }


    if (frame.channels()!=3) {
        qWarning() << "Image must have 3 channels!";
    }

    cv::dnn::blobFromImage(frame, blob, m_darknet_scale, cv::Size(m_width, m_height), cv::Scalar(), true, m_crop);

    m_net.setInput(blob);
    std::vector<cv::Mat> outs;

    try {
        m_net.forward(outs, getOutputsNames(m_net));
    } catch (const std::exception& e) {
        qWarning() << e.what();
        return false;
    }

    std::vector<int> outLayers = m_net.getUnconnectedOutLayers();
    std::string outLayerType = m_net.getLayer(outLayers[0])->type;

    if (outs.size()==0) {
        emit noObjectDetected();
        return false;
    }

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
            if (confidence > m_confidence) {
                DetectedObject o;

                o.id=classIdPoint.x;
                o.centerX = (int)(data[0] * frame.cols);
                o.centerY = (int)(data[1] * frame.rows);
                o.width = (int)(data[2] * frame.cols);
                o.height = (int)(data[3] * frame.rows);
                o.left = o.centerX - o.width / 2;
                o.top = o.centerY - o.height / 2;
                o.confidence=confidence;

                qDebug() << o.id << o.confidence << o.centerX << o.centerY;

                m_objects.push_back(o);

                emit objectDetected(o.id, o.confidence, o.centerX, o.centerY);

                //classIds.push_back(classIdPoint.x);
                //confidences.push_back((float)confidence);
                //boxes.push_back(Rect(left, top, width, height));
            }
        }
    }

    return true;
}


