/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.11

// Populate the model with some sample data.
ListModel {
    id: alarmModel

    ListElement {
        hour: 6
        minute: 0
        day: 2
        month: 8
        year: 2018
        activated: true
        label: "Wake up"
        repeat: true
        daysToRepeat: [
            ListElement { dayOfWeek: 0; repeat: false },
            ListElement { dayOfWeek: 1; repeat: false },
            ListElement { dayOfWeek: 2; repeat: false },
            ListElement { dayOfWeek: 3; repeat: false },
            ListElement { dayOfWeek: 4; repeat: false },
            ListElement { dayOfWeek: 5; repeat: false },
            ListElement { dayOfWeek: 6; repeat: false }
        ]
    }
    ListElement {
        hour: 6
        minute: 0
        day: 3
        month: 8
        year: 2018
        activated: true
        label: "Wake up"
        repeat: true
        daysToRepeat: [
            ListElement { dayOfWeek: 0; repeat: true },
            ListElement { dayOfWeek: 1; repeat: true },
            ListElement { dayOfWeek: 2; repeat: true },
            ListElement { dayOfWeek: 3; repeat: true },
            ListElement { dayOfWeek: 4; repeat: true },
            ListElement { dayOfWeek: 5; repeat: false },
            ListElement { dayOfWeek: 6; repeat: false }
        ]
    }
    ListElement {
        hour: 7
        minute: 0
        day: 3
        month: 8
        year: 2018
        activated: false
        label: "Exercise"
        repeat: true
        daysToRepeat: [
            ListElement { dayOfWeek: 0; repeat: true },
            ListElement { dayOfWeek: 1; repeat: true },
            ListElement { dayOfWeek: 2; repeat: true },
            ListElement { dayOfWeek: 3; repeat: true },
            ListElement { dayOfWeek: 4; repeat: true },
            ListElement { dayOfWeek: 5; repeat: true },
            ListElement { dayOfWeek: 6; repeat: true }
        ]
    }
    ListElement {
        hour: 5
        minute: 15
        day: 1
        month: 9
        year: 2018
        activated: true
        label: ""
        repeat: false
        daysToRepeat: [
            ListElement { dayOfWeek: 0; repeat: false },
            ListElement { dayOfWeek: 1; repeat: false },
            ListElement { dayOfWeek: 2; repeat: false },
            ListElement { dayOfWeek: 3; repeat: false },
            ListElement { dayOfWeek: 4; repeat: false },
            ListElement { dayOfWeek: 5; repeat: false },
            ListElement { dayOfWeek: 6; repeat: false }
        ]
    }
    ListElement {
        hour: 5
        minute: 45
        day: 3
        month: 9
        year: 2018
        activated: false
        label: ""
        repeat: false
        daysToRepeat: [
            ListElement { dayOfWeek: 0; repeat: false },
            ListElement { dayOfWeek: 1; repeat: false },
            ListElement { dayOfWeek: 2; repeat: false },
            ListElement { dayOfWeek: 3; repeat: false },
            ListElement { dayOfWeek: 4; repeat: false },
            ListElement { dayOfWeek: 5; repeat: false },
            ListElement { dayOfWeek: 6; repeat: false }
        ]
    }
}
