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

#ifndef QDESIGNER_RESOURCE_H
#define QDESIGNER_RESOURCE_H

#include "formeditor_global.h"
#include "qsimpleresource_p.h"

#include <QtCore/qhash.h>
#include <QtCore/qstack.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class DomCustomWidget;
class DomCustomWidgets;
class DomResource;

class QDesignerContainerExtension;
class QDesignerFormEditorInterface;
class QDesignerCustomWidgetInterface;
class QDesignerWidgetDataBaseItemInterface;

class QTabWidget;
class QStackedWidget;
class QToolBox;
class QToolBar;
class QDesignerDockWidget;
class QLayoutWidget;
class QWizardPage;

namespace qdesigner_internal {

class FormWindow;

class QT_FORMEDITOR_EXPORT QDesignerResource : public QEditorFormBuilder
{
public:
    explicit QDesignerResource(FormWindow *fw);
    ~QDesignerResource() override;

    void save(QIODevice *dev, QWidget *widget) override;

    bool copy(QIODevice *dev, const FormBuilderClipboard &selection) override;
    DomUI *copy(const FormBuilderClipboard &selection) override;

    FormBuilderClipboard paste(DomUI *ui, QWidget *widgetParent, QObject *actionParent = 0) override;
    FormBuilderClipboard paste(QIODevice *dev,  QWidget *widgetParent, QObject *actionParent = 0) override;

    bool saveRelative() const;
    void setSaveRelative(bool relative);

    QWidget *load(QIODevice *dev, QWidget *parentWidget) override;

    DomUI *readUi(QIODevice *dev);
    QWidget *loadUi(DomUI *ui, QWidget *parentWidget);

protected:
    using QEditorFormBuilder::create;
    using QEditorFormBuilder::createDom;

    void saveDom(DomUI *ui, QWidget *widget) override;
    QWidget *create(DomUI *ui, QWidget *parentWidget) override;
    QWidget *create(DomWidget *ui_widget, QWidget *parentWidget) override;
    QLayout *create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget) override;
    QLayoutItem *create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget) override;
    void applyProperties(QObject *o, const QList<DomProperty*> &properties) override;
    QList<DomProperty*> computeProperties(QObject *obj) override;
    DomProperty *createProperty(QObject *object, const QString &propertyName, const QVariant &value) override;

    QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name) override;
    QLayout *createLayout(const QString &layoutName, QObject *parent, const QString &name) override;
    void createCustomWidgets(DomCustomWidgets *) override;
    void createResources(DomResources*) override;
    void applyTabStops(QWidget *widget, DomTabStops *tabStops) override;

    bool addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout) override;
    bool addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget) override;

    DomWidget *createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive = true) override;
    DomLayout *createDom(QLayout *layout, DomLayout *ui_layout, DomWidget *ui_parentWidget) override;
    DomLayoutItem *createDom(QLayoutItem *item, DomLayout *ui_layout, DomWidget *ui_parentWidget) override;

    QAction *create(DomAction *ui_action, QObject *parent) override;
    QActionGroup *create(DomActionGroup *ui_action_group, QObject *parent) override;
    void addMenuAction(QAction *action) override;

    DomAction *createDom(QAction *action) override;
    DomActionGroup *createDom(QActionGroup *actionGroup) override;
    DomActionRef *createActionRefDom(QAction *action) override;

    QAction *createAction(QObject *parent, const QString &name) override;
    QActionGroup *createActionGroup(QObject *parent, const QString &name) override;

    bool checkProperty(QObject *obj, const QString &prop) const override;

    DomWidget *saveWidget(QTabWidget *widget, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QStackedWidget *widget, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QToolBox *widget, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QWidget *widget, QDesignerContainerExtension *container, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QToolBar *toolBar, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QDesignerDockWidget *dockWidget, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QWizardPage *wizardPage, DomWidget *ui_parentWidget);

    DomCustomWidgets *saveCustomWidgets() override;
    DomTabStops *saveTabStops() override;
    DomResources *saveResources() override;

    void layoutInfo(DomLayout *layout, QObject *parent, int *margin, int *spacing) override;

    void loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget) override;

    void changeObjectName(QObject *o, QString name);
    DomProperty *applyProperStdSetAttribute(QObject *object, const QString &propertyName, DomProperty *property);

private:
    DomResources *saveResources(const QStringList &qrcPaths);
    bool canCompressSpacings(QObject *object) const;
    QStringList mergeWithLoadedPaths(const QStringList &paths) const;
    void applyAttributesToPropertySheet(const DomWidget *ui_widget, QWidget *widget);

    typedef QList<DomCustomWidget*> DomCustomWidgetList;
    void addCustomWidgetsToWidgetDatabase(DomCustomWidgetList& list);
    FormWindow *m_formWindow;
    bool m_isMainWidget;
    QHash<QString, QString> m_internal_to_qt;
    QHash<QString, QString> m_qt_to_internal;
    QStack<QLayout*> m_chain;
    QHash<QDesignerWidgetDataBaseItemInterface*, bool> m_usedCustomWidgets;
    bool m_copyWidget;
    QWidget *m_selected;
    class QDesignerResourceBuilder *m_resourceBuilder;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_RESOURCE_H
