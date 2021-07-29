/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQUICKVISUALTESTUTIL_H
#define QQUICKVISUALTESTUTIL_H

#include <QtQuick/QQuickItem>
#include <QtQml/QQmlExpression>

#include <QtQuick/private/qquickitem_p.h>

#include <QtQuickTemplates2/private/qquickapplicationwindow_p.h>

#include "util.h"

namespace QQuickVisualTestUtil
{
    QQuickItem *findVisibleChild(QQuickItem *parent, const QString &objectName);

    void dumpTree(QQuickItem *parent, int depth = 0);

    bool delegateVisible(QQuickItem *item);

    void centerOnScreen(QQuickWindow *window);

    void moveMouseAway(QQuickWindow *window);

    /*
       Find an item with the specified objectName.  If index is supplied then the
       item must also evaluate the {index} expression equal to index
    */
    template<typename T>
    T *findItem(QQuickItem *parent, const QString &objectName, int index = -1)
    {
        const QMetaObject &mo = T::staticMetaObject;
        for (int i = 0; i < parent->childItems().count(); ++i) {
            QQuickItem *item = qobject_cast<QQuickItem*>(parent->childItems().at(i));
            if (!item)
                continue;
            if (mo.cast(item) && (objectName.isEmpty() || item->objectName() == objectName)) {
                if (index != -1) {
                    QQmlExpression e(qmlContext(item), item, "index");
                    if (e.evaluate().toInt() == index)
                        return static_cast<T*>(item);
                } else {
                    return static_cast<T*>(item);
                }
            }
            item = findItem<T>(item, objectName, index);
            if (item)
                return static_cast<T*>(item);
        }

        return 0;
    }

    template<typename T>
    QList<T*> findItems(QQuickItem *parent, const QString &objectName, bool visibleOnly = true)
    {
        QList<T*> items;
        const QMetaObject &mo = T::staticMetaObject;
        for (int i = 0; i < parent->childItems().count(); ++i) {
            QQuickItem *item = qobject_cast<QQuickItem*>(parent->childItems().at(i));
            if (!item || (visibleOnly && (!item->isVisible() || QQuickItemPrivate::get(item)->culled)))
                continue;
            if (mo.cast(item) && (objectName.isEmpty() || item->objectName() == objectName))
                items.append(static_cast<T*>(item));
            items += findItems<T>(item, objectName);
        }

        return items;
    }

    template<typename T>
    QList<T*> findItems(QQuickItem *parent, const QString &objectName, const QList<int> &indexes)
    {
        QList<T*> items;
        for (int i=0; i<indexes.count(); i++)
            items << qobject_cast<QQuickItem*>(findItem<T>(parent, objectName, indexes[i]));
        return items;
    }

    class QQuickApplicationHelper
    {
    public:
        QQuickApplicationHelper(QQmlDataTest *testCase, const QString &testFilePath) :
            component(&engine)
        {
            component.loadUrl(testCase->testFileUrl(testFilePath));
            QObject *rootObject = component.create();
            cleanup.reset(rootObject);
            if (!rootObject) {
                errorMessage = QString::fromUtf8("Failed to create window: %1").arg(component.errorString()).toUtf8();
                return;
            }

            window = qobject_cast<QQuickWindow*>(rootObject);
            appWindow = qobject_cast<QQuickApplicationWindow*>(rootObject);
            if (!window) {
                errorMessage = QString::fromUtf8("Root object must be a QQuickWindow subclass").toUtf8();
                return;
            }

            if (window->isVisible()) {
                errorMessage = QString::fromUtf8("Expected window not to be visible, but it is").toUtf8();
                return;
            }

            ready = true;
        }

        // Return a C-style string instead of QString because that's what QTest uses for error messages,
        // so it saves code at the calling site.
        inline const char *failureMessage() const
        {
            return errorMessage.constData();
        }

        QQmlEngine engine;
        QQmlComponent component;
        QScopedPointer<QObject> cleanup;
        QQuickApplicationWindow *appWindow = nullptr;
        QQuickWindow *window = nullptr;

        bool ready = false;
        // Store as a byte array so that we can return its raw data safely;
        // using qPrintable() in failureMessage() will construct a throwaway QByteArray
        // that is destroyed before the function returns.
        QByteArray errorMessage;
    };

    void addTestRowForEachControl(QQmlEngine *engine, const QString &sourcePath, const QString &targetPath, const QStringList &skiplist = QStringList());
}

#define QQUICK_VERIFY_POLISH(item) \
    QTRY_COMPARE(QQuickItemPrivate::get(item)->polishScheduled, false)

#endif // QQUICKVISUALTESTUTIL_H
