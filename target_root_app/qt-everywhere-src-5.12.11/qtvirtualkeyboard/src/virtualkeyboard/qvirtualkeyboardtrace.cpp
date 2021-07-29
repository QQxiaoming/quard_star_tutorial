/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Virtual Keyboard module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtVirtualKeyboard/qvirtualkeyboardtrace.h>
#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QVirtualKeyboardTracePrivate : public QObjectPrivate
{
public:
    QVirtualKeyboardTracePrivate() :
        QObjectPrivate(),
        traceId(0),
        final(false),
        canceled(false),
        opacity(1.0)
    { }

    int traceId;
    QVariantList points;
    QMap<QString, QVariantList> channels;
    bool final;
    bool canceled;
    qreal opacity;
};

/*!
    \class QVirtualKeyboardTrace
    \inmodule QtVirtualKeyboard
    \since QtQuick.VirtualKeyboard 2.0
    \brief Trace is a data model for touch input data.

    Trace provides the data model for coordinate data and other
    optional data associated with a single stroke.

    A typical use case for the trace object is as follows:
    \list
        \li TraceInputArea or other input device initiates
            the trace event by calling \l {InputEngine::traceBegin()}
            {InputEngine.traceBegin()} method.
        \li If the current input method accepts the event it creates
            a trace object and configures the required data channels
            (if any).
        \li TraceInputArea collects the data for the Trace object.
        \li TraceInputArea calls the \l {InputEngine::traceEnd()}
            {InputEngine.traceEnd()} method to finish the trace and
            passing the trace object back to input method.
        \li The input method processes the data and discards the object
            when it is no longer needed.
    \endlist

    The coordinate data is retrieved using the points() function.

    In addition to coordinate based data, it is possible
    to attach an arbitrary data channel for each data point.

    The data channels must be defined before the points are added.
    The data channels supported by the TraceInputArea are listed below:

    \list
        \li \c "t" Collects time for each data point. The time is
            the number of milliseconds since 1970/01/01:
    \endlist

    For example, to configure the object to collect the times for
    each point:

    \code
        QVirtualKeyboardTrace *trace = new QVirtualKeyboardTrace(this);
        trace->setChannels(QStringList() << "t");
    \endcode

    The collected data can be accessed using the channelData() function:

    \code
        QVariantList timeData = trace->channelData("t");
    \endcode

    QVirtualKeyboardTrace objects are owned by their creator, which is the input method in
    normal case. This means the objects are constructed in the
    \l {InputMethod::traceBegin()}{InputMethod.traceBegin()} (QML) method.

    By definition, the trace object can be destroyed at earliest in the
    \l {InputMethod::traceEnd()}{InputMethod.traceEnd()} (QML) method.
*/

/*!
    \qmltype Trace
    \instantiates QVirtualKeyboardTrace
    \inqmlmodule QtQuick.VirtualKeyboard
    \ingroup qtvirtualkeyboard-qml
    \since QtQuick.VirtualKeyboard 2.0
    \brief Trace is a data model for touch input data.

    Trace provides the data model for coordinate data and other
    optional data associated with a single stroke.

    A typical use case for the trace object is as follows:
    \list
        \li TraceInputArea or other input device initiates
            the trace event by calling \l {InputEngine::traceBegin()}
            {InputEngine.traceBegin()} method.
        \li If the current input method accepts the event it creates
            a trace object and configures the required data channels
            (if any).
        \li TraceInputArea collects the data for the trace object.
        \li TraceInputArea calls the \l {InputEngine::traceEnd()}
            {InputEngine.traceEnd()} method to finish the trace and
            passing the trace object back to input method.
        \li The input method processes the data and discards the object
            when it is no longer needed.
    \endlist

    The coordinate data is retrieved using the points() function.

    In addition to coordinate based data, it is possible
    to attach an arbitrary data channel for each data point.

    The data channels must be defined before the points are added.
    The data channels supported by the TraceInputArea are listed below:

    \list
        \li \c "t" Collects time for each data point. The time is
            the number of milliseconds since 1970/01/01:
    \endlist

    For example, to configure the object to collect the times for
    each point:

    \code
        QVirtualKeyboardTrace *trace = new QVirtualKeyboardTrace(this);
        trace->setChannels(QStringList() << "t");
    \endcode

    The collected data can be accessed using the channelData() function:

    \code
        QVariantList timeData = trace->channelData("t");
    \endcode

    Trace objects are owned by their creator, which is the input method in
    normal case. This means the objects are constructed in the
    \l {InputMethod::traceBegin()}{InputMethod.traceBegin()} (QML) method.

    By definition, the trace object can be destroyed at earliest in the
    \l {InputMethod::traceEnd()}{InputMethod.traceEnd()} (QML) method.
*/

