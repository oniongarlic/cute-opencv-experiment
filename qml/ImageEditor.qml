import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

Page {
    id: imageEditor
    title: "Image editor"
    Keys.onReleased: {
        if (event.key === Qt.Key_Back) {
            console.log("*** Back button")
            event.accepted = true;
            rootStack.pop()
        }
    }

    Connections {
        target: imp
        onImageChanged: {
            console.debug("*** Image changed, updating preview")
            croppedImagePreview.updatePreview();
        }
    }

    Component.onCompleted: {
        croppedImagePreview.updatePreview();
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                id: backButton
                text: "Back"
                onClicked: {
                    rootStack.pop()
                }
            }
            ToolButton {
                text: "B/C"
                enabled: controlBrightnessContrast.visible==false
                onClicked: {
                    controlBrightnessContrast.reset();
                    controlBrightnessContrast.visible=true
                }
            }
            ToolButton {
                text: "Rot"
                enabled: controlRotate.visible==false
                onClicked: {
                    controlRotate.visible=true
                }
            }
            ToolButton {
                text: "Crop"
                onClicked: {
                    controlCrop.visible=!controlCrop.visible
                    controlCrop.reset();
                }
            }
            ToolButton {
                text: "Gray"
                onClicked: {
                    imp.gray();
                    croppedImagePreview.updatePreview();
                }
            }
            ToolButton {
                text: "Mirror"
                enabled: controlMirror.visible==false
                onClicked: {
                    controlMirror.visible=true;
                }
            }
        }
    }

    footer: ToolBar {
        Layout.fillWidth: true
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: "Reset"
                onClicked: {
                    imp.reset();
                }
            }
            ToolButton {
                text: "Save"
                onClicked: {
                    fsd.open();
                }
            }
        }
    }

    ColumnLayout {
        id: imageEditorContainer
        anchors.fill: parent

        RowLayout {
            id: controlMirror
            visible: false
            Button {
                text: "Cancel"
                onClicked: {
                    imp.reset();
                    controlMirror.visible=false;
                }
            }

            function updateMirror() {
                imp.mirror(flipHorizontal.checked, flipVertical.checked)
                croppedImagePreview.updatePreview();
            }

            Switch {
                id: flipHorizontal
                text: "Horizontal"
                onCheckedChanged: controlMirror.updateMirror();
            }
            Switch {
                id: flipVertical
                text: "Vertical"
                onCheckedChanged: controlMirror.updateMirror();
            }
            Button {
                text: "OK"
                onClicked: {
                    imp.commit();
                    controlMirror.visible=false;
                }
            }
        }

        RowLayout {
            id: controlBrightnessContrast
            visible: false

            function reset() {
                adjustBrightnessSlider.value=0.0;
                adjustContrastSlider.value=0.0;
            }

            Button {
                text: "Cancel"
                onClicked: {
                    imp.reset();
                    controlBrightnessContrast.visible=false;
                }
            }
            Slider {
                id: adjustBrightnessSlider
                from: -1.0
                stepSize: 0.01
                to: 1.0
                value: 0.0
                live: false
                onValueChanged: {
                    imp.adjustContrastBrightness(adjustContrastSlider.value, value);
                    croppedImagePreview.updatePreview();
                }
                Layout.fillWidth: true
            }
            Slider {
                id: adjustContrastSlider
                from: 0.0
                value: 0.0
                to: 100.0
                stepSize: 0.01;
                live: false
                onValueChanged: {
                    imp.adjustContrastBrightness(value, adjustBrightnessSlider.value);
                    croppedImagePreview.updatePreview();
                }
                Layout.fillWidth: true
            }
            Button {
                text: "OK"
                onClicked: {
                    imp.commit();
                    controlBrightnessContrast.visible=false;
                }
            }
        }

        RowLayout {
            id: controlRotate
            visible: false
            Button {
                text: "Cancel"
                onClicked: {
                    imp.reset()
                    controlRotate.visible=false;
                }
            }

            Slider {
                id: adjustRotate
                from: 0.0
                value: 0.0
                to: 360.0
                stepSize: 0.1;
                live: false
                onValueChanged: {
                    imp.rotate(value)
                    croppedImagePreview.updatePreview();
                }
                Layout.fillWidth: true
            }

            Button {
                text: "OK"
                onClicked: {
                    imp.commit();
                    controlRotate.visible=false;
                }
            }
        }

        Image {
            id: croppedImagePreview
            cache: false
            fillMode: Image.PreserveAspectFit
            source: ""
            Layout.fillWidth: true
            Layout.fillHeight: true

            function updatePreview() {
                croppedImagePreview.source=""
                croppedImagePreview.source="image://cute/preview"
            }

            // Contains the real image
            Item {
                id: pvr
                x: parent.height==parent.paintedHeight ? parent.width/2-width/2 : 0
                y: parent.width==parent.paintedWidth ? parent.height/2-height/2 : 0
                width: parent.paintedWidth
                height: parent.paintedHeight

                CropControl {
                    id: controlCrop
                    visible: false
                    onDoubleClicked: {
                        var r=mapNormalizedRect();
                        imp.cropNormalized(r)
                        imp.commit();
                        croppedImagePreview.updatePreview();
                        reset();
                    }
                }
            }
        }


    }
}


