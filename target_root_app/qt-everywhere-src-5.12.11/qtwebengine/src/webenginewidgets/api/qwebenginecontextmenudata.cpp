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

#include "qwebenginecontextmenudata.h"

#include "web_contents_adapter_client.h"

QT_BEGIN_NAMESPACE

// Match MediaType enum
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeNone,   QWebEngineContextMenuData::MediaTypeNone)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeImage,  QWebEngineContextMenuData::MediaTypeImage)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeAudio,  QWebEngineContextMenuData::MediaTypeAudio)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeVideo,  QWebEngineContextMenuData::MediaTypeVideo)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeCanvas, QWebEngineContextMenuData::MediaTypeCanvas)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypeFile,   QWebEngineContextMenuData::MediaTypeFile)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaTypePlugin, QWebEngineContextMenuData::MediaTypePlugin)

// Match MediaFlag enum
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaInError,           QWebEngineContextMenuData::MediaInError)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaPaused,            QWebEngineContextMenuData::MediaPaused)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaMuted,             QWebEngineContextMenuData::MediaMuted)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaLoop,              QWebEngineContextMenuData::MediaLoop)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaCanSave,           QWebEngineContextMenuData::MediaCanSave)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaHasAudio,          QWebEngineContextMenuData::MediaHasAudio)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaCanToggleControls, QWebEngineContextMenuData::MediaCanToggleControls)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaControls,          QWebEngineContextMenuData::MediaControls)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaCanPrint,          QWebEngineContextMenuData::MediaCanPrint)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::MediaCanRotate,         QWebEngineContextMenuData::MediaCanRotate)

// Match EditFlag enum
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::CanUndo,       QWebEngineContextMenuData::CanUndo)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::CanRedo,       QWebEngineContextMenuData::CanRedo)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::CanCut,        QWebEngineContextMenuData::CanCut)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::CanCopy,       QWebEngineContextMenuData::CanCopy)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::CanPaste,      QWebEngineContextMenuData::CanPaste)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::CanDelete,     QWebEngineContextMenuData::CanDelete)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::CanSelectAll,  QWebEngineContextMenuData::CanSelectAll)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::CanTranslate,  QWebEngineContextMenuData::CanTranslate)
ASSERT_ENUMS_MATCH(QtWebEngineCore::WebEngineContextMenuData::CanEditRichly, QWebEngineContextMenuData::CanEditRichly)

/*!
    \class QWebEngineContextMenuData
    \since 5.7
    \brief The QWebEngineContextMenuData class provides context data for populating or extending a context menu with actions.

    \inmodule QtWebEngineWidgets

    QWebEngineContextMenuData is returned by QWebEnginePage::contextMenuData() after a context menu event,
    and contains information about where the context menu event took place. This is also in the context
    in which any context specific QWebEnginePage::WebAction will be performed.
*/

/*!
    \enum QWebEngineContextMenuData::MediaType

    This enum describes the media type of the context if any.

    \value MediaTypeNone The context is not a media type.
    \value MediaTypeImage The context is an image element.
    \value MediaTypeVideo The context is a video element.
    \value MediaTypeAudio The context is an audio element.
    \value MediaTypeCanvas The context is a canvas element.
    \value MediaTypeFile The context is a file.
    \value MediaTypePlugin The context is a plugin element.
*/

/*!
    \enum QWebEngineContextMenuData::EditFlag
    \readonly
    \since 5.11

    The available edit operations in the current context.

    \value  CanUndo Undo is available.
    \value  CanRedo Redo is available.
    \value  CanCut Cut is available.
    \value  CanCopy Copy is available.
    \value  CanPaste Paste is available.
    \value  CanDelete Delete is available.
    \value  CanSelectAll Select All is available.
    \value  CanTranslate Translate is available.
    \value  CanEditRichly Context is richly editable.
*/

/*!
    \enum QWebEngineContextMenuData::MediaFlag
    \readonly
    \since 5.11

    The current media element's status and its available operations.
    \c MediaNone if the selected web page content is not a media element.

    \value  MediaInError An error occurred.
    \value  MediaPaused Media is paused.
    \value  MediaMuted Media is muted.
    \value  MediaLoop Media can be looped.
    \value  MediaCanSave Media can be saved.
    \value  MediaHasAudio Media has audio.
    \value  MediaCanToggleControls Media can show controls.
    \value  MediaControls Media controls are shown.
    \value  MediaCanPrint Media is printable.
    \value  MediaCanRotate Media is rotatable.
*/

