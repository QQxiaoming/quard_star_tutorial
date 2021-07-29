/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef HUNSPELLWORKER_P_H
#define HUNSPELLWORKER_P_H

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

#include <QThread>
#include <QSemaphore>
#include <QMutex>
#include <QStringList>
#include <QSharedPointer>
#include <QVector>
#include <QLoggingCategory>
#include <hunspell/hunspell.h>
#include <QtHunspellInputMethod/qhunspellinputmethod_global.h>

QT_BEGIN_NAMESPACE
class QTextCodec;

namespace QtVirtualKeyboard {

Q_DECLARE_LOGGING_CATEGORY(lcHunspell)

class QHUNSPELLINPUTMETHOD_EXPORT HunspellWordList
{
public:
    enum Flag
    {
        SpellCheckOk = 0x1,
        CompoundWord = 0x2
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    HunspellWordList(int limit = 0);
    HunspellWordList(HunspellWordList &other);

    HunspellWordList &operator=(HunspellWordList &other);

    int index() const;
    void setIndex(int index);
    bool clear();
    bool clearSuggestions();
    bool hasSuggestions() const;
    int size() const;
    int isEmpty() const;
    bool contains(const QString &word);
    QString findWordCompletion(const QString &word);
    int indexOfWord(const QString &word);
    QString wordAt(int index);
    void wordAt(int index, QString &word, Flags &flags);
    const Flags &wordFlagsAt(int index);
    void appendWord(const QString &word, const Flags &flags = Flags());
    void insertWord(int index, const QString &word, const Flags &flags = Flags());
    void updateWord(int index, const QString &word, const Flags &flags = Flags());
    void moveWord(int from, int to);
    int removeWord(const QString &word);
    void removeWordAt(int index);
    void rebuildSearchIndex();

private:
    class SearchContext {
    public:
        SearchContext(const QString &word,
                      const QStringList &list) :
            word(word),
            list(list)
        {}
        const QString &word;
        const QStringList &list;
    };

private:
    QMutex _lock;
    QStringList _list;
    QVector<Flags> _flags;
    QVector<int> _searchIndex;
    int _index;
    int _limit;
};

class HunspellTask : public QObject
{
    Q_OBJECT
public:
    explicit HunspellTask(QObject *parent = nullptr) :
        QObject(parent),
        hunspell(nullptr)
    {}

    virtual void run() = 0;

    Hunhandle *hunspell;
};

class HunspellLoadDictionaryTask : public HunspellTask
{
    Q_OBJECT
public:
    explicit HunspellLoadDictionaryTask(const QString &locale, const QStringList &searchPaths);

    void run();

signals:
    void completed(bool success);

public:
    Hunhandle **hunspellPtr;
    const QString locale;
    const QStringList searchPaths;
};

class HunspellBuildSuggestionsTask : public HunspellTask
{
    Q_OBJECT
    const QTextCodec *textCodec;
public:
    QSharedPointer<HunspellWordList> wordList;
    bool autoCorrect;

    void run();
    bool spellCheck(const QString &word);
    int levenshteinDistance(const QString &s, const QString &t);
    QString removeAccentsAndDiacritics(const QString& s);
};

class HunspellUpdateSuggestionsTask : public HunspellTask
{
    Q_OBJECT
public:
    QSharedPointer<HunspellWordList> wordList;

    void run();

signals:
    void updateSuggestions(const QSharedPointer<HunspellWordList> &wordList, int tag);

public:
    int tag;
};

class HunspellAddWordTask : public HunspellTask
{
    Q_OBJECT
public:
    QSharedPointer<HunspellWordList> wordList;

    void run();

    static bool alternativeForm(const QString &word, QString &alternativeForm);
};

class HunspellRemoveWordTask : public HunspellTask
{
    Q_OBJECT
public:
    QSharedPointer<HunspellWordList> wordList;

    void run();
};

class HunspellLoadWordListTask : public HunspellTask
{
    Q_OBJECT
public:
    QSharedPointer<HunspellWordList> wordList;
    QString filePath;

    void run();
};

class HunspellSaveWordListTask : public HunspellTask
{
    Q_OBJECT
public:
    QSharedPointer<HunspellWordList> wordList;
    QString filePath;

    void run();
};

class HunspellFilterWordTask : public HunspellTask
{
    Q_OBJECT
public:
    HunspellFilterWordTask() :
        HunspellTask(),
        startIndex(1)
    {}

    QSharedPointer<HunspellWordList> wordList;
    QSharedPointer<HunspellWordList> filterList;
    int startIndex;

    void run();
};

class HunspellBoostWordTask : public HunspellTask
{
    Q_OBJECT
public:
    HunspellBoostWordTask() :
        HunspellTask()
    {}

    QSharedPointer<HunspellWordList> wordList;
    QSharedPointer<HunspellWordList> boostList;

    void run();
};

class HunspellWorker : public QThread
{
    Q_OBJECT
public:
    explicit HunspellWorker(QObject *parent = nullptr);
    ~HunspellWorker();

    void addTask(QSharedPointer<HunspellTask> task);
    void removeAllTasks();
    void waitForAllTasks();

    template <class X>
    void removeAllTasksOfType() {
        QMutexLocker guard(&taskLock);
        for (int i = 0; i < taskList.size();) {
            QSharedPointer<X> task(taskList[i].objectCast<X>());
            if (task) {
                qCDebug(lcHunspell) << "Remove task" << QLatin1String(task->metaObject()->className());
                taskList.removeAt(i);
            } else {
                i++;
            }
        }
    }

protected:
    void run();

private:
    void createHunspell();

private:
    friend class HunspellLoadDictionaryTask;
    QList<QSharedPointer<HunspellTask> > taskList;
    QSemaphore idleSema;
    QSemaphore taskSema;
    QMutex taskLock;
    Hunhandle *hunspell;
    QBasicAtomicInt abort;
};

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE

Q_DECLARE_METATYPE(QSharedPointer<QT_PREPEND_NAMESPACE(QtVirtualKeyboard)::HunspellWordList>);

#endif // HUNSPELLWORKER_P_H
