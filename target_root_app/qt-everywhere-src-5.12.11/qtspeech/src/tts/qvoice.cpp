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



#include "qvoice.h"
#include "qvoice_p.h"
#include "qtexttospeech.h"

QT_BEGIN_NAMESPACE

/*!
  \class QVoice
  \brief The QVoice class allows to set and retrieve values of a particular voice.
  \inmodule QtSpeech
*/

/*!
    \enum QVoice::Age

    The age of a voice.

    \value Child    Voice of a child
    \value Teenager Voice of a teenager
    \value Adult    Voice of an adult
    \value Senior   Voice of a senior
    \value Other    Voice of unknown age
*/

/*!
    \enum QVoice::Gender

    The gender of a voice.

    \value Male    Voice of a male
    \value Female  Voice of a female
    \value Unknown Voice of unknown gender
*/

QVoice::QVoice()
{
    d = new QVoicePrivate();
}

QVoice::QVoice(const QVoice &other)
    :d(other.d)
{
}

QVoice::QVoice(const QString &name, Gender gender, Age age, const QVariant &data)
    :d(new QVoicePrivate(name, gender, age, data))
{
}

QVoice::~QVoice()
{
}

void QVoice::operator=(const QVoice &other)
{
    d->name = other.d->name;
    d->gender = other.d->gender;
    d->age = other.d->age;
    d->data = other.d->data;
}

/*!
    Compares the \l name, \l gender, and \l age of this voice with \a other.
    Returns \c true if all of them match.
*/
bool QVoice::operator==(const QVoice &other)
{
    if (d->name != other.d->name ||
        d->gender != other.d->gender ||
        d->age != other.d->age ||
        d->data != other.d->data)
        return false;
    return true;
}

/*!
    Compares the \l name, \l gender, and \l age of this voice with \a other.
    Returns \c true if they are not identical.
*/
bool QVoice::operator!=(const QVoice &other)
{
    return !operator==(other);
}

/*!
   Assign a \a name to a voice.
*/
void QVoice::setName(const QString &name)
{
    d->name = name;
}

/*!
   Assign a \a gender to a voice.
*/
void QVoice::setGender(Gender gender)
{
    d->gender = gender;
}

/*!
   Set the \a age property.
*/
void QVoice::setAge(Age age)
{
    d->age = age;
}

void QVoice::setData(const QVariant &data)
{
    d->data = data;
}

/*!
   Returns the name of a voice.
*/
QString QVoice::name() const
{
    return d->name;
}

/*!
   Returns the age of a voice.
*/
QVoice::Age QVoice::age() const
{
    return d->age;
}

/*!
   Returns the gender of a voice.
*/
QVoice::Gender QVoice::gender() const
{
    return d->gender;
}

QVariant QVoice::data() const
{
    return d->data;
}

/*!̈́
   Returns the \a gender name of a voice.
*/
QString QVoice::genderName(QVoice::Gender gender)
{
    QString retval;
    switch (gender) {
        case QVoice::Male:
            retval = QTextToSpeech::tr("Male", "Gender of a voice");
            break;
        case QVoice::Female:
            retval = QTextToSpeech::tr("Female", "Gender of a voice");
            break;
        case QVoice::Unknown:
        default:
            retval = QTextToSpeech::tr("Unknown Gender", "Voice gender is unknown");
            break;
    }
    return retval;
}

/*!
   Returns a string representing the \a age class of a voice.
*/
QString QVoice::ageName(QVoice::Age age)
{
    QString retval;
    switch (age) {
        case QVoice::Child:
            retval = QTextToSpeech::tr("Child", "Age of a voice");
            break;
        case QVoice::Teenager:
            retval = QTextToSpeech::tr("Teenager", "Age of a voice");
            break;
        case QVoice::Adult:
            retval = QTextToSpeech::tr("Adult", "Age of a voice");
            break;
        case QVoice::Senior:
            retval = QTextToSpeech::tr("Senior", "Age of a voice");
            break;
        case QVoice::Other:
        default:
            retval = QTextToSpeech::tr("Other Age", "Unknown age of a voice");
            break;
    }
    return retval;
}

QT_END_NAMESPACE
