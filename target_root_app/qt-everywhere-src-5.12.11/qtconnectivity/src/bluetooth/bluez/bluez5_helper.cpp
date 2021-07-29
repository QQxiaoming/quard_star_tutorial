/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include <QtCore/QGlobalStatic>
#include <QtCore/QLoggingCategory>
#include <QtCore/QMap>
#include <QtCore/QVersionNumber>
#include <QtNetwork/private/qnet_unix_p.h>
#include "bluez5_helper_p.h"
#include "bluez_data_p.h"
#include "objectmanager_p.h"
#include "properties_p.h"
#include "adapter1_bluez5_p.h"
#include "manager_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

typedef enum Bluez5TestResultType
{
    BluezVersionUnknown,
    BluezVersion4,
    BluezVersion5,
    BluezNotAvailable
} Bluez5TestResult;

Q_GLOBAL_STATIC_WITH_ARGS(Bluez5TestResult, bluezVersion, (BluezVersionUnknown));
Q_GLOBAL_STATIC_WITH_ARGS(QVersionNumber, bluezDaemonVersion, (QVersionNumber()));

bool isBluez5()
{
    if (*bluezVersion() == BluezVersionUnknown) {
        OrgFreedesktopDBusObjectManagerInterface manager(QStringLiteral("org.bluez"),
                                                         QStringLiteral("/"),
                                                         QDBusConnection::systemBus());

        qDBusRegisterMetaType<InterfaceList>();
        qDBusRegisterMetaType<ManagedObjectList>();
        qDBusRegisterMetaType<ManufacturerDataList>();

        QDBusPendingReply<ManagedObjectList> reply = manager.GetManagedObjects();
        reply.waitForFinished();
        if (reply.isError()) {
            // not Bluez 5.x
            OrgBluezManagerInterface manager_bluez4(QStringLiteral("org.bluez"),
                                             QStringLiteral("/"),
                                             QDBusConnection::systemBus());
            QDBusPendingReply<QList<QDBusObjectPath> > reply
                    = manager_bluez4.ListAdapters();
            reply.waitForFinished();
            if (reply.isError()) {
                *bluezVersion() = BluezNotAvailable;
                qWarning() << "Cannot find a running Bluez. Please check the Bluez installation.";
            } else {
                *bluezVersion() = BluezVersion4;
                qCDebug(QT_BT_BLUEZ) << "Bluez 4 detected.";
            }
        } else {
            *bluezVersion() = BluezVersion5;
            qCDebug(QT_BT_BLUEZ) << "Bluez 5 detected.";
        }
    }

    return (*bluezVersion() == BluezVersion5);
}

/*
    Checks that the mandatory Bluetooth HCI ioctls are offered
    by Linux kernel. Returns \c true if the ictls are available; otherwise \c false.

    Mandatory ioctls:
            - HCIGETCONNLIST
            - HCIGETDEVINFO
            - HCIGETDEVLIST
 */
