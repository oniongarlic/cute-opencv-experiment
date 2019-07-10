QT += quick multimedia
CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS
SOURCES += \
    main.cpp \
    ocvobjectcolordetector.cpp \
    objectdetector.cpp \
    cuteopencv.cpp \
    cuteopencvbase.cpp \
    objectdetectorworker.cpp \
    ovvideofilter.cpp \
    ovvideofilterrunnable.cpp

RESOURCES += qml.qrc

# DEFINES += YOLOV2CUSTOM
# DEFINES += YOLOV2
DEFINES += YOLOV3
# DEFINES += YOLOV3TINY

RESOURCES += yolo.qrc
# RESOURCES += yolo-v2.qrc
RESOURCES += yolo-v3.qrc
RESOURCES += yolo-v3-tiny.qrc

unix:!qnx:!android {
    CONFIG +=link_pkgconfig
    packagesExist(opencv4) {
        PKGCONFIG += opencv4
    }
contains(DEFINES,YOLOV2CUSTOM) {
    DEFINES+= YOLO_WEIGHTS=\\\"/opt/yolo/test.weights\\\"
    DEFINES+= YOLO_CFG=\\\":///yolo/obj-detect.cfg\\\"
    DEFINES+= YOLO_NAMES=\\\":///yolo/obj.names\\\"
}

contains(DEFINES,YOLOV3) {
    DEFINES+= YOLO_WEIGHTS=\\\"/opt/yolo3/yolov3.weights\\\"
    DEFINES+= YOLO_CFG=\\\":///yolo3/yolov3.cfg\\\"
    DEFINES+= YOLO_NAMES=\\\":///yolo3/coco.names\\\"
}

contains(DEFINES,YOLOV3TINY) {
    DEFINES+= YOLO_WEIGHTS=\\\"/opt/yolo3tiny/yolov3-tiny.weights\\\"
    DEFINES+= YOLO_CFG=\\\":///yolo3tiny/yolov3-tiny.cfg\\\"
    DEFINES+= YOLO_NAMES=\\\":///yolo3tiny/coco.names\\\"
}

}

android {
    QT += androidextras
    HEADERS += androidhelper.h
    SOURCES += androidhelper.cpp
}

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/3rdparty/opencv-armv8/libopencv_core.so \
        $$PWD/3rdparty/opencv-armv8/libopencv_imgproc.so \
        $$PWD/3rdparty/opencv-armv8/libopencv_dnn.so \
        $$PWD/3rdparty/opencv-armv8/libtbb.so

    INCLUDEPATH = /opt/Android/opencv/opencv-armv8/install/sdk/native/jni/include
}

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    QMAKE_CXXFLAGS += -mfpu=neon

    ANDROID_EXTRA_LIBS = \
        $$PWD/3rdparty/opencv-armv7/libopencv_core.so \
        $$PWD/3rdparty/opencv-armv7/libopencv_imgproc.so \
        $$PWD/3rdparty/opencv-armv7/libopencv_dnn.so \
        $$PWD/3rdparty/opencv-armv7/libtbb.so

    INCLUDEPATH = /opt/Android/opencv/opencv-armv7/install/sdk/native/jni/include

    # Order is important, linker in old droids (4.2) are dumb
    LIBS += -L$$PWD/3rdparty/opencv-armv7/ -ltbb -lopencv_core -lopencv_imgproc -lopencv_dnn

#"/home/milang/qt/qt-openvc-helloworld/yolo/test.weights"

contains(DEFINES,YOLOV2CUSTOM) {
    yolow.path = /assets
    yolow.source = yolo
    yolow.files = yolo/test.weights
    DEFINES+= YOLO_WEIGHTS=\\\"assets:/test.weights\\\"
    DEFINES+= YOLO_CFG=\\\":///yolo/obj-detect.cfg\\\"
    DEFINES+= YOLO_NAMES=\\\":///yolo/obj.names\\\"
}

contains(DEFINES,YOLOV2) {
    yolow.path = /assets
    yolow.source = yolo2
    yolow.files = yolo3/test.weights
    DEFINES+= YOLO_WEIGHTS=\\\"assets:/test.weights\\\"
    DEFINES+= YOLO_CFG=\\\":///yolo2/yolo.cfg\\\"
    DEFINES+= YOLO_NAMES=\\\":///yolo2/obj.names\\\"
}

#coco.names  yolov3.cfg  yolov3.weights
contains(DEFINES,YOLOV3) {
    yolow.path = /assets
    yolow.source = yolo3
    yolow.files = yolo3/yolov3.weights

    DEFINES+= YOLO_WEIGHTS=\\\"assets:/yolov3.weights\\\"
    DEFINES+= YOLO_CFG=\\\":///yolo3/yolov3.cfg\\\"
    DEFINES+= YOLO_NAMES=\\\":///yolo3/coco.names\\\"
}

contains(DEFINES,YOLOV3TINY) {
    yolow.path = /assets
    yolow.source = yolo3tiny
    yolow.files = yolo3tiny/yolov3tiny.weights

    DEFINES+= YOLO_WEIGHTS=\\\"assets:/yolov3-tiny.weights\\\"
    DEFINES+= YOLO_CFG=\\\":///yolo3/yolov3-tiny.cfg\\\"
    DEFINES+= YOLO_NAMES=\\\":///yolo3/coco.names\\\"
}

INSTALLS += yolow

}

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
# QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ocvobjectcolordetector.h \
    objectdetector.h \
    cuteopencv.h \
    cuteopencvbase.h \
    objectdetectorworker.h \
    ovvideofilter.h \
    ovvideofilterrunnable.h

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
