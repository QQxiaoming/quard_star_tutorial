/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the tools applications of the QtSerialBus module.
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

#include "canbusutil.h"
#include "sigtermhandler.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>
#include <QScopedPointer>

#include <signal.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("canbusutil"));
    QCoreApplication::setApplicationVersion(QStringLiteral(QT_VERSION_STR));

    QScopedPointer<SigTermHandler> s(SigTermHandler::instance());
    if (signal(SIGINT, SigTermHandler::handle) == SIG_ERR)
        return -1;
    QObject::connect(s.data(), &SigTermHandler::sigTermSignal, &app, &QCoreApplication::quit);

    QTextStream output(stdout);
    CanBusUtil util(output, app);

    QCommandLineParser parser;
    parser.setApplicationDescription(CanBusUtil::tr(
        "Sends arbitrary CAN bus frames.\n"
        "If the -l option is set, all received CAN bus frames are dumped."));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument(QStringLiteral("plugin"),
            CanBusUtil::tr("Plugin name to use. See --list-plugins."));

    parser.addPositionalArgument(QStringLiteral("device"),
            CanBusUtil::tr("Device to use."));

    parser.addPositionalArgument(QStringLiteral("data"),
            CanBusUtil::tr(
                "Data to send if -l is not specified. Format:\n"
                "\t\t<id>#{payload}          (CAN 2.0 data frames),\n"
                "\t\t<id>#Rxx                (CAN 2.0 RTR frames with xx bytes data length),\n"
                "\t\t<id>##[flags]{payload}  (CAN FD data frames),\n"
                "where {payload} has 0..8 (0..64 CAN FD) ASCII hex-value pairs, "
                "and flags is one optional ASCII hex char for CAN FD flags: "
                "1 = Bitrate Switch, 2 = Error State Indicator\n"
                "e.g. 1#1a2b3c\n"), QStringLiteral("[data]"));

    const QCommandLineOption listeningOption({"l", "listen"},
            CanBusUtil::tr("Start listening CAN data on device."));
    parser.addOption(listeningOption);

    const QCommandLineOption listOption({"L", "list-plugins"},
            CanBusUtil::tr("List all available plugins."));
    parser.addOption(listOption);

    const QCommandLineOption showTimeStampOption({"t", "timestamp"},
            CanBusUtil::tr("Show timestamp for each received CAN bus frame."));
    parser.addOption(showTimeStampOption);

    const QCommandLineOption showFdFlagsOption({"i", "info"},
            CanBusUtil::tr("Show extra info (CAN FD flags) for each received CAN bus frame."));
    parser.addOption(showFdFlagsOption);

    const QCommandLineOption listDevicesOption({"d", "devices"},
            CanBusUtil::tr("Show available CAN bus devices for the given plugin."));
    parser.addOption(listDevicesOption);

    const QCommandLineOption canFdOption({"f", "can-fd"},
            CanBusUtil::tr("Enable CAN FD functionality when listening."));
    parser.addOption(canFdOption);

    const QCommandLineOption loopbackOption({"c", "local-loopback"},
            CanBusUtil::tr("Transmits all sent frames to other local applications."));
    parser.addOption(loopbackOption);

    const QCommandLineOption receiveOwnOption({"o", "receive-own"},
            CanBusUtil::tr("Receive each sent frame on successful transmission."));
    parser.addOption(receiveOwnOption);

    const QCommandLineOption bitrateOption({"b", "bitrate"},
            CanBusUtil::tr("Set the CAN bus bitrate to the given value."),
            QStringLiteral("bitrate"));
    parser.addOption(bitrateOption);

    const QCommandLineOption dataBitrateOption({"a", "data-bitrate"},
            CanBusUtil::tr("Set the CAN FD data bitrate to the given value."),
            QStringLiteral("bitrate"));
    parser.addOption(dataBitrateOption);

    parser.process(app);

    if (parser.isSet(listOption))
        return util.printPlugins();

    QString data;
    const QStringList args = parser.positionalArguments();

    if (parser.isSet(canFdOption))
        util.setConfigurationParameter(QCanBusDevice::CanFdKey, true);
    if (parser.isSet(loopbackOption))
        util.setConfigurationParameter(QCanBusDevice::LoopbackKey, true);
    if (parser.isSet(receiveOwnOption))
        util.setConfigurationParameter(QCanBusDevice::ReceiveOwnKey, true);
    if (!parser.value(bitrateOption).isEmpty()) {
        util.setConfigurationParameter(QCanBusDevice::BitRateKey,
                                       parser.value(bitrateOption).toInt());
    }
    if (!parser.value(dataBitrateOption).isEmpty()) {
        util.setConfigurationParameter(QCanBusDevice::DataBitRateKey,
                                       parser.value(dataBitrateOption).toInt());
    }

    if (parser.isSet(listeningOption)) {
        util.setShowTimeStamp(parser.isSet(showTimeStampOption));
        util.setShowFdFlags(parser.isSet(showFdFlagsOption));
    } else if (args.size() == 3) {
        data = args.at(2);
    } else if (args.size() == 1 && parser.isSet(listDevicesOption)) {
        return util.printDevices(args.at(0));
    } else if (args.size() != 2) {
        output << CanBusUtil::tr("Invalid number of arguments (%1 given).").arg(args.size());
        output << endl << endl << parser.helpText();
        return 1;
    }

    if (!util.start(args.at(0), args.at(1), data))
        return -1;

    return app.exec();
}
