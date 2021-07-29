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
import QtTest 1.0
import Qt.labs.platform 1.0

TestCase {
    id: testCase
    width: 200
    height: 200
    name: "MessageDialog"

    Component {
        id: messageDialog
        MessageDialog { }
    }

    SignalSpy {
        id: spy
    }

    function test_instance() {
        var dialog = messageDialog.createObject(testCase)
        verify(dialog)
        dialog.destroy()
    }

    function test_standardButtons() {
        // Q_FLAGS(QPlatformDialogHelper::StandardButtons)
        compare(MessageDialog.NoButton       , 0x00000000)
        compare(MessageDialog.Ok             , 0x00000400)
        compare(MessageDialog.Save           , 0x00000800)
        compare(MessageDialog.SaveAll        , 0x00001000)
        compare(MessageDialog.Open           , 0x00002000)
        compare(MessageDialog.Yes            , 0x00004000)
        compare(MessageDialog.YesToAll       , 0x00008000)
        compare(MessageDialog.No             , 0x00010000)
        compare(MessageDialog.NoToAll        , 0x00020000)
        compare(MessageDialog.Abort          , 0x00040000)
        compare(MessageDialog.Retry          , 0x00080000)
        compare(MessageDialog.Ignore         , 0x00100000)
        compare(MessageDialog.Close          , 0x00200000)
        compare(MessageDialog.Cancel         , 0x00400000)
        compare(MessageDialog.Discard        , 0x00800000)
        compare(MessageDialog.Help           , 0x01000000)
        compare(MessageDialog.Apply          , 0x02000000)
        compare(MessageDialog.Reset          , 0x04000000)
        compare(MessageDialog.RestoreDefaults, 0x08000000)
    }

    function test_clicked() {
        var dialog = messageDialog.createObject(testCase)
        verify(dialog)

        spy.target = dialog
        spy.signalName = "clicked"

        dialog.clicked(MessageDialog.Yes)
        compare(spy.count, 1)
        compare(spy.signalArguments[0][0], MessageDialog.Yes)

        spy.clear()
        dialog.destroy()
    }
}
