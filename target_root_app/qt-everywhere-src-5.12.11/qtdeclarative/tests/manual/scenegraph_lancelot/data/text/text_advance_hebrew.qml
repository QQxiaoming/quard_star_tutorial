import QtQuick 2.10

Item {
    width: 320
    height: 480

    property string firstWord: "תורת רב־לשוני אנא "
    property string secondWord: "של"

    Text {
        id: referenceText
        text: firstWord + secondWord
        anchors.centerIn: parent
        font.italic: true
        font.pixelSize: 30
    }

    Text {
        id: firstWordItem
        anchors.right: referenceText.right
        anchors.top: referenceText.bottom
        text: firstWord
        font: referenceText.font
    }

    Text {
        id: secondWordItem
        anchors.right: firstWordItem.left
        anchors.baseline: firstWordItem.baseline
        anchors.baselineOffset: firstWordItem.advance.height
        text: secondWord
        font: referenceText.font
    }

    Text {
        id: firstWordItemRichText
        anchors.right: referenceText.right
        anchors.top: secondWordItem.bottom
        text: firstWord
        font: referenceText.font
        textFormat: Text.RichText
    }

    Text {
        id: secondWordItemRichText
        anchors.right: firstWordItemRichText.left
        anchors.baseline: firstWordItemRichText.baseline
        anchors.baselineOffset: firstWordItemRichText.advance.height
        text: secondWord
        font: referenceText.font
        textFormat: Text.RichText
    }

}
