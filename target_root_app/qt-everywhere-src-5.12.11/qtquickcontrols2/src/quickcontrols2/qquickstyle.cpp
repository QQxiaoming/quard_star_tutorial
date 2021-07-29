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

#include "qquickstyle.h"
#include "qquickstyle_p.h"

#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qdebug.h>
#include <QtCore/qsettings.h>
#include <QtCore/qfileselector.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qmetaobject.h>
#include <QtGui/qcolor.h>
#include <QtGui/qfont.h>
#include <QtGui/qpalette.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformtheme.h>
#include <QtQml/private/qqmlmetatype_p.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlfile.h>

#include <functional>

QT_BEGIN_NAMESPACE

/*!
    \class QQuickStyle
    \brief The QQuickStyle class allows configuring the application style.
    \inmodule QtQuickControls2
    \since 5.7

    QQuickStyle provides API for querying and configuring the application
    \l {Styling Qt Quick Controls 2}{styles} of Qt Quick Controls 2.

    \code
    #include <QGuiApplication>
    #include <QQmlApplicationEngine>
    #include <QQuickStyle>

    int main(int argc, char *argv[])
    {
        QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QGuiApplication app(argc, argv);

        QQuickStyle::setStyle("Material");

        QQmlApplicationEngine engine;
        engine.load(QUrl("qrc:/main.qml"));

        return app.exec();
    }
    \endcode

    \note The style must be configured \b before loading QML that imports
    Qt Quick Controls 2. It is not possible to change the style after the QML
    types have been registered.

    The style can also be specified as a path to a custom style, such as
    \c ":/mystyle". See \l {Creating a Custom Style} for more details about
    building custom styles. Custom styles do not need to implement all controls.
    By default, the styling system uses the \l {Default style} as a fallback
    for controls that a custom style does not provide. It is possible to
    specify a different fallback style to customize or extend one of the
    built-in styles.

    \code
    QQuickStyle::setStyle(":/mystyle");
    QQuickStyle::setFallbackStyle("Material");
    \endcode

    \sa {Styling Qt Quick Controls 2}
*/

static QStringList envPathList(const QByteArray &var)
{
    QStringList paths;
    if (Q_UNLIKELY(!qEnvironmentVariableIsEmpty(var))) {
        const QByteArray value = qgetenv(var);
        paths += QString::fromLocal8Bit(value).split(QDir::listSeparator(), QString::SkipEmptyParts);
    }
    return paths;
}

static QStringList defaultImportPathList()
{
    QStringList importPaths;
    importPaths.reserve(3);
#ifndef QT_STATIC
    importPaths += QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath);
#endif
    importPaths += envPathList("QML2_IMPORT_PATH");
    importPaths += QStringLiteral(":/qt-project.org/imports");
    importPaths += QCoreApplication::applicationDirPath();
    return importPaths;
}

struct QQuickStyleSpec
{
    QQuickStyleSpec() : custom(false), resolved(false) { }

    QString name()
    {
        if (!resolved)
            resolve();
        return style.mid(style.lastIndexOf(QLatin1Char('/')) + 1);
    }

    QString path()
    {
        if (!resolved)
            resolve();
        QString s = style;
        if (QQmlFile::isLocalFile(s))
            s = QQmlFile::urlToLocalFileOrQrc(s);
        return s.left(s.lastIndexOf(QLatin1Char('/')) + 1);
    }

    void setStyle(const QString &s)
    {
        style = s;
        resolved = false;
        resolve();
    }

    void setFallbackStyle(const QString &fallback, const QByteArray &method)
    {
        fallbackStyle = fallback;
        fallbackMethod = method;
    }

    static QString findStyle(const QString &path, const QString &name)
    {
        QDir dir(path);
        if (!dir.exists())
            return QString();

        if (name.isEmpty())
            return dir.absolutePath() + QLatin1Char('/');

        const QStringList entries = dir.entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &entry : entries) {
            if (entry.compare(name, Qt::CaseInsensitive) == 0)
                return dir.absoluteFilePath(entry);
        }

