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

#include "romkan.h"
#include "strsegment.h"
#include "wnnlookuptable.h"

class RomkanPrivate
{
public:
    static const int romkanTableLength = 356;
    static const char *romkanTableKey[romkanTableLength];
    static const char *romkanTableValue[romkanTableLength];
    static const WnnLookupTable romkanTable;

    /** Max length of the target text */
    static const int MAX_LENGTH;
};

const char *RomkanPrivate::romkanTableKey[] = {
"", "!", "\"", "#", "$", "%", "&", "\'", "(", ")", "*", "+", ",", "-", ".", "/",
"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
"@", "[", "\\", "]", "^", "_", "`", "a", "ba", "bb", "be", "bi", "bo", "bu", "bya", "bye",
"byi", "byo", "byu", "ca", "cc", "ce", "cha", "che", "chi", "cho", "chu", "ci", "co", "cu", "cya", "cye",
"cyi", "cyo", "cyu", "da", "dd", "de", "dha", "dhe", "dhi", "dho", "dhu", "di", "do", "du", "dwa", "dwe",
"dwi", "dwo", "dwu", "dya", "dye", "dyi", "dyo", "dyu", "e", "fa", "fe", "ff", "fi", "fo", "fu", "fwa",
"fwe", "fwi", "fwo", "fwu", "fya", "fye", "fyi", "fyo", "fyu", "ga", "ge", "gg", "gi", "go", "gu", "gwa",
"gwe", "gwi", "gwo", "gwu", "gya", "gye", "gyi", "gyo", "gyu", "ha", "he", "hh", "hi", "ho", "hu", "hya",
"hye", "hyi", "hyo", "hyu", "i", "ja", "je", "ji", "jj", "jo", "ju", "jya", "jye", "jyi", "jyo", "jyu",
"ka", "ke", "ki", "kk", "ko", "ku", "kwa", "kya", "kye", "kyi", "kyo", "kyu", "la", "le", "li", "ll",
"lo", "ltsu", "ltu", "lu", "lwa", "lya", "lye", "lyi", "lyo", "lyu", "ma", "me", "mi", "mm", "mo", "mu",
"mya", "mye", "myi", "myo", "myu", "na", "nb", "nc", "nd", "ne", "nf", "ng", "nh", "ni", "nj", "nk",
"nl", "nm", "nn", "no", "np", "nq", "nr", "ns", "nt", "nu", "nv", "nw", "nx", "nya", "nye", "nyi",
"nyo", "nyu", "nz", "o", "pa", "pe", "pi", "po", "pp", "pu", "pya", "pye", "pyi", "pyo", "pyu", "qa",
"qe", "qi", "qo", "qq", "qu", "qwa", "qwe", "qwi", "qwo", "qwu", "qya", "qye", "qyi", "qyo", "qyu", "ra",
"re", "ri", "ro", "rr", "ru", "rya", "rye", "ryi", "ryo", "ryu", "sa", "se", "sha", "she", "shi", "sho",
"shu", "si", "so", "ss", "su", "swa", "swe", "swi", "swo", "swu", "sya", "sye", "syi", "syo", "syu", "ta",
"te", "tha", "the", "thi", "tho", "thu", "ti", "to", "tsa", "tse", "tsi", "tso", "tsu", "tt", "tu", "twa",
"twe", "twi", "two", "twu", "tya", "tye", "tyi", "tyo", "tyu", "u", "va", "ve", "vi", "vo", "vu", "vv",
"vya", "vye", "vyi", "vyo", "vyu", "wa", "we", "wha", "whe", "whi", "who", "whu", "wi", "wo", "wu", "ww",
"xa", "xe", "xi", "xn", "xo", "xtu", "xu", "xwa", "xx", "xya", "xye", "xyi", "xyo", "xyu", "ya", "ye",
"yi", "yo", "yu", "yy", "za", "ze", "zi", "zo", "zu", "zya", "zye", "zyi", "zyo", "zyu", "zz", "{",
"|", "}", "~", "¥"
};
const char *RomkanPrivate::romkanTableValue[] = {
"っ", "！", "＂", "＃", "＄", "％", "＆", "＇", "（", "）", "＊", "＋", "、", "ー", "。", "・",
"０", "１", "２", "３", "４", "５", "６", "７", "８", "９", "：", "；", "＜", "＝", "＞", "？",
"＠", "「", "＼", "」", "＾", "＿", "｀", "あ", "ば", "っb", "べ", "び", "ぼ", "ぶ", "びゃ", "びぇ",
"びぃ", "びょ", "びゅ", "か", "っc", "せ", "ちゃ", "ちぇ", "ち", "ちょ", "ちゅ", "し", "こ", "く", "ちゃ", "ちぇ",
"ちぃ", "ちょ", "ちゅ", "だ", "っd", "で", "でゃ", "でぇ", "でぃ", "でょ", "でゅ", "ぢ", "ど", "づ", "どぁ", "どぇ",
"どぃ", "どぉ", "どぅ", "ぢゃ", "ぢぇ", "ぢぃ", "ぢょ", "ぢゅ", "え", "ふぁ", "ふぇ", "っf", "ふぃ", "ふぉ", "ふ", "ふぁ",
"ふぇ", "ふぃ", "ふぉ", "ふぅ", "ふゃ", "ふぇ", "ふぃ", "ふょ", "ふゅ", "が", "げ", "っg", "ぎ", "ご", "ぐ", "ぐぁ",
"ぐぇ", "ぐぃ", "ぐぉ", "ぐぅ", "ぎゃ", "ぎぇ", "ぎぃ", "ぎょ", "ぎゅ", "は", "へ", "っh", "ひ", "ほ", "ふ", "ひゃ",
"ひぇ", "ひぃ", "ひょ", "ひゅ", "い", "じゃ", "じぇ", "じ", "っj", "じょ", "じゅ", "じゃ", "じぇ", "じぃ", "じょ", "じゅ",
"か", "け", "き", "っk", "こ", "く", "くぁ", "きゃ", "きぇ", "きぃ", "きょ", "きゅ", "ぁ", "ぇ", "ぃ", "っl",
"ぉ", "っ", "っ", "ぅ", "ゎ", "ゃ", "ぇ", "ぃ", "ょ", "ゅ", "ま", "め", "み", "っm", "も", "む",
"みゃ", "みぇ", "みぃ", "みょ", "みゅ", "な", "んb", "んc", "んd", "ね", "んf", "んg", "んh", "に", "んj", "んk",
"んl", "んm", "ん", "の", "んp", "んq", "んr", "んs", "んt", "ぬ", "んv", "んw", "んx", "にゃ", "にぇ", "にぃ",
"にょ", "にゅ", "んz", "お", "ぱ", "ぺ", "ぴ", "ぽ", "っp", "ぷ", "ぴゃ", "ぴぇ", "ぴぃ", "ぴょ", "ぴゅ", "くぁ",
"くぇ", "くぃ", "くぉ", "っq", "く", "くぁ", "くぇ", "くぃ", "くぉ", "くぅ", "くゃ", "くぇ", "くぃ", "くょ", "くゅ", "ら",
"れ", "り", "ろ", "っr", "る", "りゃ", "りぇ", "りぃ", "りょ", "りゅ", "さ", "せ", "しゃ", "しぇ", "し", "しょ",
"しゅ", "し", "そ", "っs", "す", "すぁ", "すぇ", "すぃ", "すぉ", "すぅ", "しゃ", "しぇ", "しぃ", "しょ", "しゅ", "た",
"て", "てゃ", "てぇ", "てぃ", "てょ", "てゅ", "ち", "と", "つぁ", "つぇ", "つぃ", "つぉ", "つ", "っt", "つ", "とぁ",
"とぇ", "とぃ", "とぉ", "とぅ", "ちゃ", "ちぇ", "ちぃ", "ちょ", "ちゅ", "う", "ヴぁ", "ヴぇ", "ヴぃ", "ヴぉ", "ヴ", "っv",
"ヴゃ", "ヴぇ", "ヴぃ", "ヴょ", "ヴゅ", "わ", "うぇ", "うぁ", "うぇ", "うぃ", "うぉ", "う", "うぃ", "を", "う", "っw",
"ぁ", "ぇ", "ぃ", "ん", "ぉ", "っ", "ぅ", "ゎ", "っx", "ゃ", "ぇ", "ぃ", "ょ", "ゅ", "や", "いぇ",
"い", "よ", "ゆ", "っy", "ざ", "ぜ", "じ", "ぞ", "ず", "じゃ", "じぇ", "じぃ", "じょ", "じゅ", "っz", "｛",
"｜", "｝", "～", "￥"
};
const WnnLookupTable RomkanPrivate::romkanTable = WnnLookupTable(
            RomkanPrivate::romkanTableKey,
            RomkanPrivate::romkanTableValue,
            RomkanPrivate::romkanTableLength);
