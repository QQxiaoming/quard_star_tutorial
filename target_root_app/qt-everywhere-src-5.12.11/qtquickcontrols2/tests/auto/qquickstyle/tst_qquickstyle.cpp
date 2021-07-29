/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

#include <QtTest/qtest.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQuickControls2/qquickstyle.h>
#include <QtQuickControls2/private/qquickstyle_p.h>
#include <QtQuickTemplates2/private/qquicklabel_p.h>
#include <QtQuickTemplates2/private/qquicktheme_p.h>
#include <QtGui/private/qguiapplication_p.h>

#include "../shared/util.h"

class tst_QQuickStyle : public QQmlDataTest
{
    Q_OBJECT

private slots:
    void cleanup();
    void lookup();
    void configurationFile_data();
    void configurationFile();
    void commandLineArgument();
    void environmentVariables();
    void availableStyles();
    void qrcStylePaths_data();
    void qrcStylePaths();
    void qrcInQtQuickControlsStylePathEnvVar_data();
    void qrcInQtQuickControlsStylePathEnvVar();

private:
    void loadControls();
    void unloadControls();
};

void tst_QQuickStyle::cleanup()
{
    unloadControls();

    QGuiApplicationPrivate::styleOverride.clear();
    qunsetenv("QT_QUICK_CONTROLS_STYLE");
    qunsetenv("QT_QUICK_CONTROLS_STYLE_PATH");
    qunsetenv("QT_QUICK_CONTROLS_FALLBACK_STYLE");
    qunsetenv("QT_QUICK_CONTROLS_CONF");
}

void tst_QQuickStyle::loadControls()
{
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import QtQuick 2.0; import QtQuick.Controls 2.1; Control { }", QUrl());

    QScopedPointer<QObject> object(component.create());
    QVERIFY2(!object.isNull(), qPrintable(component.errorString()));
}

void tst_QQuickStyle::unloadControls()
{
    qmlClearTypeRegistrations();
}

void tst_QQuickStyle::lookup()
{
    QVERIFY(QQuickStyle::name().isEmpty());
    QVERIFY(!QQuickStyle::path().isEmpty());

    QQuickStyle::setStyle("material");
    QCOMPARE(QQuickStyle::name(), QString("Material"));
    QVERIFY(!QQuickStyle::path().isEmpty());

    loadControls();

    // The font size for editors in the (default) Normal variant is 16.
    // If this is wrong, the style plugin may not have been loaded.
    QCOMPARE(QQuickTheme::instance()->font(QQuickTheme::TextArea).pixelSize(), 16);

    QCOMPARE(QQuickStyle::name(), QString("Material"));
    QVERIFY(!QQuickStyle::path().isEmpty());
}

void tst_QQuickStyle::configurationFile_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("expectedStyle");
    QTest::addColumn<QString>("expectedPath");

    QTest::newRow("Default") << "default.conf" << "Default" << "";
    QTest::newRow("Fusion") << "fusion.conf" << "Fusion" << "";
    QTest::newRow("Imagine") << "imagine.conf" << "Imagine" << "";
    QTest::newRow("Material") << "material.conf" << "Material" << "";
    QTest::newRow("Universal") << "universal.conf" << "Universal" << "";
    QTest::newRow("Custom") << "custom.conf" << "Custom" << ":/";
}

void tst_QQuickStyle::configurationFile()
{
    QFETCH(QString, fileName);
    QFETCH(QString, expectedStyle);
    QFETCH(QString, expectedPath);

    qputenv("QT_QUICK_CONTROLS_CONF", testFile(fileName).toLocal8Bit());

    // Load a control. The import causes the configuration file to be read.
    QQmlEngine engine;
    QQmlComponent labelComponent(&engine);
    labelComponent.setData("import QtQuick 2.0; import QtQuick.Controls 2.12; Label {}", QUrl());

    QScopedPointer<QObject> object(labelComponent.create());
    QVERIFY2(!object.isNull(), qPrintable(labelComponent.errorString()));

    QCOMPARE(QQuickStyle::name(), expectedStyle);
    if (!expectedPath.isEmpty())
        QCOMPARE(QQuickStyle::path(), expectedPath);

    // Test that fonts and palettes specified in configuration files are respected.
    QQuickLabel *label = qobject_cast<QQuickLabel *>(object.data());
    QVERIFY(label);
    // Make it small so that there's less possibility for the default/system
    // pixel size to match it and give us false positives.
    QCOMPARE(label->font().pixelSize(), 3);
    QCOMPARE(label->palette().windowText(), Qt::red);
}

