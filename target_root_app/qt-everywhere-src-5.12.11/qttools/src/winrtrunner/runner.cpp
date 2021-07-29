/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "runner.h"

#include "runnerengine.h"

#ifndef RTRUNNER_NO_APPXPHONE
#include "appxphoneengine.h"
#endif
#ifndef RTRUNNER_NO_APPXLOCAL
#include "appxlocalengine.h"
#endif

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QLoggingCategory>

QT_USE_NAMESPACE

Q_LOGGING_CATEGORY(lcWinRtRunner, "qt.winrtrunner")
Q_LOGGING_CATEGORY(lcWinRtRunnerApp, "qt.winrtrunner.app")

class RunnerPrivate
{
public:
    bool isValid;
    QString app;
    QString manifest;
    QStringList arguments;
    int deviceIndex;
    struct TestPaths {
        // File path in application bundle
        QString deviceOutputFile;
        // temporary output path (if absolute path or stdout (-) was given
        QString localOutputFile;
        // final output location. Might be equal to localOutputFile or stdout (-)
        QString finalOutputFile;
    };
    QVector<TestPaths> testPaths;

    QString profile;
    QScopedPointer<RunnerEngine> engine;

    QString defaultOutputFileName(const QString &format = QString())
    {
        QString ret = QFileInfo(engine->executable()).baseName() + QStringLiteral("_output");
        if (!format.isEmpty())
            ret += QLatin1Char('_') + format;
        ret += QStringLiteral(".txt");
        return ret;
    }
};

QMap<QString, QStringList> Runner::deviceNames()
{
    QMap<QString, QStringList> deviceNames;
#ifndef RTRUNNER_NO_APPXLOCAL
    deviceNames.insert(QStringLiteral("Appx"), AppxLocalEngine::deviceNames());
#endif
#ifndef RTRUNNER_NO_APPXPHONE
    deviceNames.insert(QStringLiteral("Phone"), AppxPhoneEngine::deviceNames());
#endif
    return deviceNames;
}

Runner::Runner(const QString &app, const QStringList &arguments,
               const QString &profile, const QString &deviceName)
    : d_ptr(new RunnerPrivate)
{
    Q_D(Runner);
    d->isValid = false;
    d->app = app;
    d->arguments = arguments;
    d->profile = profile;

    bool deviceIndexKnown;
    d->deviceIndex = deviceName.toInt(&deviceIndexKnown);
#ifndef RTRUNNER_NO_APPXLOCAL
    if (!deviceIndexKnown) {
        d->deviceIndex = AppxLocalEngine::deviceNames().indexOf(deviceName);
        if (d->deviceIndex < 0)
            d->deviceIndex = 0;
    }
    if ((d->profile.isEmpty() || d->profile.toLower() == QStringLiteral("appx"))
            && AppxLocalEngine::canHandle(this)) {
        if (RunnerEngine *engine = AppxLocalEngine::create(this)) {
            d->engine.reset(engine);
            d->isValid = true;
            qCWarning(lcWinRtRunner) << "Using the Appx profile.";
            return;
        }
    }
#endif
#ifndef RTRUNNER_NO_APPXPHONE
    if (!deviceIndexKnown) {
        d->deviceIndex = AppxPhoneEngine::deviceNames().indexOf(deviceName);
        if (d->deviceIndex < 0)
            d->deviceIndex = 0;
    }
    if ((d->profile.isEmpty() || d->profile.toLower() == QStringLiteral("appxphone"))
            && AppxPhoneEngine::canHandle(this)) {
        if (RunnerEngine *engine = AppxPhoneEngine::create(this)) {
            d->engine.reset(engine);
            d->isValid = true;
            qCWarning(lcWinRtRunner) << "Using the AppxPhone profile.";
            return;
        }
    }
#endif
    // Place other engines here

    qCWarning(lcWinRtRunner) << "Unable to find a run profile for" << app << ".";
}

Runner::~Runner()
{
}

bool Runner::isValid() const
{
    Q_D(const Runner);
    return d->isValid;
}

QString Runner::app() const
{
    Q_D(const Runner);
    return d->app;
}

QStringList Runner::arguments() const
{
    Q_D(const Runner);
    return d->arguments;
}

int Runner::deviceIndex() const
{
    Q_D(const Runner);
    return d->deviceIndex;
}

bool Runner::install(bool removeFirst)
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    return d->engine->install(removeFirst);
}

bool Runner::remove()
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    return d->engine->remove();
}

bool Runner::start()
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    return d->engine->start();
}

bool Runner::enableDebugging(const QString &debuggerExecutable, const QString &debuggerArguments)
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    return d->engine->enableDebugging(debuggerExecutable, debuggerArguments);
}

bool Runner::disableDebugging()
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    return d->engine->disableDebugging();
}

