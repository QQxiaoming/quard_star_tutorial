/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/private/qqmlglobal_p.h>

#include <QtQuickTemplates2/private/qquickabstractbutton_p.h>
#include <QtQuickTemplates2/private/qquickaction_p.h>
#include <QtQuickTemplates2/private/qquickactiongroup_p.h>
#include <QtQuickTemplates2/private/qquickapplicationwindow_p.h>
#include <QtQuickTemplates2/private/qquickbusyindicator_p.h>
#include <QtQuickTemplates2/private/qquickbutton_p.h>
#include <QtQuickTemplates2/private/qquickbuttongroup_p.h>
#include <QtQuickTemplates2/private/qquickcheckbox_p.h>
#include <QtQuickTemplates2/private/qquickcheckdelegate_p.h>
#include <QtQuickTemplates2/private/qquickcombobox_p.h>
#include <QtQuickTemplates2/private/qquickcontrol_p.h>
#include <QtQuickTemplates2/private/qquickcontainer_p.h>
#include <QtQuickTemplates2/private/qquickdelaybutton_p.h>
#include <QtQuickTemplates2/private/qquickdial_p.h>
#include <QtQuickTemplates2/private/qquickdialog_p.h>
#include <QtQuickTemplates2/private/qquickdialogbuttonbox_p.h>
#include <QtQuickTemplates2/private/qquickdrawer_p.h>
#include <QtQuickTemplates2/private/qquickframe_p.h>
#include <QtQuickTemplates2/private/qquickgroupbox_p.h>
#include <QtQuickTemplates2/private/qquickicon_p.h>
#include <QtQuickTemplates2/private/qquickitemdelegate_p.h>
#include <QtQuickTemplates2/private/qquicklabel_p.h>
#include <QtQuickTemplates2/private/qquickmenu_p.h>
#include <QtQuickTemplates2/private/qquickmenubar_p.h>
#include <QtQuickTemplates2/private/qquickmenubaritem_p.h>
#include <QtQuickTemplates2/private/qquickmenuitem_p.h>
#include <QtQuickTemplates2/private/qquickmenuseparator_p.h>
#include <QtQuickTemplates2/private/qquickoverlay_p.h>
#include <QtQuickTemplates2/private/qquickpage_p.h>
#include <QtQuickTemplates2/private/qquickpageindicator_p.h>
#include <QtQuickTemplates2/private/qquickpaletteprovider_p.h>
#include <QtQuickTemplates2/private/qquickpane_p.h>
#include <QtQuickTemplates2/private/qquickpopup_p.h>
#include <QtQuickTemplates2/private/qquickpopupanchors_p.h>
#include <QtQuickTemplates2/private/qquickprogressbar_p.h>
#include <QtQuickTemplates2/private/qquickradiobutton_p.h>
#include <QtQuickTemplates2/private/qquickradiodelegate_p.h>
#include <QtQuickTemplates2/private/qquickrangeslider_p.h>
#include <QtQuickTemplates2/private/qquickroundbutton_p.h>
#include <QtQuickTemplates2/private/qquickscrollbar_p.h>
#include <QtQuickTemplates2/private/qquickscrollindicator_p.h>
#include <QtQuickTemplates2/private/qquickscrollview_p.h>
#include <QtQuickTemplates2/private/qquickshortcutcontext_p_p.h>
#include <QtQuickTemplates2/private/qquickslider_p.h>
#include <QtQuickTemplates2/private/qquickspinbox_p.h>
#include <QtQuickTemplates2/private/qquickstackview_p.h>
#include <QtQuickTemplates2/private/qquickswipe_p.h>
#include <QtQuickTemplates2/private/qquickswipedelegate_p.h>
#include <QtQuickTemplates2/private/qquickswipeview_p.h>
#include <QtQuickTemplates2/private/qquickswitch_p.h>
#include <QtQuickTemplates2/private/qquickswitchdelegate_p.h>
#include <QtQuickTemplates2/private/qquicktabbar_p.h>
#include <QtQuickTemplates2/private/qquicktabbutton_p.h>
#include <QtQuickTemplates2/private/qquicktextarea_p.h>
#include <QtQuickTemplates2/private/qquicktextfield_p.h>
#include <QtQuickTemplates2/private/qquicktoolbar_p.h>
#include <QtQuickTemplates2/private/qquicktoolbutton_p.h>
#include <QtQuickTemplates2/private/qquicktoolseparator_p.h>
#include <QtQuickTemplates2/private/qquicktooltip_p.h>
#if QT_CONFIG(quick_listview) && QT_CONFIG(quick_pathview)
#include <QtQuickTemplates2/private/qquicktumbler_p.h>
#endif

