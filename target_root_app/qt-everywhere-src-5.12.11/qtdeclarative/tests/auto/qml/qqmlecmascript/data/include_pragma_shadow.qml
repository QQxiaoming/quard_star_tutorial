import QtQuick 2.0
import "include_pragma_shadow.js" as Shadowed
import "include_pragma_shadow.js" as Other

Item {
    property bool result

    Component.onCompleted: {
        result = false;
        var global = (function(){return this})()
        if (Shadowed.Shadowed === 2 && Other.Shadowed === 2 && global.Shadowed === 1)
            result = true;
    }
}

