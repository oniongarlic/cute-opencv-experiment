#include <QGuiApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "ovvideofilter.h"

#include "ocvobjectcolordetector.h"
#include "objectdetector.h"

#include "cuteimageprovider.h"

#ifdef Q_OS_ANDROID
#include "androidhelper.h"
#include <QtAndroidExtras/QtAndroid>

const QVector<QString> required_permissions(
{
            "android.permission.ACCESS_COARSE_LOCATION",
            "android.permission.ACCESS_FINE_LOCATION",
            "android.permission.CAMERA",
            "android.permission.INTERNET",
            "android.permission.ACCESS_NETWORK_STATE",
            "android.permission.WRITE_EXTERNAL_STORAGE",
            "android.permission.READ_EXTERNAL_STORAGE"
});
static QVariantMap checked_permissions;
#endif

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    CuteImageProvider *cuteprovider=new CuteImageProvider(&app);

    app.setApplicationName("CuteOpenCVHelloWorld");
    app.setOrganizationDomain("tal.org");

    QString image_path;

    QString dnn_config;
    QString dnn_model;
    QString dnn_classes;

#ifdef Q_OS_ANDROID
    AndroidHelper android;
    engine.rootContext()->setContextProperty("android", &android);

    for (const QString &permission : required_permissions) {
        auto result = QtAndroid::checkPermission(permission);

        qDebug() << "AndroidPermissionCheck" << permission << (result==QtAndroid::PermissionResult::Granted ? "Granted" : "Denied");

        if (result == QtAndroid::PermissionResult::Denied) {
            auto resultHash = QtAndroid::requestPermissionsSync(QStringList({permission}));
            if (resultHash[permission] == QtAndroid::PermissionResult::Denied) {
                checked_permissions.insert(permission, false);
            } else {
                checked_permissions.insert(permission, true);
            }
        } else {
            checked_permissions.insert(permission, true);
        }
    }
    qDebug() << "Android permissions" << checked_permissions;
    engine.rootContext()->setContextProperty("permissions", checked_permissions);

    qDebug() << "External storage is" << android.getExternalStorage();

    //dnn_config=YOLO_CFG;
    //dnn_classes=YOLO_NAMES;

    dnn_config=android.getExternalStorage()+YOLO_CFG;
    dnn_classes=android.getExternalStorage()+YOLO_NAMES;
    dnn_model=android.getExternalStorage()+YOLO_WEIGHTS;    
#else
    dnn_config=YOLO_CFG;
    dnn_classes=YOLO_NAMES;
    dnn_model=YOLO_WEIGHTS;
#endif       

    image_path=QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    qDebug() << image_path;

    engine.rootContext()->setContextProperty("dnnConfig", dnn_config);
    engine.rootContext()->setContextProperty("dnnWeights", dnn_model);
    engine.rootContext()->setContextProperty("dnnClasses", dnn_classes);

    engine.rootContext()->setContextProperty("imagePath", image_path);

    engine.rootContext()->setContextProperty("imp", cuteprovider); // ImageManipulatorProvider (?)

    engine.addImageProvider("cute", cuteprovider);

    qmlRegisterType<OCVObjectColorDetector>("org.tal", 1,0, "ColorDetector");
    qmlRegisterType<ObjectDetector>("org.tal", 1,0, "ObjectDetector");
    qmlRegisterType<OvVideoFilter>("org.tal", 1,0, "OpenCVVideoFilter");

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    int r=app.exec();

    engine.removeImageProvider("cute");

    return r;
}