const int RomkanPrivate::MAX_LENGTH = 4;

Romkan::Romkan(QObject *parent) :
    LetterConverter(parent)
{
}

Romkan::~Romkan()
{
}

bool Romkan::convert(ComposingText &text) const
{
    return convertImpl(text, RomkanPrivate::romkanTable);
}

bool Romkan::convertImpl(ComposingText &text, const WnnLookupTable &table) const
{
    int cursor = text.getCursor(ComposingText::LAYER1);

    if (cursor <= 0) {
        return false;
    }

    StrSegment str[RomkanPrivate::MAX_LENGTH];
    int start = RomkanPrivate::MAX_LENGTH;
    int checkLength = qMin(cursor, RomkanPrivate::MAX_LENGTH);
    for (int i = 1; i <= checkLength; i++) {
        str[RomkanPrivate::MAX_LENGTH - i] = text.getStrSegment(ComposingText::LAYER1, cursor - i);
        start--;
    }

    while (start < RomkanPrivate::MAX_LENGTH) {
        QString key;
        for (int i = start; i < RomkanPrivate::MAX_LENGTH; i++) {
            key.append(str[i].string);
        }
        bool upper = key.at(key.length() - 1).isUpper();
        QString match = table[key.toLower()];
        if (!match.isEmpty()) {
            if (upper) {
                match = match.toUpper();
            }
            QList<StrSegment> out;
            if (match.length() == 1) {
                out.append(StrSegment(match, str[start].from, str[RomkanPrivate::MAX_LENGTH - 1].to));
                text.replaceStrSegment(ComposingText::LAYER1, out, RomkanPrivate::MAX_LENGTH - start);
            } else {
                out.append(StrSegment(match.left(match.length() - 1),
                                      str[start].from, str[RomkanPrivate::MAX_LENGTH - 1].to - 1));
                out.append(StrSegment(match.mid(match.length() - 1),
                                      str[RomkanPrivate::MAX_LENGTH - 1].to, str[RomkanPrivate::MAX_LENGTH - 1].to));
                text.replaceStrSegment(ComposingText::LAYER1, out, RomkanPrivate::MAX_LENGTH - start);
            }
            return true;
        }
        start++;
    }

    return false;
}
