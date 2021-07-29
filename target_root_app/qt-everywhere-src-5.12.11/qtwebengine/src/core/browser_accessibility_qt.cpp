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

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "browser_accessibility_qt.h"

#ifndef QT_NO_ACCESSIBILITY

#include "ui/accessibility/ax_enums.mojom.h"

#include "browser_accessibility_manager_qt.h"
#include "qtwebenginecoreglobal_p.h"
#include "type_conversion.h"

using namespace blink;
using QtWebEngineCore::toQt;

namespace content {

const BrowserAccessibilityQt *ToBrowserAccessibilityQt(const BrowserAccessibility *obj)
{
    return static_cast<const BrowserAccessibilityQt *>(obj);
}

BrowserAccessibilityQt::BrowserAccessibilityQt()
{
    QAccessible::registerAccessibleInterface(this);
}

bool BrowserAccessibilityQt::isValid() const
{
    auto managerQt = static_cast<BrowserAccessibilityManagerQt *>(manager_);
    return managerQt && managerQt->isValid();
}

QObject *BrowserAccessibilityQt::object() const
{
    return 0;
}

QAccessibleInterface *BrowserAccessibilityQt::childAt(int x, int y) const
{
    for (int i = 0; i < childCount(); ++i) {
        QAccessibleInterface *childIface = child(i);
        Q_ASSERT(childIface);
        if (childIface->rect().contains(x,y))
            return childIface;
    }
    return 0;
}

void *BrowserAccessibilityQt::interface_cast(QAccessible::InterfaceType type)
{
    switch (type) {
    case QAccessible::ActionInterface:
        if (!actionNames().isEmpty())
            return static_cast<QAccessibleActionInterface*>(this);
        break;
    case QAccessible::TextInterface:
        if (HasState(ax::mojom::State::kEditable))
            return static_cast<QAccessibleTextInterface*>(this);
        break;
    case QAccessible::ValueInterface: {
        QAccessible::Role r = role();
        if (r == QAccessible::ProgressBar ||
                r == QAccessible::Slider ||
                r == QAccessible::ScrollBar ||
                r == QAccessible::SpinBox)
            return static_cast<QAccessibleValueInterface*>(this);
        break;
    }
    case QAccessible::TableInterface: {
        QAccessible::Role r = role();
        if (r == QAccessible::Table ||
            r == QAccessible::List ||
            r == QAccessible::Tree)
            return static_cast<QAccessibleTableInterface*>(this);
        break;
    }
    case QAccessible::TableCellInterface: {
        QAccessible::Role r = role();
        if (r == QAccessible::Cell ||
            r == QAccessible::ListItem ||
            r == QAccessible::TreeItem)
            return static_cast<QAccessibleTableCellInterface*>(this);
        break;
    }
    default:
        break;
    }
    return 0;
}

QAccessibleInterface *BrowserAccessibilityQt::parent() const
{
    BrowserAccessibility *p = PlatformGetParent();
    if (p)
        return static_cast<BrowserAccessibilityQt*>(p);
    return static_cast<BrowserAccessibilityManagerQt*>(manager())->rootParentAccessible();
}

QAccessibleInterface *BrowserAccessibilityQt::child(int index) const
{
    return static_cast<BrowserAccessibilityQt*>(BrowserAccessibility::PlatformGetChild(index));
}

int BrowserAccessibilityQt::childCount() const
{
    return PlatformChildCount();
}

int BrowserAccessibilityQt::indexOfChild(const QAccessibleInterface *iface) const
{

    const BrowserAccessibilityQt *child = static_cast<const BrowserAccessibilityQt*>(iface);
    return child->GetIndexInParent();
}

QString BrowserAccessibilityQt::text(QAccessible::Text t) const
{
    switch (t) {
    case QAccessible::Name:
        return toQt(GetStringAttribute(ax::mojom::StringAttribute::kName));
    case QAccessible::Description:
        return toQt(GetStringAttribute(ax::mojom::StringAttribute::kDescription));
    case QAccessible::Value:
        return toQt(GetStringAttribute(ax::mojom::StringAttribute::kValue));
    case QAccessible::Accelerator:
        return toQt(GetStringAttribute(ax::mojom::StringAttribute::kKeyShortcuts));
    default:
        break;
    }
    return QString();
}

void BrowserAccessibilityQt::setText(QAccessible::Text t, const QString &text)
{
}

QRect BrowserAccessibilityQt::rect() const
{
    if (!manager()) // needed implicitly by GetScreenBoundsRect()
        return QRect();
    gfx::Rect bounds = GetUnclippedScreenBoundsRect();
    return QRect(bounds.x(), bounds.y(), bounds.width(), bounds.height());
}

QAccessible::Role BrowserAccessibilityQt::role() const
{
    switch (GetRole()) {
    case ax::mojom::Role::kNone:
    case ax::mojom::Role::kUnknown:
        return QAccessible::NoRole;

    // Used by Chromium to distinguish between the root of the tree
    // for this page, and a web area for a frame within this page.
    case ax::mojom::Role::kWebArea:
    case ax::mojom::Role::kWebView:
    case ax::mojom::Role::kRootWebArea: // not sure if we need to make a diff here, but this seems common
        return QAccessible::WebDocument;

    // These roles all directly correspond to blink accessibility roles,
    // keep these alphabetical.
    case ax::mojom::Role::kAbbr:
        return QAccessible::StaticText;
    case ax::mojom::Role::kAlert:
    case ax::mojom::Role::kAlertDialog:
        return QAccessible::AlertMessage;
    case ax::mojom::Role::kAnchor:
        return QAccessible::Link;
    case ax::mojom::Role::kAnnotation:
        return QAccessible::StaticText;
    case ax::mojom::Role::kApplication:
        return QAccessible::Document; // returning Application here makes Qt return the top level app object
    case ax::mojom::Role::kArticle:
        return QAccessible::Section;
    case ax::mojom::Role::kAudio:
        return QAccessible::Sound;
    case ax::mojom::Role::kBanner:
        return QAccessible::Section;
    case ax::mojom::Role::kBlockquote:
        return QAccessible::Section;
    case ax::mojom::Role::kButton:
        return QAccessible::Button;
    case ax::mojom::Role::kCanvas:
        return QAccessible::Canvas;
    case ax::mojom::Role::kCaption:
        return QAccessible::Heading;
    case ax::mojom::Role::kCaret:
        return QAccessible::NoRole; // FIXME: https://codereview.chromium.org/2781613003
    case ax::mojom::Role::kCell:
        return QAccessible::Cell;
    case ax::mojom::Role::kCheckBox:
        return QAccessible::CheckBox;
    case ax::mojom::Role::kClient:
        return QAccessible::Client;
    case ax::mojom::Role::kColorWell:
        return QAccessible::ColorChooser;
    case ax::mojom::Role::kColumn:
        return QAccessible::Column;
    case ax::mojom::Role::kColumnHeader:
        return QAccessible::ColumnHeader;
    case ax::mojom::Role::kComboBoxGrouping:
    case ax::mojom::Role::kComboBoxMenuButton:
    case ax::mojom::Role::kTextFieldWithComboBox:
        return QAccessible::ComboBox;
    case ax::mojom::Role::kComplementary:
        return QAccessible::ComplementaryContent;
    case ax::mojom::Role::kContentDeletion:
    case ax::mojom::Role::kContentInsertion:
        return QAccessible::Grouping;
    case ax::mojom::Role::kContentInfo:
        return QAccessible::Section;
    case ax::mojom::Role::kDate:
    case ax::mojom::Role::kDateTime:
        return QAccessible::Clock;
    case ax::mojom::Role::kDefinition:
        return QAccessible::Paragraph;
    case ax::mojom::Role::kDescriptionList:
        return QAccessible::List;
    case ax::mojom::Role::kDescriptionListDetail:
        return QAccessible::Paragraph;
    case ax::mojom::Role::kDescriptionListTerm:
        return QAccessible::ListItem;
    case ax::mojom::Role::kDetails:
        return QAccessible::Grouping;
    case ax::mojom::Role::kDesktop:
        return QAccessible::NoRole; // FIXME
    case ax::mojom::Role::kDialog:
        return QAccessible::Dialog;
    case ax::mojom::Role::kDirectory:
        return QAccessible::NoRole; // FIXME
    case ax::mojom::Role::kDisclosureTriangle:
        return QAccessible::NoRole; // FIXME
    case ax::mojom::Role::kGenericContainer:
        return QAccessible::Section;
    case ax::mojom::Role::kDocCover:
      return QAccessible::Graphic;
    case ax::mojom::Role::kDocBackLink:
    case ax::mojom::Role::kDocBiblioRef:
    case ax::mojom::Role::kDocGlossRef:
    case ax::mojom::Role::kDocNoteRef:
        return QAccessible::Link;
    case ax::mojom::Role::kDocBiblioEntry:
    case ax::mojom::Role::kDocEndnote:
    case ax::mojom::Role::kDocFootnote:
        return QAccessible::ListItem;
    case ax::mojom::Role::kDocPageBreak:
        return QAccessible::Separator;
    case ax::mojom::Role::kDocAbstract:
    case ax::mojom::Role::kDocAcknowledgments:
    case ax::mojom::Role::kDocAfterword:
    case ax::mojom::Role::kDocAppendix:
    case ax::mojom::Role::kDocBibliography:
    case ax::mojom::Role::kDocChapter:
    case ax::mojom::Role::kDocColophon:
    case ax::mojom::Role::kDocConclusion:
    case ax::mojom::Role::kDocCredit:
    case ax::mojom::Role::kDocCredits:
    case ax::mojom::Role::kDocDedication:
    case ax::mojom::Role::kDocEndnotes:
    case ax::mojom::Role::kDocEpigraph:
    case ax::mojom::Role::kDocEpilogue:
    case ax::mojom::Role::kDocErrata:
    case ax::mojom::Role::kDocExample:
    case ax::mojom::Role::kDocForeword:
    case ax::mojom::Role::kDocGlossary:
    case ax::mojom::Role::kDocIndex:
    case ax::mojom::Role::kDocIntroduction:
    case ax::mojom::Role::kDocNotice:
    case ax::mojom::Role::kDocPageList:
    case ax::mojom::Role::kDocPart:
    case ax::mojom::Role::kDocPreface:
    case ax::mojom::Role::kDocPrologue:
    case ax::mojom::Role::kDocPullquote:
    case ax::mojom::Role::kDocQna:
    case ax::mojom::Role::kDocSubtitle:
    case ax::mojom::Role::kDocTip:
    case ax::mojom::Role::kDocToc:
        return QAccessible::Section;
    case ax::mojom::Role::kDocument:
        return QAccessible::Document;
    case ax::mojom::Role::kEmbeddedObject:
        return QAccessible::Grouping; // FIXME
    case ax::mojom::Role::kFeed:
        return QAccessible::Section;
    case ax::mojom::Role::kFigcaption:
        return QAccessible::Heading;
    case ax::mojom::Role::kFigure:
        return QAccessible::Section;
    case ax::mojom::Role::kFooter:
        return QAccessible::Footer;
    case ax::mojom::Role::kForm:
        return QAccessible::Form;
    case ax::mojom::Role::kGraphicsDocument:
        return QAccessible::Document;
    case ax::mojom::Role::kGraphicsObject:
        return QAccessible::Pane;
    case ax::mojom::Role::kGraphicsSymbol:
        return QAccessible::Graphic;
    case ax::mojom::Role::kGrid:
        return QAccessible::Table;
    case ax::mojom::Role::kGroup:
        return QAccessible::Grouping;
    case ax::mojom::Role::kHeading:
        return QAccessible::Heading;
    case ax::mojom::Role::kIframe:
        return QAccessible::Grouping;
    case ax::mojom::Role::kIframePresentational:
        return QAccessible::NoRole; // FIXME
    case ax::mojom::Role::kIgnored:
        return QAccessible::NoRole;
    case ax::mojom::Role::kImage:
        return QAccessible::Graphic;
    case ax::mojom::Role::kImageMap:
        return QAccessible::Graphic;
    case ax::mojom::Role::kInlineTextBox:
        return QAccessible::EditableText;
    case ax::mojom::Role::kInputTime:
        return QAccessible::SpinBox;
    case ax::mojom::Role::kLabelText:
        return QAccessible::StaticText;
    case ax::mojom::Role::kLayoutTable:
    case ax::mojom::Role::kLayoutTableCell:
    case ax::mojom::Role::kLayoutTableColumn:
    case ax::mojom::Role::kLayoutTableRow:
        // No role description.
        return  QAccessible::NoRole;
    case ax::mojom::Role::kLegend:
        return QAccessible::StaticText;
    case ax::mojom::Role::kLineBreak:
        return QAccessible::Separator;
    case ax::mojom::Role::kLink:
        return QAccessible::Link;
    case ax::mojom::Role::kList:
        return QAccessible::List;
    case ax::mojom::Role::kListBox:
        return QAccessible::ComboBox;
    case ax::mojom::Role::kListBoxOption:
        return QAccessible::ListItem;
    case ax::mojom::Role::kListItem:
        return QAccessible::ListItem;
    case ax::mojom::Role::kListMarker:
        return QAccessible::StaticText;
    case ax::mojom::Role::kLog:
        return QAccessible::Section;
    case ax::mojom::Role::kMain:
        return QAccessible::Grouping;
    case ax::mojom::Role::kMark:
        return QAccessible::StaticText;
    case ax::mojom::Role::kMarquee:
        return QAccessible::Section;
    case ax::mojom::Role::kMath:
        return QAccessible::Equation;
    case ax::mojom::Role::kMenu:
        return QAccessible::PopupMenu;
    case ax::mojom::Role::kMenuBar:
        return QAccessible::MenuBar;
    case ax::mojom::Role::kMenuItem:
        return QAccessible::MenuItem;
    case ax::mojom::Role::kMenuItemCheckBox:
        return QAccessible::CheckBox;
    case ax::mojom::Role::kMenuItemRadio:
        return QAccessible::RadioButton;
    case ax::mojom::Role::kMenuButton:
        return QAccessible::MenuItem;
    case ax::mojom::Role::kMenuListOption:
        return QAccessible::MenuItem;
    case ax::mojom::Role::kMenuListPopup:
        return QAccessible::PopupMenu;
    case ax::mojom::Role::kMeter:
        return QAccessible::Chart;
    case ax::mojom::Role::kNavigation:
        return QAccessible::Section;
    case ax::mojom::Role::kNote:
        return QAccessible::Note;
    case ax::mojom::Role::kPane:
        return QAccessible::Pane;
    case ax::mojom::Role::kParagraph:
        return QAccessible::Paragraph;
    case ax::mojom::Role::kPopUpButton:
        return QAccessible::ComboBox;
    case ax::mojom::Role::kPre:
        return QAccessible::Section;
    case ax::mojom::Role::kPresentational:
        return QAccessible::NoRole; // FIXME
    case ax::mojom::Role::kProgressIndicator:
        return QAccessible::ProgressBar;
    case ax::mojom::Role::kRadioButton:
        return QAccessible::RadioButton;
    case ax::mojom::Role::kRadioGroup:
        return QAccessible::Grouping;
    case ax::mojom::Role::kRegion:
        return QAccessible::Section;
    case ax::mojom::Role::kRow:
        return QAccessible::Row;
    case ax::mojom::Role::kRowHeader:
        return QAccessible::RowHeader;
    case ax::mojom::Role::kRuby:
        return QAccessible::StaticText;
    case ax::mojom::Role::kScrollBar:
        return QAccessible::ScrollBar;
    case ax::mojom::Role::kScrollView:
        return QAccessible::NoRole; // FIXME
    case ax::mojom::Role::kSearch:
        return QAccessible::Section;
    case ax::mojom::Role::kSearchBox:
        return QAccessible::EditableText;
    case ax::mojom::Role::kSlider:
        return QAccessible::Slider;
    case ax::mojom::Role::kSliderThumb:
        return QAccessible::NoRole; // FIXME
    case ax::mojom::Role::kSpinButton:
        return QAccessible::SpinBox;
    case ax::mojom::Role::kSplitter:
        return QAccessible::Splitter;
    case ax::mojom::Role::kStaticText:
        return QAccessible::StaticText;
    case ax::mojom::Role::kStatus:
        return QAccessible::Indicator;
    case ax::mojom::Role::kSvgRoot:
        return QAccessible::Graphic;
    case ax::mojom::Role::kSwitch:
        return QAccessible::Button;
    case ax::mojom::Role::kTable:
        return QAccessible::Table;
    case ax::mojom::Role::kTableHeaderContainer:
        return QAccessible::Section;
    case ax::mojom::Role::kTab:
        return QAccessible::PageTab;
    case ax::mojom::Role::kTabList:
        return QAccessible::PageTabList;
    case ax::mojom::Role::kTabPanel:
        return QAccessible::PageTab;
    case ax::mojom::Role::kTerm:
        return QAccessible::StaticText;
    case ax::mojom::Role::kTextField:
        return QAccessible::EditableText;
    case ax::mojom::Role::kTime:
    case ax::mojom::Role::kTimer:
        return QAccessible::Clock;
    case ax::mojom::Role::kTitleBar:
        return QAccessible::NoRole; // FIXME
    case ax::mojom::Role::kToggleButton:
        return QAccessible::Button;
    case ax::mojom::Role::kToolbar:
        return QAccessible::ToolBar;
    case ax::mojom::Role::kTooltip:
        return QAccessible::ToolTip;
    case ax::mojom::Role::kTree:
        return QAccessible::Tree;
    case ax::mojom::Role::kTreeGrid:
        return QAccessible::Tree;
    case ax::mojom::Role::kTreeItem:
        return QAccessible::TreeItem;
    case ax::mojom::Role::kVideo:
        return QAccessible::Animation;
    case ax::mojom::Role::kWindow:
        return QAccessible::Window;
    }
    return QAccessible::NoRole;
}

QAccessible::State BrowserAccessibilityQt::state() const
{
    QAccessible::State state = QAccessible::State();
    if (HasState(ax::mojom::State::kCollapsed))
        state.collapsed = true;
    if (HasState(ax::mojom::State::kDefault))
        state.defaultButton = true;
    if (HasState(ax::mojom::State::kEditable))
        state.editable = true;
    if (HasState(ax::mojom::State::kExpanded))
        state.expanded = true;
    if (HasState(ax::mojom::State::kFocusable))
        state.focusable = true;
    if (HasState(ax::mojom::State::kHorizontal))
    {} // FIXME
    if (HasState(ax::mojom::State::kHovered))
        state.hotTracked = true;
    if (HasState(ax::mojom::State::kIgnored))
    {} // FIXME
    if (HasState(ax::mojom::State::kInvisible))
        state.invisible = true;
    if (HasState(ax::mojom::State::kLinked))
        state.linked = true;
    if (HasState(ax::mojom::State::kMultiline))
        state.multiLine = true;
    if (HasState(ax::mojom::State::kMultiselectable))
        state.multiSelectable = true;
    if (HasState(ax::mojom::State::kProtected))
        state.passwordEdit = true;
    if (HasState(ax::mojom::State::kRequired))
    {} // FIXME
    if (HasState(ax::mojom::State::kRichlyEditable))
    {} // FIXME
    if (HasState(ax::mojom::State::kVertical))
    {} // FIXME
    if (HasState(ax::mojom::State::kVisited))
        state.traversed = true;

    if (IsOffscreen())
        state.offscreen = true;
    if (manager()->GetFocus() == this)
        state.focused = true;
    if (GetBoolAttribute(ax::mojom::BoolAttribute::kBusy))
        state.busy = true;
    if (GetBoolAttribute(ax::mojom::BoolAttribute::kModal))
        state.modal = true;
    if (HasBoolAttribute(ax::mojom::BoolAttribute::kSelected)) {
        state.selectable = true;
        state.selected = GetBoolAttribute(ax::mojom::BoolAttribute::kSelected);
    }
    if (HasIntAttribute(ax::mojom::IntAttribute::kCheckedState)) {
        state.checkable = true;
        const ax::mojom::CheckedState checkedState =
                static_cast<ax::mojom::CheckedState>(GetIntAttribute(ax::mojom::IntAttribute::kCheckedState));
        switch (checkedState) {
        case ax::mojom::CheckedState::kTrue:
            if (GetRole() == ax::mojom::Role::kToggleButton)
                state.pressed = true;
            else
                state.checked = true;
            break;
        case ax::mojom::CheckedState::kMixed:
            state.checkStateMixed = true;
            break;
        case ax::mojom::CheckedState::kFalse:
        case ax::mojom::CheckedState::kNone:
            break;
        }
    }
    if (HasIntAttribute(ax::mojom::IntAttribute::kRestriction)) {
        const ax::mojom::Restriction restriction = static_cast<ax::mojom::Restriction>(GetIntAttribute(ax::mojom::IntAttribute::kRestriction));
        switch (restriction) {
        case ax::mojom::Restriction::kReadOnly:
            state.readOnly = true;
            break;
        case ax::mojom::Restriction::kDisabled:
            state.disabled = true;
            break;
        case ax::mojom::Restriction::kNone:
            break;
        }
    }
    if (HasIntAttribute(ax::mojom::IntAttribute::kHasPopup)) {
        const ax::mojom::HasPopup hasPopup = static_cast<ax::mojom::HasPopup>(GetIntAttribute(ax::mojom::IntAttribute::kHasPopup));
        switch (hasPopup) {
        case ax::mojom::HasPopup::kFalse:
            break;
        case ax::mojom::HasPopup::kTrue:
        case ax::mojom::HasPopup::kMenu:
        case ax::mojom::HasPopup::kListbox:
        case ax::mojom::HasPopup::kTree:
        case ax::mojom::HasPopup::kGrid:
        case ax::mojom::HasPopup::kDialog:
            state.hasPopup = true;
            break;
        }
    }
    return state;
}

// Qt does not reference count accessibles
void BrowserAccessibilityQt::NativeAddReference()
{
}

// there is no reference counting, but BrowserAccessibility::Destroy
// calls this (and that is the only place in the chromium sources,
// so we can safely use it to dispose of ourselves here
// (the default implementation of this function just contains a "delete this")
void BrowserAccessibilityQt::NativeReleaseReference()
{
    // delete this
    QAccessible::Id interfaceId = QAccessible::uniqueId(this);
    QAccessible::deleteAccessibleInterface(interfaceId);
}

QStringList BrowserAccessibilityQt::actionNames() const
{
    QStringList actions;
    if (HasState(ax::mojom::State::kFocusable))
        actions << QAccessibleActionInterface::setFocusAction();
    return actions;
}

void BrowserAccessibilityQt::doAction(const QString &actionName)
{
    if (actionName == QAccessibleActionInterface::setFocusAction())
        manager()->SetFocus(*this);
}

QStringList BrowserAccessibilityQt::keyBindingsForAction(const QString &actionName) const
{
    QT_NOT_YET_IMPLEMENTED
    return QStringList();
}

void BrowserAccessibilityQt::addSelection(int startOffset, int endOffset)
{
    manager()->SetSelection(AXPlatformRange(CreatePositionAt(startOffset), CreatePositionAt(endOffset)));
}

QString BrowserAccessibilityQt::attributes(int offset, int *startOffset, int *endOffset) const
{
    *startOffset = offset;
    *endOffset = offset;
    return QString();
}

int BrowserAccessibilityQt::cursorPosition() const
{
    int pos = 0;
    GetIntAttribute(ax::mojom::IntAttribute::kTextSelStart, &pos);
    return pos;
}

QRect BrowserAccessibilityQt::characterRect(int /*offset*/) const
{
    QT_NOT_YET_IMPLEMENTED
    return QRect();
}

int BrowserAccessibilityQt::selectionCount() const
{
    int start = 0;
    int end = 0;
    GetIntAttribute(ax::mojom::IntAttribute::kTextSelStart, &start);
    GetIntAttribute(ax::mojom::IntAttribute::kTextSelEnd, &end);
    if (start != end)
        return 1;
    return 0;
}

int BrowserAccessibilityQt::offsetAtPoint(const QPoint &/*point*/) const
{
    QT_NOT_YET_IMPLEMENTED
    return 0;
}

void BrowserAccessibilityQt::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
    Q_ASSERT(startOffset && endOffset);
    *startOffset = 0;
    *endOffset = 0;
    if (selectionIndex != 0)
        return;
    GetIntAttribute(ax::mojom::IntAttribute::kTextSelStart, startOffset);
    GetIntAttribute(ax::mojom::IntAttribute::kTextSelEnd, endOffset);
}