/*! \internal */
QVirtualKeyboardTrace::QVirtualKeyboardTrace(QObject *parent) :
    QObject(*new QVirtualKeyboardTracePrivate(), parent)
{
}

/*! \internal */
QVirtualKeyboardTrace::~QVirtualKeyboardTrace()
{
}

int QVirtualKeyboardTrace::traceId() const
{
    Q_D(const QVirtualKeyboardTrace);
    return d->traceId;
}

void QVirtualKeyboardTrace::setTraceId(int id)
{
    Q_D(QVirtualKeyboardTrace);
    if (d->traceId != id) {
        d->traceId = id;
        emit traceIdChanged(id);
    }
}

QStringList QVirtualKeyboardTrace::channels() const
{
    Q_D(const QVirtualKeyboardTrace);
    return d->channels.keys();
}

void QVirtualKeyboardTrace::setChannels(const QStringList &channels)
{
    Q_D(QVirtualKeyboardTrace);
    Q_ASSERT(d->points.isEmpty());
    if (d->points.isEmpty()) {
        d->channels.clear();
        for (QStringList::ConstIterator i = channels.constBegin();
             i != channels.constEnd(); i++) {
            d->channels[*i] = QVariantList();
        }
        emit channelsChanged();
    }
}

int QVirtualKeyboardTrace::length() const
{
    Q_D(const QVirtualKeyboardTrace);
    return d->points.size();
}

/*! \qmlmethod var Trace::points(int pos, int count)

    Returns list of points. If no parameters are given, the
    function returns all the points.

    If the \a pos parameter is given, the function returns points starting
    at the position. The \a count parameter limits how many points are
    returned.

    The returned list contains \c point types.
*/

/*! Returns list of points. If no parameters are given, the
    method returns all the data.

    If the \a pos parameter is given, the method returns points starting
    at the position. The \a count parameter limits how many points are
    returned.

    The returned list contains QPointF types.
*/

QVariantList QVirtualKeyboardTrace::points(int pos, int count) const
{
    Q_D(const QVirtualKeyboardTrace);
    return d->points.mid(pos, count);
}

/*! \qmlmethod int Trace::addPoint(point point)

    Adds a \a point to the Trace.

    The method returns index of the point added, or -1 if
    the points cannot be added (i.e. the \l final is true).

    \note The returned index is required to associate additional
    data with the point using the setChannelData() function.
*/

/*! Adds a \a point to the QVirtualKeyboardTrace.

    The method returns index of the point added, or -1 if
    the points cannot be added (i.e. the \l final is true).

    \note The returned index is required to associate additional
    data with the point using the setChannelData() method.
*/

int QVirtualKeyboardTrace::addPoint(const QPointF &point)
{
    Q_D(QVirtualKeyboardTrace);
    int index;
    if (!d->final) {
        index = d->points.size();
        d->points.append(point);
        emit lengthChanged(d->points.size());
    } else {
        index = -1;
    }
    return index;
}

/*! \qmlmethod void Trace::setChannelData(int index, string channel, var data)

    Sets \a data for the point at \a index in the given data \a channel.

    If this method is not called for each data point, the channel data
    will be padded with empty values. However, the data cannot be added at
    arbitrary index, i.e., it must be added in synchronously with the point data.
*/

/*! Sets \a data for the point at \a index in the given data \a channel.

    If this method is not called for each data point, the channel data
    will be padded with empty values. However, the data cannot be added at
    arbitrary index, i.e., it must be added in synchronously with the point data.
*/

