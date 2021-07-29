import QtQuick 2.0

QtObject {
    enum MyEnum {
        EnumValue1,
        EnumValue2,
        EnumValue3
    }

    enum MyOtherEnum {
        OtherEnumValue1 = 24,
        OtherEnumValue2,
        OtherEnumValue3 = 24,
        OtherEnumValue4,
        OtherEnumValue5 = 1
    }

    property int enumValue: TypeWithEnum.EnumValue2
    property int enumValue2
    property int scopedEnumValue: TypeWithEnum.MyEnum.EnumValue2
    Component.onCompleted: enumValue2 = TypeWithEnum.EnumValue3

    property int otherEnumValue1: TypeWithEnum.OtherEnumValue1
    property int otherEnumValue2: TypeWithEnum.OtherEnumValue2
    property int otherEnumValue3: TypeWithEnum.OtherEnumValue3
    property int otherEnumValue4: TypeWithEnum.OtherEnumValue4
    property int otherEnumValue5: TypeWithEnum.OtherEnumValue5
}
