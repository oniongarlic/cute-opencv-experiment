import QtQuick 2.12

Rectangle {
    id: objectRect
    color: "transparent"
    border.width: 2
    border.color: activated ? "green" : "red"

    property int num: 0;
    property bool activated: false;
    property alias objectName: objectID.text;
    property alias objectColor: objectC.text;
    property double objectConfidence;

    x: _o.x
    y: _o.y
    width: _o.width
    height: _o.height

    //
    property rect o;
    property point c;

    // Mapped position & center of the current item
    property rect _o: previewImage.mapNormalizedRectToItem(o);
    property point _c: previewImage.mapNormalizedPointToItem(c);

    Text {
        id: objectID
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.left: parent.left
        anchors.top: parent.top
    }
    Text {
        id: objectC
        text: ""
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    Text {
        id: objectConfidence
        text: Math.round(confidence*100)+"%"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