bool mandatoryHciIoctlsAvailable()
{
    // open hci socket
    int hciSocket = ::qt_safe_socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
    if (hciSocket < 0) {
        qCWarning(QT_BT_BLUEZ) << "Cannot open HCI socket:" << qt_error_string(errno);
        return false;
    }

    // check HCIGETDEVLIST & HCIGETDEVLIST
    struct hci_dev_req *devRequest = nullptr;
    struct hci_dev_list_req *devRequestList = nullptr;
    struct hci_dev_info devInfo;
    const int devListSize = sizeof(struct hci_dev_list_req)
                        + HCI_MAX_DEV * sizeof(struct hci_dev_req);

    devRequestList = (hci_dev_list_req *) malloc(devListSize);
    if (!devRequestList) {
        qt_safe_close(hciSocket);
        return false; // if we cannot malloc nothing will help anyway
    }

    QScopedPointer<hci_dev_list_req, QScopedPointerPodDeleter> pDevList(devRequestList);
    memset(pDevList.data(), 0, devListSize);
    pDevList->dev_num = HCI_MAX_DEV;
    devRequest = pDevList->dev_req;

    if (qt_safe_ioctl(hciSocket, HCIGETDEVLIST, devRequestList) < 0) {
        qt_safe_close(hciSocket);
        qCWarning(QT_BT_BLUEZ) << "HCI icotl HCIGETDEVLIST:" << qt_error_string(errno);
        return false;
    }

    if (devRequestList->dev_num > 0) {
        devInfo.dev_id = devRequest->dev_id;
        if (qt_safe_ioctl(hciSocket, HCIGETDEVINFO, &devInfo) < 0) {
            qt_safe_close(hciSocket);
            qCWarning(QT_BT_BLUEZ) << "HCI icotl HCIGETDEVINFO:" << qt_error_string(errno);
            return false;
        }
    }

    // check HCIGETCONNLIST
    const int maxNoOfConnections = 20;
    hci_conn_list_req *infoList = nullptr;
    infoList = (hci_conn_list_req *)
            malloc(sizeof(hci_conn_list_req) + maxNoOfConnections * sizeof(hci_conn_info));

    if (!infoList) {
        qt_safe_close(hciSocket);
        return false;
    }

    QScopedPointer<hci_conn_list_req, QScopedPointerPodDeleter> pInfoList(infoList);
    pInfoList->conn_num = maxNoOfConnections;
    pInfoList->dev_id = devInfo.dev_id;

    if (qt_safe_ioctl(hciSocket, HCIGETCONNLIST, (void *) infoList) < 0) {
        qCWarning(QT_BT_BLUEZ) << "HCI icotl HCIGETCONNLIST:" << qt_error_string(errno);
        qt_safe_close(hciSocket);
        return false;
    }

    qt_safe_close(hciSocket);
    return true;
}

/*!
 * This function returns the version of bluetoothd in use on the system.
 * This is required to determine which QLEControllerPrivate implementation
 * is required. The following version tags are of significance:
 *
 * Version < 4.0 -> QLEControllerPrivateCommon
 * Version < 5.42 -> QLEControllerPrivateBluez
 * Version >= 5.42 -> QLEControllerPrivateBluezDBus
 *
 * This function utilizes a singleton pattern. It always returns a cached
 * version tag which is determined on first call. This is necessary to
 * avoid continuesly running the somewhat expensive tests.
 *
 * The function must never return a null QVersionNumber.
 */
