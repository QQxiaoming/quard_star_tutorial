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

#include "qbluetoothserviceinfo.h"
#include "osxbtsdpinquiry_p.h"
#include "qbluetoothuuid.h"
#include "osxbtutility_p.h"

#include <QtCore/qvariant.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

SDPInquiryDelegate::~SDPInquiryDelegate()
{
}

namespace {

QBluetoothUuid sdp_element_to_uuid(IOBluetoothSDPDataElement *element)
{
    QT_BT_MAC_AUTORELEASEPOOL;

    if (!element || [element getTypeDescriptor] != kBluetoothSDPDataElementTypeUUID)
        return {};

    return qt_uuid([[element getUUIDValue] getUUIDWithLength:16]);
}

QBluetoothUuid extract_service_ID(IOBluetoothSDPServiceRecord *record)
{
    Q_ASSERT(record);

    QT_BT_MAC_AUTORELEASEPOOL;

    return sdp_element_to_uuid([record getAttributeDataElement:kBluetoothSDPAttributeIdentifierServiceID]);
}

QVector<QBluetoothUuid> extract_service_class_ID_list(IOBluetoothSDPServiceRecord *record)
{
    Q_ASSERT(record);

    QT_BT_MAC_AUTORELEASEPOOL;

    IOBluetoothSDPDataElement *const idList = [record getAttributeDataElement:kBluetoothSDPAttributeIdentifierServiceClassIDList];
    if (!idList || [idList getTypeDescriptor] != kBluetoothSDPDataElementTypeDataElementSequence)
        return {};

    QVector<QBluetoothUuid> uuids;
    NSArray *const arr = [idList getArrayValue];
    for (IOBluetoothSDPDataElement *dataElement in arr) {
        const auto qtUuid = sdp_element_to_uuid(dataElement);
        if (!qtUuid.isNull())
            uuids.push_back(qtUuid);
    }

    return uuids;
}

QBluetoothServiceInfo::Sequence service_class_ID_list_to_sequence(const QVector<QBluetoothUuid> &uuids)
{
    if (uuids.isEmpty())
        return {};

    QBluetoothServiceInfo::Sequence sequence;
    for (const auto &uuid : uuids) {
        Q_ASSERT(!uuid.isNull());
        sequence.append(QVariant::fromValue(uuid));
    }

    return sequence;
}

} // unnamed namespace

QVariant extract_attribute_value(IOBluetoothSDPDataElement *dataElement)
{
    Q_ASSERT_X(dataElement, Q_FUNC_INFO, "invalid data element (nil)");

    // TODO: error handling and diagnostic messages.

    // All "temporary" obj-c objects are autoreleased.
    QT_BT_MAC_AUTORELEASEPOOL;

    const BluetoothSDPDataElementTypeDescriptor typeDescriptor = [dataElement getTypeDescriptor];

    switch (typeDescriptor) {
    case kBluetoothSDPDataElementTypeNil:
        break;
    case kBluetoothSDPDataElementTypeUnsignedInt:
        return [[dataElement getNumberValue] unsignedIntValue];
    case kBluetoothSDPDataElementTypeSignedInt:
        return [[dataElement getNumberValue] intValue];
    case kBluetoothSDPDataElementTypeUUID:
        return QVariant::fromValue(sdp_element_to_uuid(dataElement));
    case kBluetoothSDPDataElementTypeString:
    case kBluetoothSDPDataElementTypeURL:
        return QString::fromNSString([dataElement getStringValue]);
    case kBluetoothSDPDataElementTypeBoolean:
        return [[dataElement getNumberValue] boolValue];
    case kBluetoothSDPDataElementTypeDataElementSequence:
    case kBluetoothSDPDataElementTypeDataElementAlternative: // TODO: check this!
        {
        QBluetoothServiceInfo::Sequence sequence;
        NSArray *const arr = [dataElement getArrayValue];
        for (IOBluetoothSDPDataElement *element in arr)
            sequence.append(extract_attribute_value(element));

        return QVariant::fromValue(sequence);
        }
        break;// Coding style.
    default:;
    }

    return QVariant();
}

void extract_service_record(IOBluetoothSDPServiceRecord *record, QBluetoothServiceInfo &serviceInfo)
{
    QT_BT_MAC_AUTORELEASEPOOL;

    if (!record)
        return;

    NSDictionary *const attributes = record.attributes;
    NSEnumerator *const keys = attributes.keyEnumerator;
    for (NSNumber *key in keys) {
        const quint16 attributeID = [key unsignedShortValue];
        IOBluetoothSDPDataElement *const element = [attributes objectForKey:key];
        const QVariant attributeValue = OSXBluetooth::extract_attribute_value(element);
        serviceInfo.setAttribute(attributeID, attributeValue);
    }

    const QBluetoothUuid serviceUuid = extract_service_ID(record);
    if (!serviceUuid.isNull())
        serviceInfo.setServiceUuid(serviceUuid);

    const QVector<QBluetoothUuid> uuids(extract_service_class_ID_list(record));
    const auto sequence = service_class_ID_list_to_sequence(uuids);
    if (!sequence.isEmpty())
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, sequence);
}

