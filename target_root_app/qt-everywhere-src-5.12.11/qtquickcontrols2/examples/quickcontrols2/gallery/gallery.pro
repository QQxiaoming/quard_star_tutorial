TEMPLATE = app
TARGET = gallery
QT += quick quickcontrols2

SOURCES += \
    gallery.cpp

RESOURCES += \
    gallery.qml \
    pages/BusyIndicatorPage.qml \
    pages/ButtonPage.qml \
    pages/CheckBoxPage.qml \
    pages/ComboBoxPage.qml \
    pages/DelayButtonPage.qml \
    pages/DelegatePage.qml \
    pages/DialogPage.qml \
    pages/DialPage.qml \
    pages/FramePage.qml \
    pages/GroupBoxPage.qml \
    pages/PageIndicatorPage.qml \
    pages/ProgressBarPage.qml \
    pages/RadioButtonPage.qml \
    pages/RangeSliderPage.qml \
    pages/ScrollablePage.qml \
    pages/ScrollBarPage.qml \
    pages/ScrollIndicatorPage.qml \
    pages/SliderPage.qml \
    pages/SpinBoxPage.qml \
    pages/StackViewPage.qml \
    pages/SwipeViewPage.qml \
    pages/SwitchPage.qml \
    pages/TabBarPage.qml \
    pages/TextAreaPage.qml \
    pages/TextFieldPage.qml \
    pages/ToolTipPage.qml \
    pages/TumblerPage.qml \
    qtquickcontrols2.conf \
    icons/gallery/index.theme \
    icons/gallery/20x20/back.png \
    icons/gallery/20x20/drawer.png \
    icons/gallery/20x20/menu.png \
    icons/gallery/20x20@2/back.png \
    icons/gallery/20x20@2/drawer.png \
    icons/gallery/20x20@2/menu.png \
    icons/gallery/20x20@3/back.png \
    icons/gallery/20x20@3/drawer.png \
    icons/gallery/20x20@3/menu.png \
    icons/gallery/20x20@4/back.png \
    icons/gallery/20x20@4/drawer.png \
    icons/gallery/20x20@4/menu.png \
    images/arrow.png \
    images/arrow@2x.png \
    images/arrow@3x.png \
    images/arrow@4x.png \
    images/arrows.png \
    images/arrows@2x.png \
    images/arrows@3x.png \
    images/arrows@4x.png \
    images/qt-logo.png \
    images/qt-logo@2x.png \
    images/qt-logo@3x.png \
    images/qt-logo@4x.png

target.path = $$[QT_INSTALL_EXAMPLES]/quickcontrols2/gallery
INSTALLS += target
