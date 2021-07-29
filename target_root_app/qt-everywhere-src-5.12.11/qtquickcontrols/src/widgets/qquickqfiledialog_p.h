/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Dialogs module of the Qt Toolkit.
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

#ifndef QQUICKQFILEDIALOG_P_H
#define QQUICKQFILEDIALOG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtWidgets/qtwidgetsglobal.h>

#if QT_CONFIG(filedialog)

#include <QFileDialog>
#include "../dialogs/qquickabstractfiledialog_p.h"

QT_BEGIN_NAMESPACE

class QQuickQFileDialog : public QQuickAbstractFileDialog
{
    Q_OBJECT

public:
    QQuickQFileDialog(QObject *parent = 0);
    virtual ~QQuickQFileDialog();

    QList<QUrl> fileUrls() const override;

protected:
    QPlatformFileDialogHelper *helper() override;

    Q_DISABLE_COPY(QQuickQFileDialog)
};

class QFileDialogHelper : public QPlatformFileDialogHelper
{
    Q_OBJECT
public:
    QFileDialogHelper();

    bool defaultNameFilterDisables() const override { return true; }
    void setDirectory(const QUrl &dir) override { m_dialog.setDirectoryUrl(dir); }
    QUrl directory() const override { return m_dialog.directoryUrl(); }
    void selectFile(const QUrl &f) override { m_dialog.selectUrl(f); }
    QList<QUrl> selectedFiles() const override;
    void setFilter() override;
    void selectNameFilter(const QString &f) override { m_dialog.selectNameFilter(f); }
    QString selectedNameFilter() const override { return m_dialog.selectedNameFilter(); }
    void exec() override { m_dialog.exec(); }
    bool show(Qt::WindowFlags f, Qt::WindowModality m, QWindow *parent) override;
    void hide() override { m_dialog.hide(); }

private Q_SLOTS:
    void currentChanged(const QString& path);
    void directoryEntered(const QString& path);
    void fileSelected(const QString& path);
    void filesSelected(const QStringList& paths);

private:
    QFileDialog m_dialog;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickQFileDialog *)

#endif // QT_CONFIG(filedialog)

#endif // QQUICKQFILEDIALOG_P_H
