import QtQuick 2.12
import Qt.labs.platform 1.1

FileDialog {
    id: fd
    folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
    defaultSuffix: "jpg"
    fileMode:FileDialog.SaveFile
}
