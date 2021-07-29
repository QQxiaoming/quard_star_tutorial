/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
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

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qpluginloader.h>
#include <QtCore/private/qfileselector_p.h>
#include <QtQml/qqmlfile.h>
#include <QtQml/private/qqmldirparser_p.h>
#include <QtQuickControls2/qquickstyle.h>
#include <QtQuickControls2/private/qquickchecklabel_p.h>
#include <QtQuickControls2/private/qquickcolor_p.h>
#include <QtQuickControls2/private/qquickcolorimage_p.h>
#include <QtQuickControls2/private/qquickiconimage_p.h>
#include <QtQuickControls2/private/qquickmnemoniclabel_p.h>
#include <QtQuickControls2/private/qquickpaddedrectangle_p.h>
#include <QtQuickControls2/private/qquickplaceholdertext_p.h>
#include <QtQuickControls2/private/qquickiconlabel_p.h>
#include <QtQuickControls2/private/qquickstyle_p.h>
#include <QtQuickControls2/private/qquickstyleplugin_p.h>
#if QT_CONFIG(quick_listview) && QT_CONFIG(quick_pathview)
#include <QtQuickControls2/private/qquicktumblerview_p.h>
#endif
#include <QtQuickTemplates2/private/qquickoverlay_p.h>
#include <QtQuickControls2/private/qquickclippedtext_p.h>
#include <QtQuickControls2/private/qquickitemgroup_p.h>
#include <QtQuickTemplates2/private/qquicktheme_p_p.h>

#include "qquickdefaultbusyindicator_p.h"
#include "qquickdefaultdial_p.h"
#include "qquickdefaultprogressbar_p.h"
#include "qquickdefaultstyle_p.h"
#include "qquickdefaulttheme_p.h"

QT_BEGIN_NAMESPACE

class QtQuickControls2Plugin: public QQuickStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QtQuickControls2Plugin(QObject *parent = nullptr);
    ~QtQuickControls2Plugin();

    void registerTypes(const char *uri) override;

    QString name() const override;
    void initializeTheme(QQuickTheme *theme) override;

private:
    QList<QQuickStylePlugin *> loadStylePlugins();
    QQuickTheme *createTheme(const QString &name);
};

QtQuickControls2Plugin::QtQuickControls2Plugin(QObject *parent) : QQuickStylePlugin(parent)
{
}

QtQuickControls2Plugin::~QtQuickControls2Plugin()
{
    QQuickStylePrivate::reset();
}

static bool isDefaultStyle(const QString &style)
{
    return style.isEmpty() || style.compare(QStringLiteral("Default"), Qt::CaseInsensitive) == 0;
}

