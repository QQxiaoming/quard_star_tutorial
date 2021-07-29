/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "selectsignaldialog_p.h"

#include "ui_selectsignaldialog.h"

#include <QtDesigner/abstractformeditor.h>
#include <QtDesigner/abstractpromotioninterface.h>

#include "abstractintrospection_p.h"
#include "metadatabase_p.h"
#include "widgetdatabase_p.h"

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qdesktopwidget.h>
#include <QtWidgets/qpushbutton.h>
#include <QtGui/qstandarditemmodel.h>
#include <QtCore/qitemselectionmodel.h>
#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

enum { MethodRole = Qt::UserRole + 1 };

typedef QVector<SelectSignalDialog::Method> Methods;

SelectSignalDialog::SelectSignalDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::SelectSignalDialog)
    , m_model(new QStandardItemModel(0, 1, this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    m_ui->setupUi(this);
    m_okButton = m_ui->buttonBox->button(QDialogButtonBox::Ok);

    m_ui->signalList->setModel(m_model);
    connect(m_ui->signalList->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &SelectSignalDialog::currentChanged);
    connect(m_ui->signalList, &QTreeView::activated,
            this, &SelectSignalDialog::activated);
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    resize(availableGeometry.width() / 5, availableGeometry.height() / 2);
}

SelectSignalDialog::~SelectSignalDialog()
{
    delete m_ui;
}

SelectSignalDialog::Method SelectSignalDialog::selectedMethod() const
{
    return methodFromIndex(m_ui->signalList->currentIndex());
}

SelectSignalDialog::Method SelectSignalDialog::methodFromIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        const QStandardItem *item = m_model->itemFromIndex(index);
        const QVariant data = item->data(MethodRole);
        if (data.canConvert<Method>())
            return data.value<Method>();
    }
    return Method();
}

static QStandardItem *createTopLevelItem(const QString &text)
{
    QStandardItem *result = new QStandardItem(text);
    result->setFlags(Qt::ItemIsEnabled);
    return result;
}

static bool signatureLessThan(const SelectSignalDialog::Method &m1, const SelectSignalDialog::Method &m2)
{
    return m1.signature.compare(m2.signature) < 0;
}

// Append a class with alphabetically sorted methods to the model
static void appendClass(const QString &className, Methods methods, QStandardItemModel *model)
{
    if (methods.isEmpty())
        return;
    std::sort(methods.begin(), methods.end(), signatureLessThan);
    QStandardItem *topLevelItem = createTopLevelItem(className);
    model->appendRow(topLevelItem);
    for (const SelectSignalDialog::Method &m : qAsConst(methods)) {
        QStandardItem *item = new QStandardItem(m.signature);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setData(qVariantFromValue(m), MethodRole);
        topLevelItem->appendRow(item);
    }
}

static QString declaredInClass(const QDesignerMetaObjectInterface *metaObject, const QString &member)
{
    // Find class whose superclass does not contain the method.
    const QDesignerMetaObjectInterface *meta = metaObject;

    for (;;) {
        const QDesignerMetaObjectInterface *tmpMeta = meta->superClass();
        if (tmpMeta == 0)
            break;
        if (tmpMeta->indexOfMethod(member) == -1)
            break;
        meta = tmpMeta;
    }
    return meta->className();
}

static inline QString msgNoSignals()
{
    return QCoreApplication::translate("QDesignerTaskMenu", "no signals available");
}

void SelectSignalDialog::populate(QDesignerFormEditorInterface *core, QObject *object,
                                  const QString &defaultSignal)
{
    m_okButton->setEnabled(false);

    populateModel(core, object);

    if (m_model->rowCount() == 0) {
        m_model->appendRow(createTopLevelItem(msgNoSignals()));
        return;
    }

    m_ui->signalList->expandAll();
    m_ui->signalList->resizeColumnToContents(0);

    QModelIndex selectedIndex;
    if (defaultSignal.isEmpty()) {
        selectedIndex = m_model->index(0, 0, m_model->index(0, 0, QModelIndex())); // first method
    } else {
        const QList<QStandardItem *> items = m_model->findItems(defaultSignal, Qt::MatchExactly | Qt::MatchRecursive, 0);
        if (!items.isEmpty())
            selectedIndex = m_model->indexFromItem(items.constFirst());
    }

    if (selectedIndex.isValid())
        m_ui->signalList->setCurrentIndex(selectedIndex);
}

void SelectSignalDialog::populateModel(QDesignerFormEditorInterface *core, QObject *object)
{
    m_model->removeRows(0, m_model->rowCount());

    // Populate methods list in reverse order, starting from derived class.
    if (object->isWidgetType() && qobject_cast<WidgetDataBase *>(core->widgetDataBase())) {
        const QDesignerWidgetDataBaseInterface *db = core->widgetDataBase();
        const QString promotedClassName = promotedCustomClassName(core, static_cast<QWidget *>(object));
        const int index = db->indexOfClassName(promotedClassName);
        if (index >= 0) {
            Methods methods;
            WidgetDataBaseItem* item = static_cast<WidgetDataBaseItem*>(db->item(index));
            const QStringList fakeSignals = item->fakeSignals();
            for (const QString &fakeSignal : fakeSignals)
                methods.append(SelectSignalDialog::Method(promotedClassName, fakeSignal));
            appendClass(promotedClassName, methods, m_model);
        }
    }

    // fake signals
    if (qdesigner_internal::MetaDataBase *metaDataBase
        = qobject_cast<qdesigner_internal::MetaDataBase *>(core->metaDataBase())) {
        Methods methods;
        qdesigner_internal::MetaDataBaseItem *item = metaDataBase->metaDataBaseItem(object);
        Q_ASSERT(item);
        const QStringList fakeSignals = item->fakeSignals();
        for (const QString &fakeSignal : fakeSignals)
            methods.append(SelectSignalDialog::Method(item->customClassName(), fakeSignal));
        appendClass(item->customClassName(), methods, m_model);
    }

    // "real" signals
    if (const QDesignerMetaObjectInterface *metaObject = core->introspection()->metaObject(object)) {
        QString lastClassName;
        Methods methods;
        for (int i = metaObject->methodCount() - 1; i >= 0; --i) {
            const QDesignerMetaMethodInterface *metaMethod = metaObject->method(i);
            if (metaMethod->methodType() == QDesignerMetaMethodInterface::Signal) {
                const QString signature = metaMethod->signature();
                const QString className = declaredInClass(metaObject, signature);
                if (lastClassName.isEmpty()) {
                    lastClassName = className;
                } else if (className != lastClassName) {
                    appendClass(lastClassName, methods, m_model);
                    lastClassName = className;
                    methods.clear();
                }
                methods.append(SelectSignalDialog::Method(className, signature,
                                                          metaMethod->parameterNames()));
            }
        }
        appendClass(lastClassName, methods, m_model);
    }
}

void SelectSignalDialog::activated(const QModelIndex &index)
{
    if (methodFromIndex(index).isValid())
        m_okButton->animateClick();
}

void SelectSignalDialog::currentChanged(const QModelIndex &current, const QModelIndex &)
{
    m_okButton->setEnabled(methodFromIndex(current).isValid());
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
