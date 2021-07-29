/*
 * Qt implementation of OpenWnn library
 * This file is part of the Qt Virtual Keyboard module.
 * Contact: http://www.qt.io/licensing/
 *
 * Copyright (C) 2015  The Qt Company
 * Copyright (C) 2008-2012  OMRON SOFTWARE Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "kanaconverter.h"
#include "wnnlookuptable.h"

#include <QtCore/private/qobject_p.h>

class KanaConverterPrivate : public QObjectPrivate
{
public:
    void createPseudoCandidateListForQwerty(QList<WnnWord> &list, const QString &inputHiragana, const QString &inputRomaji)
    {
        /* Create pseudo candidates for half width alphabet */
        QString convHanEijiLower = inputRomaji.toLower();
        list.append(WnnWord(inputRomaji, inputHiragana, mPosDefault));
        list.append(WnnWord(convHanEijiLower, inputHiragana, mPosSymbol));
        list.append(WnnWord(convertCaps(convHanEijiLower), inputHiragana, mPosSymbol));
        list.append(WnnWord(inputRomaji.toUpper(), inputHiragana, mPosSymbol));

        /* Create pseudo candidates for the full width alphabet */
        QString convZenEiji;
        if (createCandidateString(inputRomaji, fullAlphabetTableQwerty, convZenEiji)) {
            QString convZenEijiLower = convZenEiji.toLower();
            list.append(WnnWord(convZenEiji, inputHiragana, mPosSymbol));
            list.append(WnnWord(convZenEijiLower, inputHiragana, mPosSymbol));
            list.append(WnnWord(convertCaps(convZenEijiLower), inputHiragana, mPosSymbol));
            list.append(WnnWord(convZenEiji.toUpper(), inputHiragana, mPosSymbol));
        }
    }

    static bool createCandidateString(const QString &input, const WnnLookupTable &map, QString &outBuf)
    {
        outBuf.clear();
        for (int index = 0, length = input.length(); index < length; index++) {
            QString out = map.value(input.mid(index, 1));
            if (out.isEmpty())
                return false;
            outBuf.append(out);
        }
        return true;
    }

    QString convertCaps(const QString &moji)
    {
        QString tmp;
        if (!moji.isEmpty()) {
            tmp.append(moji.left(1).toUpper());
            tmp.append(moji.mid(1).toLower());
        }
        return tmp;
    }

    static const int halfKatakanaLength = 83;
    static const char *halfKatakanaKey[KanaConverterPrivate::halfKatakanaLength];
    static const char *halfKatakanaValue[KanaConverterPrivate::halfKatakanaLength];
    static const int fullKatakanaLength = 83;
    static const char *fullKatakanaKey[KanaConverterPrivate::fullKatakanaLength];
    static const char *fullKatakanaValue[KanaConverterPrivate::fullKatakanaLength];
    static const int fullAlphabetLength = 52;
    static const char *fullAlphabetKey[KanaConverterPrivate::fullAlphabetLength];
    static const char *fullAlphabetValue[KanaConverterPrivate::fullAlphabetLength];
    static const WnnLookupTable halfKatakanaTable;
    static const WnnLookupTable fullKatakanaTable;
    static const WnnLookupTable fullAlphabetTableQwerty;

    WnnPOS mPosDefault;
    WnnPOS mPosNumber;
    WnnPOS mPosSymbol;
};

