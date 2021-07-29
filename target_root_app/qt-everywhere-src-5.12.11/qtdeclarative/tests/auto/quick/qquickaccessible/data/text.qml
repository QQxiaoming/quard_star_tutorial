import QtQuick 2.0

Item {
    width: 400
    height: 400
    Accessible.name: "root"
    Accessible.role: Accessible.Client

    Text {
        x: 100
        y: 20
        width: 200
        height: 50
        text : "Hello Accessibility"

        // Setting any value of the attached property
        // makes an item accessible.
        Accessible.name: text
    }

    Text {
        x: 100
        y: 40
        width: 100
        height: 40
        text : "Hello 2"
        Accessible.role: Accessible.StaticText
        Accessible.name: "The Hello 2 accessible text"
        Accessible.description: "A text description"
    }

    TextInput {
        x: 100
        y: 80
        width: 200
        height: 40
        text: "A text input"
        Accessible.role: Accessible.EditableText
    }

    TextEdit {
        x: 100
        y: 120
        width: 200
        height: 100
        text: "A multi-line text edit\nTesting Accessibility."
        Accessible.role: Accessible.EditableText
    }

    Text {
        x: 100
        y: 160
        width: 100
        height: 40
        text : "Hello 3"
        Accessible.name: text
        Accessible.description: "description"
    }

}
