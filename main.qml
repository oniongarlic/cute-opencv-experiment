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

        property rect o;
        property point c;

        onObjectDetected: {
            objectID.text=od.getClassName(cid);
            objectConfidence.text=cid+":"+Math.round(confidence*100)+"%";

            od.c=vc.mapNormalizedPointToItem(center);
            od.o=vc.mapNormalizedRectToItem(rect);
        }

        onNoObjectDetected: {
            console.debug("Nothing found!")
            objectID.text=""
            objectConfidence.text="-:---%"
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

        metaData.subject: "QtOpenCVHelloWorld"

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
            width: od.o.width
            height: od.o.height
            x: od.o.x
            y: od.o.y
            border.width: 2
            border.color: "red"
        }

        Rectangle {
            id: objectCenter
            color: "transparent"
            x: od.c.x-2
            y: od.c.y-2
            width: 4
            height: 4
            border.width: 2
            border.color: "green"
            visible: x>0 && y>0
        }

    }

    Component.onCompleted: {
        camera.start();
    }
}