QVersionNumber bluetoothdVersion()
{
    if (bluezDaemonVersion()->isNull()) {
        // Register DBus specific  meta types (copied from isBluez5())
        // Not all code paths run through isBluez5() but still need the
        // registration.
        qDBusRegisterMetaType<InterfaceList>();
        qDBusRegisterMetaType<ManagedObjectList>();
        qDBusRegisterMetaType<ManufacturerDataList>();

        qCDebug(QT_BT_BLUEZ) << "Detecting bluetoothd version";
        //Order of matching
        // 1. Pick whatever the user decides via BLUETOOTH_FORCE_DBUS_LE_VERSION
        // Set version to below version 5.42 to use custom/old GATT stack implementation
        const QString version = qEnvironmentVariable("BLUETOOTH_FORCE_DBUS_LE_VERSION");
        if (!version.isNull()) {
            const QVersionNumber vn = QVersionNumber::fromString(version);
            if (!vn.isNull()) {
                *bluezDaemonVersion() = vn;
                qCDebug(QT_BT_BLUEZ) << "Forcing Bluez LE API selection:"
                                     << bluezDaemonVersion()->toString();
            }
        }

        // 2. Find bluetoothd binary and check "bluetoothd --version"
        if (bluezDaemonVersion()->isNull() && qt_haveLinuxProcfs()) {
            QDBusConnection session = QDBusConnection::systemBus();
            qint64 pid = session.interface()->servicePid(QStringLiteral("org.bluez")).value();
            QByteArray buffer;

            auto determineBinaryVersion = [](const QString &binary) -> QVersionNumber {
                QProcess process;
                process.start(binary, {QStringLiteral("--version")});
                process.waitForFinished();

                const QString version = QString::fromLocal8Bit(process.readAll());
                const QVersionNumber vn = QVersionNumber::fromString(version);
                if (!vn.isNull())
                    qCDebug(QT_BT_BLUEZ) << "Detected bluetoothd version" << vn;
                return vn;
            };

            //try reading /proc/<pid>/exe first -> requires process owner
            qCDebug(QT_BT_BLUEZ) << "Using /proc/<pid>/exe";
            const QString procExe = QStringLiteral("/proc/%1/exe").arg(pid);
            const QVersionNumber vn = determineBinaryVersion(procExe);
            if (!vn.isNull())
                *bluezDaemonVersion() = vn;

            if (bluezDaemonVersion()->isNull()) {
                qCDebug(QT_BT_BLUEZ) << "Using /proc/<pid>/cmdline";
                //try to reading /proc/<pid>/cmdline (does not require additional process rights)
                QFile procFile(QStringLiteral("/proc/%1/cmdline").arg(pid));
                if (procFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
                    buffer = procFile.readAll();
                    procFile.close();

                    // cmdline params separated by character \0 -> first is bluetoothd binary
                    const QString binary = QString::fromLocal8Bit(buffer.split('\0').at(0));
                    QFileInfo info(binary);
                    if (info.isExecutable())
                        *bluezDaemonVersion() = determineBinaryVersion(binary);
                    else
                        qCDebug(QT_BT_BLUEZ) << "Cannot determine bluetoothd version via cmdline:"
                                             << binary;
                }
            }
        }

        // 3. Fall back to custom ATT backend, if possible?
        if (bluezDaemonVersion()->isNull()) {
            // Check mandatory HCI ioctls are available
            if (mandatoryHciIoctlsAvailable()) {
                // default 4.0 for now -> implies custom (G)ATT implementation
                *bluezDaemonVersion() = QVersionNumber(4, 0);
            }
        }

        // 4. Ultimate fallback -> enable dummy backend
        if (bluezDaemonVersion()->isNull()) {
            // version 3 represents disabled BTLE
            // bluezDaemonVersion should not be null to avoid repeated version tests
            *bluezDaemonVersion() = QVersionNumber(3, 0);
            qCWarning(QT_BT_BLUEZ) << "Cannot determine bluetoothd version and required Bluetooth HCI ioctols";
            qCWarning(QT_BT_BLUEZ) << "Disabling Qt Bluetooth LE feature";
        }

        qCDebug(QT_BT_BLUEZ) << "Bluetoothd:" << bluezDaemonVersion()->toString();
    }

    Q_ASSERT(!bluezDaemonVersion()->isNull());
    return *bluezDaemonVersion();
}

struct AdapterData
{
public:
    AdapterData() : reference(1), wasListeningAlready(false) {}

    int reference;
    bool wasListeningAlready;
    OrgFreedesktopDBusPropertiesInterface *propteryListener = nullptr;
};

class QtBluezDiscoveryManagerPrivate
{
public:
    QMap<QString, AdapterData *> references;
    OrgFreedesktopDBusObjectManagerInterface *manager = nullptr;
};

Q_GLOBAL_STATIC(QtBluezDiscoveryManager, discoveryManager)

/*!
    \internal
    \class QtBluezDiscoveryManager

    This class manages the access to "org.bluez.Adapter1::Start/StopDiscovery.

    The flag is a system flag. We want to ensure that the various Qt classes don't turn
    the flag on and off and thereby get into their way. If some other system component
    changes the flag (e.g. adapter removed) we notify all Qt classes about the change by emitting
    \l discoveryInterrupted(QString). Classes should indicate this via an appropriate
    error message to the user.

    Once the signal was emitted, all existing requests for discovery mode on the same adapter
    have to be renewed via \l registerDiscoveryInterest(QString).
*/

QtBluezDiscoveryManager::QtBluezDiscoveryManager(QObject *parent) :
    QObject(parent)
{
    qCDebug(QT_BT_BLUEZ) << "Creating QtBluezDiscoveryManager";
    d = new QtBluezDiscoveryManagerPrivate();

    d->manager = new OrgFreedesktopDBusObjectManagerInterface(
                QStringLiteral("org.bluez"), QStringLiteral("/"),
                QDBusConnection::systemBus(), this);
    connect(d->manager, SIGNAL(InterfacesRemoved(QDBusObjectPath,QStringList)),
            SLOT(InterfacesRemoved(QDBusObjectPath,QStringList)));
}

