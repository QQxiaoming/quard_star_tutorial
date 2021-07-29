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

#include <QtHunspellInputMethod/private/hunspellworker_p.h>
#include <QVector>
#include <QTextCodec>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTime>
#include <QFile>
#include <QDir>
#include <QtAlgorithms>

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

HunspellWordList::HunspellWordList(int limit) :
    _index(0),
    _limit(limit)
{
}

HunspellWordList::HunspellWordList(HunspellWordList &other)
{
    *this = other;
}

HunspellWordList &HunspellWordList::operator=(HunspellWordList &other)
{
    if (this != &other) {
        QMutexLocker guard(&_lock);
        QMutexLocker otherGuard(&other._lock);
        _list = other._list;
        _flags = other._flags;
        _index = other._index;
        _limit = other._limit;
        _searchIndex = other._searchIndex;
    }
    return *this;
}

int HunspellWordList::index() const
{
    return _index < _list.size() ? _index : -1;
}

void HunspellWordList::setIndex(int index)
{
    QMutexLocker guard(&_lock);
    _index = index;
}

bool HunspellWordList::clear()
{
    QMutexLocker guard(&_lock);
    bool result = !_list.isEmpty();
    _list.clear();
    _flags.clear();
    _index = 0;
    _searchIndex.clear();
    return result;
}

bool HunspellWordList::clearSuggestions()
{
    QMutexLocker guard(&_lock);
    if (_list.isEmpty())
        return false;

    _searchIndex.clear();
    if (_list.size() > 1) {
        QString word = _list.at(0);
        Flags flags = _flags.at(0);
        _list.clear();
        _flags.clear();
        if (!word.isEmpty()) {
            _index = 0;
            _list.append(word);
            _flags.append(flags);
        }
        return true;
    } else if (_list.at(0).isEmpty()) {
        _list.clear();
        _flags.clear();
        _index = 0;
        return true;
    }
    return false;
}

bool HunspellWordList::hasSuggestions() const
{
    return _list.size() > 1;
}

int HunspellWordList::size() const
{
    return _list.size();
}

int HunspellWordList::isEmpty() const
{
    return _list.isEmpty() || _list.at(0).isEmpty();
}

bool HunspellWordList::contains(const QString &word)
{
    QMutexLocker guard(&_lock);

    // Use index search when the search index is available.
    // This provides a lot faster search than QList::contains().
    // Search index is available when it has been rebuilt using
    // rebuildSearchIndex() method. Search index is automatically
    // cleared when the word list is modified.
    if (!_searchIndex.isEmpty()) {
        Q_ASSERT(_searchIndex.size() == _list.size());

        SearchContext searchContext(word, _list);
        return std::binary_search(_searchIndex.begin(), _searchIndex.end(), -1, [searchContext](const int &a, const int &b) {
            const QString &wordA = (a == -1) ? searchContext.word : searchContext.list[a];
            const QString &wordB = (b == -1) ? searchContext.word : searchContext.list[b];
            return wordA.compare(wordB, Qt::CaseInsensitive) < 0;
        });
    }

    return _list.contains(word, Qt::CaseInsensitive);
}

QString HunspellWordList::findWordCompletion(const QString &word)
{
    QMutexLocker guard(&_lock);

    if (!_searchIndex.isEmpty()) {
        Q_ASSERT(_searchIndex.size() == _list.size());

        SearchContext searchContext(word, _list);
        auto match = std::lower_bound(_searchIndex.begin(), _searchIndex.end(), -1, [searchContext](const int &a, const int &b) {
            const QString &wordA = (a == -1) ? searchContext.word : searchContext.list[a];
            const QString &wordB = (b == -1) ? searchContext.word : searchContext.list[b];
            return wordA.compare(wordB, Qt::CaseInsensitive) < 0;
        });

        if (match == _searchIndex.end())
            return QString();

        if (!word.compare(_list[*match], Qt::CaseInsensitive)) {
            match++;
            if (match == _searchIndex.end())
                return QString();
        }

        return _list[*match].startsWith(word, Qt::CaseInsensitive) ? _list[*match] : QString();
    }

    QString bestMatch;
    for (int i = 0, count = _list.size(); i < count; ++i) {
        const QString &wordB(_list[i]);
        if (wordB.length() > bestMatch.length() &&
                word.length() < wordB.length() &&
                wordB.startsWith(word, Qt::CaseInsensitive))
            bestMatch = wordB;
    }

    return bestMatch;
}