        return QString();
    }

    void resolve(const QUrl &baseUrl = QUrl())
    {
        if (style.isEmpty())
            style = QGuiApplicationPrivate::styleOverride;
        if (style.isEmpty())
            style = QString::fromLocal8Bit(qgetenv("QT_QUICK_CONTROLS_STYLE"));
        if (fallbackStyle.isEmpty())
            setFallbackStyle(QString::fromLocal8Bit(qgetenv("QT_QUICK_CONTROLS_FALLBACK_STYLE")), "QT_QUICK_CONTROLS_FALLBACK_STYLE");
#if QT_CONFIG(settings)
        if (style.isEmpty() || fallbackStyle.isEmpty()) {
            QSharedPointer<QSettings> settings = QQuickStylePrivate::settings(QStringLiteral("Controls"));
            if (settings) {
                if (style.isEmpty())
                    style = settings->value(QStringLiteral("Style")).toString();
                if (fallbackStyle.isEmpty())
                    setFallbackStyle(settings->value(QStringLiteral("FallbackStyle")).toString(), ":/qtquickcontrols2.conf");
            }
        }
#endif

        // resolve a path relative to the config
        QString configPath = QFileInfo(resolveConfigFilePath()).path();
        QString stylePath = findStyle(configPath, style);
        if (!stylePath.isEmpty()) {
            style = stylePath;
            resolved = true;
        }

        custom = style.contains(QLatin1Char('/'));

        if (baseUrl.isValid()) {
            QString path = QQmlFile::urlToLocalFileOrQrc(baseUrl);
            QString stylePath = findStyle(path, style);
            if (!stylePath.isEmpty()) {
                style = stylePath;
                resolved = true;
            }
        }

        if (QGuiApplication::instance()) {
            if (!custom) {
                const QStringList stylePaths = QQuickStylePrivate::stylePaths();
                for (const QString &path : stylePaths) {
                    QString stylePath = findStyle(path, style);
                    if (!stylePath.isEmpty()) {
                        custom = !stylePath.startsWith(QQmlFile::urlToLocalFileOrQrc(baseUrl));
                        style = stylePath;
                        resolved = true;
                        break;
                    }
                }
            }
            resolved = true;
        }
    }

    void reset()
    {
        custom = false;
        resolved = false;
        style.clear();
        fallbackStyle.clear();
        fallbackMethod.clear();
        configFilePath.clear();
    }

    QString resolveConfigFilePath()
    {
        if (configFilePath.isEmpty()) {
            configFilePath = QFile::decodeName(qgetenv("QT_QUICK_CONTROLS_CONF"));
            if (configFilePath.isEmpty() || !QFile::exists(configFilePath)) {
                if (!configFilePath.isEmpty())
                    qWarning("QT_QUICK_CONTROLS_CONF=%s: No such file", qPrintable(configFilePath));

                configFilePath = QStringLiteral(":/qtquickcontrols2.conf");
            }
        }
        return configFilePath;
    }

    bool custom;
    bool resolved;
    QString style;
    QString fallbackStyle;
    QByteArray fallbackMethod;
    QString configFilePath;
    QStringList customStylePaths;
};

Q_GLOBAL_STATIC(QQuickStyleSpec, styleSpec)

static QStringList parseStylePathsWithColon(const QString &var)
{
    QStringList paths;
    const QChar colon = QLatin1Char(':');
    int currentIndex = 0;

    do {
        int nextColonIndex = -1;
        QString path;

        if (var.at(currentIndex) == colon) {
            // This is either a list separator, or a qrc path.
            if (var.at(currentIndex + 1) == colon) {
                // It's a double colon (list separator followed by qrc path);
                // find the end of the path.
                nextColonIndex = var.indexOf(colon, currentIndex + 2);
                path = var.mid(currentIndex + 1,
                    nextColonIndex == -1 ? -1 : nextColonIndex - currentIndex - 1);
            } else {
                // It's a single colon.
                nextColonIndex = var.indexOf(colon, currentIndex + 1);
                if (currentIndex == 0) {
                    // If we're at the start of the string, then it's a qrc path.
                    path = var.mid(currentIndex,
                        nextColonIndex == -1 ? -1 : nextColonIndex - currentIndex);
                } else {
                    // Otherwise, it's a separator.
                    path = var.mid(currentIndex + 1,
                        nextColonIndex == -1 ? -1 : nextColonIndex - currentIndex - 1);
                }
            }
        } else {
            // It's a file path.
            nextColonIndex = var.indexOf(colon, currentIndex);
            path = var.mid(currentIndex,
                nextColonIndex == -1 ? -1 : nextColonIndex - currentIndex);
        }

        paths += path;
        currentIndex = nextColonIndex;

        // Keep going until we can't find any more colons,
        // or we're at the last character.
    } while (currentIndex != -1 && currentIndex < var.size() - 1);

    return paths;
}

