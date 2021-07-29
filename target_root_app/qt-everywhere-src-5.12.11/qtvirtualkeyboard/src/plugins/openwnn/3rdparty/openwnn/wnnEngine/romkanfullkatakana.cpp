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

#include "romkanfullkatakana.h"
#include "strsegment.h"
#include "wnnlookuptable.h"

class RomkanFullKatakanaPrivate
{
public:
    static const int romkanTableLength = 322;
    static const char *romkanTableKey[romkanTableLength];
    static const char *romkanTableValue[romkanTableLength];
    static const WnnLookupTable romkanTable;
};

const char *RomkanFullKatakanaPrivate::romkanTableKey[] = {
"", ",", "-", ".", "/", "?", "a", "ba", "bb", "be", "bi", "bo", "bu", "bya", "bye", "byi",
"byo", "byu", "ca", "cc", "ce", "cha", "che", "chi", "cho", "chu", "ci", "co", "cu", "cya", "cye", "cyi",
"cyo", "cyu", "da", "dd", "de", "dha", "dhe", "dhi", "dho", "dhu", "di", "do", "du", "dwa", "dwe", "dwi",
"dwo", "dwu", "dya", "dye", "dyi", "dyo", "dyu", "e", "fa", "fe", "ff", "fi", "fo", "fu", "fwa", "fwe",
"fwi", "fwo", "fwu", "fya", "fye", "fyi", "fyo", "fyu", "ga", "ge", "gg", "gi", "go", "gu", "gwa", "gwe",
"gwi", "gwo", "gwu", "gya", "gye", "gyi", "gyo", "gyu", "ha", "he", "hh", "hi", "ho", "hu", "hya", "hye",
"hyi", "hyo", "hyu", "i", "ja", "je", "ji", "jj", "jo", "ju", "jya", "jye", "jyi", "jyo", "jyu", "ka",
"ke", "ki", "kk", "ko", "ku", "kwa", "kya", "kye", "kyi", "kyo", "kyu", "la", "le", "li", "lka", "lke",
"ll", "lo", "ltsu", "ltu", "lu", "lwa", "lya", "lye", "lyi", "lyo", "lyu", "ma", "me", "mi", "mm", "mo",
"mu", "mya", "mye", "myi", "myo", "myu", "na", "nb", "nc", "nd", "ne", "nf", "ng", "nh", "ni", "nj",
"nk", "nl", "nm", "nn", "no", "np", "nq", "nr", "ns", "nt", "nu", "nv", "nw", "nx", "nya", "nye",
"nyi", "nyo", "nyu", "nz", "o", "pa", "pe", "pi", "po", "pp", "pu", "pya", "pye", "pyi", "pyo", "pyu",
"qa", "qe", "qi", "qo", "qq", "qu", "qwa", "qwe", "qwi", "qwo", "qwu", "qya", "qye", "qyi", "qyo", "qyu",
"ra", "re", "ri", "ro", "rr", "ru", "rya", "rye", "ryi", "ryo", "ryu", "sa", "se", "sha", "she", "shi",
"sho", "shu", "si", "so", "ss", "su", "swa", "swe", "swi", "swo", "swu", "sya", "sye", "syi", "syo", "syu",
"ta", "te", "tha", "the", "thi", "tho", "thu", "ti", "to", "tsa", "tse", "tsi", "tso", "tsu", "tt", "tu",
"twa", "twe", "twi", "two", "twu", "tya", "tye", "tyi", "tyo", "tyu", "u", "va", "ve", "vi", "vo", "vu",
"vv", "vya", "vye", "vyi", "vyo", "vyu", "wa", "we", "wha", "whe", "whi", "who", "whu", "wi", "wo", "wu",
"ww", "xa", "xe", "xi", "xka", "xke", "xn", "xo", "xtu", "xu", "xwa", "xx", "xya", "xye", "xyi", "xyo",
"xyu", "ya", "ye", "yi", "yo", "yu", "yy", "za", "ze", "zi", "zo", "zu", "zya", "zye", "zyi", "zyo",
"zyu", "zz"
};
const char *RomkanFullKatakanaPrivate::romkanTableValue[] = {
"ッ", "、", "ー", "。", "・", "？", "ア", "バ", "ッb", "ベ", "ビ", "ボ", "ブ", "ビャ", "ビェ", "ビィ",
"ビョ", "ビュ", "カ", "ッc", "セ", "チャ", "チェ", "チ", "チョ", "チュ", "シ", "コ", "ク", "チャ", "チェ", "チィ",
"チョ", "チュ", "ダ", "ッd", "デ", "デャ", "デェ", "ディ", "デョ", "デュ", "ヂ", "ド", "ヅ", "ドァ", "ドェ", "ドィ",
"ドォ", "ドゥ", "ヂャ", "ヂェ", "ヂィ", "ヂョ", "ヂュ", "エ", "ファ", "フェ", "ッf", "フィ", "フォ", "フ", "ファ", "フェ",
"フィ", "フォ", "フゥ", "フャ", "フェ", "フィ", "フョ", "フュ", "ガ", "ゲ", "ッg", "ギ", "ゴ", "グ", "グァ", "グェ",
"グィ", "グォ", "グゥ", "ギャ", "ギェ", "ギィ", "ギョ", "ギュ", "ハ", "ヘ", "ッh", "ヒ", "ホ", "フ", "ヒャ", "ヒェ",
"ヒィ", "ヒョ", "ヒュ", "イ", "ジャ", "ジェ", "ジ", "ッj", "ジョ", "ジュ", "ジャ", "ジェ", "ジィ", "ジョ", "ジュ", "カ",
"ケ", "キ", "ッk", "コ", "ク", "クァ", "キャ", "キェ", "キィ", "キョ", "キュ", "ァ", "ェ", "ィ", "ヵ", "ヶ",
"ッl", "ォ", "ッ", "ッ", "ゥ", "ヮ", "ャ", "ェ", "ィ", "ョ", "ュ", "マ", "メ", "ミ", "ッm", "モ",
"ム", "ミャ", "ミェ", "ミィ", "ミョ", "ミュ", "ナ", "ンb", "ンc", "ンd", "ネ", "ンf", "ンg", "ンh", "ニ", "ンj",
"ンk", "ンl", "ンm", "ン", "ノ", "ンp", "ンq", "ンr", "ンs", "ンt", "ヌ", "ンv", "ンw", "ンx", "ニャ", "ニェ",
"ニィ", "ニョ", "ニュ", "ンz", "オ", "パ", "ペ", "ピ", "ポ", "ッp", "プ", "ピャ", "ピェ", "ピィ", "ピョ", "ピュ",
"クァ", "クェ", "クィ", "クォ", "ッq", "ク", "クァ", "クェ", "クィ", "クォ", "クゥ", "クャ", "クェ", "クィ", "クョ", "クュ",
"ラ", "レ", "リ", "ロ", "ッr", "ル", "リャ", "リェ", "リィ", "リョ", "リュ", "サ", "セ", "シャ", "シェ", "シ",
"ショ", "シュ", "シ", "ソ", "ッs", "ス", "スァ", "スェ", "スィ", "スォ", "スゥ", "シャ", "シェ", "シィ", "ショ", "シュ",
"タ", "テ", "テャ", "テェ", "ティ", "テョ", "テュ", "チ", "ト", "ツァ", "ツェ", "ツィ", "ツォ", "ツ", "ッt", "ツ",
"トァ", "トェ", "トィ", "トォ", "トゥ", "チャ", "チェ", "チィ", "チョ", "チュ", "ウ", "ヴァ", "ヴェ", "ヴィ", "ヴォ", "ヴ",
"ッv", "ヴャ", "ヴェ", "ヴィ", "ヴョ", "ヴュ", "ワ", "ウェ", "ウァ", "ウェ", "ウィ", "ウォ", "ウ", "ウィ", "ヲ", "ウ",
"ッw", "ァ", "ェ", "ィ", "ヵ", "ヶ", "ン", "ォ", "ッ", "ゥ", "ヮ", "ッx", "ャ", "ェ", "ィ", "ョ",
"ュ", "ヤ", "イェ", "イ", "ヨ", "ユ", "ッy", "ザ", "ゼ", "ジ", "ゾ", "ズ", "ジャ", "ジェ", "ジィ", "ジョ",
"ジュ", "ッz"
};
const WnnLookupTable RomkanFullKatakanaPrivate::romkanTable = WnnLookupTable(
            RomkanFullKatakanaPrivate::romkanTableKey,
            RomkanFullKatakanaPrivate::romkanTableValue,
            RomkanFullKatakanaPrivate::romkanTableLength);

RomkanFullKatakana::RomkanFullKatakana(QObject *parent) :
    Romkan(parent)
{
}

RomkanFullKatakana::~RomkanFullKatakana()
{
}

bool RomkanFullKatakana::convert(ComposingText &text) const
{
    return convertImpl(text, RomkanFullKatakanaPrivate::romkanTable);
}
