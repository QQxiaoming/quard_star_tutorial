import QtQuick 2.12

Item {
    width: 1024
    height: 800

    property Component a: Component {
        id: a
        Item {
            property list<QtObject> myList: [
                QtObject {
                    property bool enabled: true
                }
            ]
        }
    }
    Component {
        id: b
        Item {
            property list<QtObject> myList

            function test() {
                for (var i = 0; i < myList.length; ++i)
                    console.log(i, "==", myList[i].enabled)
            }
        }
    }
    property Item instance
    function doAssign(o) {
        instance = b.createObject(null, {myList: o.myList})
    }
    function use() {
        instance.test()
    }

}