QStringList QQuickStylePrivate::stylePaths(bool resolve)
{
    // user-requested style path
    QStringList paths;
    if (resolve) {
        QString path = styleSpec()->path();
        if (path.endsWith(QLatin1Char('/')))
            path.chop(1);
        if (!path.isEmpty())
            paths += path;
    }

    if (Q_UNLIKELY(!qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE_PATH"))) {
        const QString value = QString::fromLocal8Bit(qgetenv("QT_QUICK_CONTROLS_STYLE_PATH"));
        const QChar listSeparator = QDir::listSeparator();
        if (listSeparator == QLatin1Char(':')) {
            // Split manually to avoid breaking paths on systems where : is the list separator,
            // since it's also used for qrc paths.
            paths += parseStylePathsWithColon(value);
        } else {
            // Fast/simpler path for systems where something other than : is used as
            // the list separator (such as ';').
            const QStringList customPaths = value.split(listSeparator, QString::SkipEmptyParts);
            paths += customPaths;
        }
    }

    // system/custom style paths
    paths += styleSpec()->customStylePaths;
    paths += envPathList("QT_QUICK_CONTROLS_STYLE_PATH");

    // built-in import paths
    const QString targetPath = QStringLiteral("QtQuick/Controls.2");
    const QStringList importPaths = defaultImportPathList();
    for (const QString &importPath : importPaths) {
        QDir dir(importPath);
        if (dir.cd(targetPath))
            paths += dir.absolutePath();
    }

    paths.removeDuplicates();
    return paths;
}

QString QQuickStylePrivate::fallbackStyle()
{
    return styleSpec()->fallbackStyle;
}

bool QQuickStylePrivate::isCustomStyle()
{
    return styleSpec()->custom;
}

void QQuickStylePrivate::init(const QUrl &baseUrl)
{
    QQuickStyleSpec *spec = styleSpec();
    spec->resolve(baseUrl);

    if (!spec->fallbackStyle.isEmpty()) {
        QString fallbackStyle = spec->findStyle(QQmlFile::urlToLocalFileOrQrc(baseUrl), spec->fallbackStyle);
        if (fallbackStyle.isEmpty()) {
            if (spec->fallbackStyle.compare(QStringLiteral("Default")) != 0) {
                qWarning() << "ERROR: unable to locate fallback style" << spec->fallbackStyle;
                qInfo().nospace().noquote() << spec->fallbackMethod << ": the fallback style must be the name of one of the built-in Qt Quick Controls 2 styles.";
            }
            spec->fallbackStyle.clear();
        }
    }
}

void QQuickStylePrivate::reset()
{
    if (styleSpec())
        styleSpec()->reset();
}

QString QQuickStylePrivate::configFilePath()
{
    return styleSpec()->resolveConfigFilePath();
}

QSharedPointer<QSettings> QQuickStylePrivate::settings(const QString &group)
{
#ifndef QT_NO_SETTINGS
    const QString filePath = QQuickStylePrivate::configFilePath();
    if (QFile::exists(filePath)) {
        QFileSelector selector;
        QSettings *settings = new QSettings(selector.select(filePath), QSettings::IniFormat);
        if (!group.isEmpty())
            settings->beginGroup(group);
        return QSharedPointer<QSettings>(settings);
    }
#endif // QT_NO_SETTINGS
    return QSharedPointer<QSettings>();
}

#if QT_CONFIG(settings)
static void readValue(const QSharedPointer<QSettings> &settings, const QString &name, std::function<void(const QVariant &)> setValue)
{
    const QVariant var = settings->value(name);
    if (var.isValid())
        setValue(var);
}

template <typename Enum>
static Enum toEnumValue(const QVariant &var)
{
    // ### TODO: expose QFont enums to the meta object system using Q_ENUM
    //QMetaEnum enumeration = QMetaEnum::fromType<Enum>();
    //bool ok = false;
    //int value = enumeration.keyToValue(var.toByteArray(), &ok);
    //if (!ok)
    //    value = var.toInt();
    //return static_cast<Enum>(value);

    return static_cast<Enum>(var.toInt());
}

const QFont *QQuickStylePrivate::readFont(const QSharedPointer<QSettings> &settings)
{
    const QVariant var = settings->value(QStringLiteral("Font"));
    if (var.isValid())
        return new QFont(var.value<QFont>());

    QFont f;
    settings->beginGroup(QStringLiteral("Font"));
    readValue(settings, QStringLiteral("Family"), [&f](const QVariant &var) { f.setFamily(var.toString()); });
    readValue(settings, QStringLiteral("PointSize"), [&f](const QVariant &var) { f.setPointSizeF(var.toReal()); });
    readValue(settings, QStringLiteral("PixelSize"), [&f](const QVariant &var) { f.setPixelSize(var.toInt()); });
    readValue(settings, QStringLiteral("StyleHint"), [&f](const QVariant &var) { f.setStyleHint(toEnumValue<QFont::StyleHint>(var.toInt())); });
    readValue(settings, QStringLiteral("Weight"), [&f](const QVariant &var) { f.setWeight(toEnumValue<QFont::Weight>(var)); });
    readValue(settings, QStringLiteral("Style"), [&f](const QVariant &var) { f.setStyle(toEnumValue<QFont::Style>(var.toInt())); });
    settings->endGroup();
    return new QFont(f);
}

static void readColorGroup(const QSharedPointer<QSettings> &settings, QPalette::ColorGroup group, QPalette *palette)
{
    const QStringList keys = settings->childKeys();
    if (keys.isEmpty())
        return;

    static const int index = QPalette::staticMetaObject.indexOfEnumerator("ColorRole");
    Q_ASSERT(index != -1);
    QMetaEnum metaEnum = QPalette::staticMetaObject.enumerator(index);

    for (const QString &key : keys) {
        bool ok = false;
        int role = metaEnum.keyToValue(key.toUtf8(), &ok);
        if (ok)
            palette->setColor(group, static_cast<QPalette::ColorRole>(role), settings->value(key).value<QColor>());
    }
}

const QPalette *QQuickStylePrivate::readPalette(const QSharedPointer<QSettings> &settings)
{
    QPalette p;
    settings->beginGroup(QStringLiteral("Palette"));
    readColorGroup(settings, QPalette::All, &p);

    settings->beginGroup(QStringLiteral("Normal"));
    readColorGroup(settings, QPalette::Normal, &p);
    settings->endGroup();

    settings->beginGroup(QStringLiteral("Disabled"));
    readColorGroup(settings, QPalette::Disabled, &p);
    settings->endGroup();
    return new QPalette(p);
}
#endif // QT_CONFIG(settings)

static bool qt_is_dark_system_theme()
{
    if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme()) {
        if (const QPalette *systemPalette = theme->palette(QPlatformTheme::SystemPalette)) {
            const QColor textColor = systemPalette->color(QPalette::WindowText);
            return textColor.red() > 128 && textColor.blue() > 128 && textColor.green() > 128;
        }
    }
    return false;
}

