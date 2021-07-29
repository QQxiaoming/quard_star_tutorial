pragma Singleton
import QtQml 2.0

QtObject {
    property string name
    property string category
    property string sound
    property int size: Animal.SizeSmall

    enum SizeType {
        SizeSmall,
        SizeMedium,
        SizeLarge
    }
}