void tst_QQuickStyle::commandLineArgument()
{
    QGuiApplicationPrivate::styleOverride = "CmdLineArgStyle";

    loadControls();

    QCOMPARE(QQuickStyle::name(), QString("CmdLineArgStyle"));
}

void tst_QQuickStyle::environmentVariables()
{
    qputenv("QT_QUICK_CONTROLS_STYLE", "EnvVarStyle");
    qputenv("QT_QUICK_CONTROLS_FALLBACK_STYLE", "EnvVarFallbackStyle");
    QCOMPARE(QQuickStyle::name(), QString("EnvVarStyle"));
    QCOMPARE(QQuickStylePrivate::fallbackStyle(), QString("EnvVarFallbackStyle"));
}

void tst_QQuickStyle::availableStyles()
{
    QString path = QFINDTESTDATA("data");
    QVERIFY(!path.isEmpty());

    QQuickStyle::addStylePath(path);
    QStringList paths = QQuickStylePrivate::stylePaths();
#ifndef Q_OS_WIN
    QVERIFY(paths.contains(path));
#else
    QVERIFY(paths.contains(path, Qt::CaseInsensitive));
#endif

    const QStringList styles = QQuickStyle::availableStyles();
    QVERIFY(!styles.isEmpty());
    QCOMPARE(styles.first(), QString("Default"));
    QVERIFY(!styles.contains("designer"));

    // QTBUG-60973
    for (const QString &style : styles) {
        QVERIFY2(!style.endsWith(".dSYM"), qPrintable(style));
    }
}

void tst_QQuickStyle::qrcStylePaths_data()
{
    QTest::addColumn<QString>("stylePath");
    QTest::addColumn<QString>("expectedStyleName");

    QTest::addRow("qrc:/qrcStyles1") << QString::fromLatin1("qrc:/qrcStyles1") << QString::fromLatin1("QrcStyle1");
    QTest::addRow(":/qrcStyles2") << QString::fromLatin1(":/qrcStyles2") << QString::fromLatin1("QrcStyle2");
}

void tst_QQuickStyle::qrcStylePaths()
{
    QFETCH(QString, stylePath);
    QFETCH(QString, expectedStyleName);

    QQuickStyle::addStylePath(stylePath);

    const QStringList paths = QQuickStylePrivate::stylePaths();
    QString expectedStylePath = stylePath;
    if (expectedStylePath.startsWith(QLatin1String("qrc")))
        expectedStylePath.remove(0, 3);
    if (!paths.contains(expectedStylePath)) {
        QString message;
        QDebug stream(&message);
        stream.nospace() << "QQuickStylePrivate::stylePaths() doesn't contain " << expectedStylePath << ":\n" << paths;
        QFAIL(qPrintable(message));
    }

    const QStringList styles = QQuickStyle::availableStyles();
    QVERIFY(!styles.isEmpty());
    if (!styles.contains(expectedStyleName)) {
        QString message;
        QDebug stream(&message);
        stream.nospace() << "QQuickStyle::availableStyles() doesn't contain " << expectedStyleName << ":\n" << styles;
        QFAIL(qPrintable(message));
    }
}

