/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#include "qhelpindexwidget.h"
#include "qhelpenginecore.h"
#include "qhelpengine_p.h"
#include "qhelpdbreader_p.h"
#include "qhelpcollectionhandler_p.h"

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtWidgets/QListView>
#include <QtWidgets/QHeaderView>

#include <algorithm>

QT_BEGIN_NAMESPACE

class QHelpIndexProvider : public QThread
{
public:
    QHelpIndexProvider(QHelpEnginePrivate *helpEngine);
    ~QHelpIndexProvider() override;
    void collectIndices(const QString &customFilterName);
    void stopCollecting();
    QStringList indices() const;

private:
    void run() override;

    QHelpEnginePrivate *m_helpEngine;
    QStringList m_indices;
    QStringList m_filterAttributes;
    mutable QMutex m_mutex;
    bool m_abort = false;
};

class QHelpIndexModelPrivate
{
public:
    QHelpIndexModelPrivate(QHelpEnginePrivate *hE)
        : helpEngine(hE),
          indexProvider(new QHelpIndexProvider(helpEngine))
    {
    }

    QHelpEnginePrivate *helpEngine;
    QHelpIndexProvider *indexProvider;
    QStringList indices;
};

QHelpIndexProvider::QHelpIndexProvider(QHelpEnginePrivate *helpEngine)
    : QThread(helpEngine),
      m_helpEngine(helpEngine)
{
}

QHelpIndexProvider::~QHelpIndexProvider()
{
    stopCollecting();
}

void QHelpIndexProvider::collectIndices(const QString &customFilterName)
{
    m_mutex.lock();
    m_filterAttributes = m_helpEngine->q->filterAttributes(customFilterName);
    m_mutex.unlock();
    if (!isRunning()) {
        start(LowPriority);
    } else {
        stopCollecting();
        start(LowPriority);
    }
}

void QHelpIndexProvider::stopCollecting()
{
    if (!isRunning())
        return;
    m_mutex.lock();
    m_abort = true;
    m_mutex.unlock();
    wait();
    m_abort = false;
}

QStringList QHelpIndexProvider::indices() const
{
    QMutexLocker lck(&m_mutex);
    return m_indices;
}

void QHelpIndexProvider::run()
{
    m_mutex.lock();
    m_indices.clear();
    const QStringList attributes = m_filterAttributes;
    const QString collectionFile = m_helpEngine->collectionHandler->collectionFile();
    m_mutex.unlock();

    if (collectionFile.isEmpty())
        return;

    QHelpCollectionHandler collectionHandler(collectionFile);
    if (!collectionHandler.openCollectionFile())
        return;

    const QStringList result = collectionHandler.indicesForFilter(attributes);

    m_mutex.lock();
    m_indices = result;
    m_mutex.unlock();
}

/*!
    \class QHelpIndexModel
    \since 4.4
    \inmodule QtHelp
    \brief The QHelpIndexModel class provides a model that
    supplies index keywords to views.


*/

/*!
    \fn void QHelpIndexModel::indexCreationStarted()

    This signal is emitted when the creation of a new index
    has started. The current index is invalid from this
    point on until the signal indexCreated() is emitted.

    \sa isCreatingIndex()
*/

/*!
    \fn void QHelpIndexModel::indexCreated()

    This signal is emitted when the index has been created.
*/

QHelpIndexModel::QHelpIndexModel(QHelpEnginePrivate *helpEngine)
    : QStringListModel(helpEngine)
{
    d = new QHelpIndexModelPrivate(helpEngine);

    connect(d->indexProvider, &QThread::finished,
            this, &QHelpIndexModel::insertIndices);
}

QHelpIndexModel::~QHelpIndexModel()
{
    delete d;
}

void QHelpIndexModel::invalidateIndex(bool onShutDown)
{
    Q_UNUSED(onShutDown)
}

/*!
    Creates a new index by querying the help system for
    keywords for the specified \a customFilterName.
*/
void QHelpIndexModel::createIndex(const QString &customFilterName)
{
    d->indexProvider->collectIndices(customFilterName);
    emit indexCreationStarted();
}

void QHelpIndexModel::insertIndices()
{
    d->indices = d->indexProvider->indices();
    filter(QString());
    emit indexCreated();
}

/*!
    Returns true if the index is currently built up, otherwise
    false.
*/
bool QHelpIndexModel::isCreatingIndex() const
{
    return d->indexProvider->isRunning();
}

