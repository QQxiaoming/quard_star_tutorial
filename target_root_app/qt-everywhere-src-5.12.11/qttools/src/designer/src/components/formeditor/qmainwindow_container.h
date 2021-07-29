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

#ifndef QMAINWINDOW_CONTAINER_H
#define QMAINWINDOW_CONTAINER_H

#include <QtDesigner/container.h>
#include <QtDesigner/default_extensionfactory.h>

#include <extensionfactory_p.h>

#include <QtWidgets/qmainwindow.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QMainWindowContainer: public QObject, public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit QMainWindowContainer(QMainWindow *widget, QObject *parent = 0);

    int count() const override;
    QWidget *widget(int index) const override;
    int currentIndex() const override;
    void setCurrentIndex(int index) override;
    void addWidget(QWidget *widget) override;
    void insertWidget(int index, QWidget *widget) override;
    void remove(int index) override;

private:
    QMainWindow *m_mainWindow;
    QWidgetList m_widgets;
};

typedef ExtensionFactory<QDesignerContainerExtension, QMainWindow, QMainWindowContainer> QMainWindowContainerFactory;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QMAINWINDOW_CONTAINER_H
