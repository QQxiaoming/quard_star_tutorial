/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include "qqmlvaluetype_p.h"
#include "qqmlmetatype_p.h"

#include <private/qqmlglobal_p.h>
#include <QtCore/qdebug.h>
#include <private/qmetaobjectbuilder_p.h>
#include <private/qqmlmodelindexvaluetype_p.h>
#include <private/qmetatype_p.h>

QT_BEGIN_NAMESPACE

namespace {

struct QQmlValueTypeFactoryImpl
{
    QQmlValueTypeFactoryImpl();
    ~QQmlValueTypeFactoryImpl();

    bool isValueType(int idx);

    const QMetaObject *metaObjectForMetaType(int);
    QQmlValueType *valueType(int);

    QQmlValueType *valueTypes[QVariant::UserType];
    QHash<int, QQmlValueType *> userTypes;
    QMutex mutex;

    QQmlValueType invalidValueType;
};

QQmlValueTypeFactoryImpl::QQmlValueTypeFactoryImpl()
{
    std::fill_n(valueTypes, int(QVariant::UserType), &invalidValueType);

    // See types wrapped in qqmlmodelindexvaluetype_p.h
    qRegisterMetaType<QItemSelectionRange>();
}

QQmlValueTypeFactoryImpl::~QQmlValueTypeFactoryImpl()
{
    for (QQmlValueType *type : valueTypes) {
        if (type != &invalidValueType)
            delete type;
    }
    qDeleteAll(userTypes);
}

bool isInternalType(int idx)
{
    // Qt internal types
    switch (idx) {
    case QMetaType::UnknownType:
    case QMetaType::QStringList:
    case QMetaType::QObjectStar:
    case QMetaType::VoidStar:
    case QMetaType::Nullptr:
    case QMetaType::QVariant:
    case QMetaType::QLocale:
    case QMetaType::QImage:  // scarce type, keep as QVariant
    case QMetaType::QPixmap: // scarce type, keep as QVariant
        return true;
    default:
        return false;
    }
}

bool QQmlValueTypeFactoryImpl::isValueType(int idx)
{
    if (idx < 0 || isInternalType(idx))
        return false;

    return valueType(idx) != nullptr;
}

const QMetaObject *QQmlValueTypeFactoryImpl::metaObjectForMetaType(int t)
{
    switch (t) {
    case QVariant::Point:
        return &QQmlPointValueType::staticMetaObject;
    case QVariant::PointF:
        return &QQmlPointFValueType::staticMetaObject;
    case QVariant::Size:
        return &QQmlSizeValueType::staticMetaObject;
    case QVariant::SizeF:
        return &QQmlSizeFValueType::staticMetaObject;
    case QVariant::Rect:
        return &QQmlRectValueType::staticMetaObject;
    case QVariant::RectF:
        return &QQmlRectFValueType::staticMetaObject;
    case QVariant::EasingCurve:
        return &QQmlEasingValueType::staticMetaObject;
    case QVariant::ModelIndex:
        return &QQmlModelIndexValueType::staticMetaObject;
    case QVariant::PersistentModelIndex:
        return &QQmlPersistentModelIndexValueType::staticMetaObject;
    default:
        if (t == qMetaTypeId<QItemSelectionRange>())
            return &QQmlItemSelectionRangeValueType::staticMetaObject;

        if (const QMetaObject *mo = QQml_valueTypeProvider()->metaObjectForMetaType(t))
            return mo;
        break;
    }

    QMetaType metaType(t);
    if (metaType.flags() & QMetaType::IsGadget)
        return metaType.metaObject();
    return nullptr;
}

QQmlValueType *QQmlValueTypeFactoryImpl::valueType(int idx)
{
    if (idx >= (int)QVariant::UserType) {
        // Protect the hash with a mutex
        mutex.lock();

        QHash<int, QQmlValueType *>::iterator it = userTypes.find(idx);
        if (it == userTypes.end()) {
            QQmlValueType *vt = nullptr;
            if (const QMetaObject *mo = metaObjectForMetaType(idx))
                vt = new QQmlValueType(idx, mo);
            it = userTypes.insert(idx, vt);
        }

        mutex.unlock();
        return *it;
    }

    QQmlValueType *rv = valueTypes[idx];
    if (rv == &invalidValueType) {
        // No need for mutex protection - the most we can lose is a valueType instance

        // TODO: Investigate the performance/memory characteristics of
        // removing the preallocated array
        if (isInternalType(idx))
            rv = valueTypes[idx] = nullptr;
        else if (const QMetaObject *mo = metaObjectForMetaType(idx))
            rv = valueTypes[idx] = new QQmlValueType(idx, mo);
        else
            rv = valueTypes[idx] = nullptr;
    }

    return rv;
}

}

