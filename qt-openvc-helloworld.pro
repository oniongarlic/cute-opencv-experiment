QT += quick multimedia widgets
CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS
SOURCES += \
    cuteimageprovider.cpp \
    main.cpp \
    ocvobjectcolordetector.cpp \
    objectdetector.cpp \
    cuteopencv.cpp \
    cuteopencvbase.cpp \
    objectdetectorworker.cpp \
    ovvideofilter.cpp \
    ovvideofilterrunnable.cpp

RESOURCES += qml.qrc

DEFINES += YOLO_CUSTOM
# DEFINES += YOLO_OLD
# RESOURCES += yolo.qrc

unix:!qnx:!android {
    CONFIG +=link_pkgconfig
    packagesExist(opencv4) {
        PKGCONFIG += opencv4
    }
    contains(DEFINES,YOLO_CUSTOM) {
        DEFINES+= YOLO_WEIGHTS=\\\"/data/AI/tk/tk320_last.weights\\\"
        DEFINES+= YOLO_CFG=\\\"/data/AI//tk/tk320test.cfg\\\"
        DEFINES+= YOLO_NAMES=\\\"/data/AI/tk//tk.names\\\"
    }
    contains(DEFINES,YOLO_OLD) {
        DEFINES+= YOLO_WEIGHTS=\\\"/data/repos/yolo-data/yolo/obj_last.weights\\\"
        DEFINES+= YOLO_CFG=\\\"/data/repos/yolo-data/yolo/obj-detect.cfg\\\"
        DEFINES+= YOLO_NAMES=\\\"/data/repos/yolo-data/yolo/obj.names\\\"
    }
}

# Android extras
android {
    QT += androidextras
    HEADERS += androidhelper.h
    SOURCES += androidhelper.cpp
}

# 64-bit Android
contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/3rdparty/opencv-armv8/libopencv_core.so \
        $$PWD/3rdparty/opencv-armv8/libopencv_imgproc.so \
        $$PWD/3rdparty/opencv-armv8/libopencv_dnn.so \
        $$PWD/3rdparty/opencv-armv8/libtbb.so

    INCLUDEPATH = /opt/Android/opencv-4.5.1/android-armv8/native/jni/include

    LIBS += -L$$PWD/3rdparty/opencv-armv8/ -ltbb -lopencv_core -lopencv_imgproc -lopencv_dnn
}

# 32-bit Android
contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    QMAKE_CXXFLAGS += -mfpu=neon

    ANDROID_EXTRA_LIBS = \
        $$PWD/3rdparty/opencv-armv7/libopencv_core.so \
        $$PWD/3rdparty/opencv-armv7/libopencv_imgproc.so \
        $$PWD/3rdparty/opencv-armv7/libopencv_dnn.so \
        $$PWD/3rdparty/opencv-armv7/libtbb.so

    INCLUDEPATH = /opt/Android/opencv-4.5.1/android-armv7/native/jni/include

    # Order is important, linker in old droids (4.2) are dumb
    LIBS += -L$$PWD/3rdparty/opencv-armv7/ -ltbb -lopencv_core -lopencv_imgproc -lopencv_dnn

}

android {

contains(DEFINES,YOLO_CUSTOM) {
    DEFINES+= YOLO_WEIGHTS=\\\"/yolov2-custom/yolov2-custom.weights\\\"
    DEFINES+= YOLO_CFG=\\\"/yolov2-custom/obj-detect.cfg\\\"
    DEFINES+= YOLO_NAMES=\\\"/yolov2-custom/obj.names\\\"
}

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
    cuteimageprovider.h \
    ocvobjectcolordetector.h \
    objectdetector.h \
    cuteopencv.h \
    cuteopencvbase.h \
    objectdetectorworker.h \
    ovvideofilter.h \
    ovvideofilterrunnable.h

DISTFILES += \
    android/AndroidManifest.xml \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
