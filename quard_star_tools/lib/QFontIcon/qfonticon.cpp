/*
MIT License

Copyright (c) 2017 Sacha Schutz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "qfonticon.h"
#include <QDebug>
#include <QFontDatabase>
QFontIcon * QFontIcon::mInstance = Q_NULLPTR;

bool QFontIcon::addFont(const QString &filename)
{
    int id = QFontDatabase::addApplicationFont(filename);

    if (id == -1){
        qWarning()<<"Cannot load font";
        return false;
    }

    QString family = QFontDatabase::applicationFontFamilies(id).first();
    instance()->addFamily(family);
    return true;
}

QFontIcon *QFontIcon::instance()
{
    if (!mInstance)
        mInstance = new QFontIcon;

    return mInstance;
}

QIcon QFontIcon::icon(const QChar &code, const QString &family)
{
    if (instance()->families().isEmpty())
    {
        qWarning()<<Q_FUNC_INFO<<"No font family installed";
        return QIcon();
    }

    QString useFamily = family;
    if (useFamily.isEmpty())
        useFamily = instance()->families().first();


    QFontIconEngine * engine = new QFontIconEngine;
    engine->setFontFamily(useFamily);
    engine->setLetter(code);
    engine->setBaseColor(instance()->baseColor);
    return QIcon(engine);


}


const QStringList &QFontIcon::families() const
{
    return mfamilies;
}

void QFontIcon::addFamily(const QString &family)
{
    mfamilies.append(family);
}

void QFontIcon::setColor(const QColor& color)
{
    baseColor = color;
}

QFontIcon::QFontIcon(QObject *parent)
    :QObject(parent)
{
    baseColor = QColor();
}

QFontIcon::~QFontIcon()
{

}

//=======================================================================================================


QFontIconEngine::QFontIconEngine()
    :QIconEngine()
{
}

QFontIconEngine::~QFontIconEngine()
{
}

void QFontIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state);
    QFont font = QFont(mFontFamily);
    int drawSize = qRound(rect.height() * 0.8);
    font.setPixelSize(drawSize);

    QColor penColor;
    if (!mBaseColor.isValid())
        penColor = QApplication::palette("QWidget").color(QPalette::Normal,QPalette::ButtonText);
    else
        penColor = mBaseColor;

    if (mode == QIcon::Disabled)
        penColor = QApplication::palette("QWidget").color(QPalette::Disabled,QPalette::ButtonText);


    if (mode == QIcon::Selected)
        penColor = QApplication::palette("QWidget").color(QPalette::Active,QPalette::ButtonText);


    painter->save();
    painter->setPen(QPen(penColor));
    painter->setFont(font);
    painter->drawText(rect, Qt::AlignCenter|Qt::AlignVCenter, mLetter);

    painter->restore();
}

QPixmap QFontIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmap pix(size);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    paint(&painter, QRect(QPoint(0,0),size), mode, state);
    return pix;

}

void QFontIconEngine::setFontFamily(const QString &family)
{
    mFontFamily = family;
}

void QFontIconEngine::setLetter(const QChar &letter)
{
    mLetter = letter;
}

void QFontIconEngine::setBaseColor(const QColor &baseColor)
{
    mBaseColor = baseColor;
}


QIconEngine *QFontIconEngine::clone() const
{
    QFontIconEngine * engine = new QFontIconEngine;
    engine->setFontFamily(mFontFamily);
    engine->setBaseColor(mBaseColor);
    return engine;
}

