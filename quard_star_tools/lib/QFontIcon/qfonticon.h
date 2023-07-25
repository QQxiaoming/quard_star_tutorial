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

#ifndef QFONTICON_H
#define QFONTICON_H

#include <QObject>
#include <QPainter>
#include <QIconEngine>
#include <QApplication>
#include <QtCore>
#include <QPalette>

class QFontIcon;
class QFontIconEngine;
#define FIcon(code) QFontIcon::icon(code)

class QFontIconEngine : public QIconEngine
{
public:
    QFontIconEngine();
    ~QFontIconEngine();
    virtual void paint(QPainter * painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)Q_DECL_OVERRIDE ;
    virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)Q_DECL_OVERRIDE;
    void setFontFamily(const QString& family);
    // define icon code using QChar or implicit using ushort ...
    void setLetter(const QChar& letter);
    // You can set a base color. I don't advice. Keep system color
    void setBaseColor(const QColor& baseColor);
    virtual QIconEngine* clone() const override;

private:
    QString mFontFamily;
    QChar mLetter;
    QColor mBaseColor;
};

class QFontIcon : public QObject
{
    Q_OBJECT

public:
    // add Font. By default, the first one is used
    static bool addFont(const QString& filename);
    static QFontIcon * instance();
    // main methods. Return icons from code
    static QIcon icon(const QChar& code, const QString& family = QString());
    void setColor(const QColor& color);
    // return added fonts
    const QStringList& families() const;

protected:
    void addFamily(const QString& family);


private:
    explicit QFontIcon(QObject *parent = 0);
    ~QFontIcon();
    static QFontIcon * mInstance;
    QColor baseColor;
    QStringList mfamilies;
};

#endif // QFONTICON_H
