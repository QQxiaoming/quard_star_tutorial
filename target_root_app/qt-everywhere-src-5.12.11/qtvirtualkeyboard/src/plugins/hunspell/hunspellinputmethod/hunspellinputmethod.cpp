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

#include <QtHunspellInputMethod/private/hunspellinputmethod_p_p.h>
#include <QtVirtualKeyboard/qvirtualkeyboardinputcontext.h>
#include <QLoggingCategory>

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

Q_LOGGING_CATEGORY(lcHunspell, "qt.virtualkeyboard.hunspell")

/*!
    \class QtVirtualKeyboard::HunspellInputMethod
    \internal
*/

HunspellInputMethod::HunspellInputMethod(HunspellInputMethodPrivate *d_ptr, QObject *parent) :
    QVirtualKeyboardAbstractInputMethod(parent),
    d_ptr(d_ptr)
{
}

HunspellInputMethod::HunspellInputMethod(QObject *parent) :
    QVirtualKeyboardAbstractInputMethod(parent),
    d_ptr(new HunspellInputMethodPrivate(this))
{
}

HunspellInputMethod::~HunspellInputMethod()
{
}

QList<QVirtualKeyboardInputEngine::InputMode> HunspellInputMethod::inputModes(const QString &locale)
{
    QList<QVirtualKeyboardInputEngine::InputMode> result;
    switch (QLocale(locale).script()) {
    case QLocale::GreekScript:
        result.append(QVirtualKeyboardInputEngine::InputMode::Greek);
        break;
    case QLocale::CyrillicScript:
        result.append(QVirtualKeyboardInputEngine::InputMode::Cyrillic);
        break;
    case QLocale::ArabicScript:
        result.append(QVirtualKeyboardInputEngine::InputMode::Arabic);
        break;
    case QLocale::HebrewScript:
        result.append(QVirtualKeyboardInputEngine::InputMode::Hebrew);
        break;
    default:
        break;
    }
    result.append(QVirtualKeyboardInputEngine::InputMode::Latin);
    result.append(QVirtualKeyboardInputEngine::InputMode::Numeric);
    return result;
}

bool HunspellInputMethod::setInputMode(const QString &locale, QVirtualKeyboardInputEngine::InputMode inputMode)
{
    Q_UNUSED(inputMode)
    Q_D(HunspellInputMethod);
    return d->createHunspell(locale);
}

bool HunspellInputMethod::setTextCase(QVirtualKeyboardInputEngine::TextCase textCase)
{
    Q_UNUSED(textCase)
    return true;
}

bool HunspellInputMethod::keyEvent(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers)
{
    Q_D(HunspellInputMethod);
    QVirtualKeyboardInputContext *ic = inputContext();
    Qt::InputMethodHints inputMethodHints = ic->inputMethodHints();
    bool accept = false;
    switch (key) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Tab:
    case Qt::Key_Space:
        update();
        break;
    case Qt::Key_Backspace:
    {
        QString word = d->wordCandidates.wordAt(0);
        if (!word.isEmpty()) {
            word.remove(word.length() - 1, 1);
            ic->setPreeditText(word);
            if (!word.isEmpty()) {
                d->wordCandidates.updateWord(0, word);
                if (d->updateSuggestions()) {
                    emit selectionListChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
                    emit selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList, d->wordCandidates.index());
                }
            } else {
                d->reset();
            }
            accept = true;
        }
        break;
    }
    default:
        if (inputMethodHints.testFlag(Qt::ImhNoPredictiveText))
            break;
        if (d->dictionaryState == HunspellInputMethodPrivate::DictionaryNotLoaded) {
            update();
            break;
        }
        if (text.length() > 0) {
            QChar c = text.at(0);
            QString word = d->wordCandidates.wordAt(0);
            bool addToWord = d->isValidInputChar(c) && (!word.isEmpty() || !d->isJoiner(c));
            if (addToWord) {
                /*  Automatic space insertion. */
                if (word.isEmpty()) {
                    QString surroundingText = ic->surroundingText();
                    int cursorPosition = ic->cursorPosition();
                    /*  Rules for automatic space insertion:
                        - Surrounding text is not empty
                        - Cursor is at the end of the line
                        - No space before the cursor
                        - No spefic characters before the cursor; minus and apostrophe
                    */
                    if (!surroundingText.isEmpty() && cursorPosition == surroundingText.length()) {
                        QChar lastChar = surroundingText.at(cursorPosition - 1);
                        if (!lastChar.isSpace() &&
                            lastChar != Qt::Key_Minus &&
                            d->isAutoSpaceAllowed()) {
                            ic->commit(QLatin1String(" "));
                        }
                    }
                }
                /*  Ignore possible call to update() function when sending initial
                    pre-edit text. The update is triggered if the text editor has
                    a selection which the pre-edit text will replace.
                */
                d->ignoreUpdate = word.isEmpty();
                word.append(text);
                d->wordCandidates.updateWord(0, word);
                ic->setPreeditText(word);
                d->ignoreUpdate = false;
                if (d->updateSuggestions()) {
                    emit selectionListChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
                    emit selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList, d->wordCandidates.index());
                }
                accept = true;
            } else if (text.length() > 1) {
                bool addSpace = !word.isEmpty() || d->autoSpaceAllowed;
                update();
                d->autoSpaceAllowed = true;
                if (addSpace && d->isAutoSpaceAllowed())
                    ic->commit(QLatin1String(" "));
                ic->commit(text);
                d->autoSpaceAllowed = addSpace;
                accept = true;
            } else {
                update();
                inputContext()->sendKeyClick(key, text, modifiers);
                d->autoSpaceAllowed = true;
                accept = true;
            }
        }
        break;
    }
    return accept;
}

