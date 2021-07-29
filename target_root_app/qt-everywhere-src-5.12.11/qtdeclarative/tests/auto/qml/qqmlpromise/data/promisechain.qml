import QtQml 2.0

QtObject {
    property int x: 0
    id: root;
    Component.onCompleted: {
        new Promise((res) => {
            res(1)
        })
        .then((data) => {
            console.debug(data)
            return new Promise((res) => {res(2)});
        })
        .then((data) => {
            console.debug(data)
            return new Promise((res) => {res(3)});
        })
        .then((data) => {
            console.debug(data);
            root.x = 42;
        });
    }

}