void QVirtualKeyboardTrace::setChannelData(const QString &channel, int index, const QVariant &data)
{
    Q_D(QVirtualKeyboardTrace);
    if (!d->final && (index + 1) == d->points.size() && d->channels.contains(channel)) {
        QVariantList &channelData = d->channels[channel];
        while (index > channelData.size())
            channelData.append(QVariant());
        if (index == channelData.size())
            channelData.append(data);
    }
}

/*! \qmlmethod var Trace::channelData(string channel, int pos, int count)

    Returns data from the specified \a channel. If no other parameters
    are given, the function returns all the data.

    If the \a pos parameter is given, the function returns data starting
    at the position. The \a count parameter limits how many items are
    returned.
*/

/*! Returns data from the specified \a channel. If no other parameters
    are given, the method returns all the data.

    If the \a pos parameter is given, the method returns data starting
    at the position. The \a count parameter limits how many items are
    returned.
*/

QVariantList QVirtualKeyboardTrace::channelData(const QString &channel, int pos, int count) const
{
    Q_D(const QVirtualKeyboardTrace);
    return d->channels.value(channel).mid(pos, count);
}

bool QVirtualKeyboardTrace::isFinal() const
{
    Q_D(const QVirtualKeyboardTrace);
    return d->final;
}

void QVirtualKeyboardTrace::setFinal(bool final)
{
    Q_D(QVirtualKeyboardTrace);
    if (d->final != final) {
        d->final = final;
        emit finalChanged(final);
    }
}

bool QVirtualKeyboardTrace::isCanceled() const
{
    Q_D(const QVirtualKeyboardTrace);
    return d->canceled;
}

void QVirtualKeyboardTrace::setCanceled(bool canceled)
{
    Q_D(QVirtualKeyboardTrace);
    if (d->canceled != canceled) {
        d->canceled = canceled;
        emit canceledChanged(canceled);
    }
}

qreal QVirtualKeyboardTrace::opacity() const
{
    Q_D(const QVirtualKeyboardTrace);
    return d->opacity;
}

void QVirtualKeyboardTrace::setOpacity(qreal opacity)
{
    Q_D(QVirtualKeyboardTrace);
    if (d->opacity != opacity) {
        d->opacity = opacity;
        emit opacityChanged(opacity);
    }
}

/*! \qmlproperty int Trace::traceId

    Unique id of this Trace.
*/

/*! \property QVirtualKeyboardTrace::traceId
    \brief unique id of this QVirtualKeyboardTrace.
*/

/*! \qmlproperty list<strings> Trace::channels

    List of additional data channels in the Trace.
    This property must be initialized before the data
    is added.
*/

/*! \property QVirtualKeyboardTrace::channels
    \brief list of data channels in the QQTrace.

    This property must be initialized before the data
    is added.
*/

/*! \qmlproperty int QVirtualKeyboardTrace::length

    The number of points in the QVirtualKeyboardTrace.
*/

/*! \property QVirtualKeyboardTrace::length
    \brief the number of of points in the QVirtualKeyboardTrace.
*/

/*! \qmlproperty bool Trace::final

    This property defines whether the Trace can accept more data.
    If the value is \c true, no more data is accepted.
*/

/*! \property QVirtualKeyboardTrace::final
    \brief defines whether the QVirtualKeyboardTrace can accept more data.
    If the value is \c true, no more data is accepted.
*/

/*! \qmlproperty bool Trace::canceled

    This property defines whether the Trace is canceled.
    The input data should not be processed from the Traces
    whose \c canceled property set to true.
*/

/*! \property QVirtualKeyboardTrace::canceled
    \brief defines whether the QVirtualKeyboardTrace is canceled.

    The input data should not be processed from the Traces
    whose \c canceled property set to true.
*/

/*! \qmlproperty qreal Trace::opacity
    \since QtQuick.VirtualKeyboard 2.4

    This property defines how opaque the Trace is.

    A lower value results in a more transparent trace: \c 0.0 is fully
    transparent, and \c 1.0 is fully opaque.

    This property is useful for making older traces more transparent as new
    ones are added.
*/

/*! \property QVirtualKeyboardTrace::opacity

    This property defines how opaque the QVirtualKeyboardTrace is.

    A lower value results in a more transparent trace: \c 0.0 is fully
    transparent, and \c 1.0 is fully opaque.

    This property is useful for making older traces more transparent as new
    ones are added.
*/

QT_END_NAMESPACE