QString BrowserAccessibilityQt::text(int startOffset, int endOffset) const
{
    return text(QAccessible::Value).mid(startOffset, endOffset - startOffset);
}

void BrowserAccessibilityQt::removeSelection(int selectionIndex)
{
    manager()->SetSelection(AXPlatformRange(CreatePositionAt(0), CreatePositionAt(0)));
}

void BrowserAccessibilityQt::setCursorPosition(int position)
{
    manager()->SetSelection(AXPlatformRange(CreatePositionAt(position), CreatePositionAt(position)));
}

void BrowserAccessibilityQt::setSelection(int selectionIndex, int startOffset, int endOffset)
{
    if (selectionIndex != 0)
        return;
    manager()->SetSelection(AXPlatformRange(CreatePositionAt(startOffset), CreatePositionAt(endOffset)));
}

int BrowserAccessibilityQt::characterCount() const
{
    return text(QAccessible::Value).length();
}

void BrowserAccessibilityQt::scrollToSubstring(int startIndex, int endIndex)
{
    int count = characterCount();
    if (startIndex < endIndex && endIndex < count)
        manager()->ScrollToMakeVisible(*this, GetPageBoundsForRange(startIndex, endIndex - startIndex));
}

QVariant BrowserAccessibilityQt::currentValue() const
{
    QVariant result;
    float value;
    if (GetFloatAttribute(ax::mojom::FloatAttribute::kValueForRange, &value)) {
        result = (double) value;
    }
    return result;
}