/*!
    \obsolete
    Use QHelpEngineCore::linksForKeyword() instead.
*/
QMap<QString, QUrl> QHelpIndexModel::linksForKeyword(const QString &keyword) const
{
    return d->helpEngine->q->linksForKeyword(keyword);
}

/*!
    Filters the indices and returns the model index of the best
    matching keyword. In a first step, only the keywords containing
    \a filter are kept in the model's index list. Analogously, if
    \a wildcard is not empty, only the keywords matched are left
    in the index list. In a second step, the best match is
    determined and its index model returned. When specifying a
    wildcard expression, the \a filter string is used to
    search for the best match.
*/
QModelIndex QHelpIndexModel::filter(const QString &filter, const QString &wildcard)
{
    if (filter.isEmpty()) {
        setStringList(d->indices);
        return index(-1, 0, QModelIndex());
    }

    QStringList lst;
    int goodMatch = -1;
    int perfectMatch = -1;

    if (!wildcard.isEmpty()) {
        const QRegExp regExp(wildcard, Qt::CaseInsensitive, QRegExp::Wildcard);
        for (const QString &index : qAsConst(d->indices)) {
            if (index.contains(regExp)) {
                lst.append(index);
                if (perfectMatch == -1 && index.startsWith(filter, Qt::CaseInsensitive)) {
                    if (goodMatch == -1)
                        goodMatch = lst.count() - 1;
                    if (filter.length() == index.length()){
                        perfectMatch = lst.count() - 1;
                    }
                } else if (perfectMatch > -1 && index == filter) {
                    perfectMatch = lst.count() - 1;
                }
            }
        }
    } else {
        for (const QString &index : qAsConst(d->indices)) {
            if (index.contains(filter, Qt::CaseInsensitive)) {
                lst.append(index);
                if (perfectMatch == -1 && index.startsWith(filter, Qt::CaseInsensitive)) {
                    if (goodMatch == -1)
                        goodMatch = lst.count() - 1;
                    if (filter.length() == index.length()){
                        perfectMatch = lst.count() - 1;
                    }
                } else if (perfectMatch > -1 && index == filter) {
                    perfectMatch = lst.count() - 1;
                }
            }
        }

    }

    if (perfectMatch == -1)
        perfectMatch = qMax(0, goodMatch);

    setStringList(lst);
    return index(perfectMatch, 0, QModelIndex());
}



/*!
    \class QHelpIndexWidget
    \inmodule QtHelp
    \since 4.4
    \brief The QHelpIndexWidget class provides a list view
    displaying the QHelpIndexModel.
*/

/*!
    \fn void QHelpIndexWidget::linkActivated(const QUrl &link,
        const QString &keyword)

    This signal is emitted when an item is activated and its
    associated \a link should be shown. To know where the link
    belongs to, the \a keyword is given as a second paremeter.
*/

/*!
    \fn void QHelpIndexWidget::linksActivated(const QMap<QString, QUrl> &links,
        const QString &keyword)

    This signal is emitted when the item representing the \a keyword
    is activated and the item has more than one link associated.
    The \a links consist of the document titles and their URLs.
*/

QHelpIndexWidget::QHelpIndexWidget()
    : QListView(nullptr)
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setUniformItemSizes(true);
    connect(this, &QAbstractItemView::activated,
            this, &QHelpIndexWidget::showLink);
}

void QHelpIndexWidget::showLink(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QHelpIndexModel *indexModel = qobject_cast<QHelpIndexModel*>(model());
    if (!indexModel)
        return;

    const QVariant &v = indexModel->data(index, Qt::DisplayRole);
    const QString name = v.isValid() ? v.toString() : QString();

    const QMap<QString, QUrl> &links = indexModel->linksForKeyword(name);
    if (links.count() > 1)
        emit linksActivated(links, name);
    else if (!links.isEmpty())
        emit linkActivated(links.first(), name);
}

/*!
    Activates the current item which will result eventually in
    the emitting of a linkActivated() or linksActivated()
    signal.
*/
void QHelpIndexWidget::activateCurrentItem()
{
    showLink(currentIndex());
}

/*!
    Filters the indices according to \a filter or \a wildcard.
    The item with the best match is set as current item.

    \sa QHelpIndexModel::filter()
*/
void QHelpIndexWidget::filterIndices(const QString &filter, const QString &wildcard)
{
    QHelpIndexModel *indexModel = qobject_cast<QHelpIndexModel*>(model());
    if (!indexModel)
        return;
    const QModelIndex &idx = indexModel->filter(filter, wildcard);
    if (idx.isValid())
        setCurrentIndex(idx);
}

QT_END_NAMESPACE