const char *KanaConverterPrivate::halfKatakanaKey[] = {
"ぁ", "あ", "ぃ", "い", "ぅ", "う", "ぇ", "え", "ぉ", "お", "か", "が", "き", "ぎ", "く", "ぐ",
"け", "げ", "こ", "ご", "さ", "ざ", "し", "じ", "す", "ず", "せ", "ぜ", "そ", "ぞ", "た", "だ",
"ち", "ぢ", "っ", "つ", "づ", "て", "で", "と", "ど", "な", "に", "ぬ", "ね", "の", "は", "ば",
"ぱ", "ひ", "び", "ぴ", "ふ", "ぶ", "ぷ", "へ", "べ", "ぺ", "ほ", "ぼ", "ぽ", "ま", "み", "む",
"め", "も", "ゃ", "や", "ゅ", "ゆ", "ょ", "よ", "ら", "り", "る", "れ", "ろ", "ゎ", "わ", "を",
"ん", "ヴ", "ー"
};
const char *KanaConverterPrivate::halfKatakanaValue[] = {
"ｧ", "ｱ", "ｨ", "ｲ", "ｩ", "ｳ", "ｪ", "ｴ", "ｫ", "ｵ", "ｶ", "ｶﾞ", "ｷ", "ｷﾞ", "ｸ", "ｸﾞ",
"ｹ", "ｹﾞ", "ｺ", "ｺﾞ", "ｻ", "ｻﾞ", "ｼ", "ｼﾞ", "ｽ", "ｽﾞ", "ｾ", "ｾﾞ", "ｿ", "ｿﾞ", "ﾀ", "ﾀﾞ",
"ﾁ", "ﾁﾞ", "ｯ", "ﾂ", "ﾂﾞ", "ﾃ", "ﾃﾞ", "ﾄ", "ﾄﾞ", "ﾅ", "ﾆ", "ﾇ", "ﾈ", "ﾉ", "ﾊ", "ﾊﾞ",
"ﾊﾟ", "ﾋ", "ﾋﾞ", "ﾋﾟ", "ﾌ", "ﾌﾞ", "ﾌﾟ", "ﾍ", "ﾍﾞ", "ﾍﾟ", "ﾎ", "ﾎﾞ", "ﾎﾟ", "ﾏ", "ﾐ", "ﾑ",
"ﾒ", "ﾓ", "ｬ", "ﾔ", "ｭ", "ﾕ", "ｮ", "ﾖ", "ﾗ", "ﾘ", "ﾙ", "ﾚ", "ﾛ", "ﾜ", "ﾜ", "ｦ",
"ﾝ", "ｳﾞ", "ｰ"
};
const char *KanaConverterPrivate::fullKatakanaKey[] = {
"ぁ", "あ", "ぃ", "い", "ぅ", "う", "ぇ", "え", "ぉ", "お", "か", "が", "き", "ぎ", "く", "ぐ",
"け", "げ", "こ", "ご", "さ", "ざ", "し", "じ", "す", "ず", "せ", "ぜ", "そ", "ぞ", "た", "だ",
"ち", "ぢ", "っ", "つ", "づ", "て", "で", "と", "ど", "な", "に", "ぬ", "ね", "の", "は", "ば",
"ぱ", "ひ", "び", "ぴ", "ふ", "ぶ", "ぷ", "へ", "べ", "ぺ", "ほ", "ぼ", "ぽ", "ま", "み", "む",
"め", "も", "ゃ", "や", "ゅ", "ゆ", "ょ", "よ", "ら", "り", "る", "れ", "ろ", "ゎ", "わ", "を",
"ん", "ヴ", "ー"
};
const char *KanaConverterPrivate::fullKatakanaValue[] = {
"ァ", "ア", "ィ", "イ", "ゥ", "ウ", "ェ", "エ", "ォ", "オ", "カ", "ガ", "キ", "ギ", "ク", "グ",
"ケ", "ゲ", "コ", "ゴ", "サ", "ザ", "シ", "ジ", "ス", "ズ", "セ", "ゼ", "ソ", "ゾ", "タ", "ダ",
"チ", "ヂ", "ッ", "ツ", "ヅ", "テ", "デ", "ト", "ド", "ナ", "ニ", "ヌ", "ネ", "ノ", "ハ", "バ",
"パ", "ヒ", "ビ", "ピ", "フ", "ブ", "プ", "ヘ", "ベ", "ペ", "ホ", "ボ", "ポ", "マ", "ミ", "ム",
"メ", "モ", "ャ", "ヤ", "ュ", "ユ", "ョ", "ヨ", "ラ", "リ", "ル", "レ", "ロ", "ヮ", "ワ", "ヲ",
"ン", "ヴ", "ー"
};
const char *KanaConverterPrivate::fullAlphabetKey[] = {
"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
"Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "a", "b", "c", "d", "e", "f",
"g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v",
"w", "x", "y", "z"
};
const char *KanaConverterPrivate::fullAlphabetValue[] = {
"Ａ", "Ｂ", "Ｃ", "Ｄ", "Ｅ", "Ｆ", "Ｇ", "Ｈ", "Ｉ", "Ｊ", "Ｋ", "Ｌ", "Ｍ", "Ｎ", "Ｏ", "Ｐ",
"Ｑ", "Ｒ", "Ｓ", "Ｔ", "Ｕ", "Ｖ", "Ｗ", "Ｘ", "Ｙ", "Ｚ", "ａ", "ｂ", "ｃ", "ｄ", "ｅ", "ｆ",
"ｇ", "ｈ", "ｉ", "ｊ", "ｋ", "ｌ", "ｍ", "ｎ", "ｏ", "ｐ", "ｑ", "ｒ", "ｓ", "ｔ", "ｕ", "ｖ",
"ｗ", "ｘ", "ｙ", "ｚ"
};
const WnnLookupTable KanaConverterPrivate::halfKatakanaTable = WnnLookupTable(
            KanaConverterPrivate::halfKatakanaKey,
            KanaConverterPrivate::halfKatakanaValue,
            KanaConverterPrivate::halfKatakanaLength);
