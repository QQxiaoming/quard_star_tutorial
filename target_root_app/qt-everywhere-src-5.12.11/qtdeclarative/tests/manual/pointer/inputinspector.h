/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the manual tests of the Qt Toolkit.
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

#ifndef INPUTINSPECTOR_H
#define INPUTINSPECTOR_H

#include <QObject>
class QQuickWindow;
class QQuickPointerHandler;
class QQuickPointerDevice;

class InputInspector : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString mouseGrabber READ mouseGrabber NOTIFY mouseGrabberChanged)
    Q_PROPERTY(QString passiveGrabbers READ passiveGrabbers NOTIFY passiveGrabbersChanged)
    Q_PROPERTY(QString exclusiveGrabbers READ exclusiveGrabbers NOTIFY exclusiveGrabbersChanged)
    Q_PROPERTY(QQuickWindow * source READ source WRITE setSource NOTIFY sourceChanged)
public:
    explicit InputInspector(QObject *parent = nullptr);
    ~InputInspector();
    QString mouseGrabber() const;
    QString passiveGrabbers() const;
    QString exclusiveGrabbers() const;
    QQuickWindow *source() const;
    void setSource(QQuickWindow *window);

    void timerEvent(QTimerEvent *event);
    Q_INVOKABLE void update();

signals:
    void mouseGrabberChanged();
    void passiveGrabbersChanged();
    void exclusiveGrabbersChanged();
    void sourceChanged();
private:
    QVector<QObject*> passiveGrabbers_helper(int pointId = 0) const;
    QVector<QObject*> exclusiveGrabbers_helper(int pointId = 0) const;
    static QString objectIdentifier(QObject *o);
    QQuickPointerDevice *pointerDevice() const;
    QString vectorStringJoin(const QVector<QObject*> &arr) const;

private:
    mutable struct LastState {
        QString mouseGrabber;
        QString passiveGrabbers;
        QString exclusiveGrabbers;
    } lastState;

    QQuickWindow *m_window = nullptr;
    int m_timerId = 0;
};

#endif // INPUTINSPECTOR_H
