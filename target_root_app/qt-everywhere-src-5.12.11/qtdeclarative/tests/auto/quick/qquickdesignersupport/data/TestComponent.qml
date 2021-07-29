import QtQuick 2.0

Item {
    width: 100
    height: 62

    // Add a dummy dependency to parent.x to ensure that the
    // binding stays for the test.
    x: parent.x + Math.max(0, 200) - parent.x
}

