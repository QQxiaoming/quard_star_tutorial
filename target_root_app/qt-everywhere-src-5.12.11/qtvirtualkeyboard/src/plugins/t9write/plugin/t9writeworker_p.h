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

#ifndef T9WRITEWORKER_H
#define T9WRITEWORKER_H

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

#include <QtVirtualKeyboard/qvirtualkeyboardtrace.h>

#include <QThread>
#include <QSemaphore>
#include <QMutex>
#include <QStringList>
#include <QSharedPointer>
#include <QPointer>
#include <QMap>
#include <QVector>

#include "t9write_p.h"
#include "t9writedictionary_p.h"

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

class T9WriteTask : public QObject
{
    Q_OBJECT
public:
    explicit T9WriteTask(QObject *parent = nullptr);

    virtual void run() = 0;

    void wait();

    friend class T9WriteWorker;

protected:
    DECUMA_SESSION *decumaSession;
    bool cjk;

private:
    QSemaphore runSema;
};

class T9WriteDictionaryTask : public T9WriteTask
{
    Q_OBJECT
public:
    explicit T9WriteDictionaryTask(QSharedPointer<T9WriteDictionary> dictionary,
                                   const QString &dictionaryFileName,
                                   bool convertDictionary,
                                   const DECUMA_SRC_DICTIONARY_INFO &dictionaryInfo);

    void run();

    QSharedPointer<T9WriteDictionary> dictionary;
    const QString dictionaryFileName;
    bool convertDictionary;
    const DECUMA_SRC_DICTIONARY_INFO dictionaryInfo;

signals:
    void completed(QSharedPointer<T9WriteDictionary> dictionary);
};

class T9WriteAddArcTask : public T9WriteTask
{
    Q_OBJECT
public:
    explicit T9WriteAddArcTask(QVirtualKeyboardTrace *trace);

    void run();

private:
    QVirtualKeyboardTrace *trace;
};

class T9WriteRecognitionResult
{
    Q_DISABLE_COPY(T9WriteRecognitionResult)

public:
    explicit T9WriteRecognitionResult(int id, int maxResults, int maxCharsPerWord);

    DECUMA_STATUS status;
    QVector<DECUMA_HWR_RESULT> results;
    DECUMA_UINT16 numResults;
    int instantGesture;
    const int id;
    const int maxResults;
    const int maxCharsPerWord;

private:
    QVector<DECUMA_UNICODE> _chars;
    QVector<DECUMA_INT16> _symbolChars;
    QVector<DECUMA_INT16> _symbolStrokes;
};

class T9WriteRecognitionTask : public T9WriteTask
{
    Q_OBJECT
public:
    explicit T9WriteRecognitionTask(QSharedPointer<T9WriteRecognitionResult> result,
                                    const DECUMA_INSTANT_GESTURE_SETTINGS &instantGestureSettings,
                                    BOOST_LEVEL boostLevel,
                                    const QString &stringStart);

    void run();
    bool cancelRecognition();
    int resultId() const;

private:
    static int shouldAbortRecognize(void *pUserData);
    friend int shouldAbortRecognize(void *pUserData);

private:
    QSharedPointer<T9WriteRecognitionResult> result;
    DECUMA_INSTANT_GESTURE_SETTINGS instantGestureSettings;
    BOOST_LEVEL boostLevel;
    QString stringStart;
    QMutex stateLock;
    bool stateCancelled;
};

class T9WriteRecognitionResultsTask : public T9WriteTask
{
    Q_OBJECT
public:
    explicit T9WriteRecognitionResultsTask(QSharedPointer<T9WriteRecognitionResult> result);

    void run();

signals:
    void resultsAvailable(const QVariantList &resultList);
    void recognitionError(int status);

private:
    QSharedPointer<T9WriteRecognitionResult> result;
};

class T9WriteWorker : public QThread
{
    Q_OBJECT
public:
    explicit T9WriteWorker(DECUMA_SESSION *decumaSession, const bool cjk, QObject *parent = nullptr);
    ~T9WriteWorker();

    void addTask(QSharedPointer<T9WriteTask> task);
    int removeTask(QSharedPointer<T9WriteTask> task);
    int removeAllTasks();
    void waitForAllTasks();
    int numberOfPendingTasks();

    template <class X>
    int removeAllTasks() {
        QMutexLocker guard(&taskLock);
        int count = 0;
        for (int i = 0; i < taskList.size();) {
            QSharedPointer<X> task(taskList[i].objectCast<X>());
            if (task) {
                taskList.removeAt(i);
                ++count;
            } else {
                ++i;
            }
        }
        return count;
    }

protected:
    void run();

private:
    QList<QSharedPointer<T9WriteTask> > taskList;
    QSemaphore idleSema;
    QSemaphore taskSema;
    QMutex taskLock;
    DECUMA_SESSION *decumaSession;
    QBasicAtomicInt abort;
    const bool cjk;
};

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE

#endif // T9WRITEWORKER_H