void QtQuickControls2Plugin::registerTypes(const char *uri)
{
    QQuickStylePrivate::init(baseUrl());

    const QString style = QQuickStyle::name();
    if (!style.isEmpty())
        QFileSelectorPrivate::addStatics(QStringList() << style.toLower());

    QQuickTheme *theme = createTheme(style.isEmpty() ? name() : style);
    if (isDefaultStyle(style))
        initializeTheme(theme);

    // load the style's plugins to get access to its resources and initialize the theme
    QList<QQuickStylePlugin *> stylePlugins = loadStylePlugins();
    for (QQuickStylePlugin *stylePlugin : stylePlugins)
        stylePlugin->initializeTheme(theme);
    qDeleteAll(stylePlugins);

    // Register the latest version, even if there are no new types or new revisions for existing types yet.
    // Before Qt 5.12, we would do the following:
    //
    // qmlRegisterModule(uri, 2, QT_VERSION_MINOR - 7); // Qt 5.7->2.0, 5.8->2.1, 5.9->2.2...
    //
    // However, we want to align with the rest of Qt Quick which uses Qt's minor version.
    qmlRegisterModule(uri, 2, QT_VERSION_MINOR);

    // QtQuick.Controls 2.0 (originally introduced in Qt 5.7)
    qmlRegisterType(resolvedUrl(QStringLiteral("AbstractButton.qml")), uri, 2, 0, "AbstractButton");
    qmlRegisterType(resolvedUrl(QStringLiteral("ApplicationWindow.qml")), uri, 2, 0, "ApplicationWindow");
    qmlRegisterType(resolvedUrl(QStringLiteral("BusyIndicator.qml")), uri, 2, 0, "BusyIndicator");
    qmlRegisterType(resolvedUrl(QStringLiteral("Button.qml")), uri, 2, 0, "Button");
    qmlRegisterType(resolvedUrl(QStringLiteral("ButtonGroup.qml")), uri, 2, 0, "ButtonGroup");
    qmlRegisterType(resolvedUrl(QStringLiteral("CheckBox.qml")), uri, 2, 0, "CheckBox");
    qmlRegisterType(resolvedUrl(QStringLiteral("CheckDelegate.qml")), uri, 2, 0, "CheckDelegate");
    qmlRegisterType(resolvedUrl(QStringLiteral("ComboBox.qml")), uri, 2, 0, "ComboBox");
    qmlRegisterType(resolvedUrl(QStringLiteral("Container.qml")), uri, 2, 0, "Container");
    qmlRegisterType(resolvedUrl(QStringLiteral("Control.qml")), uri, 2, 0, "Control");
    qmlRegisterType(resolvedUrl(QStringLiteral("Dial.qml")), uri, 2, 0, "Dial");
    qmlRegisterType(resolvedUrl(QStringLiteral("Drawer.qml")), uri, 2, 0, "Drawer");
    qmlRegisterType(resolvedUrl(QStringLiteral("Frame.qml")), uri, 2, 0, "Frame");
    qmlRegisterType(resolvedUrl(QStringLiteral("GroupBox.qml")), uri, 2, 0, "GroupBox");
    qmlRegisterType(resolvedUrl(QStringLiteral("ItemDelegate.qml")), uri, 2, 0, "ItemDelegate");
    qmlRegisterType(resolvedUrl(QStringLiteral("Label.qml")), uri, 2, 0, "Label");
    qmlRegisterType(resolvedUrl(QStringLiteral("Menu.qml")), uri, 2, 0, "Menu");
    qmlRegisterType(resolvedUrl(QStringLiteral("MenuItem.qml")), uri, 2, 0, "MenuItem");
    qmlRegisterType(resolvedUrl(QStringLiteral("Page.qml")), uri, 2, 0, "Page");
    qmlRegisterType(resolvedUrl(QStringLiteral("PageIndicator.qml")), uri, 2, 0, "PageIndicator");
    qmlRegisterType(resolvedUrl(QStringLiteral("Pane.qml")), uri, 2, 0, "Pane");
    qmlRegisterType(resolvedUrl(QStringLiteral("Popup.qml")), uri, 2, 0, "Popup");
    qmlRegisterType(resolvedUrl(QStringLiteral("ProgressBar.qml")), uri, 2, 0, "ProgressBar");
    qmlRegisterType(resolvedUrl(QStringLiteral("RadioButton.qml")), uri, 2, 0, "RadioButton");
    qmlRegisterType(resolvedUrl(QStringLiteral("RadioDelegate.qml")), uri, 2, 0, "RadioDelegate");
    qmlRegisterType(resolvedUrl(QStringLiteral("RangeSlider.qml")), uri, 2, 0, "RangeSlider");
    qmlRegisterType(resolvedUrl(QStringLiteral("ScrollBar.qml")), uri, 2, 0, "ScrollBar");
    qmlRegisterType(resolvedUrl(QStringLiteral("ScrollIndicator.qml")), uri, 2, 0, "ScrollIndicator");
    qmlRegisterType(resolvedUrl(QStringLiteral("Slider.qml")), uri, 2, 0, "Slider");
    qmlRegisterType(resolvedUrl(QStringLiteral("SpinBox.qml")), uri, 2, 0, "SpinBox");
    qmlRegisterType(resolvedUrl(QStringLiteral("StackView.qml")), uri, 2, 0, "StackView");
    qmlRegisterType(resolvedUrl(QStringLiteral("SwipeDelegate.qml")), uri, 2, 0, "SwipeDelegate");
    qmlRegisterType(resolvedUrl(QStringLiteral("SwipeView.qml")), uri, 2, 0, "SwipeView");
    qmlRegisterType(resolvedUrl(QStringLiteral("Switch.qml")), uri, 2, 0, "Switch");
    qmlRegisterType(resolvedUrl(QStringLiteral("SwitchDelegate.qml")), uri, 2, 0, "SwitchDelegate");
    qmlRegisterType(resolvedUrl(QStringLiteral("TabBar.qml")), uri, 2, 0, "TabBar");
    qmlRegisterType(resolvedUrl(QStringLiteral("TabButton.qml")), uri, 2, 0, "TabButton");
    qmlRegisterType(resolvedUrl(QStringLiteral("TextArea.qml")), uri, 2, 0, "TextArea");
    qmlRegisterType(resolvedUrl(QStringLiteral("TextField.qml")), uri, 2, 0, "TextField");
    qmlRegisterType(resolvedUrl(QStringLiteral("ToolBar.qml")), uri, 2, 0, "ToolBar");
    qmlRegisterType(resolvedUrl(QStringLiteral("ToolButton.qml")), uri, 2, 0, "ToolButton");
    qmlRegisterType(resolvedUrl(QStringLiteral("ToolTip.qml")), uri, 2, 0, "ToolTip");
#if QT_CONFIG(quick_listview) && QT_CONFIG(quick_pathview)
    qmlRegisterType(resolvedUrl(QStringLiteral("Tumbler.qml")), uri, 2, 0, "Tumbler");
#endif

    // QtQuick.Controls 2.1 (new types in Qt 5.8)
    qmlRegisterType(resolvedUrl(QStringLiteral("Dialog.qml")), uri, 2, 1, "Dialog");
    qmlRegisterType(resolvedUrl(QStringLiteral("DialogButtonBox.qml")), uri, 2, 1, "DialogButtonBox");
    qmlRegisterType(resolvedUrl(QStringLiteral("MenuSeparator.qml")), uri, 2, 1, "MenuSeparator");
    qmlRegisterType(resolvedUrl(QStringLiteral("RoundButton.qml")), uri, 2, 1, "RoundButton");
    qmlRegisterType(resolvedUrl(QStringLiteral("ToolSeparator.qml")), uri, 2, 1, "ToolSeparator");

    // QtQuick.Controls 2.2 (new types in Qt 5.9)
    qmlRegisterType(resolvedUrl(QStringLiteral("DelayButton.qml")), uri, 2, 2, "DelayButton");
    qmlRegisterType(resolvedUrl(QStringLiteral("ScrollView.qml")), uri, 2, 2, "ScrollView");

    // QtQuick.Controls 2.3 (new types in Qt 5.10)
    qmlRegisterType(resolvedUrl(QStringLiteral("Action.qml")), uri, 2, 3, "Action");
    qmlRegisterType(resolvedUrl(QStringLiteral("ActionGroup.qml")), uri, 2, 3, "ActionGroup");
    qmlRegisterType(resolvedUrl(QStringLiteral("MenuBar.qml")), uri, 2, 3, "MenuBar");
    qmlRegisterType(resolvedUrl(QStringLiteral("MenuBarItem.qml")), uri, 2, 3, "MenuBarItem");
    qmlRegisterUncreatableType<QQuickOverlay>(uri, 2, 3, "Overlay", QStringLiteral("Overlay is only available as an attached property."));

    // Register the latest version, even if there are no new types or new revisions for existing types yet.
    // Before Qt 5.12, we would do the following:
    //
    // qmlRegisterModule(import, 2, QT_VERSION_MINOR - 7); // Qt 5.7->2.0, 5.8->2.1, 5.9->2.2...
    //
    // However, we want to align with the rest of Qt Quick which uses Qt's minor version.
    const QByteArray import = QByteArray(uri) + ".impl";
    qmlRegisterModule(import, 2, QT_VERSION_MINOR);

    // QtQuick.Controls.impl 2.0 (Qt 5.7)
    qmlRegisterType<QQuickDefaultBusyIndicator>(import, 2, 0, "BusyIndicatorImpl");
    qmlRegisterType<QQuickDefaultDial>(import, 2, 0, "DialImpl");
    qmlRegisterType<QQuickPaddedRectangle>(import, 2, 0, "PaddedRectangle");
    qmlRegisterType<QQuickDefaultProgressBar>(import, 2, 0, "ProgressBarImpl");

    // QtQuick.Controls.impl 2.1 (Qt 5.8)
#if QT_CONFIG(quick_listview) && QT_CONFIG(quick_pathview)
    qmlRegisterType<QQuickTumblerView>(import, 2, 1, "TumblerView");
#endif
    qmlRegisterSingletonType<QQuickDefaultStyle>(import, 2, 1, "Default", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject* {
            Q_UNUSED(engine);
            Q_UNUSED(scriptEngine);
            return new QQuickDefaultStyle;
    });

    // QtQuick.Controls.impl 2.2 (Qt 5.9)
    qmlRegisterType<QQuickClippedText>(import, 2, 2, "ClippedText");
    qmlRegisterType<QQuickItemGroup>(import, 2, 2, "ItemGroup");
    qmlRegisterType<QQuickPlaceholderText>(import, 2, 2, "PlaceholderText");

    // QtQuick.Controls.impl 2.3 (Qt 5.10)
    qmlRegisterType<QQuickColorImage>(import, 2, 3, "ColorImage");
    qmlRegisterType<QQuickIconImage>(import, 2, 3, "IconImage");
    qmlRegisterSingletonType<QQuickColor>(import, 2, 3, "Color", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject* {
            Q_UNUSED(engine);
            Q_UNUSED(scriptEngine);
            return new QQuickColor;
    });
    qmlRegisterType<QQuickIconLabel>(import, 2, 3, "IconLabel");
    qmlRegisterType<QQuickCheckLabel>(import, 2, 3, "CheckLabel");
    qmlRegisterType<QQuickMnemonicLabel>(import, 2, 3, "MnemonicLabel");
    qmlRegisterRevision<QQuickText, 6>(import, 2, 3);
}

