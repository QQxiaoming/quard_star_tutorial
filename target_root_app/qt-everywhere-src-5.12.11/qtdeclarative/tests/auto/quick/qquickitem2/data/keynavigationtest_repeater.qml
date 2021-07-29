import QtQuick 2.0

Grid {
    property var textModel: ["1", "2", "3", "4", "5"]
    columns: 5
    width: 50*textModel.length

    Repeater {
        id: repeater
        model: textModel.length
        Rectangle {
            width: 50
            height: 50
            color: focus ? "red" : "lightgrey"
            focus: index  == 2
            Text {
                id: t
                text: textModel[index]
            }
            KeyNavigation.left:  repeater.itemAt(index - 1)
            KeyNavigation.right: repeater.itemAt(index + 1)
        }
    }

    function verify() {
        for (var i = 0; i < repeater.count; i++) {
            var item = repeater.itemAt(i);
            var prev = repeater.itemAt(i - 1);
            var next = repeater.itemAt(i + 1);
            if (item.KeyNavigation.left != prev || item.KeyNavigation.right != next)
                return false;
        }

        return true;
    }
}