bool QQuickStylePrivate::isDarkSystemTheme()
{
    static bool dark = qt_is_dark_system_theme();
    return dark;
}

/*!
    Returns the name of the application style.

    \note The application style can be specified by passing a \c -style command
          line argument. Therefore \c name() may not return a fully resolved
          value if called before constructing a QGuiApplication.
*/
QString QQuickStyle::name()
{
    return styleSpec()->name();
}

/*!
    Returns the path of an overridden application style, or an empty
    string if the style is one of the built-in Qt Quick Controls 2 styles.

    \note The application style can be specified by passing a \c -style command
          line argument. Therefore \c path() may not return a fully resolved
          value if called before constructing a QGuiApplication.
*/
QString QQuickStyle::path()
{
    return styleSpec()->path();
}

/*!
    Sets the application style to \a style.

    \note The style must be configured \b before loading QML that imports Qt Quick Controls 2.
          It is not possible to change the style after the QML types have been registered.

    \sa setFallbackStyle(), {Using Styles in Qt Quick Controls 2}
*/
void QQuickStyle::setStyle(const QString &style)
{
    if (QQmlMetaType::isModule(QStringLiteral("QtQuick.Controls"), 2, 0)) {
        qWarning() << "ERROR: QQuickStyle::setStyle() must be called before loading QML that imports Qt Quick Controls 2.";
        return;
    }

    styleSpec()->setStyle(style);
}

