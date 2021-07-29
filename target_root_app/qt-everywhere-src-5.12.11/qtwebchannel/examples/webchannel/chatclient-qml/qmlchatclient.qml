/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2016 basysKom GmbH, author Bernd Lamecker <bernd.lamecker@basyskom.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebChannel module of the Qt Toolkit.
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

import QtQuick 2.2
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.0
import QtQuick.Window 2.0
import QtQuick.Layouts 1.1
import Qt.WebSockets 1.0
import "qwebchannel.js" as WebChannel

ApplicationWindow {
    id: root

    property var channel
    property string loginName: loginUi.userName.text

    title: "Chat client"
    width: 640
    height: 480
    visible: true

    WebSocket {
        id: socket

        // the following three properties/functions are required to align the QML WebSocket API
        // with the HTML5 WebSocket API.
        property var send: function(arg) {
            sendTextMessage(arg);
        }

        onTextMessageReceived: {
            onmessage({data: message});
        }

        property var onmessage

        active: true
        url: "ws://localhost:12345"

        onStatusChanged: {
            switch (socket.status) {
            case WebSocket.Error:
                errorDialog.text = "Error: " + socket.errorString;
                errorDialog.visible = true;
                break;
            case WebSocket.Closed:
                errorDialog.text = "Error: Socket at " + url + " closed.";
                errorDialog.visible = true;
                break;
            case WebSocket.Open:
                //open the webchannel with the socket as transport
                new WebChannel.QWebChannel(socket, function(ch) {
                    root.channel = ch;

                    //connect to the changed signal of the userList property
                    ch.objects.chatserver.userListChanged.connect(function(args) {
                        mainUi.userlist.text = '';
                        ch.objects.chatserver.userList.forEach(function(user) {
                            mainUi.userlist.text += user + '\n';
                        });
                    });

                    //connect to the newMessage signal
                    ch.objects.chatserver.newMessage.connect(function(time, user, message) {
                        var line = "[" + time + "] " + user + ": " + message + '\n';
                        mainUi.chat.text = mainUi.chat.text + line;
                    });

                    //connect to the keep alive signal
                    ch.objects.chatserver.keepAlive.connect(function(args) {
                        if (loginName !== '')
                            //and call the keep alive response method as an answer
                            ch.objects.chatserver.keepAliveResponse(loginName);
                    });
                });

                loginWindow.show();
                break;
            }
        }
    }

    MainForm {
        id: mainUi
        anchors.fill: parent

        Connections {
            target: mainUi.message
            onEditingFinished: {
                if (mainUi.message.text.length) {
                    //call the sendMessage method to send the message
                    root.channel.objects.chatserver.sendMessage(loginName,
                                                                mainUi.message.text);
                }
                mainUi.message.text = '';
            }
        }
    }

    Window {
        id: loginWindow

        title: "Login"
        modality: Qt.ApplicationModal
        width: 300
        height: 200

        LoginForm {
            id: loginUi
            anchors.fill: parent

            nameInUseError.visible: false

            Connections {
                target: loginUi.loginButton

                onClicked: {
                    //call the login method
                    root.channel.objects.chatserver.login(loginName, function(arg) {
                        //check the return value for success
                        if (arg === true) {
                            loginUi.nameInUseError.visible = false;
                            loginWindow.close();
                        } else {
                            loginUi.nameInUseError.visible = true;
                        }
                    });
                }
            }
        }
    }

    MessageDialog {
        id: errorDialog

        icon: StandardIcon.Critical
        standardButtons: StandardButton.Close
        title: "Chat client"

        onAccepted: {
            Qt.quit();
        }
        onRejected: {
            Qt.quit();
        }
    }
}
