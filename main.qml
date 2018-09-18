import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtMultimedia 5.5

import org.tal 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Scroll")
    header: mainToolbar

    ToolBar {
        id: mainToolbar
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: "Capture"
                onClicked: {
                     camera.imageCapture.capture();
                }
            }
            Text {
                id: cname
                text: "n/a"
            }
            Text {
                id: cgroup
                text: "n/a"
            }
            Rectangle {
                id: crect
                Layout.fillHeight: true
                width: 40;
            }
        }        
    }

    ColorDetector {
        id: cd

        onColorFound: {
            cname.text=getColorName();
            cgroup.text=getColorGroup();
            crect.color=getColorRGB();
        }

        onColorNotFound: {
            cname.text=cgroup.text="";
        }
    }

    Camera {
        id: camera
        //deviceId: "/dev/video0" // XXX
        captureMode: Camera.CaptureStillImage;
        onErrorStringChanged: console.debug("Error: "+errorString)
        onCameraStateChanged: {
            console.debug("State: "+cameraState)
        }
        onCameraStatusChanged: console.debug("Status: "+cameraStatus)

        focus {
            focusMode: Camera.FocusContinuous
            focusPointMode: Camera.FocusPointCenter
        }

        metaData.subject: "OpenCV Test"

        imageCapture {
            onImageCaptured: {
                console.debug("Image captured!")
                console.debug(camera.imageCapture.capturedImagePath)
                //previewImage.source=preview
            }
            onCaptureFailed: {
                console.debug("Capture failed")
            }
            onImageSaved: {
               console.debug("Image saved: "+path)
                cd.processImageFile(path);
            }
        }

        flash.mode: Camera.FlashAuto

        Component.onCompleted: {

        }
    }

    VideoOutput {
        anchors.fill: parent
        source: camera

        MouseArea {
            anchors.fill: parent
            onPressAndHold: {
                console.debug(mouse.x)
                console.debug(mouse.y)

                console.debug(mouse.x/width)
                console.debug(mouse.y/height)

                // cd.setRoi();
            }
            onClicked: {
                camera.searchAndLock();
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: 33
            height: 33
            border.width: 4
            border.color: "green"
            color: "transparent"
        }

    }

    Component.onCompleted: {
        camera.start();
    }
}