Q_GLOBAL_STATIC(QQmlValueTypeFactoryImpl, factoryImpl);

bool QQmlValueTypeFactory::isValueType(int idx)
{
    return factoryImpl()->isValueType(idx);
}

QQmlValueType *QQmlValueTypeFactory::valueType(int idx)
{
    return factoryImpl()->valueType(idx);
}

const QMetaObject *QQmlValueTypeFactory::metaObjectForMetaType(int type)
{
    return factoryImpl()->metaObjectForMetaType(type);
}

void QQmlValueTypeFactory::registerValueTypes(const char *uri, int versionMajor, int versionMinor)
{
    qmlRegisterValueTypeEnums<QQmlEasingValueType>(uri, versionMajor, versionMinor, "Easing");
}

QQmlValueType::QQmlValueType() :
    _metaObject(nullptr),
    gadgetPtr(nullptr),
    metaType(QMetaType::UnknownType)
{
}

QQmlValueType::QQmlValueType(int typeId, const QMetaObject *gadgetMetaObject)
    : gadgetPtr(QMetaType::create(typeId))
    , typeId(typeId)
    , metaType(typeId)
{
    QObjectPrivate *op = QObjectPrivate::get(this);
    Q_ASSERT(!op->metaObject);
    op->metaObject = this;

    QMetaObjectBuilder builder(gadgetMetaObject);
    _metaObject = builder.toMetaObject();

    *static_cast<QMetaObject*>(this) = *_metaObject;
}

QQmlValueType::~QQmlValueType()
{
    QObjectPrivate *op = QObjectPrivate::get(this);
    Q_ASSERT(op->metaObject == nullptr || op->metaObject == this);
    op->metaObject = nullptr;
    ::free(const_cast<QMetaObject *>(_metaObject));
    metaType.destroy(gadgetPtr);
}

void QQmlValueType::read(QObject *obj, int idx)
{
    void *a[] = { gadgetPtr, nullptr };
    QMetaObject::metacall(obj, QMetaObject::ReadProperty, idx, a);
}

void QQmlValueType::write(QObject *obj, int idx, QQmlPropertyData::WriteFlags flags)
{
    Q_ASSERT(gadgetPtr);
    int status = -1;
    void *a[] = { gadgetPtr, nullptr, &status, &flags };
    QMetaObject::metacall(obj, QMetaObject::WriteProperty, idx, a);
}

QVariant QQmlValueType::value()
{
    Q_ASSERT(gadgetPtr);
    return QVariant(typeId, gadgetPtr);
}

void QQmlValueType::setValue(const QVariant &value)
{
    Q_ASSERT(typeId == value.userType());
    metaType.destruct(gadgetPtr);
    metaType.construct(gadgetPtr, value.constData());
}

QAbstractDynamicMetaObject *QQmlValueType::toDynamicMetaObject(QObject *)
{
    return this;
}

void QQmlValueType::objectDestroyed(QObject *)
{
}

int QQmlValueType::metaCall(QObject *, QMetaObject::Call type, int _id, void **argv)
{
    const QMetaObject *mo = _metaObject;
    QQmlMetaObject::resolveGadgetMethodOrPropertyIndex(type, &mo, &_id);
    mo->d.static_metacall(reinterpret_cast<QObject*>(gadgetPtr), type, _id, argv);
    return _id;
}

QString QQmlPointFValueType::toString() const
{
    return QString::asprintf("QPointF(%g, %g)", v.x(), v.y());
}

qreal QQmlPointFValueType::x() const
{
    return v.x();
}

qreal QQmlPointFValueType::y() const
{
    return v.y();
}

void QQmlPointFValueType::setX(qreal x)
{
    v.setX(x);
}

void QQmlPointFValueType::setY(qreal y)
{
    v.setY(y);
}


int QQmlPointValueType::x() const
{
    return v.x();
}

int QQmlPointValueType::y() const
{
    return v.y();
}

void QQmlPointValueType::setX(int x)
{
    v.setX(x);
}

void QQmlPointValueType::setY(int y)
{
    v.setY(y);
}


QString QQmlSizeFValueType::toString() const
{
    return QString::asprintf("QSizeF(%g, %g)", v.width(), v.height());
}

qreal QQmlSizeFValueType::width() const
{
    return v.width();
}

qreal QQmlSizeFValueType::height() const
{
    return v.height();
}

void QQmlSizeFValueType::setWidth(qreal w)
{
    v.setWidth(w);
}

void QQmlSizeFValueType::setHeight(qreal h)
{
    v.setHeight(h);
}


int QQmlSizeValueType::width() const
{
    return v.width();
}

int QQmlSizeValueType::height() const
{
    return v.height();
}