QList<QVirtualKeyboardSelectionListModel::Type> HunspellInputMethod::selectionLists()
{
    Q_D(const HunspellInputMethod);
    QVirtualKeyboardInputContext *ic = inputContext();
    if (!ic)
        return QList<QVirtualKeyboardSelectionListModel::Type>();
    Qt::InputMethodHints inputMethodHints = ic->inputMethodHints();
    if (d->dictionaryState == HunspellInputMethodPrivate::DictionaryNotLoaded || inputMethodHints.testFlag(Qt::ImhNoPredictiveText) || inputMethodHints.testFlag(Qt::ImhHiddenText))
        return QList<QVirtualKeyboardSelectionListModel::Type>();
    return QList<QVirtualKeyboardSelectionListModel::Type>() << QVirtualKeyboardSelectionListModel::Type::WordCandidateList;
}

int HunspellInputMethod::selectionListItemCount(QVirtualKeyboardSelectionListModel::Type type)
{
    Q_UNUSED(type)
    Q_D(HunspellInputMethod);
    return d->wordCandidates.size();
}

QVariant HunspellInputMethod::selectionListData(QVirtualKeyboardSelectionListModel::Type type, int index, QVirtualKeyboardSelectionListModel::Role role)
{
    QVariant result;
    Q_D(HunspellInputMethod);
    switch (role) {
    case QVirtualKeyboardSelectionListModel::Role::Display:
        result = QVariant(d->wordCandidates.wordAt(index));
        break;
    case QVirtualKeyboardSelectionListModel::Role::WordCompletionLength:
    {
        const QString wordCandidate(d->wordCandidates.wordAt(index));
        const QString word(d->wordCandidates.wordAt(0));
        int wordCompletionLength = wordCandidate.length() - word.length();
        result.setValue((wordCompletionLength > 0 && wordCandidate.startsWith(word)) ? wordCompletionLength : 0);
        break;
    }
    case QVirtualKeyboardSelectionListModel::Role::Dictionary:
    {
        const QString wordCandidate(d->wordCandidates.wordAt(index));
        QVirtualKeyboardSelectionListModel::DictionaryType dictionaryType =
                d->userDictionaryWords && d->userDictionaryWords->contains(wordCandidate) ?
                    QVirtualKeyboardSelectionListModel::DictionaryType::User : QVirtualKeyboardSelectionListModel::DictionaryType::Default;
        result = QVariant(static_cast<int>(dictionaryType));
        break;
    }
    case QVirtualKeyboardSelectionListModel::Role::CanRemoveSuggestion:
        result.setValue(index > 0 && d->wordCandidates.wordFlagsAt(index).testFlag(HunspellWordList::SpellCheckOk));
        break;
    default:
        result = QVirtualKeyboardAbstractInputMethod::selectionListData(type, index, role);
        break;
    }
    return result;
}

void HunspellInputMethod::selectionListItemSelected(QVirtualKeyboardSelectionListModel::Type type, int index)
{
    Q_UNUSED(type)
    Q_D(HunspellInputMethod);
    d->wordCandidates.setIndex(index);
    d->addToDictionary();
    QString finalWord = d->wordCandidates.wordAt(index);
    reset();
    inputContext()->commit(finalWord);
    d->autoSpaceAllowed = true;
}

bool HunspellInputMethod::selectionListRemoveItem(QVirtualKeyboardSelectionListModel::Type type, int index)
{
    Q_D(HunspellInputMethod);
    Q_UNUSED(type)

    if (index <= 0 || index >= d->wordCandidates.size())
        return false;

    QString word = d->wordCandidates.wordAt(index);
    d->removeFromDictionary(word);

    return true;
}

