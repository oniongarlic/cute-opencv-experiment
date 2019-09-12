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
        console.debug("*** onCompleted")
        croppedImagePreview.updatePreview();
    }

    Component.onDestruction: {
        console.debug("*** onDestruction")
        imp.clear();
    }

    Popup {
        id: imageScale
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        ColumnLayout {
            Label {
                text: "Scale image"
            }
            TextInput {
                id: scaleWidth
                text: imp.getWidth()
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator{bottom: 1; top: imp.getWidth();}
            }
            TextInput {
                id: scaleHeight
                text: imp.getHeight()
                validator: IntValidator{bottom: 1; top: imp.getHeight();}
            }
            Switch {
                id: imageScaleAspect
                text: "Keep aspect ratio"
                checked: true
            }
            Switch {
                id: imageScaleSmooth
                text: "Smooth scaling"
                checked: true
            }

            RowLayout {
                Button {
                    text: "OK"
                    enabled: scaleHeight.acceptableInput && scaleWidth.acceptableInput
                    onClicked: {
                        imp.scale(scaleWidth.text, scaleHeight.text, imageScaleAspect.checked, imageScaleSmooth.checked)
                        imp.commit();
                        imageScale.close();
                    }
                }
                Button {
                    text: "Cancel"
                    onClicked: imageScale.close();

                }
            }
        }
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
                text: "Sc"
                onClicked: {
                    imageScale.open();
                }
            }

            ToolButton {
                text: "BW"
                onClicked: {
                    imp.gray();                    
                }
            }
            ToolButton {
                text: "Gam"
                enabled: controlGamma.visible==false
                onClicked: {
                    controlGamma.visible=true
                }
            }
            ToolButton {
                text: "M"
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

    Image {
        id: croppedImagePreview
        anchors.fill: parent
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
                    reset();
                    controlCrop.visible=false;
                }
            }
        }
    }

    ColumnLayout {
        id: imageEditorContainer
        anchors.fill: parent
        Layout.alignment: Qt.AlignTop

        RowLayout {
            id: controlMirror
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
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
            Layout.alignment: Qt.AlignTop
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
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Slider {
                    id: adjustBrightnessSlider
                    from: -1.0
                    stepSize: 0.01
                    to: 1.0
                    value: 0.0
                    live: false
                    onValueChanged: {
                        imp.adjustContrastBrightness(adjustContrastSlider.value, value);                        
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
                    }
                    Layout.fillWidth: true
                }
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
            Layout.alignment: Qt.AlignTop
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
                live: true
                onValueChanged: {
                    imp.rotate(value, !pressed)
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

        RowLayout {
            id: controlGamma
            Layout.alignment: Qt.AlignTop
            visible: false
            Button {
                text: "Cancel"
                onClicked: {
                    imp.reset()
                    controlGamma.visible=false;
                }
            }

            Slider {
                id: adjustGamma
                from: 0.0
                value: 1.0
                to: 2.0
                stepSize: 0.01;
                live: false
                onValueChanged: {
                    imp.gamma(value)                    
                }
                Layout.fillWidth: true
            }

            Button {
                text: "OK"
                onClicked: {
                    imp.commit();
                    controlGamma.visible=false;
                }
            }
        }


    }
}