const WnnLookupTable KanaConverterPrivate::fullKatakanaTable = WnnLookupTable(
            KanaConverterPrivate::fullKatakanaKey,
            KanaConverterPrivate::fullKatakanaValue,
            KanaConverterPrivate::fullKatakanaLength);
const WnnLookupTable KanaConverterPrivate::fullAlphabetTableQwerty = WnnLookupTable(
            KanaConverterPrivate::fullAlphabetKey,
            KanaConverterPrivate::fullAlphabetValue,
            KanaConverterPrivate::fullAlphabetLength);

KanaConverter::KanaConverter(QObject *parent) :
    QObject(*new KanaConverterPrivate(), parent)
{
}

KanaConverter::~KanaConverter()
{
}

void KanaConverter::setDictionary(OpenWnnDictionary *dict)
{
    Q_D(KanaConverter);
    /* get part of speech tags */
    d->mPosDefault  = dict->getPOS(OpenWnnDictionary::POS_TYPE_MEISI);
    d->mPosNumber   = dict->getPOS(OpenWnnDictionary::POS_TYPE_SUUJI);
    d->mPosSymbol   = dict->getPOS(OpenWnnDictionary::POS_TYPE_KIGOU);
}

QList<WnnWord> KanaConverter::createPseudoCandidateList(const QString &inputHiragana, const QString &inputRomaji)
{
    Q_D(KanaConverter);
    QList<WnnWord> list;

    if (inputHiragana.length() == 0) {
        return list;
    }

    /* Create pseudo candidates for all keyboard type */
    /* Hiragana(reading) / Full width katakana / Half width katakana */
    list.append(WnnWord(inputHiragana, inputHiragana));
    QString stringBuff;
    if (d->createCandidateString(inputHiragana, KanaConverterPrivate::fullKatakanaTable, stringBuff)) {
        list.append(WnnWord(stringBuff, inputHiragana, d->mPosDefault));
    }
    if (d->createCandidateString(inputHiragana, KanaConverterPrivate::halfKatakanaTable, stringBuff)) {
        list.append(WnnWord(stringBuff, inputHiragana, d->mPosDefault));
    }

    /* Create pseudo candidates for Qwerty keyboard */
    d->createPseudoCandidateListForQwerty(list, inputHiragana, inputRomaji);
    return list;
}
