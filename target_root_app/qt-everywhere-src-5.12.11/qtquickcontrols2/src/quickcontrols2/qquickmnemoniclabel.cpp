/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickmnemoniclabel_p.h"

#include <QtQuick/private/qquicktext_p_p.h>

QT_BEGIN_NAMESPACE

QQuickMnemonicLabel::QQuickMnemonicLabel(QQuickItem *parent)
    : QQuickText(parent)
{
}

QString QQuickMnemonicLabel::text() const
{
    return m_fullText;
}

void QQuickMnemonicLabel::setText(const QString &text)
{
    if (m_fullText == text)
        return;

    m_fullText = text;
    updateMnemonic();
}

bool QQuickMnemonicLabel::isMnemonicVisible() const
{
    return m_mnemonicVisible;
}

void QQuickMnemonicLabel::setMnemonicVisible(bool visible)
{
    if (m_mnemonicVisible == visible)
        return;

    m_mnemonicVisible = visible;
    updateMnemonic();

    if (isComponentComplete())
        forceLayout();
}

static QTextLayout::FormatRange underlineRange(int start, int length = 1)
{
    QTextLayout::FormatRange range;
    range.start = start;
    range.length = length;
    range.format.setFontUnderline(true);
    return range;
}

// based on QPlatformTheme::removeMnemonics()
void QQuickMnemonicLabel::updateMnemonic()
{
    QString text(m_fullText.size(), 0);
    int idx = 0;
    int pos = 0;
    int len = m_fullText.length();
    QVector<QTextLayout::FormatRange> formats;
    while (len) {
        if (m_fullText.at(pos) == QLatin1Char('&') && (len == 1 || m_fullText.at(pos + 1) != QLatin1Char('&'))) {
            if (m_mnemonicVisible && (pos == 0 || m_fullText.at(pos - 1) != QLatin1Char('&')))
                formats += underlineRange(pos);
            ++pos;
            --len;
            if (len == 0)
                break;
        } else if (m_fullText.at(pos) == QLatin1Char('(') && len >= 4 &&
                   m_fullText.at(pos + 1) == QLatin1Char('&') &&
                   m_fullText.at(pos + 2) != QLatin1Char('&') &&
                   m_fullText.at(pos + 3) == QLatin1Char(')')) {
            // a mnemonic with format "\s*(&X)"
            if (m_mnemonicVisible) {
                formats += underlineRange(pos + 1);
            } else {
                int n = 0;
                while (idx > n && text.at(idx - n - 1).isSpace())
                    ++n;
                idx -= n;
                pos += 4;
                len -= 4;
                continue;
            }
        }
        text[idx] = m_fullText.at(pos);
        ++pos;
        ++idx;
        --len;
    }
    text.truncate(idx);

    QQuickTextPrivate::get(this)->layout.setFormats(formats);
    QQuickText::setText(text);
}

QT_END_NAMESPACE