bool HunspellInputMethod::reselect(int cursorPosition, const QVirtualKeyboardInputEngine::ReselectFlags &reselectFlags)
{
    Q_D(HunspellInputMethod);
    QString word(d->wordCandidates.wordAt(0));
    Q_ASSERT(word.isEmpty());

    if (d->dictionaryState == HunspellInputMethodPrivate::DictionaryNotLoaded)
        return false;

    QVirtualKeyboardInputContext *ic = inputContext();
    if (!ic)
        return false;

    const QString surroundingText = ic->surroundingText();
    int replaceFrom = 0;

    if (reselectFlags.testFlag(QVirtualKeyboardInputEngine::ReselectFlag::WordBeforeCursor)) {
        for (int i = cursorPosition - 1; i >= 0; --i) {
            QChar c = surroundingText.at(i);
            if (!d->isValidInputChar(c))
                break;
            word.insert(0, c);
            --replaceFrom;
        }

        while (replaceFrom < 0 && d->isJoiner(word.at(0))) {
            word.remove(0, 1);
            ++replaceFrom;
        }
    }

    if (reselectFlags.testFlag(QVirtualKeyboardInputEngine::ReselectFlag::WordAtCursor) && replaceFrom == 0)
        return false;

    if (reselectFlags.testFlag(QVirtualKeyboardInputEngine::ReselectFlag::WordAfterCursor)) {
        for (int i = cursorPosition; i < surroundingText.length(); ++i) {
            QChar c = surroundingText.at(i);
            if (!d->isValidInputChar(c))
                break;
            word.append(c);
        }

        while (replaceFrom > -word.length()) {
            int lastPos = word.length() - 1;
            if (!d->isJoiner(word.at(lastPos)))
                break;
            word.remove(lastPos, 1);
        }
    }

    if (word.isEmpty())
        return false;

    if (reselectFlags.testFlag(QVirtualKeyboardInputEngine::ReselectFlag::WordAtCursor) && replaceFrom == -word.length())
        return false;

    if (d->isJoiner(word.at(0)))
        return false;

    if (d->isJoiner(word.at(word.length() - 1)))
        return false;

    d->wordCandidates.updateWord(0, word);
    ic->setPreeditText(word, QList<QInputMethodEvent::Attribute>(), replaceFrom, word.length());

    d->autoSpaceAllowed = false;
    if (d->updateSuggestions()) {
        emit selectionListChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
        emit selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList, d->wordCandidates.index());
    }

    return true;
}

void HunspellInputMethod::reset()
{
    Q_D(HunspellInputMethod);
    d->reset();
}

void HunspellInputMethod::update()
{
    Q_D(HunspellInputMethod);
    if (d->ignoreUpdate)
        return;

    QString finalWord;
    if (!d->wordCandidates.isEmpty()) {
        d->addToDictionary();
        finalWord = d->wordCandidates.wordAt(d->wordCandidates.index());
    }
    d->reset();
    inputContext()->commit(finalWord);
    d->autoSpaceAllowed = false;
}

void HunspellInputMethod::updateSuggestions(const QSharedPointer<HunspellWordList> &wordList, int tag)
{
    Q_D(HunspellInputMethod);
    if (d->dictionaryState == HunspellInputMethodPrivate::DictionaryNotLoaded) {
        qCDebug(lcHunspell) << "updateSuggestions: skip (dictionary not loaded)";
        update();
        return;
    }
    if (d->wordCandidatesUpdateTag != tag) {
        qCDebug(lcHunspell) << "updateSuggestions: skip tag" << tag << "current" << d->wordCandidatesUpdateTag;
        return;
    }
    QString word(d->wordCandidates.wordAt(0));
    d->wordCandidates = *wordList;
    if (d->wordCandidates.wordAt(0).compare(word) != 0)
        d->wordCandidates.updateWord(0, word);
    emit selectionListChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
    emit selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList, d->wordCandidates.index());
}

void HunspellInputMethod::dictionaryLoadCompleted(bool success)
{
    Q_D(HunspellInputMethod);
    QVirtualKeyboardInputContext *ic = inputContext();
    if (!ic)
        return;

    QList<QVirtualKeyboardSelectionListModel::Type> oldSelectionLists = selectionLists();
    d->dictionaryState = success ? HunspellInputMethodPrivate::DictionaryReady :
                                   HunspellInputMethodPrivate::DictionaryNotLoaded;
    QList<QVirtualKeyboardSelectionListModel::Type> newSelectionLists = selectionLists();
    if (oldSelectionLists != newSelectionLists)
        emit selectionListsChanged();
}

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE
