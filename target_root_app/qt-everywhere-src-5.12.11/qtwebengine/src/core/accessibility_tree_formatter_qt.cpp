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

#include "content/browser/accessibility/accessibility_tree_formatter_browser.h"

#include <utility>

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"

#include "browser_accessibility_qt.h"

namespace content {

#ifndef QT_NO_ACCESSIBILITY
class AccessibilityTreeFormatterQt : public AccessibilityTreeFormatterBrowser {
public:
    explicit AccessibilityTreeFormatterQt();
    ~AccessibilityTreeFormatterQt() override;

private:
    const base::FilePath::StringType GetExpectedFileSuffix() override;
    const std::string GetAllowEmptyString() override;
    const std::string GetAllowString() override;
    const std::string GetDenyString() override;
    void AddProperties(const BrowserAccessibility &node, base::DictionaryValue* dict) override;
    base::string16 ProcessTreeForOutput(const base::DictionaryValue &node, base::DictionaryValue * = nullptr) override;
};

AccessibilityTreeFormatterQt::AccessibilityTreeFormatterQt()
{
}

AccessibilityTreeFormatterQt::~AccessibilityTreeFormatterQt()
{
}

void AccessibilityTreeFormatterQt::AddProperties(const BrowserAccessibility &node, base::DictionaryValue *dict)
{
    dict->SetInteger("id", node.GetId());
    const BrowserAccessibilityQt *acc_node = ToBrowserAccessibilityQt(&node);

    dict->SetString("role", qAccessibleRoleString(acc_node->role()));

    QAccessible::State state = acc_node->state();

    std::vector<base::Value> states;
    if (state.busy)
        states.push_back(base::Value("busy"));
    if (state.checkable)
        states.push_back(base::Value("checkable"));
    if (state.checked)
        states.push_back(base::Value("checked"));
    if (acc_node->IsClickable())
        states.push_back(base::Value("clickable"));
    if (state.collapsed)
        states.push_back(base::Value("collapsed"));
    if (state.disabled)
        states.push_back(base::Value("disabled"));
    if (state.editable)
        states.push_back(base::Value("editable"));
    if (state.expandable)
        states.push_back(base::Value("expandable"));
    if (state.expanded)
        states.push_back(base::Value("expanded"));
    if (state.focusable)
        states.push_back(base::Value("focusable"));
    if (state.focused)
        states.push_back(base::Value("focused"));
    if (state.hasPopup)
        states.push_back(base::Value("hasPopup"));
    if (state.hotTracked)
        states.push_back(base::Value("hotTracked"));
    if (state.invisible)
        states.push_back(base::Value("invisible"));
    if (state.linked)
        states.push_back(base::Value("linked"));
    if (state.multiLine)
        states.push_back(base::Value("multiLine"));
    if (state.multiSelectable)
        states.push_back(base::Value("multiSelectable"));
    if (state.modal)
        states.push_back(base::Value("modal"));
    if (state.offscreen)
        states.push_back(base::Value("offscreen"));
    if (state.passwordEdit)
        states.push_back(base::Value("password"));
    if (state.pressed)
        states.push_back(base::Value("pressed"));
    if (state.readOnly)
        states.push_back(base::Value("readOnly"));
    if (state.selectable)
        states.push_back(base::Value("selectable"));
    if (state.selected)
        states.push_back(base::Value("selected"));
    if (state.traversed)
        states.push_back(base::Value("traversed"));
    dict->SetKey("states", base::Value(states));

    dict->SetString("name", acc_node->text(QAccessible::Name).toStdString());
    dict->SetString("description", acc_node->text(QAccessible::Description).toStdString());
}

base::string16 AccessibilityTreeFormatterQt::ProcessTreeForOutput(const base::DictionaryValue &node, base::DictionaryValue *)
{
    base::string16 error_value;
    if (node.GetString("error", &error_value))
        return error_value;

    base::string16 line;
    std::string role_value;
    node.GetString("role", &role_value);
    if (!role_value.empty())
        WriteAttribute(true, base::StringPrintf("%s", role_value.c_str()), &line);

    const base::ListValue *states_value = nullptr;
    node.GetList("states", &states_value);
    if (states_value) {
        for (const auto &state : *states_value) {
            std::string state_value;
            if (state.GetAsString(&state_value))
                WriteAttribute(true, state_value, &line);
        }
    }

    std::string name_value;
    if (node.GetString("name", &name_value))
        WriteAttribute(true, base::StringPrintf("name='%s'", name_value.c_str()), &line);

    std::string description_value;
    if (node.GetString("description", &description_value))
        WriteAttribute(false, base::StringPrintf("description='%s'", description_value.c_str()), &line);

    int id_value;
    node.GetInteger("id", &id_value);
    WriteAttribute(false, base::StringPrintf("id=%d", id_value), &line);

    return line + base::ASCIIToUTF16("\n");
}

const base::FilePath::StringType AccessibilityTreeFormatterQt::GetExpectedFileSuffix()
{
    return FILE_PATH_LITERAL("-expected-qt.txt");
}

const std::string AccessibilityTreeFormatterQt::GetAllowEmptyString()
{
    return "@QT-ALLOW-EMPTY:";
}

const std::string AccessibilityTreeFormatterQt::GetAllowString()
{
    return "@QT-ALLOW:";
}

const std::string AccessibilityTreeFormatterQt::GetDenyString()
{
    return "@QT-DENY:";
}

#endif // QT_NO_ACCESSIBILITY

// static
AccessibilityTreeFormatter* AccessibilityTreeFormatter::Create()
{
#ifndef QT_NO_ACCESSIBILITY
    return new AccessibilityTreeFormatterQt();
#else
    return nullptr;
#endif
}

} // namespace content
