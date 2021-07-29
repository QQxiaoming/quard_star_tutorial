/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

import QtQuick 2.0

Rectangle {
    width: 360
    height: 360

    property alias loader: loader

    Loader {
        id: loader
    }

    property Component component: Item {
        property bool trigger: false
        onTriggerChanged: {
            objectInRootContext.doIt() // make sure we can resolve objectInRootContext
            loader.active = false
            objectInRootContext.doIt() // make sure we can STILL resolve objectInRootContext
            anotherProperty = true // see if we can trigger subsequent signal handlers (we shouldn't)
        }
        property bool anotherProperty: false
        onAnotherPropertyChanged: {
            // this should never be executed
            objectInRootContext.doIt()
        }
    }
}