QtBluezDiscoveryManager::~QtBluezDiscoveryManager()
{
    qCDebug(QT_BT_BLUEZ) << "Destroying QtBluezDiscoveryManager";

    const QList<QString> adapterPaths = d->references.keys();
    for (const QString &adapterPath : adapterPaths) {
        AdapterData *data = d->references.take(adapterPath);
        delete data->propteryListener;

        // turn discovery off if it wasn't on already
        if (!data->wasListeningAlready) {
            OrgBluezAdapter1Interface iface(QStringLiteral("org.bluez"), adapterPath,
                                            QDBusConnection::systemBus());
            iface.StopDiscovery();
        }

        delete data;
    }

    delete d;
}

QtBluezDiscoveryManager *QtBluezDiscoveryManager::instance()
{
    if (isBluez5())
        return discoveryManager();

    Q_ASSERT(false);
    return nullptr;
}

bool QtBluezDiscoveryManager::registerDiscoveryInterest(const QString &adapterPath)
{
    if (adapterPath.isEmpty())
        return false;

    // already monitored adapter? -> increase ref count -> done
    if (d->references.contains(adapterPath)) {
        d->references[adapterPath]->reference++;
        return true;
    }

    AdapterData *data = new AdapterData();

    OrgFreedesktopDBusPropertiesInterface *propIface = new OrgFreedesktopDBusPropertiesInterface(
                QStringLiteral("org.bluez"), adapterPath, QDBusConnection::systemBus());
    connect(propIface, SIGNAL(PropertiesChanged(QString,QVariantMap,QStringList)),
            SLOT(PropertiesChanged(QString,QVariantMap,QStringList)));
    data->propteryListener = propIface;

    OrgBluezAdapter1Interface iface(QStringLiteral("org.bluez"), adapterPath,
                                    QDBusConnection::systemBus());
    data->wasListeningAlready = iface.discovering();

    d->references[adapterPath] = data;

    if (!data->wasListeningAlready)
        iface.StartDiscovery();

    return true;
}

void QtBluezDiscoveryManager::unregisterDiscoveryInterest(const QString &adapterPath)
{
    if (!d->references.contains(adapterPath))
        return;

    AdapterData *data = d->references[adapterPath];
    data->reference--;

    if (data->reference > 0) // more than one client requested discovery mode
        return;

    d->references.remove(adapterPath);
    if (!data->wasListeningAlready) { // Qt turned discovery mode on, Qt has to turn it off again
        OrgBluezAdapter1Interface iface(QStringLiteral("org.bluez"), adapterPath,
                                        QDBusConnection::systemBus());
        iface.StopDiscovery();
    }

    delete data->propteryListener;
    delete data;
}

//void QtBluezDiscoveryManager::dumpState() const
//{
//    qCDebug(QT_BT_BLUEZ) << "-------------------------";
//    if (d->references.isEmpty()) {
//        qCDebug(QT_BT_BLUEZ) << "No running registration";
//    } else {
//        const QList<QString> paths = d->references.keys();
//        for (const QString &path : paths) {
//            qCDebug(QT_BT_BLUEZ) << path << "->" << d->references[path]->reference;
//        }
//    }
//    qCDebug(QT_BT_BLUEZ) << "-------------------------";
//}

void QtBluezDiscoveryManager::InterfacesRemoved(const QDBusObjectPath &object_path,
                                                const QStringList &interfaces)
{
    if (!d->references.contains(object_path.path())
            || !interfaces.contains(QStringLiteral("org.bluez.Adapter1")))
        return;

    removeAdapterFromMonitoring(object_path.path());
}

