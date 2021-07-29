/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Virtual Keyboard module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

.pragma library

.import "unipen_data.js" as UnipenData
.import "unipen_data_simp_chinese.js" as UnipenDataSimpChinese
.import "unipen_data_trad_chinese.js" as UnipenDataTradChinese
.import "unipen_data_japanese.js" as UnipenDataJapanese
.import "unipen_data_korean.js" as UnipenDataKorean
.import "unipen_data_greek.js" as UnipenDataGreek
.import "unipen_data_cyrillic.js" as UnipenDataCyrillic
.import "unipen_data_arabic.js" as UnipenDataArabic
.import "unipen_data_hebrew.js" as UnipenDataHebrew
.import "unipen_data_vietnamese.js" as UnipenDataVietnamese
.import "unipen_data_thai.js" as UnipenDataThai

function emulate(testcase, hwrInputArea, ch, instant, locale) {
    var chKey = (((typeof ch == "number") ? ch : ch.charCodeAt(0)) + 0x100000000).toString(16).substr(1)
    while (chKey.length > 4 && chKey[0] === '0')
        chKey = chKey.substring(1)
    chKey = "0x" + chKey
    var unipenData
    if (locale === "vi_VN" && UnipenDataVietnamese.unipenData.hasOwnProperty(chKey))
        unipenData = UnipenDataVietnamese
    else if (UnipenData.unipenData.hasOwnProperty(chKey))
        unipenData = UnipenData
    else if (UnipenDataSimpChinese.unipenData.hasOwnProperty(chKey))
        unipenData = UnipenDataSimpChinese
    else if (UnipenDataTradChinese.unipenData.hasOwnProperty(chKey))
        unipenData = UnipenDataTradChinese
    else if (UnipenDataJapanese.unipenData.hasOwnProperty(chKey))
        unipenData = UnipenDataJapanese
    else if (UnipenDataKorean.unipenData.hasOwnProperty(chKey))
        unipenData = UnipenDataKorean
    else if (UnipenDataGreek.unipenData.hasOwnProperty(chKey))
        unipenData = UnipenDataGreek
    else if (UnipenDataCyrillic.unipenData.hasOwnProperty(chKey))
        unipenData = UnipenDataCyrillic
    else if (UnipenDataArabic.unipenData.hasOwnProperty(chKey))
        unipenData = UnipenDataArabic
    else if (UnipenDataHebrew.unipenData.hasOwnProperty(chKey))
        unipenData = UnipenDataHebrew
    else if (UnipenDataThai.unipenData.hasOwnProperty(chKey))
        unipenData = UnipenDataThai
    else
        return false
    var chData = unipenData.unipenData[chKey]
    var scale = Math.min(hwrInputArea.width / chData[".X_DIM"], hwrInputArea.height / chData[".Y_DIM"])
    var strokes = unipenData.unipenData[chKey][".PEN"]
    var t = 0
    for (var strokeIndex = 0; strokeIndex < strokes.length; strokeIndex++) {
        var stroke = strokes[strokeIndex]
        for (var i = 0; i < stroke.length; i++) {
            var strokeData = stroke[i]
            var pt = Qt.point(strokeData[0] * scale, strokeData[1] * scale)
            if (instant)
                t = strokeData[2]
            if (i == 0) {
                t = strokeData[2]
                testcase.mousePress(hwrInputArea, pt.x, pt.y, Qt.LeftButton, 0, strokeData[2] - t)
            } else {
                testcase.mouseMove(hwrInputArea, pt.x, pt.y, strokeData[2] - t, Qt.LeftButton)
            }
            if (i + 1 === stroke.length)
                testcase.mouseRelease(hwrInputArea, pt.x, pt.y, Qt.LeftButton, 0, instant ? 1 : strokeData[2] - t)
            t = strokeData[2]
        }
    }
    return true
}