#if QT_CONFIG(shortcut)
// qtdeclarative/src/quick/util/qquickshortcut.cpp
typedef bool (*ShortcutContextMatcher)(QObject *, Qt::ShortcutContext);
extern ShortcutContextMatcher qt_quick_shortcut_context_matcher();
extern void qt_quick_set_shortcut_context_matcher(ShortcutContextMatcher matcher);
#endif

QT_BEGIN_NAMESPACE

static void initProviders()
{
    QQuickPaletteProvider::init();
}

static void cleanupProviders()
{
    QQuickPaletteProvider::cleanup();
}

class QtQuickTemplates2Plugin: public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QtQuickTemplates2Plugin(QObject *parent = nullptr);
    ~QtQuickTemplates2Plugin();

    void registerTypes(const char *uri) override;

private:
    bool registered;
#if QT_CONFIG(shortcut)
    ShortcutContextMatcher originalContextMatcher;
#endif
};

QtQuickTemplates2Plugin::QtQuickTemplates2Plugin(QObject *parent)
    : QQmlExtensionPlugin(parent), registered(false)
{
#if QT_CONFIG(shortcut)
    originalContextMatcher = qt_quick_shortcut_context_matcher();
    qt_quick_set_shortcut_context_matcher(QQuickShortcutContext::matcher);
#endif
}

QtQuickTemplates2Plugin::~QtQuickTemplates2Plugin()
{
    if (registered)
        cleanupProviders();

#if QT_CONFIG(shortcut)
    qt_quick_set_shortcut_context_matcher(originalContextMatcher);
#endif
}

