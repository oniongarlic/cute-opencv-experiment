#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "ovvideofilter.h"

#include "ocvobjectcolordetector.h"
#include "objectdetector.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    app.setApplicationName("HelloWorldQtOpenCV");
    app.setOrganizationDomain("tal.org");

    qmlRegisterType<OCVObjectColorDetector>("org.tal", 1,0, "ColorDetector");
    qmlRegisterType<ObjectDetector>("org.tal", 1,0, "ObjectDetector");

    qmlRegisterType<OvVideoFilter>("org.tal", 1,0, "OpenCVVideoFilter");

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