int HunspellWordList::indexOfWord(const QString &word)
{
    QMutexLocker guard(&_lock);

    if (!_searchIndex.isEmpty()) {
        Q_ASSERT(_searchIndex.size() == _list.size());

        SearchContext searchContext(word, _list);
        auto match = std::lower_bound(_searchIndex.begin(), _searchIndex.end(), -1, [searchContext](int a, int b) {
            const QString &wordA = (a == -1) ? searchContext.word : searchContext.list[a];
            const QString &wordB = (b == -1) ? searchContext.word : searchContext.list[b];
            return wordA.compare(wordB, Qt::CaseInsensitive) < 0;
        });
        return (match != _searchIndex.end()) ? *match : -1;
    }

    return _list.indexOf(word);
}

QString HunspellWordList::wordAt(int index)
{
    QMutexLocker guard(&_lock);

    return index >= 0 && index < _list.size() ? _list.at(index) : QString();
}

void HunspellWordList::wordAt(int index, QString &word, Flags &flags)
{
    QMutexLocker guard(&_lock);
    Q_ASSERT(index >= 0 && index < _list.size());

    word = _list.at(index);
    flags = _flags.at(index);
}

const HunspellWordList::Flags &HunspellWordList::wordFlagsAt(int index)
{
    QMutexLocker guard(&_lock);

    return _flags[index];
}

void HunspellWordList::appendWord(const QString &word, const Flags &flags)
{
    QMutexLocker guard(&_lock);

    _searchIndex.clear();
    if (_limit > 0) {
        while (_list.size() >= _limit) {
            _list.removeAt(0);
            _flags.removeAt(0);
        }
    }
    _list.append(word);
    _flags.append(flags);
}

void HunspellWordList::insertWord(int index, const QString &word, const Flags &flags)
{
    QMutexLocker guard(&_lock);
    Q_ASSERT(_limit == 0);

    _searchIndex.clear();
    _list.insert(index, word);
    _flags.insert(index, flags);
}

void HunspellWordList::updateWord(int index, const QString &word, const Flags &flags)
{
    Q_ASSERT(index >= 0);
    QMutexLocker guard(&_lock);

    if (index < _list.size()) {
        if (word != _list[index])
            _searchIndex.clear();
        _list[index] = word;
        _flags[index] = flags;
    } else {
        _searchIndex.clear();
        _list.append(word);
        _flags.append(flags);
    }
}

void HunspellWordList::moveWord(int from, int to)
{
    QMutexLocker guard(&_lock);

    if (from < 0 || from >= _list.size())
        return;
    if (to < 0 || to >= _list.size())
        return;
    if (from == to)
        return;

    _searchIndex.clear();
    _list.move(from, to);
    _flags.move(from, to);
}

int HunspellWordList::removeWord(const QString &word)
{
    QMutexLocker guard(&_lock);
    int removeCount = 0;
    for (int i = 0, count = _list.size(); i < count;) {
        if (!_list[i].compare(word, Qt::CaseInsensitive)) {
            _list.removeAt(i);
            _flags.removeAt(i);
            --count;
            ++removeCount;
        } else {
            ++i;
        }
    }
    if (removeCount > 0)
        _searchIndex.clear();
    return removeCount;
}

void HunspellWordList::removeWordAt(int index)
{
    QMutexLocker guard(&_lock);

    _list.removeAt(index);
}

void HunspellWordList::rebuildSearchIndex()
{
    QMutexLocker guard(&_lock);
    _searchIndex.clear();

    if (_list.isEmpty())
        return;

    _searchIndex.resize(_list.size());
    std::iota(_searchIndex.begin(), _searchIndex.end(), 0);

    const QStringList list(_list);
    std::sort(_searchIndex.begin(), _searchIndex.end(), [list](int a, int b) { return list[a].compare(list[b], Qt::CaseInsensitive) < 0; });
}

/*!
    \class QtVirtualKeyboard::HunspellTask
    \internal
*/

/*!
    \class QtVirtualKeyboard::HunspellWordList
    \internal
*/

/*!
    \class QtVirtualKeyboard::HunspellLoadDictionaryTask
    \internal
*/

HunspellLoadDictionaryTask::HunspellLoadDictionaryTask(const QString &locale, const QStringList &searchPaths) :
    HunspellTask(),
    hunspellPtr(nullptr),
    locale(locale),
    searchPaths(searchPaths)
{
}

