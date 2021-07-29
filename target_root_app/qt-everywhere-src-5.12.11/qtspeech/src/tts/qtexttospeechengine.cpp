/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Speech module of the Qt Toolkit.
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

#include "qtexttospeechengine.h"

#include <QLoggingCategory>

QT_BEGIN_NAMESPACE


/*!
  \class QTextToSpeechEngine
  \inmodule QtSpeech
  \brief The QTextToSpeechEngine class is the base for text-to-speech engine integrations.

  An engine implementation must derive from QTextToSpeechEngine and implement all
  its pure virtual methods.
*/

/*! \fn QVector<QLocale> QTextToSpeechEngine::availableLocales() const

  Implementation of \l QTextToSpeech::availableLocales().
*/

/*! \fn QVector<QVoice> QTextToSpeechEngine::availableVoices() const

  Implementation of \l QTextToSpeech::availableVoices().
*/

/*! \fn void QTextToSpeechEngine::say(const QString &text)

  Implementation of \l {QTextToSpeech::say()}{QTextToSpeech::say}(\a text).
*/

/*! \fn void QTextToSpeechEngine::stop()

  Implementation of \l QTextToSpeech::stop().
*/

/*! \fn void QTextToSpeechEngine::pause()

  Implementation of \l QTextToSpeech::pause().
*/

/*! \fn void QTextToSpeechEngine::resume()

  Implementation of \l QTextToSpeech::resume().
*/

/*! \fn void QTextToSpeechEngine::rate() const

  Implementation of \l QTextToSpeech::rate().
*/

/*! \fn bool QTextToSpeechEngine::setRate(double rate)

  Implementation of \l {QTextToSpeech::setRate()}{QTextToSpeech::setRate}(\a rate).

  Return \c true if the operation was successful.
*/

/*! \fn void QTextToSpeechEngine::pitch() const

  Implementation of \l QTextToSpeech::pitch().
*/

/*! \fn bool QTextToSpeechEngine::setPitch(double pitch)

  Implementation of \l {QTextToSpeech::setPitch()}{QTextToSpeech::setPitch}(\a pitch).

  Return \c true if the operation was successful.
*/

/*! \fn QLocale QTextToSpeechEngine::locale() const

  Implementation of QTextToSpeech::locale().
*/

/*! \fn bool QTextToSpeechEngine::setLocale(const QLocale &locale)

  Implementation \l {QTextToSpeech::setLocale()}{QTextToSpeech::setLocale}(\a locale).

  Return \c true if the operation was successful. In this case, the
  current voice (as returned by voice()) should also be updated to a
  new, valid value.
*/

/*! \fn double QTextToSpeechEngine::volume() const

  Implementation of QTextToSpeech::volume().
*/

/*! \fn bool QTextToSpeechEngine::setVolume(double volume)

  Implementation of \l {QTextToSpeech::setVolume()}{QTextToSpeech::setVolume}(\a volume).

  Return \c true if the operation was successful.
*/

/*! \fn QVoice QTextToSpeechEngine::voice() const

  Implementation of \l QTextToSpeech::voice().
*/

/*! \fn bool QTextToSpeechEngine::setVoice(const QVoice &voice)

  Implementation of \l {QTextToSpeech::setVoice()}{QTextToSpeech::setVoice}(\a voice).

  Return \c true if the operation was successful.
*/

/*! \fn QTextToSpeech::State QTextToSpeechEngine::state() const

  Implementation of QTextToSpeech::state().
*/

/*! \fn void QTextToSpeechEngine::stateChanged(QTextToSpeech::State state)

  Emitted when the text-to-speech engine \a state has changed.

  This signal is connected to QTextToSpeech::stateChanged() signal.
*/

/*!
  Constructs the text-to-speech engine base class with \a parent.
*/
QTextToSpeechEngine::QTextToSpeechEngine(QObject *parent):
    QObject(parent)
{
}

QTextToSpeechEngine::~QTextToSpeechEngine()
{
}

/*!
  Creates a voice for a text-to-speech engine.

  Parameters \a name, \a gender, \a age and \a data are directly stored in the QVoice instance.
*/
QVoice QTextToSpeechEngine::createVoice(const QString &name, QVoice::Gender gender, QVoice::Age age, const QVariant &data)
{
    return QVoice(name, gender, age, data);
}

/*!
  Returns the engine-specific private data for the given \a voice.

*/
QVariant QTextToSpeechEngine::voiceData(const QVoice &voice)
{
    return voice.data();
}

QT_END_NAMESPACE
