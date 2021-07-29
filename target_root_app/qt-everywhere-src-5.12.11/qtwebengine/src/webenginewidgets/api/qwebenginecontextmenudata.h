/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBENGINECONTEXTDATA_H
#define QWEBENGINECONTEXTDATA_H

#include <QtWebEngineWidgets/qtwebenginewidgetsglobal.h>
#include <QtCore/qpoint.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>

namespace QtWebEngineCore {
class WebEngineContextMenuData;
}

QT_BEGIN_NAMESPACE

class QWEBENGINEWIDGETS_EXPORT QWebEngineContextMenuData {
    Q_GADGET

public:
    QWebEngineContextMenuData();
    QWebEngineContextMenuData(const QWebEngineContextMenuData &other);
    QWebEngineContextMenuData &operator=(const QWebEngineContextMenuData &other);
    ~QWebEngineContextMenuData();

    enum MediaType {
        MediaTypeNone,
        MediaTypeImage,
        MediaTypeVideo,
        MediaTypeAudio,
        MediaTypeCanvas,
        MediaTypeFile,
        MediaTypePlugin
    };

    // Must match QWebEngineCore::WebEngineContextMenuData::MediaFlags:
    enum MediaFlag {
        MediaInError = 0x1,
        MediaPaused = 0x2,
        MediaMuted = 0x4,
        MediaLoop = 0x8,
        MediaCanSave = 0x10,
        MediaHasAudio = 0x20,
        MediaCanToggleControls = 0x40,
        MediaControls = 0x80,
        MediaCanPrint = 0x100,
        MediaCanRotate = 0x200,
    };
    Q_DECLARE_FLAGS(MediaFlags, MediaFlag)
    Q_FLAG(MediaFlags)

    // Must match QWebEngineCore::WebEngineContextMenuData::EditFlags:
    enum EditFlag {
        CanUndo = 0x1,
        CanRedo = 0x2,
        CanCut = 0x4,
        CanCopy = 0x8,
        CanPaste = 0x10,
        CanDelete = 0x20,
        CanSelectAll = 0x40,
        CanTranslate = 0x80,
        CanEditRichly = 0x100,
    };
    Q_DECLARE_FLAGS(EditFlags, EditFlag)
    Q_FLAG(EditFlags)

    bool isValid() const;

    QPoint position() const;
    QString selectedText() const;
    QString linkText() const;
    QUrl linkUrl() const;
    QUrl mediaUrl() const;
    MediaType mediaType() const;
    bool isContentEditable() const;
    QString misspelledWord() const;
    QStringList spellCheckerSuggestions() const;
    MediaFlags mediaFlags() const;
    EditFlags editFlags() const;

private:
    void reset();
    typedef QtWebEngineCore::WebEngineContextMenuData QWebEngineContextDataPrivate;
    QWebEngineContextMenuData &operator=(const QWebEngineContextDataPrivate &priv);
    const QWebEngineContextDataPrivate *d;

    friend class QWebEnginePagePrivate;
    friend class QWebEnginePage;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWebEngineContextMenuData::MediaFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QWebEngineContextMenuData::EditFlags)

QT_END_NAMESPACE

#endif // QWEBENGINECONTEXTDATA_H