void HunspellLoadDictionaryTask::run()
{
    Q_ASSERT(hunspellPtr != nullptr);

    qCDebug(lcHunspell) << "HunspellLoadDictionaryTask::run(): locale:" << locale;

    if (*hunspellPtr) {
        Hunspell_destroy(*hunspellPtr);
        *hunspellPtr = nullptr;
    }

    QString affPath;
    QString dicPath;
    for (const QString &searchPath : searchPaths) {
        affPath = QStringLiteral("%1/%2.aff").arg(searchPath, locale);
        if (QFileInfo::exists(affPath)) {
            dicPath = QStringLiteral("%1/%2.dic").arg(searchPath, locale);
            if (QFileInfo::exists(dicPath))
                break;
            dicPath.clear();
        }
        affPath.clear();
    }

    if (!affPath.isEmpty() && !dicPath.isEmpty()) {
        *hunspellPtr = Hunspell_create(affPath.toUtf8().constData(), dicPath.toUtf8().constData());
        if (*hunspellPtr) {
            /*  Make sure the encoding used by the dictionary is supported
                by the QTextCodec.
            */
            if (!QTextCodec::codecForName(Hunspell_get_dic_encoding(*hunspellPtr))) {
                qCWarning(lcHunspell) << "The Hunspell dictionary" << dicPath << "cannot be used because it uses an unknown text codec" << QLatin1String(Hunspell_get_dic_encoding(*hunspellPtr));
                Hunspell_destroy(*hunspellPtr);
                *hunspellPtr = nullptr;
            }
        }
    } else {
        qCWarning(lcHunspell) << "Hunspell dictionary is missing for" << locale << ". Search paths" << searchPaths;
    }

    emit completed(*hunspellPtr != nullptr);
}

/*!
    \class QtVirtualKeyboard::HunspellBuildSuggestionsTask
    \internal
*/

void HunspellBuildSuggestionsTask::run()
{
    if (wordList->isEmpty())
        return;

    wordList->clearSuggestions();
    QString word = wordList->wordAt(0);

    /*  Select text codec based on the dictionary encoding.
        Hunspell_get_dic_encoding() should always return at least
        "ISO8859-1", but you can never be too sure.
     */
    textCodec = QTextCodec::codecForName(Hunspell_get_dic_encoding(hunspell));
    if (!textCodec)
        return;

    char **slst = nullptr;
    int n = Hunspell_suggest(hunspell, &slst, textCodec->fromUnicode(word).constData());
    if (n > 0) {
        /*  Collect word candidates from the Hunspell suggestions.
            Insert word completions in the beginning of the list.
        */
        const int firstWordCompletionIndex = wordList->size();
        int lastWordCompletionIndex = firstWordCompletionIndex;
        bool suggestCapitalization = false;
        for (int i = 0; i < n; i++) {
            QString wordCandidate(textCodec->toUnicode(slst[i]));
            wordCandidate.replace(QChar(0x2019), QLatin1Char('\''));
            QString normalizedWordCandidate = removeAccentsAndDiacritics(wordCandidate);
            /*  Prioritize word Capitalization */
            if (!wordCandidate.compare(word, Qt::CaseInsensitive)) {
                if (suggestCapitalization) {
                    bool wordCandidateIsCapital = wordCandidate.at(0).isUpper();
                    bool wordIsCapital = word.at(0).isUpper();
                    if (wordCandidateIsCapital == wordIsCapital) {
                        if (wordCandidateIsCapital)
                            wordCandidate = wordCandidate.toLower();
                        else
                            wordCandidate[0] = wordCandidate.at(0).toUpper();
                    }
                    wordList->insertWord(1, wordCandidate);
                    lastWordCompletionIndex++;
                    suggestCapitalization = true;
                }
            /*  Prioritize word completions, missing punctuation or missing accents */
            } else if ((normalizedWordCandidate.length() > word.length() &&
                        normalizedWordCandidate.startsWith(word)) ||
                       wordCandidate.contains(QLatin1Char('\''))) {
                wordList->insertWord(lastWordCompletionIndex++, wordCandidate);
            } else {
                wordList->appendWord(wordCandidate);
            }
        }
        /*  Prioritize words with missing spaces next to word completions.
        */
        for (int i = lastWordCompletionIndex; i < wordList->size(); i++) {
            QString wordCandidate(wordList->wordAt(i));
            if (wordCandidate.contains(QLatin1String(" "))) {
                wordList->updateWord(i, wordCandidate, wordList->wordFlagsAt(i) | HunspellWordList::CompoundWord);
                if (i != lastWordCompletionIndex) {
                    wordList->moveWord(i, lastWordCompletionIndex);
                }
                lastWordCompletionIndex++;
            }
        }
        /*  Do spell checking and suggest the first candidate, if:
            - the word matches partly the suggested word; or
            - the quality of the suggested word is good enough.

            The quality is measured here using the Levenshtein Distance,
            which may be suboptimal for the purpose, but gives some clue
            how much the suggested word differs from the given word.
        */
        if (autoCorrect && wordList->size() > 1 && (!spellCheck(word) || suggestCapitalization)) {
            if (lastWordCompletionIndex > firstWordCompletionIndex || levenshteinDistance(word, wordList->wordAt(firstWordCompletionIndex)) < 3)
                wordList->setIndex(firstWordCompletionIndex);
        }
    }
    Hunspell_free_list(hunspell, &slst, n);

    for (int i = 0, count = wordList->size(); i < count; ++i) {
        HunspellWordList::Flags flags;
        wordList->wordAt(i, word, flags);
        if (flags.testFlag(HunspellWordList::CompoundWord))
            continue;
        if (Hunspell_spell(hunspell, textCodec->fromUnicode(word).constData()) != 0)
            wordList->updateWord(i, word, wordList->wordFlagsAt(i) | HunspellWordList::SpellCheckOk);
    }
}

