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

import QtQuick 2.0
import QtQml.Models 2.1
import "../shared" as Examples

Item {
    height: 480
    width: 320
    Examples.LauncherList {
        id: ll
        anchors.fill: parent
        Component.onCompleted: {
            addExample("GridView", "A simple GridView", Qt.resolvedUrl("gridview/gridview-example.qml"))
            addExample("Dynamic List", "A dynamically alterable list", Qt.resolvedUrl("listview/dynamiclist.qml"))
            addExample("Expanding Delegates", "A ListView with delegates that expand", Qt.resolvedUrl("listview/expandingdelegates.qml"))
            addExample("Highlight", "A ListView with a custom highlight", Qt.resolvedUrl("listview/highlight.qml"))
            addExample("Highlight Ranges", "The three highlight ranges of ListView", Qt.resolvedUrl("listview/highlightranges.qml"))
            addExample("Sections", "ListView section headers and footers", Qt.resolvedUrl("listview/sections.qml"))
            addExample("Packages", "Transitions between a ListView and GridView", Qt.resolvedUrl("package/view.qml"))
            addExample("PathView", "A simple PathView", Qt.resolvedUrl("pathview/pathview-example.qml"))
            addExample("ObjectModel", "Using a ObjectModel", Qt.resolvedUrl("objectmodel/objectmodel.qml"))
            addExample("Display Margins", "A ListView with display margins", Qt.resolvedUrl("listview/displaymargin.qml"))
            addExample("DelegateModel", "A PathView using DelegateModel to instantiate delegates", Qt.resolvedUrl("delegatemodel/slideshow.qml"))
            addExample("Draggable Selections", "Enabling drag-and-drop on DelegateModel delegates", Qt.resolvedUrl("delegatemodel/dragselection.qml"))
        }
    }
}
