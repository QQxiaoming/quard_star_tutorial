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

#ifndef HUNSPELLINPUTMETHOD_P_P_H
#define HUNSPELLINPUTMETHOD_P_P_H

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

#include <QtHunspellInputMethod/private/hunspellinputmethod_p.h>
#include <QtHunspellInputMethod/private/hunspellworker_p.h>

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

class QHUNSPELLINPUTMETHOD_EXPORT HunspellInputMethodPrivate
{
    Q_DECLARE_PUBLIC(HunspellInputMethod)

public:
    HunspellInputMethodPrivate(HunspellInputMethod *q_ptr);
    ~HunspellInputMethodPrivate();

    enum DictionaryState {
        DictionaryNotLoaded,
        DictionaryLoading,
        DictionaryReady
    };

    bool createHunspell(const QString &locale);
    void reset();
    bool updateSuggestions();
    bool clearSuggestions(bool clearInputWord = false);
    void clearSuggestionsRelatedTasks();
    bool isAutoSpaceAllowed() const;
    bool isValidInputChar(const QChar &c) const;
    bool isJoiner(const QChar &c) const;
    QString customDictionaryLocation(const QString &dictionaryType) const;
    void loadCustomDictionary(const QSharedPointer<HunspellWordList> &wordList, const QString &dictionaryType) const;
    void saveCustomDictionary(const QSharedPointer<HunspellWordList> &wordList, const QString &dictionaryType) const;
    void addToHunspell(const QSharedPointer<HunspellWordList> &wordList) const;
    void removeFromHunspell(const QSharedPointer<HunspellWordList> &wordList) const;
    void removeFromDictionary(const QString &word);
    void addToDictionary();

    HunspellInputMethod *q_ptr;
    QScopedPointer<HunspellWorker> hunspellWorker;
    QString locale;
    HunspellWordList wordCandidates;
    int wordCompletionPoint;
    bool ignoreUpdate;
    bool autoSpaceAllowed;
    DictionaryState dictionaryState;
    QSharedPointer<HunspellWordList> userDictionaryWords;
    QSharedPointer<HunspellWordList> blacklistedWords;
    int wordCandidatesUpdateTag;
    static const int userDictionaryMaxSize;
};

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE

#endif // HUNSPELLINPUTMETHOD_P_P_H
