import QtQuick 2.10

Item {
    width: 320
    height: 480

    property string firstWord: "One,\nTwo, "
    property string secondWord: "Three"

    Text {
        id: referenceText
        text: firstWord + secondWord
        anchors.centerIn: parent
        font.italic: true
        font.pixelSize: 30
    }

    Text {
        id: firstWordItem
        anchors.left: referenceText.left
        anchors.top: referenceText.bottom
        text: firstWord
        font: referenceText.font
    }

    Text {
        id: secondWordItem
        anchors.left: firstWordItem.left
        anchors.leftMargin: firstWordItem.advance.width
        anchors.baseline: firstWordItem.baseline
        anchors.baselineOffset: firstWordItem.advance.height
        text: secondWord
        font: referenceText.font
    }

    Text {
        id: firstWordItemRichText
        anchors.left: referenceText.left
        anchors.top: secondWordItem.bottom
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
