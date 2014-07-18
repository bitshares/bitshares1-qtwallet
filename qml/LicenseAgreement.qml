import QtQuick 2.2
import QtQuick.Controls 1.1

ApplicationWindow {
    visible: true
    width: 1024
    height: 768
    title: qsTr("License Agreement")

    property bool accepted: false
    property alias licenseText: licenseTextDisplay.text

    Image {
        anchors.fill: parent
        source: "/images/background.jpg"
        fillMode: Image.PreserveAspectCrop
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: parent.width / 15
        radius: anchors.margins / 2
        color: "#88FFFFFF"
        clip: true

        Flickable {
            anchors {
                fill: parent
                leftMargin: parent.radius
                rightMargin: anchors.leftMargin
            }
            contentHeight: licenseArea.height
            contentWidth: width
            maximumFlickVelocity: 3000

            Rectangle {
                id: licenseArea
                color: "transparent"
                height: childrenRect.height
                width: parent.width

                Text {
                    id: licenseTextDisplay
                    textFormat: TextEdit.RichText
                    font.family: "helvetica"
                    font.pointSize: 14
                    font.wordSpacing: 5
                    width: parent.width
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                }

                Row {
                    anchors.top: licenseTextDisplay.bottom
                    anchors.topMargin: 30
                    anchors.horizontalCenter: parent.horizontalCenter
                    Button {
                        text: qsTr("Reject")
                        onClicked: {
                            accepted = false
                            Qt.quit()
                        }
                    }
                    Button {
                        text: qsTr("Accept")
                        onClicked: {
                            accepted = true
                            Qt.quit()
                        }
                    }
                }
            }
        }
    }
}