void BrowserAccessibilityQt::setCurrentValue(const QVariant &value)
{
    // not yet implemented anywhere in blink
    QT_NOT_YET_IMPLEMENTED
}

QVariant BrowserAccessibilityQt::maximumValue() const
{
    QVariant result;
    float value;
    if (GetFloatAttribute(ax::mojom::FloatAttribute::kMaxValueForRange, &value)) {
        result = (double) value;
    }
    return result;
}

QVariant BrowserAccessibilityQt::minimumValue() const
{
    QVariant result;
    float value;
    if (GetFloatAttribute(ax::mojom::FloatAttribute::kMinValueForRange, &value)) {
        result = (double) value;
    }
    return result;
}

QVariant BrowserAccessibilityQt::minimumStepSize() const
{
    QVariant result;
    float value;
    if (GetFloatAttribute(ax::mojom::FloatAttribute::kStepValueForRange, &value)) {
        result = (double) value;
    }
    return result;
}

QAccessibleInterface *BrowserAccessibilityQt::cellAt(int row, int column) const
{
    int columns = 0;
    int rows = 0;
    if (!GetIntAttribute(ax::mojom::IntAttribute::kTableColumnCount, &columns) ||
        !GetIntAttribute(ax::mojom::IntAttribute::kTableRowCount, &rows) ||
        columns <= 0 ||
        rows <= 0) {
      return 0;
    }

    if (row < 0 || row >= rows || column < 0 || column >= columns)
      return 0;

    int cell_id = GetCellId(row, column);
    BrowserAccessibility* cell = manager()->GetFromID(cell_id);
    if (cell) {
      QAccessibleInterface *iface = static_cast<BrowserAccessibilityQt*>(cell);
      return iface;
    }

    return 0;
}

