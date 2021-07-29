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

#ifndef WIDGETBOX_H
#define WIDGETBOX_H

#include "widgetbox_global.h"
#include <qdesigner_widgetbox_p.h>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class WidgetBoxTreeWidget;

class QT_WIDGETBOX_EXPORT WidgetBox : public QDesignerWidgetBox
{
    Q_OBJECT
public:
    explicit WidgetBox(QDesignerFormEditorInterface *core, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~WidgetBox() override;

    QDesignerFormEditorInterface *core() const;

    int categoryCount() const override;
    Category category(int cat_idx) const override;
    void addCategory(const Category &cat) override;
    void removeCategory(int cat_idx) override;

    int widgetCount(int cat_idx) const override;
    Widget widget(int cat_idx, int wgt_idx) const override;
    void addWidget(int cat_idx, const Widget &wgt) override;
    void removeWidget(int cat_idx, int wgt_idx) override;

    void dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list, const QPoint &global_mouse_pos) override;

    void setFileName(const QString &file_name) override;
    QString fileName() const override;
    bool load() override;
    bool save() override;

    bool loadContents(const QString &contents) override;
    QIcon iconForWidget(const QString &className, const QString &category = QString()) const override;

protected:
    void dragEnterEvent (QDragEnterEvent * event) override;
    void dragMoveEvent(QDragMoveEvent * event) override;
    void dropEvent (QDropEvent * event) override;

private slots:
    void handleMousePress(const QString &name, const QString &xml, const QPoint &global_mouse_pos);

private:
    QDesignerFormEditorInterface *m_core;
    WidgetBoxTreeWidget *m_view;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // WIDGETBOX_H