/*!
    Constructs null context menu data.
*/
QWebEngineContextMenuData::QWebEngineContextMenuData() : d(nullptr)
{
}

/*!
    Constructs context menu data from \a other.
*/
QWebEngineContextMenuData::QWebEngineContextMenuData(const QWebEngineContextMenuData &other)
{
    d = new QtWebEngineCore::WebEngineContextMenuData(*other.d);
}

/*!
    Assigns the \a other context menu data to this.
*/
QWebEngineContextMenuData &QWebEngineContextMenuData::operator=(const QWebEngineContextMenuData &other)
{
    delete d;
    d = new QtWebEngineCore::WebEngineContextMenuData(*other.d);
    return *this;
}

/*!
    Destroys the context menu data.
*/
QWebEngineContextMenuData::~QWebEngineContextMenuData()
{
    delete d;
}

/*!
    Returns \c true if the context data is valid; otherwise returns \c false.
*/
bool QWebEngineContextMenuData::isValid() const
{
    return d;
}

/*!
    Resets the context data, making it invalid.
    \internal

    \sa isValid()
*/
void QWebEngineContextMenuData::reset()
{
    delete d;
    d = nullptr;
}

/*!
    Returns the position of the context, usually the mouse position where the context menu event was triggered.
*/
QPoint QWebEngineContextMenuData::position() const
{
    return d ? d->position() : QPoint();
}

/*!
    Returns the text of a link if the context is a link.
*/
QString QWebEngineContextMenuData::linkText() const
{
    return d ? d->linkText() : QString();
}

/*!
    Returns the URL of a link if the context is a link.
    It is not guaranteed to be a valid URL.
*/
QUrl QWebEngineContextMenuData::linkUrl() const
{
    return d ? d->unfilteredLinkUrl() : QUrl();
}

/*!
    Returns the selected text of the context.
*/
QString QWebEngineContextMenuData::selectedText() const
{
    return d ? d->selectedText() : QString();
}

/*!
    If the context is a media element, returns the URL of that media.
*/
QUrl QWebEngineContextMenuData::mediaUrl() const
{
    return d ? d->mediaUrl() : QUrl();
}

/*!
    Returns the type of the media element or \c MediaTypeNone if the context is not a media element.
*/
QWebEngineContextMenuData::MediaType QWebEngineContextMenuData::mediaType() const
{
    return d ? static_cast<QWebEngineContextMenuData::MediaType>(d->mediaType()) : MediaTypeNone;
}

/*!
    Returns \c true if the content is editable by the user; otherwise returns \c false.
*/
bool QWebEngineContextMenuData::isContentEditable() const
{
    return d ? d->isEditable() : false;
}

/*!
    If the context is a word considered misspelled by the spell-checker, returns the misspelled word.

    For possible replacements of the word, see spellCheckerSuggestions().

    \since 5.8
*/
QString QWebEngineContextMenuData::misspelledWord() const
{
    if (d)
        return d->misspelledWord();
    return QString();
}

/*!
    If the context is a word considered misspelled by the spell-checker, returns a list of suggested replacements
    for misspelledWord().

    \since 5.8
*/
QStringList QWebEngineContextMenuData::spellCheckerSuggestions() const
{
    if (d)
        return d->spellCheckerSuggestions();
    return QStringList();
}

/*!
    \internal
*/
QWebEngineContextMenuData &QWebEngineContextMenuData::operator=(const QWebEngineContextDataPrivate &priv)
{
    delete d;
    d = new QtWebEngineCore::WebEngineContextMenuData(priv);
    return *this;
}

/*!
    Returns the current media element's status and its available operations.
    \c MediaNone if the selected web page content is not a media element.
*/
QWebEngineContextMenuData::MediaFlags QWebEngineContextMenuData::mediaFlags() const
{
    return static_cast<QWebEngineContextMenuData::MediaFlags>(d->mediaFlags());
}

/*!
    Returns the available edit operations in the current context or \c CanDoNone if no actions are available.
*/
QWebEngineContextMenuData::EditFlags QWebEngineContextMenuData::editFlags() const
{
    return static_cast<QWebEngineContextMenuData::EditFlags>(d->editFlags());
}

QT_END_NAMESPACE
