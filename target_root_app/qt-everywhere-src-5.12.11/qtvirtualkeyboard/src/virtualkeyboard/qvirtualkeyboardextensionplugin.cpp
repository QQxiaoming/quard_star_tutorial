/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include <QVirtualKeyboardExtensionPlugin>

QT_BEGIN_NAMESPACE

/*!
    \class QVirtualKeyboardExtensionPlugin
    \inmodule QtVirtualKeyboard
    \brief Extension plugin for the Qt Virtual Keyboard.

    Extension plugin allows customizing and extending the Qt Virtual Keyboard
    functionality. Extension plugin can provide additional keyboard layouts and
    input methods.

    Virtual keyboard loads all the extension plugins at startup. It searches for
    \c plugins/virtualkeyboard directory and matches the metadata found in the
    plugin. If there are two or more extension plugins with the same \c Name, it
    loads the one with the highest \c Version number.

    \sa {Virtual Keyboard Extension Plugin}
*/

QVirtualKeyboardExtensionPlugin::~QVirtualKeyboardExtensionPlugin()
{
}

/*!
    If the plugin metadata contains \c InputMethod field defining an input method
    name, Qt Virtual Keyboard will call registerTypes() for registering the input
    method as QML type. The type must be registered with a \a uri if the input method
    is used by the default keyboard layouts. If the input method type is only used in
    private layouts (known only by the plugin), the uri can be omitted and chosen
    freely.
*/
void QVirtualKeyboardExtensionPlugin::registerTypes(const char *uri) const
{
    Q_UNUSED(uri)
}

QT_END_NAMESPACE
