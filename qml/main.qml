import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

import QtMultimedia 5.12

import Qt.labs.folderlistmodel 2.12

import org.tal 1.0

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    title: qsTr("QtOpenCV Hello World")
    header: mainToolbar

    property bool inProgress: false

    ToolBar {
        id: mainToolbar
        RowLayout {
            anchors.fill: parent
            Text {
                id: cnameText
                text: "n/a"
                Layout.preferredWidth: 100
            }
            Text {
                id: cgroupText
                text: "n/a"
                Layout.preferredWidth: 100
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
            ToolButton {
                text: "Image"
                enabled: !inProgress
                onClicked: {
                    filesDialog.startSelector();
                }
            }
            ToolButton {
                text: "Capture"
                enabled: !inProgress && camera.cameraState==Camera.ActiveState
                onClicked: {
                    camera.imageCapture.capture();
                }
            }
            ToolButton {
                text: "Camera"
                enabled: camera.cameraState!=Camera.ActiveState
                onClicked: {
                    camera.start();
                }
            }
            ToolButton {
                text: "Focus"
                enabled: camera.cameraState==Camera.ActiveState
                onClicked: {
                    camera.searchAndLock();
                }
            }
        }
    }

    function processImageFile(file) {
        previewImage.source=file
        previewImage.visible=true;
        // cd.processImageFile(file);
        od.processImageFile(file);
        imp.setImage(file);
    }

    ImageGallerySelector {
        id: filesDialog
        onFileSelected: {
            camera.stop();
            processImageFile(src)
        }
    }

    ColorDetector {
        id: cd

        onColorFound: {
            //cnameText.text=getColorName();
            cgroupText.text=getColorGroup();
            crect.color=getColorRGB();
        }

        onColorNotFound: {
            cnameText.text=cgroupText.text="";
        }
    }

    ListModel {
        id: detectedItems
    }

    ObjectDetector {
        id: od
        config: dnnConfig
        model: dnnWeights
        classes: dnnClasses

        confidence: 0.60

        Component.onCompleted: {
            loadClasses();
            start();
        }

        property rect o;
        property point c;

        onObjectDetected: {
            detectedItems.append({"cid": cid,
                                     "confidence": confidence,
                                     "name":od.getClassName(cid),
                                     "rgb": rgb,
                                     "ocolor": color,
                                     "center": center,
                                     "centerX": center.x,
                                     "centerY": center.y,
                                     "ox": rect.x,
                                     "oy": rect.y,
                                     "owidth": rect.width,
                                     "oheight": rect.height
                                 })
        }

        onNoObjectDetected: {
            console.debug("Nothing found!")
            objectID.text=""
            objectConfidence.text="-:---%"
            previewImage.visible=false;
        }

        onDetectionEnded: {
            inProgress=false;
            previewImage.visible=true;
            if (found>0)
                detectedItemsList.currentIndex=0;
            else
                detectedItemsList.currentIndex=-1;
        }

        onDetectionStarted: {
            detectedItems.clear();
            inProgress=true;
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
                console.debug(preview)
                console.debug(camera.imageCapture.capturedImagePath)
                previewImage.source=preview;
                previewImage.visible=true;
                camera.stop();
            }
            onCaptureFailed: {
                console.debug("Capture failed")
            }
            onImageSaved: {
                console.debug("Image saved: "+path)
                //cd.processImageFile(path);
                od.processImageFile(path);
            }
        }

        flash.mode: Camera.FlashAuto

        Component.onCompleted: {

        }
    }

    OpenCVVideoFilter {
        id: cvfilter

        onColorFound: {
            console.debug(cgroup+":"+cname)
            cnameText.text=cname;
            cgroupText.text=cgroup;
            crect.color=chex;
        }
    }

    FolderListModel {
        id: fileModel
        folder: "file://"+imagePath
        showDotAndDotDot: true
        showDirsFirst: true

        nameFilters: ["*.jpg"]
    }

    RowLayout {
        id: mainRow
        anchors.fill: parent

        VideoOutput {
            id: vc
            Layout.fillHeight: true
            Layout.fillWidth: true
            source: camera
            autoOrientation: true
            fillMode: Image.PreserveAspectFit

            filters: [ cvfilter ]

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
                onDoubleClicked: {
                    camera.imageCapture.capture();
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

            Image {
                id: previewImage
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                Item {
                    id: pvr
                    x: parent.height==parent.paintedHeight ? parent.width/2-width/2 : 0
                    y: parent.width==parent.paintedWidth ? parent.height/2-height/2 : 0
                    width: parent.paintedWidth
                    height: parent.paintedHeight

                    Rectangle {
                        id: objectCenter
                        color: "transparent"
                        x: detectedItemsList.c.x-2
                        y: detectedItemsList.c.y-2
                        width: 4
                        height: 4
                        border.width: 2
                        border.color: "green"
                        visible: x>0 && y>0
                    }

                    Repeater {
                        model: detectedItems
                        delegate: objectMarkerDelegate
                    }

                    Component {
                        id: objectMarkerDelegate
                        ObjectMarker {
                            o: Qt.rect(ox, oy, owidth, oheight);
                            objectConfidence: confidence
                            objectName: name
                            objectColor: ocolor
                            activated: index==detectedItemsList.currentIndex
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: previewImage.visible=false;
                }

                function mapNormalizedRectToItem(r) {
                    console.debug(r)
                    return Qt.rect(r.x*pvr.width, r.y*pvr.height, r.width*pvr.width, r.height*pvr.height)
                }

                function mapNormalizedPointToItem(r) {
                    //console.debug(r)
                    return Qt.point(r.x*pvr.width, r.y*pvr.height)
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                width: 128
                height: 128
                visible: inProgress
                running: inProgress
            }
        }

        ListView {
            id: detectedItemsList
            Layout.fillHeight: true
            Layout.minimumWidth: 160
            Layout.maximumWidth: 240
            clip: true

            model: detectedItems
            delegate: detectedItemDelegate
            highlight: Rectangle { color: "lightsteelblue"; radius: 2 }

            Component {
                id: detectedItemDelegate
                Item {
                    width: parent.width
                    height: r.height
                    Row {
                        id: r
                        spacing: 8
                        width: parent.width
                        Text { text: name }
                        Text { text: Math.round(confidence*100)+"%" }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            detectedItemsList.currentIndex = index
                        }
                        onDoubleClicked: {
                            detectedItemsList.currentIndex = index
                            var r=Qt.rect(ox, oy, owidth, oheight);
                            console.debug(r);
                            if (!imp.isEmpty()) {
                                imp.cropNormalized(r);
                                croppedImagePreview.source=""
                                croppedImagePreview.source="image://cute/preview"
                                croppedImagePopup.open();
                                imp.save("/tmp/cropped-image.jpg");
                            } else {
                                console.debug("*** Image is NULL!");
                            }
                        }
                    }
                }
            }
        }

        ListView {
            id: fileList
            model: fileModel
            delegate: fileDelegate

            Layout.fillHeight: true
            Layout.minimumWidth: 160
            Layout.maximumWidth: 240
            clip: true

            highlight: Rectangle { color: "lightsteelblue"; radius: 2 }

            function processItem(index) {
                fileList.currentIndex=index
                var f=fileModel.get(currentIndex, "fileURL")
                if (fileModel.isFolder(currentIndex))
                    fileModel.folder=f;
                else
                    processImageFile(f);
            }

            Component {
                id: fileDelegate
                Item {
                    width: parent.width
                    height: r.height
                    Row {
                        id: r
                        spacing: 8
                        width: parent.width
                        Text { text: fileName }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            fileList.processItem(index);
                        }
                        onDoubleClicked: {

                        }
                    }
                }
            }

        }
    }

    Popup {
        id: croppedImagePopup
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        width: parent.width/1.5
        height: parent.height/1.5
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        ColumnLayout {
            anchors.fill: parent
            Image {
                id: croppedImagePreview
                cache: false
                fillMode: Image.PreserveAspectFit
                source: ""
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            ToolBar {
                Layout.fillWidth: true
                ToolButton {
                    text: "Save"
                    onClicked: {
                        imp.save("cropped.jpg")
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        // camera.start();
    }
}
