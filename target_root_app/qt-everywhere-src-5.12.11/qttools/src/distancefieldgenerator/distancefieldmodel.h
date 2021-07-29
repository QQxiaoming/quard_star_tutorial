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

#ifndef DISTANCEFIELDMODEL_H
#define DISTANCEFIELDMODEL_H

#include <QAbstractListModel>
#include <QRawFont>
#include <QtGui/private/qtextengine_p.h>
#include <QMultiHash>
#include <QScopedPointer>
#include <QMetaEnum>
#include <QThread>

QT_BEGIN_NAMESPACE

class QThread;
class DistanceFieldModelWorker;
class DistanceFieldModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum UnicodeRange {
        Other                                = 0x00,
        BasicLatin                           = 0x20,
        Latin1Supplement                     = 0xa0,
        LatinExtendedA                       = 0x100,
        LatinExtendedB                       = 0x180,
        IPAExtensions                        = 0x250,
        SpacingModifierLetters               = 0x2b0,
        CombiningDiacriticalMarks            = 0x300,
        GreekAndCoptic                       = 0x370,
        Cyrillic                             = 0x400,
        CyrillicSupplementary                = 0x500,
        Armenian                             = 0x530,
        Hebrew                               = 0x590,
        Arabic                               = 0x600,
        Syriac                               = 0x700,
        Thaana                               = 0x780,
        Devanagari                           = 0x900,
        Bengali                              = 0x980,
        Gurmukhi                             = 0xa00,
        Gujarati                             = 0xa80,
        Oriya                                = 0xb00,
        Tamil                                = 0xb80,
        Telugu                               = 0xc00,
        Kannada                              = 0xc80,
        Malayalam                            = 0xd00,
        Sinhala                              = 0xd80,
        Thai                                 = 0xe00,
        Lao                                  = 0xe80,
        Tibetan                              = 0xf00,
        Myanmar                              = 0x1000,
        Georgian                             = 0x10a0,
        HangulJamo                           = 0x1100,
        Ethiopic                             = 0x1200,
        Cherokee                             = 0x13a0,
        UnifiedCanadianAboriginalSyllabics   = 0x1400,
        Ogham                                = 0x1680,
        Runic                                = 0x16a0,
        Tagalog                              = 0x1700,
        Hanunoo                              = 0x1720,
        Buhid                                = 0x1740,
        Tagbanwa                             = 0x1760,
        Khmer                                = 0x1780,
        Mongolian                            = 0x1800,
        Limbu                                = 0x1900,
        TaiLe                                = 0x1950,
        KhmerSymbols                         = 0x19e0,
        PhoneticExtensions                   = 0x1d00,
        LatinExtendedAdditional              = 0x1e00,
        GreekExtended                        = 0x1f00,
        GeneralPunctuation                   = 0x2000,
        SuperscriptsAndSubscripts            = 0x2070,
        CurrencySymbols                      = 0x20a0,
        CombiningDiacriticalMarksForSymbols  = 0x20d0,
        LetterlikeSymbols                    = 0x2100,
        NumberForms                          = 0x2150,
        Arrows                               = 0x2190,
        MathematicalOperators                = 0x2200,
        MiscellaneousTechnical               = 0x2300,
        ControlPictures                      = 0x2400,
        OpticalCharacterRecognition          = 0x2440,
        EnclosedAlphanumerics                = 0x2460,
        BoxDrawing                           = 0x2500,
        BlockElements                        = 0x2580,
        GeometricShapes                      = 0x25a0,
        MiscellaneousSymbols                 = 0x2600,
        Dingbats                             = 0x2700,
        MiscellaneousMathematicalSymbolsA    = 0x27c0,
        SupplementalArrowsA                  = 0x27f0,
        BraillePatterns                      = 0x2800,
        SupplementalArrowsB                  = 0x2900,
        MiscellaneousMathematicalSymbolsB    = 0x2980,
        SupplementalMathematicalOperators    = 0x2a00,
        MiscellaneousSymbolsAndArrows        = 0x2b00,
        CJKRadicalsSupplement                = 0x2e80,
        KangxiRadicals                       = 0x2f00,
        IdeographicDescriptionCharacters     = 0x2ff0,
        CJKSymbolsAndPunctuation             = 0x3000,
        Hiragana                             = 0x3040,
        Katakana                             = 0x30a0,
        Bopomofo                             = 0x3100,
        HangulCompatibilityJamo              = 0x3130,
        Kanbun                               = 0x3190,
        BopomofoExtended                     = 0x31a0,
        KatakanaPhoneticExtensions           = 0x31f0,
        EnclosedCJKLettersAndMonths          = 0x3200,
        CJKCompatibility                     = 0x3300,
        CJKUnifiedIdeographsExtensionA       = 0x3400,
        YijingHexagramSymbols                = 0x4dc0,
        CJKUnifiedIdeographs                 = 0x4e00,
        YiSyllables                          = 0xa000,
        YiRadicals                           = 0xa490,
        HangulSyllables                      = 0xac00,
        HighSurrogates                       = 0xd800,
        HighPrivateUseSurrogates             = 0xdb80,
        LowSurrogates                        = 0xdc00,
        PrivateUseArea                       = 0xe000,
        CJKCompatibilityIdeographs           = 0xf900,
        AlphabeticPresentationForms          = 0xfb00,
        ArabicPresentationFormsA             = 0xfb50,
        VariationSelectors                   = 0xfe00,
        CombiningHalfMarks                   = 0xfe20,
        CJKCompatibilityForms                = 0xfe30,
        SmallFormVariants                    = 0xfe50,
        ArabicPresentationFormsB             = 0xfe70,
        HalfwidthAndFullwidthForms           = 0xff00,
        Specials                             = 0xfff0,
        LinearBSyllabary                     = 0x10000,
        LinearBIdeograms                     = 0x10080,
        AegeanNumbers                        = 0x10100,
        OldItalic                            = 0x10300,
        Gothic                               = 0x10330,
        Ugaritic                             = 0x10380,
        Deseret                              = 0x10400,
        Shavian                              = 0x10450,
        Osmanya                              = 0x10480,
        CypriotSyllabary                     = 0x10800,
        ByzantineMusicalSymbols              = 0x1d000,
        MusicalSymbols                       = 0x1d100,
        TaiXuanJingSymbols                   = 0x1d300,
        MathematicalAlphanumericSymbols      = 0x1d400,
        CJKUnifiedIdeographsExtensionB       = 0x20000,
        CJKCompatibilityIdeographsSupplement = 0x2f800,
        Tags                                 = 0xe0000,
        End                                  = 0xe007f
    };
    Q_ENUM(UnicodeRange)

    explicit DistanceFieldModel(QObject *parent = nullptr);
    ~DistanceFieldModel() override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setFont(const QString &fileName);

    QList<UnicodeRange> unicodeRanges() const;
    QList<glyph_t> glyphIndexesForUnicodeRange(UnicodeRange range) const;
    QString nameForUnicodeRange(UnicodeRange range) const;
    glyph_t glyphIndexForUcs4(quint32 ucs4) const;

    QImage distanceField(int row) const
    {
        return m_distanceFields.at(row);
    }

    QPainterPath path(int row) const
    {
        return m_paths.at(row);
    }

    qreal pixelSize() const { return m_pixelSize; }
    bool doubleGlyphResolution() const { return m_doubleGlyphResolution; }

signals:
    void startGeneration(quint16 glyphCount);
    void stopGeneration();
    void distanceFieldGenerated();
    void error(const QString &errorString);

private slots:
    void addDistanceField(const QImage &distanceField,
                          const QPainterPath &path,
                          glyph_t glyphId,
                          quint32 ucs4);
    void reserveSpace(quint16 glyphCount,
                      bool doubleResolution,
                      qreal pixelSize);

private:
    UnicodeRange unicodeRangeForUcs4(quint32 ucs4) const;

    QRawFont m_font;
    DistanceFieldModelWorker *m_worker;
    QScopedPointer<QThread> m_workerThread;
    quint16 m_glyphCount;
    QVector<QImage> m_distanceFields;
    QVector<QPainterPath> m_paths;
    QMultiHash<UnicodeRange, glyph_t> m_glyphsPerUnicodeRange;
    QHash<quint32, glyph_t> m_glyphsPerUcs4;
    bool m_doubleGlyphResolution;
    qreal m_pixelSize;
    QMetaEnum m_rangeEnum;
};

QT_END_NAMESPACE

#endif // DISTANCEFIELDMODEL_H
