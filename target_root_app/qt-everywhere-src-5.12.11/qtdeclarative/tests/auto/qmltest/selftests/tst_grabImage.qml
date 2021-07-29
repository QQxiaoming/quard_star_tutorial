/****************************************************************************
**
** Copyright (C) 2017 Crimson AS <info@crimson.no>
** Copyright (C) 2016 The Qt Company Ltd.
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

import QtQuick 2.0
import QtTest 1.1

TestCase {
    id: testCase
    name: "tst_grabImage"
    when: windowShown

    function test_equals() {
        var rect = createTemporaryQmlObject("import QtQuick 2.0; Rectangle { color: 'red'; width: 10; height: 10; }", testCase);
        verify(rect);
        var oldImage = grabImage(rect);
        rect.width += 10;
        var newImage = grabImage(rect);
        verify(!newImage.equals(oldImage));

        oldImage = grabImage(rect);
        // Don't change anything...
        newImage = grabImage(rect);
        try {
            compare(newImage.size, oldImage.size);
            verify(newImage.equals(oldImage));
        } catch (ex) {
            oldImage.save("tst_grabImage_test_equals_oldImage.png")
            newImage.save("tst_grabImage_test_equals_newImage.png")
            throw ex;
        }

        verify(!newImage.equals(null));
        verify(!newImage.equals(undefined));
    }

    function test_sizeProps() {
        var rect = createTemporaryQmlObject("import QtQuick 2.0; Rectangle { color: 'red'; width: 10; height: 20; }", testCase);
        var image = grabImage(rect);

        try {
            compare(image.width, 10)
            compare(image.height, 20)
            compare(image.size, Qt.size(10, 20))
        } catch (ex) {
            image.save("tst_grabImage_test_sizeProps.png")
            throw ex;
        }
    }

    function test_save() {
        var rect = createTemporaryQmlObject("import QtQuick 2.0; Rectangle { color: 'red'; width: 10; height: 20; }", testCase);
        var grabbedImage = grabImage(rect);
        grabbedImage.save("tst_grabImage_test_save.png")

        // Now try to load it
        var url = Qt.resolvedUrl("tst_grabImage_test_save.png")
        var image = createTemporaryQmlObject("import QtQuick 2.0; Image { source: \"" + url + "\" }", testCase);
        tryCompare(image, "status", Image.Ready)
        var grabbedImage2 = grabImage(image);

        try {
            verify(grabbedImage2.equals(grabbedImage))
        } catch (ex) {
            grabbedImage2.save("tst_grabImage_test_save2.png")
            throw ex;
        }
    }

    function test_saveThrowsWhenFailing() {
        var rect = createTemporaryQmlObject("import QtQuick 2.0; Rectangle { color: 'red'; width: 10; height: 20; }", testCase);
        var grabbedImage = grabImage(rect);
        var didThrow = false;

        try {
            // Format doesn't exist, so this will throw
            grabbedImage.save("tst_grabImage_test_saveThrowsWhenFailing.never-gonna-give-you-up");
        } catch (ex) {
            didThrow = true;
        }

        if (!didThrow) {
            fail("save() should have thrown, but didn't!")
        }
    }
}