bool HunspellBuildSuggestionsTask::spellCheck(const QString &word)
{
    if (!hunspell)
        return false;
    if (word.contains(QRegularExpression(QLatin1Literal("[0-9]"))))
        return true;
    return Hunspell_spell(hunspell, textCodec->fromUnicode(word).constData()) != 0;
}

// source: http://en.wikipedia.org/wiki/Levenshtein_distance
int HunspellBuildSuggestionsTask::levenshteinDistance(const QString &s, const QString &t)
{
    if (s == t)
        return 0;
    if (s.length() == 0)
        return t.length();
    if (t.length() == 0)
        return s.length();
    QVector<int> v0(t.length() + 1);
    QVector<int> v1(t.length() + 1);
    for (int i = 0; i < v0.size(); i++)
        v0[i] = i;
    for (int i = 0; i < s.size(); i++) {
        v1[0] = i + 1;
        for (int j = 0; j < t.length(); j++) {
            int cost = (s[i].toLower() == t[j].toLower()) ? 0 : 1;
            v1[j + 1] = qMin(qMin(v1[j] + 1, v0[j + 1] + 1), v0[j] + cost);
        }
        for (int j = 0; j < v0.size(); j++)
            v0[j] = v1[j];
    }
    return v1[t.length()];
}

QString HunspellBuildSuggestionsTask::removeAccentsAndDiacritics(const QString& s)
{
    QString normalized = s.normalized(QString::NormalizationForm_D);
    for (int i = 0; i < normalized.length();) {
        QChar::Category category = normalized[i].category();
        if (category <= QChar::Mark_Enclosing) {
            normalized.remove(i, 1);
        } else {
            i++;
        }
    }
    return normalized;
}

/*!
    \class QtVirtualKeyboard::HunspellUpdateSuggestionsTask
    \internal
*/

void HunspellUpdateSuggestionsTask::run()
{
    emit updateSuggestions(wordList, tag);
}

void HunspellAddWordTask::run()
{
    const QTextCodec *textCodec;
    textCodec = QTextCodec::codecForName(Hunspell_get_dic_encoding(hunspell));
    if (!textCodec)
        return;

    QString tmpWord;
    tmpWord.reserve(64);
    for (int i = 0, count = wordList->size(); i < count; ++i) {
        const QString word(wordList->wordAt(i));
        if (word.length() < 2)
            continue;
        Hunspell_add(hunspell, textCodec->fromUnicode(word).constData());
        if (HunspellAddWordTask::alternativeForm(word, tmpWord))
            Hunspell_add(hunspell, textCodec->fromUnicode(tmpWord).constData());
    }
}

bool HunspellAddWordTask::alternativeForm(const QString &word, QString &alternativeForm)
{
    if (word.length() < 2)
        return false;
    if (!word.mid(1).isLower())
        return false;

    const QChar initial(word.at(0));
    const QChar newInitial = initial.isUpper() ? initial.toLower() : initial.toUpper();
    if (newInitial == initial)
        return false;

    alternativeForm.truncate(0);
    alternativeForm.append(word);
    alternativeForm[0] = newInitial;

    return true;
}

void HunspellRemoveWordTask::run()
{
    const QTextCodec *textCodec;
    textCodec = QTextCodec::codecForName(Hunspell_get_dic_encoding(hunspell));
    if (!textCodec)
        return;

    QString tmpWord;
    tmpWord.reserve(64);
    for (int i = 0, count = wordList->size(); i < count; ++i) {
        const QString word(wordList->wordAt(i));
        if (word.isEmpty())
            continue;
        Hunspell_remove(hunspell, textCodec->fromUnicode(word).constData());
        if (HunspellAddWordTask::alternativeForm(word, tmpWord))
            Hunspell_remove(hunspell, textCodec->fromUnicode(tmpWord).constData());
    }
}