void QtQuickTemplates2Plugin::registerTypes(const char *uri)
{
    registered = true;
    initProviders();

    // Register the latest version, even if there are no new types or new revisions for existing types yet.
    // Before Qt 5.12, we would do the following:
    //
    // qmlRegisterModule(uri, 2, QT_VERSION_MINOR - 7); // Qt 5.7->2.0, 5.8->2.1, 5.9->2.2...
    //
    // However, we want to align with the rest of Qt Quick which uses Qt's minor version.
    qmlRegisterModule(uri, 2, QT_VERSION_MINOR);

    // QtQuick.Templates 2.0 (originally introduced in Qt 5.7)
    qmlRegisterType<QQuickAbstractButton>(uri, 2, 0, "AbstractButton");
    qmlRegisterType<QQuickApplicationWindow>(uri, 2, 0, "ApplicationWindow");
    qmlRegisterType<QQuickApplicationWindowAttached>();
    qmlRegisterType<QQuickBusyIndicator>(uri, 2, 0, "BusyIndicator");
    qmlRegisterType<QQuickButton>(uri, 2, 0, "Button");
    qmlRegisterType<QQuickButtonGroup>(uri, 2, 0, "ButtonGroup");
    qmlRegisterType<QQuickButtonGroupAttached>();
    qmlRegisterType<QQuickCheckBox>(uri, 2, 0, "CheckBox");
    qmlRegisterType<QQuickCheckDelegate>(uri, 2, 0, "CheckDelegate");
    qmlRegisterType<QQuickComboBox>(uri, 2, 0, "ComboBox");
    qmlRegisterType<QQuickContainer>(uri, 2, 0, "Container");
    qmlRegisterType<QQuickControl>(uri, 2, 0, "Control");
    qmlRegisterType<QQuickDial>(uri, 2, 0, "Dial");
    qmlRegisterType<QQuickDrawer>(uri, 2, 0, "Drawer");
    qmlRegisterType<QQuickFrame>(uri, 2, 0, "Frame");
    qmlRegisterType<QQuickGroupBox>(uri, 2, 0, "GroupBox");
    qmlRegisterType<QQuickItemDelegate>(uri, 2, 0, "ItemDelegate");
    qmlRegisterType<QQuickLabel>(uri, 2, 0, "Label");
    qmlRegisterType<QQuickMenu>(uri, 2, 0, "Menu");
    qmlRegisterType<QQuickMenuItem>(uri, 2, 0, "MenuItem");
    qmlRegisterType<QQuickOverlay>();
    qmlRegisterType<QQuickPage>(uri, 2, 0, "Page");
    qmlRegisterType<QQuickPageIndicator>(uri, 2, 0, "PageIndicator");
    qmlRegisterType<QQuickPane>(uri, 2, 0, "Pane");
    qmlRegisterType<QQuickPopup>(uri, 2, 0, "Popup");
    qmlRegisterType<QQuickProgressBar>(uri, 2, 0, "ProgressBar");
    qmlRegisterType<QQuickRadioButton>(uri, 2, 0, "RadioButton");
    qmlRegisterType<QQuickRadioDelegate>(uri, 2, 0, "RadioDelegate");
    qmlRegisterType<QQuickRangeSlider>(uri, 2, 0, "RangeSlider");
    qmlRegisterType<QQuickRangeSliderNode>();
    qmlRegisterType<QQuickScrollBar>(uri, 2, 0, "ScrollBar");
    qmlRegisterType<QQuickScrollBarAttached>();
    qmlRegisterType<QQuickScrollIndicator>(uri, 2, 0, "ScrollIndicator");
    qmlRegisterType<QQuickScrollIndicatorAttached>();
    qmlRegisterType<QQuickSlider>(uri, 2, 0, "Slider");
    qmlRegisterType<QQuickSpinBox>(uri, 2, 0, "SpinBox");
    qmlRegisterType<QQuickSpinButton>();
    qmlRegisterType<QQuickStackView>(uri, 2, 0, "StackView");
    qmlRegisterType<QQuickStackViewAttached>();
    qmlRegisterType<QQuickSwipe>();
    qmlRegisterType<QQuickSwipeDelegate>(uri, 2, 0, "SwipeDelegate");
    qmlRegisterType<QQuickSwipeView>(uri, 2, 0, "SwipeView");
    qmlRegisterType<QQuickSwipeViewAttached>();
    qmlRegisterType<QQuickSwitch>(uri, 2, 0, "Switch");
    qmlRegisterType<QQuickSwitchDelegate>(uri, 2, 0, "SwitchDelegate");
    qmlRegisterType<QQuickTabBar>(uri, 2, 0, "TabBar");
    qmlRegisterType<QQuickTabButton>(uri, 2, 0, "TabButton");
    qmlRegisterType<QQuickTextArea>(uri, 2, 0, "TextArea");
    qmlRegisterType<QQuickTextAreaAttached>();
    qmlRegisterType<QQuickTextField>(uri, 2, 0, "TextField");
    qmlRegisterType<QQuickToolBar>(uri, 2, 0, "ToolBar");
    qmlRegisterType<QQuickToolButton>(uri, 2, 0, "ToolButton");
    qmlRegisterType<QQuickToolTip>(uri, 2, 0, "ToolTip");
    qmlRegisterType<QQuickToolTipAttached>();
#if QT_CONFIG(quick_listview) && QT_CONFIG(quick_pathview)
    qmlRegisterType<QQuickTumblerAttached>();
    qmlRegisterType<QQuickTumbler>(uri, 2, 0, "Tumbler");
#endif

    // NOTE: register the latest revisions of all template/control base classes to
    // make revisioned properties available to their subclasses (synced with Qt 5.7)
    qmlRegisterRevision<QQuickItem, 7>(uri, 2, 0);
    qmlRegisterRevision<QQuickText, 6>(uri, 2, 0);
    qmlRegisterRevision<QQuickTextInput, 7>(uri, 2, 0);
    qmlRegisterRevision<QQuickTextEdit, 7>(uri, 2, 0);
    qmlRegisterRevision<QQuickWindow, 2>(uri, 2, 0);
    qmlRegisterRevision<QWindow, 3>(uri, 2, 0);

    // QtQuick.Templates 2.1 (new types and revisions in Qt 5.8)
    qmlRegisterType<QQuickButtonGroup, 1>(uri, 2, 1, "ButtonGroup");
    qmlRegisterType<QQuickComboBox, 1>(uri, 2, 1, "ComboBox");
    qmlRegisterType<QQuickContainer, 1>(uri, 2, 1, "Container");
    qmlRegisterType<QQuickDialog>(uri, 2, 1, "Dialog");
    qmlRegisterType<QQuickDialogButtonBox>(uri, 2, 1, "DialogButtonBox");
    qmlRegisterType<QQuickDialogButtonBoxAttached>();
    qmlRegisterType<QQuickMenuSeparator>(uri, 2, 1, "MenuSeparator");
    qmlRegisterType<QQuickPage, 1>(uri, 2, 1, "Page");
    qmlRegisterType<QQuickPopup, 1>(uri, 2, 1, "Popup");
    qmlRegisterType<QQuickRangeSlider, 1>(uri, 2, 1, "RangeSlider");
    qmlRegisterType<QQuickRoundButton>(uri, 2, 1, "RoundButton");
    qmlRegisterType<QQuickSlider, 1>(uri, 2, 1, "Slider");
    qmlRegisterType<QQuickSpinBox, 1>(uri, 2, 1, "SpinBox");
    qmlRegisterType<QQuickStackView, 1>(uri, 2, 1, "StackView");
    qmlRegisterType<QQuickSwipeDelegate, 1>(uri, 2, 1, "SwipeDelegate");
    qmlRegisterType<QQuickSwipeView, 1>(uri, 2, 1, "SwipeView");
    qmlRegisterType<QQuickTextArea, 1>(uri, 2, 1, "TextArea");
    qmlRegisterType<QQuickTextField, 1>(uri, 2, 1, "TextField");
    qmlRegisterType<QQuickToolSeparator>(uri, 2, 1, "ToolSeparator");
#if QT_CONFIG(quick_listview) && QT_CONFIG(quick_pathview)
    qmlRegisterType<QQuickTumbler, 1>(uri, 2, 1, "Tumbler");
#endif

    // QtQuick.Templates 2.2 (new types and revisions in Qt 5.9)
    qmlRegisterType<QQuickAbstractButton, 2>(uri, 2, 2, "AbstractButton");
    qmlRegisterType<QQuickComboBox, 2>(uri, 2, 2, "ComboBox");
    qmlRegisterType<QQuickDelayButton>(uri, 2, 2, "DelayButton");
    qmlRegisterType<QQuickDial, 2>(uri, 2, 2, "Dial");
    qmlRegisterType<QQuickDrawer, 2>(uri, 2, 2, "Drawer");
    qmlRegisterType<QQuickRangeSlider, 2>(uri, 2, 2, "RangeSlider");
    qmlRegisterType<QQuickScrollBar, 2>(uri, 2, 2, "ScrollBar");
    qmlRegisterType<QQuickScrollView>(uri, 2, 2, "ScrollView");
    qmlRegisterType<QQuickSlider, 2>(uri, 2, 2, "Slider");
    qmlRegisterType<QQuickSpinBox, 2>(uri, 2, 2, "SpinBox");
    qmlRegisterType<QQuickSwipeDelegate, 2>(uri, 2, 2, "SwipeDelegate");
    qmlRegisterType<QQuickSwipeView, 2>(uri, 2, 2, "SwipeView");
    qmlRegisterType<QQuickTabBar, 2>(uri, 2, 2, "TabBar");
#if QT_CONFIG(quick_listview) && QT_CONFIG(quick_pathview)
    qmlRegisterType<QQuickTumbler, 2>(uri, 2, 2, "Tumbler");
#endif

    // NOTE: register the latest revisions of all template/control base classes to
    // make revisioned properties available to their subclasses (synced with Qt 5.9)
    qmlRegisterRevision<QQuickText, 9>(uri, 2, 2);
    qmlRegisterRevision<QQuickTextInput, 9>(uri, 2, 2);
    qmlRegisterRevision<QQuickWindowQmlImpl, 2>(uri, 2, 2);

    // QtQuick.Templates 2.3 (new types and revisions in Qt 5.10)
    qmlRegisterType<QQuickAbstractButton, 3>(uri, 2, 3, "AbstractButton");
    qmlRegisterType<QQuickAction>(uri, 2, 3, "Action");
    qmlRegisterType<QQuickActionGroup>(uri, 2, 3, "ActionGroup");
    qmlRegisterType<QQuickApplicationWindow, 3>(uri, 2, 3, "ApplicationWindow");
    qmlRegisterType<QQuickButtonGroup, 3>(uri, 2, 3, "ButtonGroup");
    qmlRegisterType<QQuickControl, 3>(uri, 2, 3, "Control");
    qmlRegisterType<QQuickContainer, 3>(uri, 2, 3, "Container");
    qmlRegisterType<QQuickDialog, 3>(uri, 2, 3, "Dialog");
    qmlRegisterType<QQuickDialogButtonBox, 3>(uri, 2, 3, "DialogButtonBox");
    qmlRegisterType<QQuickIcon>();
    qRegisterMetaType<QQuickIcon>();
    qmlRegisterType<QQuickLabel, 3>(uri, 2, 3, "Label");
    qmlRegisterType<QQuickMenu, 3>(uri, 2, 3, "Menu");
    qmlRegisterType<QQuickMenuBar>(uri, 2, 3, "MenuBar");
    qmlRegisterType<QQuickMenuBarItem>(uri, 2, 3, "MenuBarItem");
    qmlRegisterType<QQuickMenuItem, 3>(uri, 2, 3, "MenuItem");
    qmlRegisterUncreatableType<QQuickOverlay>(uri, 2, 3, "Overlay", QStringLiteral("Overlay is only available as an attached property."));
    qmlRegisterType<QQuickOverlayAttached>();
    qmlRegisterType<QQuickPopup, 3>(uri, 2, 3, "Popup");
    qmlRegisterType<QQuickRangeSlider, 3>(uri, 2, 3, "RangeSlider");
    qmlRegisterType<QQuickScrollBar, 3>(uri, 2, 3, "ScrollBar");
    qmlRegisterType<QQuickScrollIndicator, 3>(uri, 2, 3, "ScrollIndicator");
    qmlRegisterType<QQuickSlider, 3>(uri, 2, 3, "Slider");
    qmlRegisterType<QQuickSpinBox, 3>(uri, 2, 3, "SpinBox");
    qmlRegisterType<QQuickTextArea, 3>(uri, 2, 3, "TextArea");
    qmlRegisterType<QQuickTextField, 3>(uri, 2, 3, "TextField");

    // NOTE: register the latest revisions of all template/control base classes to
    // make revisioned properties available to their subclasses (synced with Qt 5.10)
    qmlRegisterRevision<QQuickText, 10>(uri, 2, 3);
    qmlRegisterRevision<QQuickTextEdit, 10>(uri, 2, 3);

    // QtQuick.Templates 2.4 (new types and revisions in Qt 5.11)
    qmlRegisterType<QQuickAbstractButton, 4>(uri, 2, 4, "AbstractButton");
    qmlRegisterType<QQuickButtonGroup, 4>(uri, 2, 4, "ButtonGroup");
    qmlRegisterType<QQuickCheckBox, 4>(uri, 2, 4, "CheckBox");
    qmlRegisterType<QQuickCheckDelegate, 4>(uri, 2, 4, "CheckDelegate");
    qmlRegisterType<QQuickScrollBar, 4>(uri, 2, 4, "ScrollBar");
    qmlRegisterType<QQuickScrollIndicator, 4>(uri, 2, 4, "ScrollIndicator");
    qmlRegisterType<QQuickSpinBox, 4>(uri, 2, 4, "SpinBox");

    // QtQuick.Templates 2.5 (new types and revisions in Qt 5.12)
    qmlRegisterType<QQuickAbstractButton, 5>(uri, 2, 5, "AbstractButton");
    qmlRegisterType<QQuickDialogButtonBox, 5>(uri, 2, 5, "DialogButtonBox");
    qmlRegisterType<QQuickComboBox, 5>(uri, 2, 5, "ComboBox");
    qmlRegisterType<QQuickControl, 5>(uri, 2, 5, "Control");
    qmlRegisterType<QQuickContainer, 5>(uri, 2, 5, "Container");
    qmlRegisterType<QQuickDial, 5>(uri, 2, 5, "Dial");
    qmlRegisterType<QQuickDialog, 5>(uri, 2, 5, "Dialog");
    qmlRegisterType<QQuickGroupBox, 5>(uri, 2, 5, "GroupBox");
    qmlRegisterType<QQuickLabel, 5>(uri, 2, 5, "Label");
    qmlRegisterType<QQuickPage, 5>(uri, 2, 5, "Page");
    qmlRegisterType<QQuickPopup, 5>(uri, 2, 5, "Popup");
    qmlRegisterType<QQuickPopupAnchors>();
    qmlRegisterType<QQuickRangeSlider, 5>(uri, 2, 5, "RangeSlider");
    qmlRegisterType<QQuickSlider, 5>(uri, 2, 5, "Slider");
    qmlRegisterType<QQuickSpinBox, 5>(uri, 2, 5, "SpinBox");
    qmlRegisterType<QQuickTextArea, 5>(uri, 2, 5, "TextArea");
    qmlRegisterType<QQuickTextField, 5>(uri, 2, 5, "TextField");
    qmlRegisterType<QQuickToolTip, 5>(uri, 2, 5, "ToolTip");
}

QT_END_NAMESPACE

#include "qtquicktemplates2plugin.moc"
