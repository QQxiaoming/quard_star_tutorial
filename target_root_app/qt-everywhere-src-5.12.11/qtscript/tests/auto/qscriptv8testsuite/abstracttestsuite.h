/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef ABSTRACTTESTSUITE_H
#define ABSTRACTTESTSUITE_H

#include <QtCore/qobject.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qdir.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qvector.h>
#include <QtCore/qtextstream.h>

QT_FORWARD_DECLARE_CLASS(QMetaObjectBuilder)

namespace TestConfig {
enum Mode {
    Skip,
    ExpectFail
};
}

// For receiving callbacks from the config parser.
class TestConfigClientInterface
{
public:
    virtual ~TestConfigClientInterface() {}
    virtual void configData(TestConfig::Mode mode,
                            const QStringList &parts) = 0;
    virtual void configError(const QString &path,
                             const QString &message,
                             int lineNumber) = 0;
};

class AbstractTestSuite : public QObject,
                          public TestConfigClientInterface
{
// No Q_OBJECT macro, we implement the meta-object ourselves.
public:
    AbstractTestSuite(const QByteArray &className,
                      const QString &defaultTestsPath,
                      const QString &defaultConfigPath);
    virtual ~AbstractTestSuite();

    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **argv);

    static QString readFile(const QString &);
    static QString escape(const QString &);

protected:
    enum DataFunctionCreation {
        DontCreateDataFunction,
        CreateDataFunction
    };

    void addTestFunction(const QString &,
                         DataFunctionCreation = DontCreateDataFunction);
    void finalizeMetaObject();

    virtual void initTestCase();
    virtual void cleanupTestCase();

    virtual void writeSkipConfigFile(QTextStream &) = 0;
    virtual void writeExpectFailConfigFile(QTextStream &) = 0;

    virtual void runTestFunction(int index) = 0;

    virtual void configError(const QString &path, const QString &message, int lineNumber);

    QDir testsDir;
    bool shouldGenerateExpectedFailures;

private:
    static void qt_static_metacall(QObject *, QMetaObject::Call, int, void **);

    void addPrivateSlot(const QByteArray &signature);

    QMetaObject *dynamicMetaObject;
    QScopedPointer<QMetaObjectBuilder> metaBuilder;
    QString skipConfigPath, expectFailConfigPath;

private:
    void createSkipConfigFile();
    void createExpectFailConfigFile();
};

#endif
