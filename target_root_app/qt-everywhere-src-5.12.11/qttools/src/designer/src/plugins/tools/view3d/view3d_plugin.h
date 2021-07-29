/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#ifndef Q3VIEW3D_PLUGIN_H
#define Q3VIEW3D_PLUGIN_H

#include "view3d_global.h"

#include <QtCore/qlist.h>
#include <QtCore/qhash.h>
#include <QtCore/qpointer.h>
#include <QtDesigner/qdesignerformeditorplugininterface.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QView3DTool;
class QAction;

class VIEW3D_EXPORT QView3DPlugin : public QObject, public QDesignerFormEditorPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerFormEditorPluginInterface)
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.Designer.QDesignerFormEditorPluginInterface")

public:
    QView3DPlugin();
    bool isInitialized() const override;
    void initialize(QDesignerFormEditorInterface *core) override;
    QAction *action() const override;
    QDesignerFormEditorInterface *core() const override;

public slots:
    void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow);

private slots:
    void addFormWindow(QDesignerFormWindowInterface *formWindow);
    void removeFormWindow(QDesignerFormWindowInterface *formWindow);

private:
    QPointer<QDesignerFormEditorInterface> m_core;
    QHash<QDesignerFormWindowInterface*, QView3DTool*> m_tool_list;
    QAction *m_action;
};

#endif // QVIEW3D_PLUGIN_H

QT_END_NAMESPACE
