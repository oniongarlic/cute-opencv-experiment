import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import QtMultimedia

import Qt.labs.folderlistmodel

import org.tal
import org.tal.decklink

ApplicationWindow {
    id: root
    visible: true
    width: 1024
    height: 768
    title: qsTr("QtOpenCV Hello World")
    header: mainToolbar

    property bool inProgress: false

    menuBar: MenuBar {
        Menu {
            title: "Profile"
            MenuItem {
                text: "Clear"
                onClicked: dls.setProfile(0)
            }
            MenuItem {
                text: "1 1-Full"
                onClicked: dls.setProfile(1)
            }
            MenuItem {
                text: "2 1-Half"
                onClicked: dls.setProfile(2)
            }
            MenuItem {
                text: "3 2-Full"
                onClicked: dls.setProfile(3)
            }
            MenuItem {
                text: "4 2-Half"
                onClicked: dls.setProfile(4)
            }
            MenuItem {
                text: "5 4-Half"
                onClicked: dls.setProfile(5)
            }
        }
        Menu {
            title: "Capture"
            MenuItem {
                id: captureAudio
                text: "Audio"
                checkable: true
                checked: dlsrc.audio
            }
        }
    }

    ToolBar {
        id: mainToolbar
        RowLayout {
            anchors.fill: parent
            Label {
                text: "Input"
            }
            ComboBox {
                id: inputCombo
                textRole: "name"
                valueRole: "mode"
                onActivated: {
                    console.debug("Input mode", currentValue)
                    dlsrc.setMode(currentValue)
                }
            }

            Label {
                text: "Output"
            }
            ComboBox {
                id: outputCombo
                textRole: "name"
                valueRole: "mode"
                onActivated: {
                    console.debug("Output mode", currentValue)
                    dls.setMode(currentValue)
                }
            }

            CheckBox {
                id: sdi1
                text: "1"
            }
            CheckBox {
                id: sdi2
                text: "2"
            }
            CheckBox {
                id: sdi3
                text: "3"
            }
            CheckBox {
                id: sdi4
                text: "4"
            }

            Slider {
                from: 0
                to: 255
                stepSize: 1
                wheelEnabled: true
                onMoved: {
                    dls.keyerLevel(value)
                }
            }

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
            Text {
                text: dlsrc.frameCount
            }
            Rectangle {
                id: crect
                Layout.fillHeight: true
                width: 40;
            }
            ToolSeparator {

            }
            ToolButton {
                text: "F"
                onClicked: videoFilesDialog.open()
            }
            ToolButton {
                text: "P"
                onClicked: mediaPlayer.play()
            }
            ToolButton {
                text: "S"
                onClicked: mediaPlayer.stop()
            }
        }
    }

    function processImageFile(file) {
        previewImage.source=file
        previewImage.visible=true;
        // cd.processImageFile(file);
        od.processImageFile(file);
        //imp.setImage(file);

        odface.processImageFile(file)

        var it=od.getImage();
        console.debug(it)

        imp.setImage(it);

        //dls.displayImage(it)

        if (sdi1.checked)
            dls.displayImage(file)

        if (sdi2.checked)
            dls2.displayImage(file)
    }

    MediaPlayer {
        id: mediaPlayer
        videoOutput: vc
    }

    FileDialog {
        id: videoFilesDialog
        nameFilters: [ "*.mp4", "*.mov" ]
        title: qsTr("Select video file")
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        onAccepted: {
            mediaPlayer.source=selectedFile;
        }
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
        id: odface
        config: ""
        model: "/data/repos/yolov8-face-landmarks-opencv-dnn/weights/yolov8n-face.onnx"
        //classes: dnnClasses
        width: 640
        height: 640
        confidence: 0.50

        onObjectDetected: {
            console.debug("FACE-FOUND")
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
        onDetectionStarted: {
            console.debug("FACE-START")
        }
        onDetectionEnded: {
            console.debug("FACE-END")
        }

        Component.onCompleted: {
            start();
        }
    }

    ObjectDetector {
        id: od
        config: dnnConfig
        model: dnnWeights
        classes: dnnClasses
        width: 416
        height: 416
        confidence: 0.50

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
            //objectConfidence.text="-:---%"
            //previewImage.visible=false;
        }

        onDetectionEnded: {
            inProgress=false;
            previewImage.visible=true;
            if (found>0) {
                detectedItemsList.currentIndex=0;
            } else {
                detectedItemsList.currentIndex=-1;
                //messageDialog.show("Nothing found", "No objects where detected")
                console.debug("No objects where detected")
            }
        }

        onDetectionStarted: {
            detectedItems.clear();
            inProgress=true;
        }
    }

    MediaDevices {
        id: mediaDevices
    }

    CaptureSession {
        id: videoCaptureSession
        camera: camera
        videoOutput: vc
    }

    Camera {
        id: camera
        cameraDevice: mediaDevices.defaultVideoInput
        focusMode: Camera.FocusModeAuto
        onErrorOccurred: console.debug("CameraError: "+errorString)
        onActiveChanged: console.debug("CameraActive:"+active)
        Component.onCompleted: {

        }
    }

    DeckLink {
        id: dl
        onDevicesChanged: {
            console.debug("*** We have decklink devices", devices)
        }
        onHaveDeckLinkChanged: {
            console.debug("*** We have decklink support")


            // Sinks
            dls.setOutput(0) // Duo 2 SDI-1 (1+2)
            var d0=dl.getDeviceProperties(0)
            outputCombo.model=d0["outputModes"]            
            outputCombo.currentIndex=outputCombo.indexOfValue(dls.getMode());

            dls2.setOutput(1) // Duo 2 SDI-3
            dls2.setMode(DeckLinkSource.VideoHD1080p30)

            // Sources            
            dlsrc.setInput(3)
            var d3=dl.getDeviceProperties(3)
            inputCombo.model=d3["inputModes"]
            inputCombo.currentIndex=inputCombo.indexOfValue(dlsrc.getMode());
        }
    }

    DeckLinkSink {
        id: dls
        decklink: dl
        objectName: "Sink 1"
    }
    DeckLinkSink {
        id: dls2
        decklink: dl
        objectName: "Sink 2"
        videoSink: vc.videoSink
    }

    DeckLinkSource {
        id: dlsrc
        decklink: dl
        objectName: "Source 4"
        audio: captureAudio.checked        
        videoSink: vodeck.videoSink
        onInvalidSignal: {
            console.debug("*** Invalid input signal")
        }
        onValidSignal: {
            console.debug("*** Got valid input signal")
        }
        onFrameGrabbed: {
            imp.setImage(getImage());
            // just some random string to get it set
            previewImage.source="image://cute/frame_"+frameCount
        }
    }

    OpenCVVideoFilter {
        id: cvfilter
        // videoSink: vc.videoSink

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
        nameFilters: ["*.jpg", "*.png", "*.jpeg"]
    }

    StackView {
        id: rootStack
        initialItem: mainView
        anchors.fill: parent
    }

    Page {
        id: mainView

        footer: ToolBar {
            RowLayout {
                anchors.fill: parent
                ToolButton {
                    text: "Load Image"
                    enabled: !inProgress
                    onClicked: {
                        filesDialog.startSelector();
                    }
                }
                ToolButton {
                    text: "Grab"
                    enabled: dlsrc.streaming
                    onClicked: {
                        dlsrc.grabFrame();
                    }
                }
                ToolButton {
                    text: "Capture"
                    enabled: !inProgress && camera.active
                    onClicked: {
                        videoInput.imageCapture.capture();
                    }
                }
                ToolButton {
                    text: "Camera"
                    enabled: !camera.active
                    onClicked: {
                        camera.start();
                    }
                }
                ToolButton {
                    text: "Edit"
                    onClicked: {
                        imp.setImage(previewImage.source);
                        if (!imp.isEmpty()) {
                            rootStack.push(imageEditor);
                        } else {
                            messageDialog.show("Operation failed", "Failed to set image for editing")
                        }
                    }
                }

                ToolSeparator {

                }

                ToolButton {
                    text: "E2"
                    onClicked: {
                        dls2.enableOutput();
                    }
                }
                ToolButton {
                    text: "D2"
                    onClicked: {
                        dls2.disableOutput();
                    }
                }
                ToolButton {
                    text: ""
                    onClicked: {
                        dls2.displayImage()
                    }
                }

                ToolSeparator {

                }

                ToolButton {
                    text: "E1"
                    onClicked: {
                        dls.enableOutput();
                    }
                }
                ToolButton {
                    text: "D1"
                    onClicked: {
                        dls.disableOutput();
                    }
                }
                ToolButton {
                    text: "Clear"
                    onClicked: {
                        dls.clearBuffer();
                    }
                }

                ToolButton {
                    text: "K-ON"
                    onClicked: {
                        dls.setKeyer(true)
                    }
                }
                ToolButton {
                    text: "K-Off"
                    onClicked: {
                        dls.setKeyer(false)
                    }
                }
                ToolButton {
                    text: "KR-Up"
                    onClicked: {
                        dls.keyerRampUp(60)
                    }
                }
                ToolButton {
                    text: "KR-Down"
                    onClicked: {
                        dls.keyerRampDown(60)
                    }
                }
                ToolSeparator {

                }

                ToolButton {
                    text: "I-E"
                    enabled: !dlsrc.streaming
                    onClicked: {
                        dlsrc.enableInput()
                    }
                }
                ToolButton {
                    text: "I-D"
                    enabled: dlsrc.streaming
                    onClicked: {
                        dlsrc.disableInput()
                    }
                }
            }
        }

        ColumnLayout {
            id: mainRow
            anchors.fill: parent
            spacing: 4

            VideoOutput {
                id: vc
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                Layout.minimumHeight: 256
                Layout.minimumWidth: parent.width
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
                            x: detectedItemsList.c ? detectedItemsList.c.x-2 : 0
                            y: detectedItemsList.c ? detectedItemsList.c.y-2 : 0
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
                        enabled: !inProgress
                        onClicked: {
                            previewImage.visible=false;
                            detectedItems.clear();
                        }
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

            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumHeight: root.height/5
                Layout.maximumHeight: root.height/3

                VideoOutput {
                    id: vodeck                    
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumHeight: 128
                    Layout.maximumHeight: 256
                    Layout.minimumWidth: 128
                    Layout.maximumWidth: 384
                    fillMode: Image.PreserveAspectFit

                    Rectangle {
                        anchors.fill: parent
                        border.color: "red"
                        border.width: 1
                        color: "transparent"
                    }
                }

                ListView {
                    id: detectedItemsList
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumWidth: 128
                    clip: true
                    //visible: detectedItems.count>0
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
                                    imp.setImage(od.getImage());
                                    console.debug("isEmpty")
                                    if (!imp.isEmpty()) {
                                        imp.cropNormalized(r);
                                        imp.commit();
                                        rootStack.push(imageEditor)
                                    } else {
                                        console.debug("*** Image is NULL!");
                                        messageDialog.show("Operation failed", "Failed to set image for editing")
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
                    Layout.fillWidth: true
                    Layout.minimumWidth: 128
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
                            width: ListView.view.width
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
        }

    }

    MessageDialog {
        id: messageDialog
        title: ""
        text: ""

        onAccepted: {
            messageDialog.close();
        }

        function show(title, message) {
            messageDialog.title=title;
            messageDialog.text=message;
            messageDialog.open();
        }
    }

    Component {
        id: imageEditor
        ImageEditor {

        }
    }

    FileSaveDialog {
        id: fsd
        onAccepted: {
            imp.commit();
            console.debug("SaveAs: "+file)
            if (!imp.save(file))
                messageDialog.show("Save As", "Image save failed")
        }
    }

    Component.onCompleted: {
        // camera.start();
    }
}
