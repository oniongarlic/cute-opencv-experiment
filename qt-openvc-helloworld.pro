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

RESOURCES += qml.qrc \
    yolo.qrc

unix:!qnx:!android {
    CONFIG +=link_pkgconfig

packagesExist(opencv4) {
    PKGCONFIG += opencv4
}

}

android {
 QT += androidextras
 HEADERS += androidhelper.h
 SOURCES += androidhelper.cpp
}

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    QMAKE_CXXFLAGS += -mfpu=neon

    ANDROID_EXTRA_LIBS = \
        $$PWD/3rdparty/opencv-armv7/libopencv_core.so \
        $$PWD/3rdparty/opencv-armv7/libopencv_imgproc.so \
        $$PWD/3rdparty/opencv-armv7/libopencv_dnn.so \
        $$PWD/3rdparty/opencv-armv7/libtbb.so

    INCLUDEPATH = /home/milang/repos/opencv/buildandroidgcc/install/sdk/native/jni/include

    # Order is important, linker in old droids (4.2) are dumb
    LIBS += -L$$PWD/3rdparty/opencv-armv7/ -ltbb -lopencv_core -lopencv_imgproc -lopencv_dnn

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
