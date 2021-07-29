import QtQuick 2.12

QtObject {
    Component.onCompleted: {
        // create getter
        var getFoo = function(o) { return o.foo; }

        // create two diffrent shapes for x,y
        var x = { foo:1 , bar:2 }
        var y = { bar:2 , foo:1 }

        // initialize inline cache with getFoo
        getFoo(x);
        getFoo(y);

        // hit getter lookup with string "crash"
        getFoo('crash');
    }
}

