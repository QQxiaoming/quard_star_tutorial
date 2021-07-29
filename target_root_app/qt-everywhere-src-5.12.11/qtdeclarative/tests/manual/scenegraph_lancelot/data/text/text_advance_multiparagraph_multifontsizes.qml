import QtQuick 2.10

Item {
    width: 320
    height: 480

    property string firstWord: "<p style=\"font-size: 40pt\">One,</p><p>Two, "
    property string secondWord: "Three</p>"

    Text {
        id: referenceText
        text: firstWord + secondWord
        anchors.centerIn: parent
        font.italic: true
        font.pixelSize: 30
        textFormat: Text.RichText
    }


    Text {
        id: firstWordItemRichText
        anchors.left: referenceText.left
        anchors.top: referenceText.bottom
        text: firstWord
        font: referenceText.font
        textFormat: Text.RichText
    }

    Text {
        id: secondWordItemRichText
        anchors.left: firstWordItemRichText.left
        anchors.leftMargin: firstWordItemRichText.advance.width
        anchors.baseline: firstWordItemRichText.baseline
        anchors.baselineOffset: firstWordItemRichText.advance.height
        text: secondWord
        font: referenceText.font
        textFormat: Text.RichText
    }
}
