import QtQml 2.0
import Test.Singletons 1.0

QtObject {
    property Component singletonAccessor : Component {
        QtObject {
            property var singletonHolder;
            property int result: singletonHolder.testVar
        }
    }

    property int firstLookup: singletonAccessor.createObject(this, { singletonHolder: CppSingleton1 }).result;
    property int secondLookup: singletonAccessor.createObject(this, { singletonHolder: CppSingleton2 }).result;
}
