/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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
#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include <QtCore/QMutex>
#include <QtWidgets/QTreeView>

#include "ui_bookmarkwidget.h"

QT_BEGIN_NAMESPACE

class BookmarkManagerWidget;
class BookmarkModel;
class BookmarkFilterModel;
class QKeyEvent;
class QSortFilterProxyModel;
class QToolBar;

class BookmarkManager : public QObject
{
    Q_OBJECT
    class BookmarkWidget;
    class BookmarkTreeView;
    class BookmarkListView;
    Q_DISABLE_COPY(BookmarkManager)

public:
    static BookmarkManager* instance();
    static void destroy();

    QWidget* bookmarkDockWidget() const;
    void setBookmarksMenu(QMenu* menu);
    void setBookmarksToolbar(QToolBar *toolBar);

public slots:
    void addBookmark(const QString &title, const QString &url);

signals:
    void escapePressed();
    void setSource(const QUrl &url);
    void setSourceInNewTab(const QUrl &url);

private:
    BookmarkManager();
    ~BookmarkManager() override;

    void removeItem(const QModelIndex &index);
    bool eventFilter(QObject *object, QEvent *event) override;
    void buildBookmarksMenu(const QModelIndex &index, QMenu *menu);
    void showBookmarkDialog(const QString &name, const QString &url);

private slots:
    void setupFinished();
    void storeBookmarks();

    void addBookmarkActivated();
    void removeBookmarkActivated();
    void manageBookmarks();
    void refreshBookmarkMenu();
    void refreshBookmarkToolBar();
    void renameBookmark(const QModelIndex &index);

    void setSourceFromAction();
    void setSourceFromIndex(const QModelIndex &index, bool newTab);

    void focusInEventOccurred();
    void managerWidgetAboutToClose();
    void textChanged(const QString &text);
    void customContextMenuRequested(const QPoint &point);

private:
    bool typeAndSearch = false;

    static QMutex mutex;
    static BookmarkManager *bookmarkManager;

    QMenu *bookmarkMenu = nullptr;
    QToolBar *m_toolBar = nullptr;

    BookmarkModel *bookmarkModel;
    BookmarkFilterModel *bookmarkFilterModel = nullptr;
    QSortFilterProxyModel *typeAndSearchModel = nullptr;

    BookmarkWidget *bookmarkWidget;
    BookmarkTreeView *bookmarkTreeView;
    BookmarkManagerWidget *bookmarkManagerWidget = nullptr;
};

class BookmarkManager::BookmarkWidget : public QWidget
{
    Q_OBJECT
public:
    BookmarkWidget(QWidget *parent = nullptr)
        : QWidget(parent) { ui.setupUi(this); }
    virtual ~BookmarkWidget() override {}

    Ui::BookmarkWidget ui;

signals:
    void focusInEventOccurred();

private:
    void focusInEvent(QFocusEvent *event) override;
};

class BookmarkManager::BookmarkTreeView : public QTreeView
{
    Q_OBJECT
public:
    BookmarkTreeView(QWidget *parent = nullptr);
    ~BookmarkTreeView() override {}

    void subclassKeyPressEvent(QKeyEvent *event);

signals:
    void editingDone();

protected slots:
    void commitData(QWidget *editor) override;

private slots:
    void setExpandedData(const QModelIndex &index);
};

QT_END_NAMESPACE

#endif  // BOOKMARKMANAGER_H
