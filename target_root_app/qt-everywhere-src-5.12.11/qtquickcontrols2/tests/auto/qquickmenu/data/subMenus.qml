/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

import QtQuick 2.12
import QtQuick.Controls 2.12

ApplicationWindow {
    width: 600
    height: 400

    property alias mainMenu: mainMenu
    property alias subMenu1: subMenu1
    property alias subMenu2: subMenu2
    property alias subSubMenu1: subSubMenu1

    Menu {
        id: mainMenu
        MenuItem {
            id: mainMenuItem1
            objectName: "mainMenuItem1"
            text: "Main 1"
        }

        Menu {
            id: subMenu1
            objectName: "subMenu1"
            title: "Sub Menu 1"

            MenuItem {
                id: subMenuItem1
                objectName: "subMenuItem1"
                text: "Sub 1"
            }
            MenuItem {
                id: subMenuItem2
                objectName: "subMenuItem2"
                text: "Sub 2"
            }

            Menu {
                id: subSubMenu1
                objectName: "subSubMenu1"
                title: "Sub Sub Menu 1"

                MenuItem {
                    id: subSubMenuItem1
                    objectName: "subSubMenuItem1"
                    text: "Sub Sub 1"
                }
                MenuItem {
                    id: subSubMenuItem2
                    objectName: "subSubMenuItem2"
                    text: "Sub Sub 2"
                }
            }
        }

        MenuItem {
            id: mainMenuItem2
            objectName: "mainMenuItem2"
            text: "Main 2"
        }

        Menu {
            id: subMenu2
            objectName: "subMenu2"
            title: "Sub Menu 2"

            MenuItem {
                id: subMenuItem3
                objectName: "subMenuItem3"
                text: "Sub 3"
            }
            MenuItem {
                id: subMenuItem4
                objectName: "subMenuItem4"
                text: "Sub 4"
            }
        }

        MenuItem {
            id: mainMenuItem3
            objectName: "mainMenuItem3"
            text: "Main 3"
        }
    }
}
