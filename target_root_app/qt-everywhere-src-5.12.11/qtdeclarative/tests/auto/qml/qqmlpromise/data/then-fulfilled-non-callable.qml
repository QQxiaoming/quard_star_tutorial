/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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
import QtQuick 2.0

QtObject {
    property int resolveValue: 5

    property bool promise1WasResolved: false
    property bool promise2WasResolved: false
    property bool promise3WasResolved: false
    property bool promise4WasResolved: true

    property bool wasTestSuccessful: promise1WasResolved && promise2WasResolved &&
                                    promise3WasResolved && promise4WasResolved

    // TODO: Should this work as well?
    // property Promise promise
    property var promise1: new Promise(function (resolve, reject) {
        resolve(resolveValue)
    })
    property var promise2: new Promise(function (resolve, reject) {
        resolve(resolveValue)
    })
    property var promise3: new Promise(function (resolve, reject) {
        resolve(resolveValue)
    })
    property var promise4: new Promise(function (resolve, reject) {
        resolve(resolveValue)
    })

    Component.onCompleted: {
        promise1.then().then(function (result) {
            promise1WasResolved = (result === resolveValue);
        }, function() {
            throw new Error("Should never be called")
        })
        promise2.then(3, 5).then(function (result) {
            promise2WasResolved = (result === resolveValue);
        }, function() {
            throw new Error("Should never be called")
        })
        promise3.then(null, function() {
            throw new Error("Should never be called")
        }).then(function (result) {
            promise3WasResolved = (result === resolveValue);
        }, function() {
            throw new Error("Should never be called")
        })
        /*
        promise4.then(undefined, undefined).then(function (result) {
            promise4WasResolved = (result === resolveValue);
        }, function() {
            throw new Error("Should never be called")
        })
        */
    }
}