void HunspellLoadWordListTask::run()
{
    wordList->clear();

    QFile inputFile(filePath);
    if (inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream inStream(&inputFile);
        inStream.setCodec(QTextCodec::codecForName("UTF-8"));
        QString word;
        word.reserve(64);
        while (inStream.readLineInto(&word)) {
            if (!word.isEmpty())
                wordList->appendWord(word);
        }
        inputFile.close();
    }
}

void HunspellSaveWordListTask::run()
{
    QFile outputFile(filePath);
    if (!QFileInfo::exists(filePath))
        QDir().mkpath(QFileInfo(filePath).absoluteDir().path());
    if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream outStream(&outputFile);
        outStream.setCodec(QTextCodec::codecForName("UTF-8"));
        for (int i = 0, count = wordList->size(); i < count; ++i) {
            const QString word(wordList->wordAt(i));
            outStream << word.toUtf8() << '\n';
        }
        outputFile.close();
    }
}

void HunspellFilterWordTask::run()
{
    if (filterList->isEmpty())
        return;

    filterList->rebuildSearchIndex();

    for (int i = startIndex, count = wordList->size(); i < count;) {
        if (filterList->contains(wordList->wordAt(i))) {
            wordList->removeWordAt(i);
            --count;
        } else {
            ++i;
        }
    }
}

void HunspellBoostWordTask::run()
{
    if (boostList->isEmpty())
        return;

    boostList->rebuildSearchIndex();

    const QString word(wordList->wordAt(0));
    const QString wordCompletion(boostList->findWordCompletion(word));
    if (!wordCompletion.isEmpty()) {
        int from = wordList->indexOfWord(wordCompletion);
        if (from != 1) {
            int to;
            for (to = 1; to < wordList->size() && wordList->wordAt(to).startsWith(word); ++to)
                ;
            if (from != -1) {
                if (to < from)
                    wordList->moveWord(from, to);
            } else {
                wordList->insertWord(to, wordCompletion, HunspellWordList::SpellCheckOk);
            }
        }
    }
}

/*!
    \class QtVirtualKeyboard::HunspellWorker
    \internal
*/

HunspellWorker::HunspellWorker(QObject *parent) :
    QThread(parent),
    idleSema(),
    taskSema(),
    taskLock(),
    hunspell(nullptr)
{
    abort = false;
    qRegisterMetaType<QSharedPointer<HunspellWordList>>("QSharedPointer<HunspellWordList>");
}

HunspellWorker::~HunspellWorker()
{
    abort = true;
    taskSema.release(1);
    wait();
}

void HunspellWorker::addTask(QSharedPointer<HunspellTask> task)
{
    if (task) {
        QMutexLocker guard(&taskLock);
        taskList.append(task);
        taskSema.release();
    }
}

void HunspellWorker::removeAllTasks()
{
    QMutexLocker guard(&taskLock);
    taskList.clear();
}

void HunspellWorker::waitForAllTasks()
{
    qCDebug(lcHunspell) << "waitForAllTasks enter";
    while (isRunning()) {
        idleSema.acquire();
        QMutexLocker guard(&taskLock);
        if (taskList.isEmpty()) {
            idleSema.release();
            break;
        }
        idleSema.release();
    }
    qCDebug(lcHunspell) << "waitForAllTasks leave";
}

void HunspellWorker::run()
{
    QTime perf;
    while (!abort) {
        idleSema.release();
        taskSema.acquire();
        if (abort)
            break;
        idleSema.acquire();
        QSharedPointer<HunspellTask> currentTask;
        {
            QMutexLocker guard(&taskLock);
            if (!taskList.isEmpty()) {
                currentTask = taskList.front();
                taskList.pop_front();
            }
        }
        if (currentTask) {
            QSharedPointer<HunspellLoadDictionaryTask> loadDictionaryTask(currentTask.objectCast<HunspellLoadDictionaryTask>());
            if (loadDictionaryTask)
                loadDictionaryTask->hunspellPtr = &hunspell;
            else if (hunspell)
                currentTask->hunspell = hunspell;
            else
                continue;
            perf.start();
            currentTask->run();
            qCDebug(lcHunspell) << QString(QLatin1String(currentTask->metaObject()->className()) + QLatin1String("::run(): time:")).toLatin1().constData() << perf.elapsed() << "ms";
        }
    }
    if (hunspell) {
        Hunspell_destroy(hunspell);
        hunspell = nullptr;
    }
}

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE
