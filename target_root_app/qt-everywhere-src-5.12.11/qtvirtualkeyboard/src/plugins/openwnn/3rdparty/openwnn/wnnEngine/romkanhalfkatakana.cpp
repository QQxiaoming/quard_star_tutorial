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

#include "romkanhalfkatakana.h"
#include "strsegment.h"
#include "wnnlookuptable.h"

class RomkanHalfKatakanaPrivate
{
public:
    static const int romkanTableLength = 317;
    static const char *romkanTableKey[romkanTableLength];
    static const char *romkanTableValue[romkanTableLength];
    static const WnnLookupTable romkanTable;
};

const char *RomkanHalfKatakanaPrivate::romkanTableKey[] = {
"", ",", "-", ".", "/", "a", "ba", "bb", "be", "bi", "bo", "bu", "bya", "bye", "byi", "byo",
"byu", "ca", "cc", "ce", "cha", "che", "chi", "cho", "chu", "ci", "co", "cu", "cya", "cye", "cyi", "cyo",
"cyu", "da", "dd", "de", "dha", "dhe", "dhi", "dho", "dhu", "di", "do", "du", "dwa", "dwe", "dwi", "dwo",
"dwu", "dya", "dye", "dyi", "dyo", "dyu", "e", "fa", "fe", "ff", "fi", "fo", "fu", "fwa", "fwe", "fwi",
"fwo", "fwu", "fya", "fye", "fyi", "fyo", "fyu", "ga", "ge", "gg", "gi", "go", "gu", "gwa", "gwe", "gwi",
"gwo", "gwu", "gya", "gye", "gyi", "gyo", "gyu", "ha", "he", "hh", "hi", "ho", "hu", "hya", "hye", "hyi",
"hyo", "hyu", "i", "ja", "je", "ji", "jj", "jo", "ju", "jya", "jye", "jyi", "jyo", "jyu", "ka", "ke",
"ki", "kk", "ko", "ku", "kwa", "kya", "kye", "kyi", "kyo", "kyu", "la", "le", "li", "ll", "lo", "ltsu",
"ltu", "lu", "lwa", "lya", "lye", "lyi", "lyo", "lyu", "ma", "me", "mi", "mm", "mo", "mu", "mya", "mye",
"myi", "myo", "myu", "na", "nb", "nc", "nd", "ne", "nf", "ng", "nh", "ni", "nj", "nk", "nl", "nm",
"nn", "no", "np", "nq", "nr", "ns", "nt", "nu", "nv", "nw", "nx", "nya", "nye", "nyi", "nyo", "nyu",
"nz", "o", "pa", "pe", "pi", "po", "pp", "pu", "pya", "pye", "pyi", "pyo", "pyu", "qa", "qe", "qi",
"qo", "qq", "qu", "qwa", "qwe", "qwi", "qwo", "qwu", "qya", "qye", "qyi", "qyo", "qyu", "ra", "re", "ri",
"ro", "rr", "ru", "rya", "rye", "ryi", "ryo", "ryu", "sa", "se", "sha", "she", "shi", "sho", "shu", "si",
"so", "ss", "su", "swa", "swe", "swi", "swo", "swu", "sya", "sye", "syi", "syo", "syu", "ta", "te", "tha",
"the", "thi", "tho", "thu", "ti", "to", "tsa", "tse", "tsi", "tso", "tsu", "tt", "tu", "twa", "twe", "twi",
"two", "twu", "tya", "tye", "tyi", "tyo", "tyu", "u", "va", "ve", "vi", "vo", "vu", "vv", "vya", "vye",
"vyi", "vyo", "vyu", "wa", "we", "wha", "whe", "whi", "who", "whu", "wi", "wo", "wu", "ww", "xa", "xe",
"xi", "xn", "xo", "xtu", "xu", "xwa", "xx", "xya", "xye", "xyi", "xyo", "xyu", "ya", "ye", "yi", "yo",
"yu", "yy", "za", "ze", "zi", "zo", "zu", "zya", "zye", "zyi", "zyo", "zyu", "zz"
};
const char *RomkanHalfKatakanaPrivate::romkanTableValue[] = {
"ｯ", "､", "ｰ", "｡", "･", "ｱ", "ﾊﾞ", "ｯb", "ﾍﾞ", "ﾋﾞ", "ﾎﾞ", "ﾌﾞ", "ﾋﾞｬ", "ﾋﾞｪ", "ﾋﾞｨ", "ﾋﾞｮ",
"ﾋﾞｭ", "ｶ", "ｯc", "ｾ", "ﾁｬ", "ﾁｪ", "ﾁ", "ﾁｮ", "ﾁｭ", "ｼ", "ｺ", "ｸ", "ﾁｬ", "ﾁｪ", "ﾁｨ", "ﾁｮ",
"ﾁｭ", "ﾀﾞ", "ｯd", "ﾃﾞ", "ﾃﾞｬ", "ﾃﾞｪ", "ﾃﾞｨ", "ﾃﾞｮ", "ﾃﾞｭ", "ﾁﾞ", "ﾄﾞ", "ﾂﾞ", "ﾄﾞｧ", "ﾄﾞｪ", "ﾄﾞｨ", "ﾄﾞｫ",
"ﾄﾞｩ", "ﾁﾞｬ", "ﾁﾞｪ", "ﾁﾞｨ", "ﾁﾞｮ", "ﾁﾞｭ", "ｴ", "ﾌｧ", "ﾌｪ", "ｯf", "ﾌｨ", "ﾌｫ", "ﾌ", "ﾌｧ", "ﾌｪ", "ﾌｨ",
"ﾌｫ", "ﾌｩ", "ﾌｬ", "ﾌｪ", "ﾌｨ", "ﾌｮ", "ﾌｭ", "ｶﾞ", "ｹﾞ", "ｯg", "ｷﾞ", "ｺﾞ", "ｸﾞ", "ｸﾞｧ", "ｸﾞｪ", "ｸﾞｨ",
"ｸﾞｫ", "ｸﾞｩ", "ｷﾞｬ", "ｷﾞｪ", "ｷﾞｨ", "ｷﾞｮ", "ｷﾞｭ", "ﾊ", "ﾍ", "ｯh", "ﾋ", "ﾎ", "ﾌ", "ﾋｬ", "ﾋｪ", "ﾋｨ",
"ﾋｮ", "ﾋｭ", "ｲ", "ｼﾞｬ", "ｼﾞｪ", "ｼﾞ", "ｯj", "ｼﾞｮ", "ｼﾞｭ", "ｼﾞｬ", "ｼﾞｪ", "ｼﾞｨ", "ｼﾞｮ", "ｼﾞｭ", "ｶ", "ｹ",
"ｷ", "ｯk", "ｺ", "ｸ", "ｸｧ", "ｷｬ", "ｷｪ", "ｷｨ", "ｷｮ", "ｷｭ", "ｧ", "ｪ", "ｨ", "ｯl", "ｫ", "ｯ",
"ｯ", "ｩ", "ﾜ", "ｬ", "ｪ", "ｨ", "ｮ", "ｭ", "ﾏ", "ﾒ", "ﾐ", "ｯm", "ﾓ", "ﾑ", "ﾐｬ", "ﾐｪ",
"ﾐｨ", "ﾐｮ", "ﾐｭ", "ﾅ", "ﾝb", "ﾝc", "ﾝd", "ﾈ", "ﾝf", "ﾝg", "ﾝh", "ﾆ", "ﾝj", "ﾝk", "ﾝl", "ﾝm",
"ﾝ", "ﾉ", "ﾝp", "ﾝq", "ﾝr", "ﾝs", "ﾝt", "ﾇ", "ﾝv", "ﾝw", "ﾝx", "ﾆｬ", "ﾆｪ", "ﾆｨ", "ﾆｮ", "ﾆｭ",
"ﾝz", "ｵ", "ﾊﾟ", "ﾍﾟ", "ﾋﾟ", "ﾎﾟ", "ｯp", "ﾌﾟ", "ﾋﾟｬ", "ﾋﾟｪ", "ﾋﾟｨ", "ﾋﾟｮ", "ﾋﾟｭ", "ｸｧ", "ｸｪ", "ｸｨ",
"ｸｫ", "ｯq", "ｸ", "ｸｧ", "ｸｪ", "ｸｨ", "ｸｫ", "ｸｩ", "ｸｬ", "ｸｪ", "ｸｨ", "ｸｮ", "ｸｭ", "ﾗ", "ﾚ", "ﾘ",
"ﾛ", "ｯr", "ﾙ", "ﾘｬ", "ﾘｪ", "ﾘｨ", "ﾘｮ", "ﾘｭ", "ｻ", "ｾ", "ｼｬ", "ｼｪ", "ｼ", "ｼｮ", "ｼｭ", "ｼ",
"ｿ", "ｯs", "ｽ", "ｽｧ", "ｽｪ", "ｽｨ", "ｽｫ", "ｽｩ", "ｼｬ", "ｼｪ", "ｼｨ", "ｼｮ", "ｼｭ", "ﾀ", "ﾃ", "ﾃｬ",
"ﾃｪ", "ﾃｨ", "ﾃｮ", "ﾃｭ", "ﾁ", "ﾄ", "ﾂｧ", "ﾂｪ", "ﾂｨ", "ﾂｫ", "ﾂ", "ｯt", "ﾂ", "ﾄｧ", "ﾄｪ", "ﾄｨ",
"ﾄｫ", "ﾄｩ", "ﾁｬ", "ﾁｪ", "ﾁｨ", "ﾁｮ", "ﾁｭ", "ｳ", "ｳﾞｧ", "ｳﾞｪ", "ｳﾞｨ", "ｳﾞｫ", "ｳﾞ", "ｯv", "ｳﾞｬ", "ｳﾞｪ",
"ｳﾞｨ", "ｳﾞｮ", "ｳﾞｭ", "ﾜ", "ｳｪ", "ｳｧ", "ｳｪ", "ｳｨ", "ｳｫ", "ｳ", "ｳｨ", "ｦ", "ｳ", "ｯw", "ｧ", "ｪ",
"ｨ", "ﾝ", "ｫ", "ｯ", "ｩ", "ﾜ", "ｯx", "ｬ", "ｪ", "ｨ", "ｮ", "ｭ", "ﾔ", "ｲｪ", "ｲ", "ﾖ",
"ﾕ", "ｯy", "ｻﾞ", "ｾﾞ", "ｼﾞ", "ｿﾞ", "ｽﾞ", "ｼﾞｬ", "ｼﾞｪ", "ｼﾞｨ", "ｼﾞｮ", "ｼﾞｭ", "ｯz"
};
const WnnLookupTable RomkanHalfKatakanaPrivate::romkanTable = WnnLookupTable(
            RomkanHalfKatakanaPrivate::romkanTableKey,
            RomkanHalfKatakanaPrivate::romkanTableValue,
            RomkanHalfKatakanaPrivate::romkanTableLength);

RomkanHalfKatakana::RomkanHalfKatakana(QObject *parent) :
    Romkan(parent)
{
}

RomkanHalfKatakana::~RomkanHalfKatakana()
{
}

bool RomkanHalfKatakana::convert(ComposingText &text) const
{
    return convertImpl(text, RomkanHalfKatakanaPrivate::romkanTable);
}
