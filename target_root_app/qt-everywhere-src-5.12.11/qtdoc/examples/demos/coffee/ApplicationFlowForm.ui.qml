
/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
import QtQuick 2.4
import Coffee 1.0

Rectangle {
    id: root
    width: Constants.width
    height: Constants.height
    property alias choosingCoffee: choosingCoffee
    property alias brewing: brewing
    property alias emptyCup: emptyCup

    color: Constants.backgroundColor

    state: intital

    EmptyCupForm {
        id: emptyCup
        x: Constants.width
        y: 0
    }

    Brewing {
        id: brewing
        x: Constants.width * 2
        y: 0
    }

    ChoosingCoffee {
        id: choosingCoffee
    }

    states: [
        State {
            name: "initial"
        },
        State {
            name: "selection"

            PropertyChanges {
                target: choosingCoffee
                selected: true
            }
        },
        State {
            name: "settings"

            PropertyChanges {
                target: choosingCoffee
                x: -Constants.leftSideBarWidth
                selected: false
                inSettings: true
            }

            PropertyChanges {
                target: emptyCup
                x: Constants.width
                y: 0
            }

            PropertyChanges {
                target: brewing
                x: Constants.width * 2
                y: 0
            }
        },
        State {
            name: "empty cup"

            PropertyChanges {
                target: emptyCup
                x: 0
            }

            PropertyChanges {
                target: choosingCoffee
                x: -Constants.width - Constants.leftSideBarWidth
                inSettings: true
                selected: false
            }

            PropertyChanges {
                target: brewing
                x: Constants.width
            }
        },
        State {
            name: "brewing"

            PropertyChanges {
                target: emptyCup
                x: -Constants.width
            }

            PropertyChanges {
                target: brewing
                x: 0
            }

            PropertyChanges {
                target: choosingCoffee
                x: -Constants.width * 2 - Constants.leftSideBarWidth
            }
        },
        State {
            name: "finished"

            PropertyChanges {
                target: emptyCup
                x: -Constants.width
                y: 0
                visible: false
            }

            PropertyChanges {
                target: brewing
                x: 0
            }

            PropertyChanges {
                target: choosingCoffee
                x: -Constants.leftSideBarWidth - Constants.width
                questionVisible: true
                inSettings: true
            }
        },
        State {
            name: "start"

            PropertyChanges {
                target: emptyCup
                x: 0
                visible: false
            }

            PropertyChanges {
                target: brewing
                x: Constants.width
                y: 0
            }

            PropertyChanges {
                target: choosingCoffee
                x: 0
                inSettings: true
            }
        }
    ]
}