QString QtQuickControls2Plugin::name() const
{
    return QStringLiteral("Default");
}

void QtQuickControls2Plugin::initializeTheme(QQuickTheme *theme)
{
    QQuickDefaultTheme::initialize(theme);
}

QList<QQuickStylePlugin *> QtQuickControls2Plugin::loadStylePlugins()
{
    QList<QQuickStylePlugin *> stylePlugins;

    QFileInfo fileInfo = QQmlFile::urlToLocalFileOrQrc(resolvedUrl(QStringLiteral("qmldir")));
    if (fileInfo.exists() && fileInfo.path() != QQmlFile::urlToLocalFileOrQrc(baseUrl())) {
        QFile file(fileInfo.filePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QQmlDirParser parser;
            parser.parse(QString::fromUtf8(file.readAll()));
            if (!parser.hasError()) {
#ifdef QT_STATIC
                const auto plugins = QPluginLoader::staticInstances();
                for (QObject *instance : plugins) {
                    QQuickStylePlugin *stylePlugin = qobject_cast<QQuickStylePlugin *>(instance);
                    if (!stylePlugin || parser.className() != QLatin1String(instance->metaObject()->className()))
                        continue;
                    stylePlugins += stylePlugin;
                }
#elif QT_CONFIG(library)
                QPluginLoader loader;
                const auto plugins = parser.plugins();
                for (const QQmlDirParser::Plugin &plugin : plugins) {
                    QDir dir = fileInfo.dir();
                    if (!plugin.path.isEmpty() && !dir.cd(plugin.path))
                        continue;
                    QString filePath = dir.filePath(plugin.name);
#if defined(Q_OS_MACOS) && defined(QT_DEBUG)
                    // Avoid mismatching plugins on macOS so that we don't end up loading both debug and
                    // release versions of the same Qt libraries (due to the plugin's dependencies).
                    filePath += QStringLiteral("_debug");
#endif // Q_OS_MACOS && QT_DEBUG
#if defined(Q_OS_WIN) && defined(QT_DEBUG)
                    // Debug versions of plugins have a "d" prefix on Windows.
                    filePath += QLatin1Char('d');
#endif // Q_OS_WIN && QT_DEBUG
                    loader.setFileName(filePath);
                    QQuickStylePlugin *stylePlugin = qobject_cast<QQuickStylePlugin *>(loader.instance());
                    if (stylePlugin)
                        stylePlugins += stylePlugin;
                }
#endif
            }
        }
    }
    return stylePlugins;
}

QQuickTheme *QtQuickControls2Plugin::createTheme(const QString &name)
{
    QQuickTheme *theme = new QQuickTheme;
#if QT_CONFIG(settings)
    QQuickThemePrivate *p = QQuickThemePrivate::get(theme);
    QSharedPointer<QSettings> settings = QQuickStylePrivate::settings(name);
    if (settings) {
        p->defaultFont.reset(QQuickStylePrivate::readFont(settings));
        // Set the default font as the System scope, because that's what
        // QQuickControlPrivate::parentFont() uses as its fallback if no
        // parent item has a font explicitly set. QQuickControlPrivate::parentFont()
        // is used as the starting point for font inheritance/resolution.
        // The same goes for palettes below.
        theme->setFont(QQuickTheme::System, *p->defaultFont);

        p->defaultPalette.reset(QQuickStylePrivate::readPalette(settings));
        theme->setPalette(QQuickTheme::System, *p->defaultPalette);
    }
#endif
    QQuickThemePrivate::instance.reset(theme);
    return theme;
}

QT_END_NAMESPACE

#include "qtquickcontrols2plugin.moc"
