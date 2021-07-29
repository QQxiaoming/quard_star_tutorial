import QtQuick 2.0
pragma Singleton

Item {
    enum EnumInSingleton {
        EnumValue = 42,
        AnotherEnumValue
    }

    enum AnotherEnumInSingleton {
        AnotherEnumValue = 2
    }
}
