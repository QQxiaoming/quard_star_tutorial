/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

import QtQuick 2.3
import QtTest 1.1

TestCase {
    name: "Third"

    function init_data() {
        return [
            { tag: "init_0" },
            { tag: "skip_1" },
            { tag: "init_2" },
            { tag: "skip_3" },
            { tag: "init_4" },
        ]
    }

    function test_default_tags(data) {
        if (data.tag.startsWith("skip_"))
            skip("skip '" + data.tag + "' tag")
    }

    function test_tags_data() {
         return [
             { tag: "foo" },
             { tag: "bar" },
             { tag: "baz" },
         ]
    }

    function test_tags(data) {
        if (data.tag === "bar")
            skip("skip '" + data.tag + "' tag")
    }
}
