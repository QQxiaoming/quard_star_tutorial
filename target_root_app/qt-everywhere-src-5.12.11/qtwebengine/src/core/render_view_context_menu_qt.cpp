/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include <QtCore/QCoreApplication>
#include "render_view_context_menu_qt.h"

namespace QtWebEngineCore {

    const QString RenderViewContextMenuQt::getMenuItemName(RenderViewContextMenuQt::ContextMenuItem menuItem) {
        Q_ASSERT(menuItem <= ContextMenuItem::ViewSource);
        static const char *names[ContextMenuItem::ViewSource + 1] = {
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Back"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Forward"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Reload"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Cut"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Copy"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Paste"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Undo"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Redo"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Select all"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Paste and match style"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Open link in new window"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Open link in new tab"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Copy link address"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Save link"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Copy image"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Copy image address"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Save image"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Copy media address"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Show controls"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Loop"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Save media"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Inspect"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Exit full screen"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "Save page"),
            QT_TRANSLATE_NOOP("RenderViewContextMenuQt", "View page source")
        };
        return QCoreApplication::translate("RenderViewContextMenuQt", qUtf8Printable(names[menuItem]));
    }

    RenderViewContextMenuQt::RenderViewContextMenuQt(const WebEngineContextMenuData &data)
        : m_contextData(data)
    {
    }

    void RenderViewContextMenuQt::initMenu()
    {
        if (isFullScreenMode()) {
            appendExitFullscreenItem();
            appendSeparatorItem();
        }

        if (m_contextData.isEditable() && !m_contextData.spellCheckerSuggestions().isEmpty()) {
            appendSpellingSuggestionItems();
            appendSeparatorItem();
        }

        if (m_contextData.linkText().isEmpty() && !m_contextData.linkUrl().isValid() && !m_contextData.mediaUrl().isValid()) {
            if (m_contextData.isEditable())
                appendEditableItems();
            else if (!m_contextData.selectedText().isEmpty())
                appendCopyItem();
            else
                appendPageItems();
        }

        if (m_contextData.linkUrl().isValid() || !m_contextData.unfilteredLinkUrl().isEmpty() || !m_contextData.linkUrl().isEmpty())
            appendLinkItems();

        if (m_contextData.mediaUrl().isValid()) {
            switch (m_contextData.mediaType()) {
            case WebEngineContextMenuData::MediaTypeImage:
                appendSeparatorItem();
                appendImageItems();
                break;
            case WebEngineContextMenuData::MediaTypeCanvas:
                Q_UNREACHABLE();    // mediaUrl is invalid for canvases
                break;
            case WebEngineContextMenuData::MediaTypeAudio:
            case WebEngineContextMenuData::MediaTypeVideo:
                appendSeparatorItem();
                appendMediaItems();
                break;
            default:
                break;
            }
        } else if (m_contextData.mediaType() == WebEngineContextMenuData::MediaTypeCanvas) {
            appendSeparatorItem();
            appendCanvasItems();
        }

        if (canViewSource() || hasInspector()) {
            appendSeparatorItem();
            appendDeveloperItems();
        }
    }

    void RenderViewContextMenuQt::appendCanvasItems()
    {
        addMenuItem(RenderViewContextMenuQt::DownloadImageToDisk);
        addMenuItem(RenderViewContextMenuQt::CopyImageToClipboard);
    }

    void RenderViewContextMenuQt::appendCopyItem()
    {
        addMenuItem(RenderViewContextMenuQt::Copy);
    }

    void RenderViewContextMenuQt::appendDeveloperItems()
    {
        if (canViewSource())
            addMenuItem(RenderViewContextMenuQt::ViewSource);
        if (hasInspector())
            addMenuItem(RenderViewContextMenuQt::InspectElement);
    }

    void RenderViewContextMenuQt::appendEditableItems()
    {
        addMenuItem(RenderViewContextMenuQt::Undo);
        addMenuItem(RenderViewContextMenuQt::Redo);
        appendSeparatorItem();
        addMenuItem(RenderViewContextMenuQt::Cut);
        addMenuItem(RenderViewContextMenuQt::Copy);
        addMenuItem(RenderViewContextMenuQt::Paste);
        if (m_contextData.misspelledWord().isEmpty()) {
            addMenuItem(RenderViewContextMenuQt::PasteAndMatchStyle);
            addMenuItem(RenderViewContextMenuQt::SelectAll);
        }
    }

    void RenderViewContextMenuQt::appendExitFullscreenItem()
    {
        addMenuItem(RenderViewContextMenuQt::ExitFullScreen);
    }

    void RenderViewContextMenuQt::appendImageItems()
    {
        addMenuItem(RenderViewContextMenuQt::DownloadImageToDisk);
        addMenuItem(RenderViewContextMenuQt::CopyImageToClipboard);
        addMenuItem(RenderViewContextMenuQt::CopyImageUrlToClipboard);
    }

    void RenderViewContextMenuQt::appendLinkItems()
    {
        addMenuItem(RenderViewContextMenuQt::OpenLinkInNewTab);
        addMenuItem(RenderViewContextMenuQt::OpenLinkInNewWindow);
        appendSeparatorItem();
        addMenuItem(RenderViewContextMenuQt::DownloadLinkToDisk);
        addMenuItem(RenderViewContextMenuQt::CopyLinkToClipboard);
    }

    void RenderViewContextMenuQt::appendMediaItems()
    {
        addMenuItem(RenderViewContextMenuQt::ToggleMediaLoop);
        if (m_contextData.mediaFlags() & QtWebEngineCore::WebEngineContextMenuData::MediaCanToggleControls)
            addMenuItem(RenderViewContextMenuQt::ToggleMediaControls);
        addMenuItem(RenderViewContextMenuQt::DownloadMediaToDisk);
        addMenuItem(RenderViewContextMenuQt::CopyMediaUrlToClipboard);
    }

    void RenderViewContextMenuQt::appendPageItems()
    {
        addMenuItem(RenderViewContextMenuQt::Back);
        addMenuItem(RenderViewContextMenuQt::Forward);
        addMenuItem(RenderViewContextMenuQt::Reload);
        appendSeparatorItem();
        addMenuItem(RenderViewContextMenuQt::SavePage);
    }

    void RenderViewContextMenuQt::appendSpellingSuggestionItems()
    {
        addMenuItem(RenderViewContextMenuQt::SpellingSuggestions);
    }

    void RenderViewContextMenuQt::appendSeparatorItem()
    {
        addMenuItem(RenderViewContextMenuQt::Separator);
    }

    bool RenderViewContextMenuQt::canViewSource()
    {
        return m_contextData.linkText().isEmpty()
               && !m_contextData.linkUrl().isValid()
               && !m_contextData.mediaUrl().isValid()
               && !m_contextData.isEditable()
               && m_contextData.selectedText().isEmpty();
    }
}