bool Runner::setLoopbackExemptClientEnabled(bool enabled)
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    return d->engine->setLoopbackExemptClientEnabled(enabled);
}

bool Runner::setLoopbackExemptServerEnabled(bool enabled)
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    return d->engine->setLoopbackExemptServerEnabled(enabled);
}

bool Runner::setLoggingRules(const QByteArray &rules)
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    return d->engine->setLoggingRules(rules);
}

bool Runner::suspend()
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    return d->engine->suspend();
}

bool Runner::stop()
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    return d->engine->stop();
}

bool Runner::wait(int maxWaitTime)
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    return d->engine->waitForFinished(maxWaitTime);
}

bool Runner::setupTest()
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    // Fix-up output path
    int outputIndex = d->arguments.indexOf(QStringLiteral("-o"));
    // if no -o was given: Use the default location winrtrunner can read from and write on stdout
    if (outputIndex == -1) {
        RunnerPrivate::TestPaths out;
        out.localOutputFile = d->defaultOutputFileName();
        out.finalOutputFile = QLatin1Char('-');
        d->arguments.append(QStringLiteral("-o"));
        out.deviceOutputFile = d->engine->devicePath(out.localOutputFile);
        d->arguments.append(out.deviceOutputFile);
        d->testPaths.append(out);
    } else {
        while (outputIndex != -1) {
            ++outputIndex;
            QString format;
            RunnerPrivate::TestPaths out;
            if (d->arguments.size() <= outputIndex) {
                qCWarning(lcWinRtRunner) << "-o needs an extra parameter specifying the filename and optional format";
                return false;
            }

            QString output = d->arguments.at(outputIndex);
            int commaIndex = output.indexOf(QLatin1Char(','));
            // -o <name>,<format>
            if (commaIndex != -1) {
                format = output.mid(commaIndex + 1);
                output = output.left(commaIndex);
            }
            out.finalOutputFile = output;
            if (QFileInfo(output).isAbsolute() || output == QLatin1Char('-'))
                out.localOutputFile = d->defaultOutputFileName(format);
            else
                out.localOutputFile = output;
            out.deviceOutputFile = d->engine->devicePath(out.localOutputFile);
            d->arguments[outputIndex] = out.deviceOutputFile;
            if (!format.isEmpty())
                d->arguments[outputIndex] += QLatin1Char(',') + format;
            d->testPaths.append(out);
            outputIndex = d->arguments.indexOf(QStringLiteral("-o"), outputIndex);
        }
    }

    // Write a qt.conf to the executable directory
    QDir executableDir = QFileInfo(d->engine->executable()).absoluteDir();
    QFile qtConf(executableDir.absoluteFilePath(QStringLiteral("qt.conf")));
    if (!qtConf.exists()) {
        if (!qtConf.open(QFile::WriteOnly)) {
            qCWarning(lcWinRtRunner) << "Could not open qt.conf for writing.";
            return false;
        }
        qtConf.write(QByteArrayLiteral("[Paths]\nPlugins=/"));
    }

    return true;
}

bool Runner::collectTest()
{
    Q_D(Runner);
    Q_ASSERT(d->engine);

    // Fetch test output
    for (RunnerPrivate::TestPaths output : d->testPaths) {
        if (!d->engine->receiveFile(output.deviceOutputFile, output.localOutputFile)) {
            qCWarning(lcWinRtRunner).nospace().noquote()
                << "Unable to copy test output file \""
                << QDir::toNativeSeparators(output.deviceOutputFile)
                << "\" to local file \"" << QDir::toNativeSeparators(output.localOutputFile) << "\".";
            return false;
        }

        if (output.finalOutputFile == QLatin1Char('-')) {
            QFile testResults(output.localOutputFile);
            if (!testResults.open(QFile::ReadOnly)) {
                qCWarning(lcWinRtRunner) << "Unable to read test results:" << testResults.errorString();
                return false;
            }

            const QByteArray contents = testResults.readAll();
            std::fputs(contents.constData(), stdout);
        } else if (output.localOutputFile != output.finalOutputFile) {
            if (QFile::exists(output.finalOutputFile) && !QFile::remove(output.finalOutputFile)) {
                qCWarning(lcWinRtRunner) << "Could not remove file" << output.finalOutputFile;
                return false;
            }
            if (!QFile(output.localOutputFile).copy(output.finalOutputFile)) {
                qCWarning(lcWinRtRunner) << "Could not copy intermediate file" << output.localOutputFile
                    << "to final destination" << output.finalOutputFile;
                return false;
            }
        }
    }
    return true;
}

qint64 Runner::pid()
{
    Q_D(Runner);
    if (!d->engine)
        return -1;

    return d->engine->pid();
}

int Runner::exitCode()
{
    Q_D(Runner);
    if (!d->engine)
        return -1;

    return d->engine->exitCode();
}
