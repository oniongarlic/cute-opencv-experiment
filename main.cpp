#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "ovvideofilter.h"

#include "ocvobjectcolordetector.h"
#include "objectdetector.h"

#ifdef Q_OS_ANDROID
#include "androidhelper.h"
#endif

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    app.setApplicationName("QtOpenCVHelloWorld");
    app.setOrganizationDomain("tal.org");

    QString dnn_config;
    QString dnn_model;
    QString dnn_classes;

#ifdef Q_OS_ANDROID
    AndroidHelper android;
    engine.rootContext()->setContextProperty("android", &android);

    qDebug() << "External storage is" << android.getExternalStorage();

    dnn_config=YOLO_CFG;
    dnn_classes=YOLO_NAMES;

    //dnn_config=android.getExternalStorage()+YOLO_CFG;
    //dnn_classes=android.getExternalStorage()+YOLO_NAMES;
    dnn_model=android.getExternalStorage()+YOLO_WEIGHTS;    
#else
    dnn_config=YOLO_CFG;
    dnn_classes=YOLO_NAMES;
    dnn_model=YOLO_WEIGHTS;
#endif

    engine.rootContext()->setContextProperty("dnnConfig", dnn_config);
    engine.rootContext()->setContextProperty("dnnWeights", dnn_model);
    engine.rootContext()->setContextProperty("dnnClasses", dnn_classes);

    qmlRegisterType<OCVObjectColorDetector>("org.tal", 1,0, "ColorDetector");
    qmlRegisterType<ObjectDetector>("org.tal", 1,0, "ObjectDetector");
    qmlRegisterType<OvVideoFilter>("org.tal", 1,0, "OpenCVVideoFilter");

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
