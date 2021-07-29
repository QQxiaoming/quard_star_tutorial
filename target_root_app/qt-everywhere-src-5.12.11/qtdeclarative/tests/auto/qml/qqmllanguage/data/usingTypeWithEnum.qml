import QtQuick 2.0
import org.qtproject.MixedModule 1.0

QtObject {
    property int enumValue: TypeWithEnum.EnumValue2
    property int enumValue2: -1
    property int scopedEnumValue: TypeWithEnum.MyEnum.EnumValue3
    property int enumValueFromSingleton: { var x = SingletonType.EnumValue; return x; }
    Component.onCompleted: enumValue2 = TypeWithEnum.EnumValue1

    property int duplicatedEnumValueFromSingleton: SingletonType.AnotherEnumValue
    property int scopedEnumValueFromSingleton1: SingletonType.EnumInSingleton.AnotherEnumValue
    property int scopedEnumValueFromSingleton2: SingletonType.AnotherEnumInSingleton.AnotherEnumValue
    property int scopedEnumValueFromSingleton3: { var x = SingletonType.AnotherEnumInSingleton.AnotherEnumValue; return x; }
}