QAccessibleInterface *BrowserAccessibilityQt::caption() const
{
    return 0;
}

QAccessibleInterface *BrowserAccessibilityQt::summary() const
{
    return 0;
}

QString BrowserAccessibilityQt::columnDescription(int column) const
{
    return QString();
}

QString BrowserAccessibilityQt::rowDescription(int row) const
{
    return QString();
}

int BrowserAccessibilityQt::columnCount() const
{
    int columns = 0;
    if (GetIntAttribute(ax::mojom::IntAttribute::kTableColumnCount, &columns))
        return columns;

    return 0;
}

int BrowserAccessibilityQt::rowCount() const
{
    int rows = 0;
    if (GetIntAttribute(ax::mojom::IntAttribute::kTableRowCount, &rows))
      return rows;
    return 0;
}

int BrowserAccessibilityQt::selectedCellCount() const
{
    return 0;
}

int BrowserAccessibilityQt::selectedColumnCount() const
{
    return 0;
}

int BrowserAccessibilityQt::selectedRowCount() const
{
    return 0;
}

QList<QAccessibleInterface *> BrowserAccessibilityQt::selectedCells() const
{
    return QList<QAccessibleInterface *>();
}

QList<int> BrowserAccessibilityQt::selectedColumns() const
{
    return QList<int>();
}