void QtBluezDiscoveryManager::PropertiesChanged(const QString &interface,
                                                const QVariantMap &changed_properties,
                                                const QStringList &invalidated_properties)
{
    Q_UNUSED(invalidated_properties);

    OrgFreedesktopDBusPropertiesInterface *propIface =
            qobject_cast<OrgFreedesktopDBusPropertiesInterface *>(sender());

    if (!propIface)
        return;

    if (interface == QStringLiteral("org.bluez.Adapter1")
            && d->references.contains(propIface->path())
            && changed_properties.contains(QStringLiteral("Discovering"))) {
        bool isDiscovering = changed_properties.value(QStringLiteral("Discovering")).toBool();
        if (!isDiscovering) {

            /*
              Once we stop the Discovering flag will switch a few ms later. This comes through this code
              path. If a new device discovery is started while we are still
              waiting for the flag change signal, then the new device discovery will be aborted prematurely.
              To compensate we check whether there was renewed interest.
             */

            AdapterData *data = d->references[propIface->path()];
            if (!data) {
                removeAdapterFromMonitoring(propIface->path());
            } else {
                OrgBluezAdapter1Interface iface(QStringLiteral("org.bluez"), propIface->path(),
                                                QDBusConnection::systemBus());
                iface.StartDiscovery();
            }
        }
    }
}

void QtBluezDiscoveryManager::removeAdapterFromMonitoring(const QString &dbusPath)
{
    // remove adapter from monitoring
    AdapterData *data = d->references.take(dbusPath);
    delete data->propteryListener;
    delete data;

    emit discoveryInterrupted(dbusPath);
}

/*!
    Finds the path for the local adapter with \a wantedAddress or an empty string
    if no local adapter with the given address can be found.
    If \a wantedAddress is \c null it returns the first/default adapter or an empty
    string if none is available.

    If \a ok is false the lookup was aborted due to a dbus error and this function
    returns an empty string.
 */
QString findAdapterForAddress(const QBluetoothAddress &wantedAddress, bool *ok = nullptr)
{
    OrgFreedesktopDBusObjectManagerInterface manager(QStringLiteral("org.bluez"),
                                                     QStringLiteral("/"),
                                                     QDBusConnection::systemBus());

    QDBusPendingReply<ManagedObjectList> reply = manager.GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError()) {
        if (ok)
            *ok = false;

        return QString();
    }

    typedef QPair<QString, QBluetoothAddress> AddressForPathType;
    QList<AddressForPathType> localAdapters;

    ManagedObjectList managedObjectList = reply.value();
    for (ManagedObjectList::const_iterator it = managedObjectList.constBegin(); it != managedObjectList.constEnd(); ++it) {
        const QDBusObjectPath &path = it.key();
        const InterfaceList &ifaceList = it.value();

        for (InterfaceList::const_iterator jt = ifaceList.constBegin(); jt != ifaceList.constEnd(); ++jt) {
            const QString &iface = jt.key();

            if (iface == QStringLiteral("org.bluez.Adapter1")) {
                AddressForPathType pair;
                pair.first = path.path();
                pair.second = QBluetoothAddress(ifaceList.value(iface).value(
                                          QStringLiteral("Address")).toString());
                if (!pair.second.isNull())
                    localAdapters.append(pair);
                break;
            }
        }
    }

    if (ok)
        *ok = true;

    if (localAdapters.isEmpty())
        return QString(); // -> no local adapter found

    if (wantedAddress.isNull())
        return localAdapters.front().first; // -> return first found adapter

    for (const AddressForPathType &pair : qAsConst(localAdapters)) {
        if (pair.second == wantedAddress)
            return pair.first; // -> found local adapter with wanted address
    }

    return QString(); // nothing matching found
}

/*
    Removes every character that cannot be used in QDbusObjectPath

    See QDbusUtil::isValidObjectPath(QString) for more details.
 */
QString sanitizeNameForDBus(const QString &text)
{
    QString appName = text;
    for (int i = 0; i < appName.length(); i++) {
        ushort us = appName[i].unicode();
        bool valid = (us >= 'a' && us <= 'z')
                      || (us >= 'A' && us <= 'Z')
                      || (us >= '0' && us <= '9')
                      || (us == '_');

        if (!valid)
            appName[i] = QLatin1Char('_');
    }

    return appName;
}

QT_END_NAMESPACE