QList<QBluetoothUuid> extract_services_uuids(IOBluetoothDevice *device)
{
    QList<QBluetoothUuid> uuids;

    // All "temporary" obj-c objects are autoreleased.
    QT_BT_MAC_AUTORELEASEPOOL;

    if (!device || !device.services)
        return uuids;

    NSArray * const records = device.services;
    for (IOBluetoothSDPServiceRecord *record in records) {
        const QBluetoothUuid serviceID = extract_service_ID(record);
        if (!serviceID.isNull())
            uuids.push_back(serviceID);

        const QVector<QBluetoothUuid> idList(extract_service_class_ID_list(record));
        if (idList.size())
            uuids.append(idList.toList());
    }

    return uuids;
}

}

QT_END_NAMESPACE

QT_USE_NAMESPACE

using namespace OSXBluetooth;

@implementation QT_MANGLE_NAMESPACE(OSXBTSDPInquiry)
{
    QT_PREPEND_NAMESPACE(OSXBluetooth::SDPInquiryDelegate) *delegate;
    IOBluetoothDevice *device;
    bool isActive;
}

- (id)initWithDelegate:(SDPInquiryDelegate *)aDelegate
{
    Q_ASSERT_X(aDelegate, Q_FUNC_INFO, "invalid delegate (null)");

    if (self = [super init]) {
        delegate = aDelegate;
        isActive = false;
    }

    return self;
}

- (void)dealloc
{
    //[device closeConnection]; //??? - synchronous, "In the future this API will be changed to allow asynchronous operation."
    [device release];
    [super dealloc];
}

- (IOReturn)performSDPQueryWithDevice:(const QBluetoothAddress &)address
{
    Q_ASSERT_X(!isActive, Q_FUNC_INFO, "SDP query in progress");

    QList<QBluetoothUuid> emptyFilter;
    return [self performSDPQueryWithDevice:address filters:emptyFilter];
}

- (IOReturn)performSDPQueryWithDevice:(const QBluetoothAddress &)address
                              filters:(const QList<QBluetoothUuid> &)qtFilters
{
    Q_ASSERT_X(!isActive, Q_FUNC_INFO, "SDP query in progress");
    Q_ASSERT_X(!address.isNull(), Q_FUNC_INFO, "invalid target device address");

    QT_BT_MAC_AUTORELEASEPOOL;

    // We first try to allocate "filters":
    ObjCScopedPointer<NSMutableArray> array;
    if (qtFilters.size()) {
        array.reset([[NSMutableArray alloc] init]);
        if (!array) {
            qCCritical(QT_BT_OSX) << "failed to allocate an uuid filter";
            return kIOReturnError;
        }

        for (const QBluetoothUuid &qUuid : qtFilters) {
            ObjCStrongReference<IOBluetoothSDPUUID> uuid(iobluetooth_uuid(qUuid));
            if (uuid)
                [array addObject:uuid];
        }

        if (int([array count]) != qtFilters.size()) {
            qCCritical(QT_BT_OSX) << "failed to create an uuid filter";
            return kIOReturnError;
        }
    }

    const BluetoothDeviceAddress iobtAddress(iobluetooth_address(address));
    ObjCScopedPointer<IOBluetoothDevice> newDevice([[IOBluetoothDevice deviceWithAddress:&iobtAddress] retain]);
    if (!newDevice) {
        qCCritical(QT_BT_OSX) << "failed to create an IOBluetoothDevice object";
        return kIOReturnError;
    }

    ObjCScopedPointer<IOBluetoothDevice> oldDevice(device);
    device = newDevice.data();

    IOReturn result = kIOReturnSuccess;
    if (qtFilters.size())
        result = [device performSDPQuery:self uuids:array];
    else
        result = [device performSDPQuery:self];

    if (result != kIOReturnSuccess) {
        qCCritical(QT_BT_OSX) << "failed to start an SDP query";
        device = oldDevice.take();
    } else {
        isActive = true;
        newDevice.take();
    }

    return result;
}

- (void)stopSDPQuery
{
    // There is no API to stop it,
    // but there is a 'stop' member-function in Qt and
    // after it's called sdpQueryComplete must be somehow ignored.

    [device release];
    device = nil;
}

- (void)sdpQueryComplete:(IOBluetoothDevice *)aDevice status:(IOReturn)status
{
    // Can happen - there is no legal way to cancel an SDP query,
    // after the 'reset' device can never be
    // the same as the cancelled one.
    if (device != aDevice)
        return;

    Q_ASSERT_X(delegate, Q_FUNC_INFO, "invalid delegate (null)");

    isActive = false;

    if (status != kIOReturnSuccess)
        delegate->SDPInquiryError(aDevice, status);
    else
        delegate->SDPInquiryFinished(aDevice);
}

@end