void tst_QQuickStyle::qrcInQtQuickControlsStylePathEnvVar_data()
{
    QTest::addColumn<QString>("environmentVariable");
    QTest::addColumn<QStringList>("expectedAvailableStyles");

    const QChar listSeparator = QDir::listSeparator();
    const QStringList defaultAvailableStyles = QQuickStyle::availableStyles();

    {
        QString environmentVariable;
        QDebug stream(&environmentVariable);
        // We use qrcStyles3 and qrcStyles4 in order to not conflict with
        // qrcStylePaths(), since we currently have no way of clearing customStylePaths.
        stream.noquote().nospace() << "/some/bogus/path/" << listSeparator
            << ":/qrcStyles3";

        QStringList expectedAvailableStyles = defaultAvailableStyles;
        // We need to keep the Default style at the start of the list,
        // as that's what availableStyles() does.
        expectedAvailableStyles.insert(1, QLatin1String("QrcStyle3"));

        QTest::addRow("%s", qPrintable(environmentVariable))
            << environmentVariable << expectedAvailableStyles;
    }

    {
        QString environmentVariable;
        QDebug stream(&environmentVariable);
        stream.noquote().nospace() << ":/qrcStyles4" << listSeparator
            << "/some/bogus/path";

        QStringList expectedAvailableStyles = defaultAvailableStyles;
        expectedAvailableStyles.insert(1, QLatin1String("QrcStyle4"));

        QTest::addRow("%s", qPrintable(environmentVariable))
            << environmentVariable << expectedAvailableStyles;
    }

    {
        QString environmentVariable;
        QDebug stream(&environmentVariable);
        stream.noquote().nospace() << ":/qrcStyles3" << listSeparator
            << ":/qrcStyles4" << listSeparator
            << QFINDTESTDATA("data/dummyStyles");

        QStringList expectedAvailableStyles = defaultAvailableStyles;
        expectedAvailableStyles.insert(1, QLatin1String("DummyStyle"));
        expectedAvailableStyles.insert(1, QLatin1String("QrcStyle4"));
        expectedAvailableStyles.insert(1, QLatin1String("QrcStyle3"));

        QTest::addRow("%s", qPrintable(environmentVariable))
            << environmentVariable << expectedAvailableStyles;
    }

    {
        QString environmentVariable;
        QDebug stream(&environmentVariable);
        stream.noquote().nospace() << QFINDTESTDATA("data/dummyStyles") << listSeparator
            << ":/qrcStyles3" << listSeparator
            << ":/qrcStyles4";

        QStringList expectedAvailableStyles = defaultAvailableStyles;
        expectedAvailableStyles.insert(1, QLatin1String("QrcStyle4"));
        expectedAvailableStyles.insert(1, QLatin1String("QrcStyle3"));
        expectedAvailableStyles.insert(1, QLatin1String("DummyStyle"));

        QTest::addRow("%s", qPrintable(environmentVariable))
            << environmentVariable << expectedAvailableStyles;
    }

    {
        QString environmentVariable;
        QDebug stream(&environmentVariable);
        // Same as the last row, except it adds a superfluous separator
        // to ensure that it handles it gracefully rather than failing an assertion.
        stream.noquote().nospace() << QFINDTESTDATA("data/dummyStyles") << listSeparator
            << ":/qrcStyles3" << listSeparator
            << ":/qrcStyles4" << listSeparator;

        QStringList expectedAvailableStyles = defaultAvailableStyles;
        expectedAvailableStyles.insert(1, QLatin1String("QrcStyle4"));
        expectedAvailableStyles.insert(1, QLatin1String("QrcStyle3"));
        expectedAvailableStyles.insert(1, QLatin1String("DummyStyle"));

        QTest::addRow("%s", qPrintable(environmentVariable))
            << environmentVariable << expectedAvailableStyles;
    }
}

/*
    Tests that qrc paths work with QT_QUICK_CONTROLS_STYLE_PATH.
*/
void tst_QQuickStyle::qrcInQtQuickControlsStylePathEnvVar()
{
    QFETCH(QString, environmentVariable);
    QFETCH(QStringList, expectedAvailableStyles);

    qputenv("QT_QUICK_CONTROLS_STYLE_PATH", environmentVariable.toLocal8Bit());

    const QStringList availableStyles = QQuickStyle::availableStyles();
    if (availableStyles != expectedAvailableStyles) {
        QString failureMessage;
        QDebug stream(&failureMessage);
        stream << "Mismatch in actual vs expected available styles:"
               << "\n   Expected:" << expectedAvailableStyles
               << "\n   Actual:" << availableStyles;
        QFAIL(qPrintable(failureMessage));
    }
}

QTEST_MAIN(tst_QQuickStyle)

#include "tst_qquickstyle.moc"
