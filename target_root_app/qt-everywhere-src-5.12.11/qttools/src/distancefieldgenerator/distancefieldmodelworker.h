/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
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
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DISTANCEFIELDMODELWORKER_H
#define DISTANCEFIELDMODELWORKER_H

#include <QObject>
#include <QRawFont>
#include <QtGui/private/qtextengine_p.h>

QT_BEGIN_NAMESPACE

struct CmapSubtable0;
struct CmapSubtable4;
struct CmapSubtable6;
struct CmapSubtable10;
struct CmapSubtable12;
class DistanceFieldModelWorker : public QObject
{
    Q_OBJECT
public:
    explicit DistanceFieldModelWorker(QObject *parent = nullptr);

    Q_INVOKABLE void generateOneDistanceField();
    Q_INVOKABLE void loadFont(const QString &fileName);

    void readCmapSubtable(const CmapSubtable0 *subtable, const void *end);
    void readCmapSubtable(const CmapSubtable4 *subtable, const void *end);
    void readCmapSubtable(const CmapSubtable6 *subtable, const void *end);
    void readCmapSubtable(const CmapSubtable10 *subtable, const void *end);
    void readCmapSubtable(const CmapSubtable12 *subtable, const void *end);

signals:
    void fontLoaded(quint16 glyphCount, bool doubleResolution, qreal pixelSize);
    void fontGenerated();
    void distanceFieldGenerated(const QImage &distanceField,
                                const QPainterPath &path,
                                glyph_t glyphId,
                                quint32 cmapAssignment);
    void error(const QString &errorString);

private:
    void readGlyphCount();
    void readCmap();

    QRawFont m_font;
    quint16 m_glyphCount;
    quint16 m_nextGlyphId;
    bool m_doubleGlyphResolution;
    QHash<glyph_t, quint32> m_cmapping;
};

QT_END_NAMESPACE

#endif // DISTANCEFIELDMODELWORKER_H
