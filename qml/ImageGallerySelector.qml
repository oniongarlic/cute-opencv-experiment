import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: igs

    signal fileSelected(string src);

    function startSelector() {
        filesDialog.open();
    }

    FileDialog {
        id: filesDialog
        nameFilters: [ "*.jpg" ]
        title: qsTr("Select image file")
        onAccepted: {
            // XXX: Need to convert to string, otherwise sucka
            var f=""+fileUrl
            fileSelected(f);
        }
    }
}
