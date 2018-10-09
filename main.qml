import QtQuick 2.9
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2
import QtMultimedia 5.5

import org.tal 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("OpenCV QtQuick Test")
    header: mainToolbar

    ToolBar {
        id: mainToolbar
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: "Open..."
                onClicked: {
                    filesDialog.open();
                }
            }
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

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            Text {
                id: objectID
            }
            Text {
                id: objectConfidence
                text: "n/a"
            }
        }
    }

    FileDialog {
        id: filesDialog
        nameFilters: [ "*.jpg" ]
        title: qsTr("Select image file")
        selectExisting: true
        selectFolder: false
        selectMultiple: false
        onAccepted: {
            var f=""+fileUrl
            od.processImageFile(f)
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

    ObjectDetector {
        id: od

        Component.onCompleted: {
            loadModel();
        }

        onObjectDetected: {
            objectID.text="CID: "+cid;
            objectConfidence.text=confidence;
            console.debug(x+" : "+y)

            var vx=((x-w/2)*vc.width);
            var vy=((y-h/2)*vc.height);

            objectRect.x=vx;
            objectRect.y=vy;
            objectRect.width=w*vc.width;
            objectRect.height=h*vc.height;
        }

        onNoObjectDetected: {
            objectID.text="<Nothing found>"
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
                od.processImageFile(path);
            }
        }

        flash.mode: Camera.FlashAuto

        Component.onCompleted: {

        }
    }

    VideoOutput {
        id: vc
        anchors.fill: parent
        source: camera
        autoOrientation: true
        fillMode: Image.PreserveAspectFit

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

        Rectangle {
            id: objectRect
            color: "transparent"
            width: 5
            height: 5
            border.width: 2
            border.color: "white"
        }

    }

    Component.onCompleted: {
        camera.start();
    }
}
