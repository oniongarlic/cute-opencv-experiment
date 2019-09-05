import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2

Popup {
    id: imageEditor
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: parent.width/1.1
    height: parent.height/1.1
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    Connections {
        target: imp
        onImageChanged: {
            console.debug("*** Image was modified")
            croppedImagePreview.updatePreview();
        }
    }

    ColumnLayout {
        anchors.fill: parent
        ToolBar {
            RowLayout {
                ToolButton {
                    text: "Brightness/Contrast"
                    enabled: controlBrightnessContrast.visible==false
                    onClicked: {
                        controlBrightnessContrast.visible=true
                    }
                }
                ToolButton {
                    text: "Rotate"
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
            }
        }

        RowLayout {
            id: controlBrightnessContrast
            visible: false
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
                    imp.adjustContrastBrightness(adjustContrastSlider.value,value);
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
                    imp.adjustContrastBrightness(value,adjustBrightnessSlider.value);
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

        ToolBar {
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                ToolButton {
                    text: "Gray"
                    onClicked: {
                        imp.gray();
                        croppedImagePreview.updatePreview();
                    }
                }
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
    }

    onClosed: {
        croppedImagePreview.source=""
    }

    onOpened: {
        croppedImagePreview.updatePreview()
    }

}
