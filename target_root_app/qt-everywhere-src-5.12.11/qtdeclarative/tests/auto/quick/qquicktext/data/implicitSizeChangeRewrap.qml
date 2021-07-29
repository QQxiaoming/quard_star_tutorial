import QtQuick 2.0
import QtQuick.Layouts 1.0

Item
{
  id : _rootItem
  width : 200
  height : 1000
        ColumnLayout
        {
            id : _textContainer
            anchors.centerIn: parent
            Layout.maximumWidth: (_rootItem.width - 40) // to have some space left / right
            Text
            {
                id : text
                objectName: "text"
                font.italic: true
                textFormat: Text.RichText
                horizontalAlignment :   Text.AlignHCenter
                verticalAlignment :     Text.AlignVCenter
                wrapMode: Text.Wrap
                Layout.maximumWidth: (_rootItem.width - 60)
                Component.onCompleted: text.text = "This is a too long text for the interface with a stupid path also too long -> /home/long/long/long/to/force/it/to/need/to/wrap This is a too long text for the interface with a stupid path also too long -> /home/long/long/long/to/force/it/to/need/to/wrap This is a too long text for the interface with a stupid path also too long -> /home/long/long/long/to/force/it/to/need/to/wrap This is a too long text for the interface with a stupid path also too long -> /home/long/long/long/to/force/it/to/need/to/wrap"
            }
    }
}