void QQmlSizeValueType::setWidth(int w)
{
    v.setWidth(w);
}

void QQmlSizeValueType::setHeight(int h)
{
    v.setHeight(h);
}

QString QQmlRectFValueType::toString() const
{
    return QString::asprintf("QRectF(%g, %g, %g, %g)", v.x(), v.y(), v.width(), v.height());
}

qreal QQmlRectFValueType::x() const
{
    return v.x();
}

qreal QQmlRectFValueType::y() const
{
    return v.y();
}

void QQmlRectFValueType::setX(qreal x)
{
    v.moveLeft(x);
}

void QQmlRectFValueType::setY(qreal y)
{
    v.moveTop(y);
}

qreal QQmlRectFValueType::width() const
{
    return v.width();
}

qreal QQmlRectFValueType::height() const
{
    return v.height();
}

void QQmlRectFValueType::setWidth(qreal w)
{
    v.setWidth(w);
}

void QQmlRectFValueType::setHeight(qreal h)
{
    v.setHeight(h);
}

qreal QQmlRectFValueType::left() const
{
    return v.left();
}

qreal QQmlRectFValueType::right() const
{
    return v.right();
}

qreal QQmlRectFValueType::top() const
{
    return v.top();
}

qreal QQmlRectFValueType::bottom() const
{
    return v.bottom();
}

int QQmlRectValueType::x() const
{
    return v.x();
}

int QQmlRectValueType::y() const
{
    return v.y();
}

void QQmlRectValueType::setX(int x)
{
    v.moveLeft(x);
}

void QQmlRectValueType::setY(int y)
{
    v.moveTop(y);
}

int QQmlRectValueType::width() const
{
    return v.width();
}

int QQmlRectValueType::height() const
{
    return v.height();
}

void QQmlRectValueType::setWidth(int w)
{
    v.setWidth(w);
}

void QQmlRectValueType::setHeight(int h)
{
    v.setHeight(h);
}

int QQmlRectValueType::left() const
{
    return v.left();
}

int QQmlRectValueType::right() const
{
    return v.right();
}

int QQmlRectValueType::top() const
{
    return v.top();
}

int QQmlRectValueType::bottom() const
{
    return v.bottom();
}

QQmlEasingValueType::Type QQmlEasingValueType::type() const
{
    return (QQmlEasingValueType::Type)v.type();
}

qreal QQmlEasingValueType::amplitude() const
{
    return v.amplitude();
}

qreal QQmlEasingValueType::overshoot() const
{
    return v.overshoot();
}

qreal QQmlEasingValueType::period() const
{
    return v.period();
}

void QQmlEasingValueType::setType(QQmlEasingValueType::Type type)
{
    v.setType((QEasingCurve::Type)type);
}

void QQmlEasingValueType::setAmplitude(qreal amplitude)
{
    v.setAmplitude(amplitude);
}

void QQmlEasingValueType::setOvershoot(qreal overshoot)
{
    v.setOvershoot(overshoot);
}

void QQmlEasingValueType::setPeriod(qreal period)
{
    v.setPeriod(period);
}

void QQmlEasingValueType::setBezierCurve(const QVariantList &customCurveVariant)
{
    if (customCurveVariant.isEmpty())
        return;

    if ((customCurveVariant.count() % 6) != 0)
        return;

    auto convert = [](const QVariant &v, qreal &r) {
        bool ok;
        r = v.toReal(&ok);
        return ok;
    };

    QEasingCurve newEasingCurve(QEasingCurve::BezierSpline);
    for (int i = 0, ei = customCurveVariant.size(); i < ei; i += 6) {
        qreal c1x, c1y, c2x, c2y, c3x, c3y;
        if (!convert(customCurveVariant.at(i    ), c1x)) return;
        if (!convert(customCurveVariant.at(i + 1), c1y)) return;
        if (!convert(customCurveVariant.at(i + 2), c2x)) return;
        if (!convert(customCurveVariant.at(i + 3), c2y)) return;
        if (!convert(customCurveVariant.at(i + 4), c3x)) return;
        if (!convert(customCurveVariant.at(i + 5), c3y)) return;

        const QPointF c1(c1x, c1y);
        const QPointF c2(c2x, c2y);
        const QPointF c3(c3x, c3y);

        newEasingCurve.addCubicBezierSegment(c1, c2, c3);
    }

    v = newEasingCurve;
}

QVariantList QQmlEasingValueType::bezierCurve() const
{
    QVariantList rv;
    const QVector<QPointF> points = v.toCubicSpline();
    rv.reserve(points.size() * 2);
    for (const auto &point : points)
        rv << QVariant(point.x()) << QVariant(point.y());
    return rv;
}

QT_END_NAMESPACE

#include "moc_qqmlvaluetype_p.cpp"
