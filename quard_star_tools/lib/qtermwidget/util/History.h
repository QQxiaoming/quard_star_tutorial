/*
 This file is part of Konsole, an X terminal.
 Copyright 1997,1998 by Lars Doelle <lars.doelle@on-line.de>
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301  USA.
*/
#ifndef HISTORY_H
#define HISTORY_H

#include <QBitRef>
#include <QHash>
#include <QVector>
#include <QTemporaryFile>

#include "Character.h"

//////////////////////////////////////////////////////////////////////
// Abstract base class for file and buffer versions
//////////////////////////////////////////////////////////////////////
class HistoryType;
class HistoryScroll
{
public:
    HistoryScroll(HistoryType*);
    virtual ~HistoryScroll();

    virtual bool hasScroll();

    // access to history
    virtual int  getLines() = 0;
    virtual int  getLineLen(int lineno) = 0;
    virtual void getCells(int lineno, int colno, int count, Character res[]) = 0;
    virtual bool isWrappedLine(int lineno) = 0;

    // backward compatibility (obsolete)
    Character   getCell(int lineno, int colno) { Character res; getCells(lineno,colno,1,&res); return res; }

    // adding lines.
    virtual void addCells(const Character a[], int count) = 0;
    // convenience method - this is virtual so that subclasses can take advantage
    // of QVector's implicit copying
    virtual void addCellsVector(const QVector<Character>& cells) {
        addCells(cells.data(),cells.size());
    }

    virtual void addLine(bool previousWrapped=false) = 0;

    //
    // FIXME:  Passing around constant references to HistoryType instances
    // is very unsafe, because those references will no longer
    // be valid if the history scroll is deleted.
    //
    const HistoryType& getType() const { return *m_histType; }

protected:
    HistoryType* m_histType;
};

class HistoryScrollBuffer : public HistoryScroll
{
public:
    typedef QVector<Character> HistoryLine;

    HistoryScrollBuffer(unsigned int maxNbLines = 1000);
    ~HistoryScrollBuffer() override;

    int  getLines() override;
    int  getLineLen(int lineno) override;
    void getCells(int lineno, int colno, int count, Character res[]) override;
    bool isWrappedLine(int lineno) override;

    void addCells(const Character a[], int count) override;
    void addCellsVector(const QVector<Character>& cells) override;
    void addLine(bool previousWrapped=false) override;

    void setMaxNbLines(unsigned int nbLines);
    unsigned int maxNbLines() const { return _maxLineCount; }

private:
    int bufferIndex(int lineNumber) const;

    HistoryLine* _historyBuffer;
    QBitArray _wrappedLine;
    int _maxLineCount;
    int _usedLines;
    int _head;
};

class HistoryScrollNone : public HistoryScroll
{
public:
    HistoryScrollNone();
    ~HistoryScrollNone() override;

    bool hasScroll() override;

    int  getLines() override;
    int  getLineLen(int lineno) override;
    void getCells(int lineno, int colno, int count, Character res[]) override;
    bool isWrappedLine(int lineno) override;

    void addCells(const Character a[], int count) override;
    void addLine(bool previousWrapped=false) override;
};


typedef QVector<Character> TextLine;

class CharacterFormat
{
public:
    bool equalsFormat(const CharacterFormat &other) const {
        return other.rendition==rendition && other.fgColor==fgColor && other.bgColor==bgColor;
    }

    bool equalsFormat(const Character &c) const {
        return c.rendition==rendition && c.foregroundColor==fgColor && c.backgroundColor==bgColor;
    }

    void setFormat(const Character& c) {
        rendition=c.rendition;
        fgColor=c.foregroundColor;
        bgColor=c.backgroundColor;
    }

    CharacterColor fgColor, bgColor;
    quint16 startPos;
    quint8 rendition;
};

class HistoryType
{
public:
    HistoryType();
    virtual ~HistoryType();

    /**
     * Returns true if the history is enabled ( can store lines of output )
     * or false otherwise.
     */
    virtual bool isEnabled()           const = 0;
    /**
     * Returns true if the history size is unlimited.
     */
    bool isUnlimited() const { return maximumLineCount() == 0; }
    /**
     * Returns the maximum number of lines which this history type
     * can store or 0 if the history can store an unlimited number of lines.
     */
    virtual int maximumLineCount()    const = 0;

    virtual HistoryScroll* scroll(HistoryScroll *) const = 0;
};

class HistoryTypeNone : public HistoryType
{
public:
    HistoryTypeNone();

    bool isEnabled() const override;
    int maximumLineCount() const override;

    HistoryScroll* scroll(HistoryScroll *) const override;
};

class HistoryTypeBuffer : public HistoryType
{
    friend class HistoryScrollBuffer;

public:
    HistoryTypeBuffer(unsigned int nbLines);

    bool isEnabled() const override;
    int maximumLineCount() const override;

    HistoryScroll* scroll(HistoryScroll *) const override;

protected:
  unsigned int m_nbLines;
};

#endif // HISTORY_H
