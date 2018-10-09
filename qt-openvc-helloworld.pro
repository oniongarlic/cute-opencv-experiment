QT += quick
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
    ocvobjectcolordetector.cpp \
    objectdetector.cpp \
    cuteopencv.cpp \
    cuteopencvbase.cpp

RESOURCES += qml.qrc \
    yolo.qrc

unix:!qnx:!android {
    CONFIG +=link_pkgconfig

packagesExist(opencv4) {
    PKGCONFIG += opencv4
}

}

android {
# QT += androidextras
}

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    QMAKE_CXXFLAGS += -mfpu=neon

    ANDROID_EXTRA_LIBS = \
        $$PWD/3rdparty/opencv-armv7/libopencv_core.so \
        $$PWD/3rdparty/opencv-armv7/libopencv_imgproc.so \
        $$PWD/3rdparty/opencv-armv7/libopencv_dnn.so

    INCLUDEPATH = /home/milang/repos/opencv/buildandroidgcc/install/sdk/native/jni/include

    LIBS += -L/home/milang/repos/opencv/buildandroidgcc/install/sdk/native/libs/armeabi-v7a -lopencv_core -lopencv_imgproc -lopencv_dnn

yolow.path = /assets
yolow.source = yolo
yolow.files = yolo/test.weights
INSTALLS += yolow
}

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ocvobjectcolordetector.h \
    objectdetector.h \
    cuteopencv.h \
    cuteopencvbase.h
