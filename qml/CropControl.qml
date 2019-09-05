import QtQuick 2.12

Item {
    id: pImage
    anchors.fill: parent

    property int dragMargin: 32

    function mapNormalizedRect() {
        return Qt.rect(cropRect.x/pImage.width,
                       cropRect.y/pImage.height,
                       cropRect.width/pImage.width,
                       cropRect.height/pImage.height)
    }

    signal doubleClicked();

    function reset() {
        cropRect.x=dragMargin
        cropRect.y=dragMargin
        cropRect.width=pImage.width-dragMargin*2
        cropRect.height=pImage.height-dragMargin*2
    }

    Rectangle {
        id: cropRect
        x: dragMargin
        y: dragMargin
        width: pImage.width-dragMargin*2
        height: pImage.height-dragMargin*2
        color: "#4c57d4e1"
        border.color: "#8c57d4e1"
        border.width: dragMargin/8

        Rectangle {
            id: cropCenterRectangle
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            color: "white"
            opacity: cropCenterArea.pressed ? 0.2 : 0.0
        }

        MouseArea {
            id: cropCenterArea
            anchors.fill: cropCenterRectangle
            anchors.margins: 8
            drag.target: cropRect
            drag.minimumX: 0
            drag.maximumX: pImage.width-cropRect.width

            drag.minimumY: 0
            drag.maximumY: pImage.height-cropRect.height

            onPressAndHold: {
                cropRect.x=0;
                cropRect.y=0;
                cropRect.width=pImage.width
                cropRect.height=pImage.height
            }

            onDoubleClicked: {
                var r=pImage.mapNormalizedRect();
                console.debug(r)

                pImage.doubleClicked();
            }
        }
    }

    Rectangle {
        id: topLeftDrag
        anchors.horizontalCenter: cropRect.left
        anchors.verticalCenter: cropRect.top
        color: "#4c57d4e1"
        opacity: 0.7
        width: dragMargin
        height: dragMargin
        radius: dragMargin/2

        MouseArea {
            id: tLD
            anchors.fill: topLeftDrag
            drag.target: parent
            onPositionChanged: {
                if(drag.active){
                    cropRect.x = cropRect.x + mouseX

                    if (cropRect.x <= 0)
                        cropRect.x = 0
                    else
                        cropRect.width = cropRect.width - mouseX

                    if (cropRect.width < dragMargin)
                        cropRect.width = dragMargin

                    cropRect.y = cropRect.y + mouseY

                    if (cropRect.y <= 0)
                        cropRect.y = 0
                    else
                        cropRect.height = cropRect.height - mouseY

                    if (cropRect.height < dragMargin)
                        cropRect.height = dragMargin
                }
            }
        }
    }

    // BottomRightDrag
    Rectangle {
        id: bottomRightDrag
        anchors.horizontalCenter: cropRect.right
        anchors.verticalCenter: cropRect.bottom
        color: "#4c57d4e1"
        opacity: 0.7
        width: dragMargin
        height: dragMargin
        radius: dragMargin/2

        MouseArea {
            id: bRD
            anchors.fill: bottomRightDrag
            drag.target: parent

            onPositionChanged: {
                if(drag.active){
                    cropRect.width = cropRect.width + mouseX
                    cropRect.height = cropRect.height + mouseY

                    if (cropRect.width < dragMargin)
                        cropRect.width = dragMargin
                    else if (cropRect.width > pImage.width-cropRect.x)
                        cropRect.width=pImage.width-cropRect.x

                    if (cropRect.height < dragMargin)
                        cropRect.height = dragMargin
                    else if (cropRect.height > pImage.height-cropRect.y)
                        cropRect.height=pImage.height-cropRect.y
                }
            }
        }
    }
}
