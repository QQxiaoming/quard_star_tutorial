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
import QtQuick 2.4

Item {
    width: 500
    height: 160
    visible: true

    property var model1: ["1","2","3","4","5","6","7","8","9","10",
    "11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30",
    "31","32","33","34","a"]
    property var model2: ["a","b","c","d","e","f","g","h","i","j","k","l","m","1"]
    property bool useModel1: true

    function changeModel() {
        useModel1 = !useModel1
        grid.loadModel(useModel1 ? model1 : model2)
    }

    function scrollToTop() {
        grid.contentY = grid.originY;
    }

    GridView {
        id: grid
        anchors.fill: parent

        model: ListModel {
        }

        onCurrentIndexChanged: {
            positionViewAtIndex(currentIndex, GridView.Contain)
        }

        Component.onCompleted: {
            loadModel(model1)
            grid.currentIndex = 34
            grid.positionViewAtIndex(34, GridView.Contain)
        }

        function loadModel(m) {
            var remove = {};
            var add = {};
            var i;
            for (i=0; i < model.count; ++i)
                remove[model.get(i).name] = true;
            for (i=0; i < m.length; ++i)
                if (remove[m[i]])
                    delete remove[m[i]];
                else
                    add[m[i]] = true;

            for (i=model.count-1; i>= 0; --i)
                if (remove[model.get(i).name])
                    model.remove(i, 1);

            for (i=0; i<m.length; ++i)
                if (add[m[i]])
                    model.insert(i, { "name": m[i] })
        }

        delegate: Rectangle {
            height: grid.cellHeight
            width: grid.cellWidth
            color: GridView.isCurrentItem ? "gray" : "white"
            Text {
                anchors.fill: parent
                text: name
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    grid.currentIndex = index
                }
            }
        }
    }
}