/*!
    \since 5.8
    Sets the application fallback style to \a style.

    \note The fallback style must be the name of one of the built-in Qt Quick Controls 2 styles, e.g. "Material".

    \note The style must be configured \b before loading QML that imports Qt Quick Controls 2.
          It is not possible to change the style after the QML types have been registered.

    The fallback style can be also specified by setting the \c QT_QUICK_CONTROLS_FALLBACK_STYLE
    \l {Supported Environment Variables in Qt Quick Controls 2}{environment variable}.

    \sa setStyle(), {Using Styles in Qt Quick Controls 2}
*/
void QQuickStyle::setFallbackStyle(const QString &style)
{
    if (QQmlMetaType::isModule(QStringLiteral("QtQuick.Controls"), 2, 0)) {
        qWarning() << "ERROR: QQuickStyle::setFallbackStyle() must be called before loading QML that imports Qt Quick Controls 2.";
        return;
    }

    styleSpec()->setFallbackStyle(style, "QQuickStyle::setFallbackStyle()");
}

/*!
    \since 5.9
    Returns the names of the available styles.

    \note The method must be called \b after creating an instance of QGuiApplication.

    \sa stylePathList(), addStylePath()
*/
QStringList QQuickStyle::availableStyles()
{
    QStringList styles;
    if (!QGuiApplication::instance()) {
        qWarning() << "ERROR: QQuickStyle::availableStyles() must be called after creating an instance of QGuiApplication.";
        return styles;
    }

    const QStringList stylePaths = QQuickStylePrivate::stylePaths();
    for (const QString &path : stylePaths) {
        const QList<QFileInfo> entries = QDir(path).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo &entry : entries) {
            const QString name = entry.fileName();
            if (!name.endsWith(QLatin1String(".dSYM")) && name != QLatin1String("designer"))
                styles += name;
        }
    }
    styles.prepend(QStringLiteral("Default"));
    styles.removeDuplicates();
    return styles;
}

/*!
    \since 5.12

    Returns the list of directories where Qt Quick Controls 2 searches for available styles.

    By default, the list contains paths specified in the \c QT_QUICK_CONTROLS_STYLE_PATH
    environment variable, and any existing \c QtQuick/Controls.2 sub-directories in
    \l QQmlEngine::importPathList().

    \sa addStylePath(), availableStyles()
*/
QStringList QQuickStyle::stylePathList()
{
    return QQuickStylePrivate::stylePaths();
}

/*!
    \since 5.12

    Adds \a path as a directory where Qt Quick Controls 2 searches for available styles.

    The \a path may be any local filesystem directory or \l {The Qt Resource System}{Qt Resource} directory.
    For example, the following paths are all valid:

    \list
        \li \c {/path/to/styles/}
        \li \c {file:///path/to/styles/}
        \li \c {:/path/to/styles/}
        \li \c {qrc:/path/to/styles/})
    \endlist

    The \a path will be converted into \l {QDir::canonicalPath}{canonical form} before it is added to
    the style path list.

    The newly added \a path will be first in the stylePathList().

    \sa stylePathList(), availableStyles()
*/
void QQuickStyle::addStylePath(const QString &path)
{
    if (path.isEmpty())
        return;

    const QUrl url = QUrl(path);
    if (url.isRelative() || url.scheme() == QLatin1String("file")
            || (url.scheme().length() == 1 && QFile::exists(path)) ) {  // windows path
        styleSpec()->customStylePaths.prepend(QDir(path).canonicalPath());
    } else if (url.scheme() == QLatin1String("qrc")) {
        styleSpec()->customStylePaths.prepend(QLatin1Char(':') + url.path());
    } else {
        styleSpec()->customStylePaths.prepend(path);
    }
}

QT_END_NAMESPACE