QList<int> BrowserAccessibilityQt::selectedRows() const
{
    return QList<int>();
}

bool BrowserAccessibilityQt::isColumnSelected(int /*column*/) const
{
    return false;
}

bool BrowserAccessibilityQt::isRowSelected(int /*row*/) const
{
    return false;
}

bool BrowserAccessibilityQt::selectRow(int /*row*/)
{
    return false;
}

bool BrowserAccessibilityQt::selectColumn(int /*column*/)
{
    return false;
}

bool BrowserAccessibilityQt::unselectRow(int /*row*/)
{
    return false;
}

bool BrowserAccessibilityQt::unselectColumn(int /*column*/)
{
    return false;
}

int BrowserAccessibilityQt::columnExtent() const
{
    return 1;
}

QList<QAccessibleInterface *> BrowserAccessibilityQt::columnHeaderCells() const
{
    return QList<QAccessibleInterface*>();
}

int BrowserAccessibilityQt::columnIndex() const
{
    int column = 0;
    if (GetIntAttribute(ax::mojom::IntAttribute::kTableCellColumnIndex, &column))
      return column;
    return 0;
}

int BrowserAccessibilityQt::rowExtent() const
{
    return 1;
}

QList<QAccessibleInterface *> BrowserAccessibilityQt::rowHeaderCells() const
{
    return QList<QAccessibleInterface*>();
}

int BrowserAccessibilityQt::rowIndex() const
{
    int row = 0;
    if (GetIntAttribute(ax::mojom::IntAttribute::kTableCellRowIndex, &row))
      return row;
    return 0;
}

bool BrowserAccessibilityQt::isSelected() const
{
    return false;
}

QAccessibleInterface *BrowserAccessibilityQt::table() const
{
    BrowserAccessibility* find_table = PlatformGetParent();
    while (find_table && find_table->GetRole() != ax::mojom::Role::kTable)
        find_table = find_table->PlatformGetParent();
    if (!find_table)
        return 0;
    return static_cast<BrowserAccessibilityQt*>(find_table);
}

void BrowserAccessibilityQt::modelChange(QAccessibleTableModelChangeEvent *)
{

}

} // namespace content

#endif // QT_NO_ACCESSIBILITY
