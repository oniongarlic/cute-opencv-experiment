#include "objectdetector.h"

#include <string>

#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>

ObjectDetector::ObjectDetector(QObject *parent) :
    CuteOpenCVBase(parent),
    #ifdef Q_OS_ANDROID
    m_model("assets:/test.weights"),
    #else
    m_model("/home/milang/qt/qt-openvc-helloworld/yolo/test.weights"),
    #endif
    m_config(":///yolo/obj-detect.cfg"),
    m_class(":///yolo/obj.names"),
    m_confidence(0.75),
    m_darknet_scale(0.00392),
    m_width(480),
    m_height(480),
    m_crop(false)
{
    m_scaledown=true;
    m_scaleheight=480;
    m_scalewidth=480;

    m_classes.insert(0, "Shelf");
    m_classes.insert(1, "Cabinet");
    m_classes.insert(2, "Drawer");
    m_classes.insert(3, "Instrument");
    m_classes.insert(4, "Table");
    m_classes.insert(5, "Soffa");
    m_classes.insert(6, "Chair");
    m_classes.insert(7, "Office chair");
    m_classes.insert(8, "Bicycle");
    m_classes.insert(9, "Coffemaker");
    m_classes.insert(10, "Microwaveoven");
    m_classes.insert(11, "School desk");
}

ObjectDetector::~ObjectDetector()
{

}

#if 0
class AsyncFileLoader : public QObject
{
    Q_OBJECT
public:
    AsyncFileLoader(const QString file, QObject *parent=nullptr) : QObject (parent), m_file(file) {

    }
    virtual ~AsyncFileLoader() {
        m_data.clear();
    }

signals:
    void dataLoaded(const QByteArray &data);

private:
    QString m_file;
    QByteArray m_data;
};
#endif

void ObjectDetector::dataLoaded(const QByteArray &data)
{

}

bool ObjectDetector::loadModelAsync()
{
#if 0
    AsyncFileLoader *afl=new AsyncFileLoader(m_model, this);
    afl->moveToThread(&m_thread);
    connect(&m_thread, &QThread::finished, afl, &QObject::deleteLater);
    m_thread.setObjectName("ModelLoader");
    m_thread.start();
#endif
    return true;
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

    if (m_class!="" && !QFile::exists(m_class)) {
        qWarning() << "Class ID to name mapping configuration set but not found.";
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    try {
        if (m_config.startsWith(":///")) {
            QFile config(m_config);
            config.open(QIODevice::ReadOnly);
            const QByteArray cdata=config.readAll();

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
            }

            QFile model(m_model);
            model.open(QIODevice::ReadOnly);
            const QByteArray mdata=model.readAll();

            m_net = cv::dnn::readNetFromDarknet(cdata, cdata.size(), mdata, mdata.size());
        } else {
            m_net = cv::dnn::readNetFromDarknet(m_config.toStdString(), m_model.toStdString());
        }

        //m_net.getMemoryConsumption()

        //m_net = cv::dnn::readNetFromDarknet(m_config.toStdString(), m_model.toStdString());
        //m_net=cv::dnn::readNet(m_model.toStdString(), m_config.toStdString());
        m_net.setPreferableBackend(0);
        m_net.setPreferableTarget(0);

        qDebug() << "Model loaded in " << timer.elapsed()/1000.0 << "s" << (m_net.empty() ? "Empty net" : "Net OK");

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

QString ObjectDetector::getClassName(int i) const
{
    return m_classes.value(i, "");
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

    qDebug() << "Starting...";
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
        return false;
    }

    qDebug() << "Frame processed in " << timer.elapsed()/1000.0 << "s";

    std::vector<int> outLayers = m_net.getUnconnectedOutLayers();
    std::string outLayerType = m_net.getLayer(outLayers[0])->type;

    m_objects.clear();

    bool found=false;

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

                found=true;

                o.id=classIdPoint.x;

                o.cxf=data[0];
                o.cyf=data[1];
                o.wf=data[2];
                o.hf=data[3];

                o.center.setX(o.cxf);
                o.center.setY(o.cyf);

                o.relative.setRect(o.cxf-o.wf/2, o.cyf-o.hf/2, o.wf, o.hf);

                o.centerX = (int)(data[0] * frame.cols);
                o.centerY = (int)(data[1] * frame.rows);
                o.width = (int)(data[2] * frame.cols);
                o.height = (int)(data[3] * frame.rows);
                o.left = o.centerX - o.width / 2;
                o.top = o.centerY - o.height / 2;
                o.confidence=confidence;

                qDebug() << o.id << o.confidence << o.centerX << o.centerY;

                m_objects.push_back(o);

                emit objectDetected(o.id, o.confidence, o.center, o.relative);
            }
        }
    }

    if (!found)
        emit noObjectDetected();

    return found;
}


