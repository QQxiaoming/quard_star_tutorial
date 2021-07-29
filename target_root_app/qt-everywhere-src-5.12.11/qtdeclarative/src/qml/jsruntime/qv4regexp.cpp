/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qv4regexp_p.h"
#include "qv4engine_p.h"
#include "qv4scopedvalue_p.h"
#include <private/qv4mm_p.h>
#include <runtime/VM.h>

using namespace QV4;

static JSC::RegExpFlags jscFlags(uint flags)
{
    JSC::RegExpFlags jscFlags = JSC::NoFlags;
    if (flags & CompiledData::RegExp::RegExp_Global)
        jscFlags = static_cast<JSC::RegExpFlags>(flags | JSC::FlagGlobal);
    if (flags & CompiledData::RegExp::RegExp_IgnoreCase)
        jscFlags = static_cast<JSC::RegExpFlags>(flags | JSC::FlagIgnoreCase);
    if (flags & CompiledData::RegExp::RegExp_Multiline)
        jscFlags = static_cast<JSC::RegExpFlags>(flags | JSC::FlagMultiline);
    if (flags & CompiledData::RegExp::RegExp_Unicode)
        jscFlags = static_cast<JSC::RegExpFlags>(flags | JSC::FlagUnicode);
    if (flags & CompiledData::RegExp::RegExp_Sticky)
        jscFlags = static_cast<JSC::RegExpFlags>(flags | JSC::FlagSticky);
    return jscFlags;
}

RegExpCache::~RegExpCache()
{
    for (RegExpCache::Iterator it = begin(), e = end(); it != e; ++it) {
        if (RegExp *re = it.value().as<RegExp>())
            re->d()->cache = nullptr;
    }
}

DEFINE_MANAGED_VTABLE(RegExp);

uint RegExp::match(const QString &string, int start, uint *matchOffsets)
{
    static const uint offsetJITFail = std::numeric_limits<unsigned>::max() - 1;

    if (!isValid())
        return JSC::Yarr::offsetNoMatch;

    WTF::String s(string);

#if ENABLE(YARR_JIT)
    auto *priv = d();
    if (priv->hasValidJITCode()) {
        uint ret = JSC::Yarr::offsetNoMatch;
#if ENABLE(YARR_JIT_ALL_PARENS_EXPRESSIONS)
        char buffer[8192];
        ret = uint(priv->jitCode->execute(s.characters16(), start, s.length(),
                                          (int*)matchOffsets, buffer, 8192).start);
#else
        ret = uint(priv->jitCode->execute(s.characters16(), start, s.length(),
                                          (int*)matchOffsets).start);
#endif
        if (ret != offsetJITFail)
            return ret;

        // JIT failed. We need byteCode to run the interpreter.
        if (!priv->byteCode) {
            JSC::Yarr::ErrorCode error = JSC::Yarr::ErrorCode::NoError;
            JSC::Yarr::YarrPattern yarrPattern(WTF::String(*priv->pattern), jscFlags(priv->flags),
                                               error);

            // As we successfully parsed the pattern before, we should still be able to.
            Q_ASSERT(error == JSC::Yarr::ErrorCode::NoError);

            priv->byteCode = JSC::Yarr::byteCompile(
                                     yarrPattern,
                                     priv->internalClass->engine->bumperPointerAllocator).release();
        }
    }
#endif // ENABLE(YARR_JIT)

    return JSC::Yarr::interpret(byteCode(), s.characters16(), string.length(), start, matchOffsets);
}

QString RegExp::getSubstitution(const QString &matched, const QString &str, int position, const Value *captures, int nCaptures, const QString &replacement)
{
    QString result;

    int matchedLength = matched.length();
    Q_ASSERT(position >= 0 && position <= str.length());
    int tailPos = position + matchedLength;
    int seenDollar = -1;
    for (int i = 0; i < replacement.length(); ++i) {
        QChar ch = replacement.at(i);
        if (seenDollar >= 0) {
            if (ch.unicode() == '$') {
                result += QLatin1Char('$');
            } else if (ch.unicode() == '&') {
                result += matched;
            } else if (ch.unicode() == '`') {
                result += str.left(position);
            } else if (ch.unicode() == '\'') {
                result += str.mid(tailPos);
            } else if (ch.unicode() >= '0' && ch.unicode() <= '9') {
                int n = ch.unicode() - '0';
                if (i + 1 < replacement.length()) {
                    ch = replacement.at(i + 1);
                    if (ch.unicode() >= '0' && ch.unicode() <= '9') {
                        n = n*10 + (ch.unicode() - '0');
                        ++i;
                    }
                }
                if (n > 0 && n <= nCaptures) {
                    String *s = captures[n].stringValue();
                    if (s)
                        result += s->toQString();
                } else {
                    for (int j = seenDollar; j <= i; ++j)
                        result += replacement.at(j);
                }
            } else {
                result += QLatin1Char('$');
                result += ch;
            }
            seenDollar = -1;
        } else {
            if (ch == QLatin1Char('$')) {
                seenDollar = i;
                continue;
            }
            result += ch;
        }
    }
    if (seenDollar >= 0)
        result += QLatin1Char('$');
    return result;
}

QString Heap::RegExp::flagsAsString() const
{
    QString result;
    if (flags & CompiledData::RegExp::RegExp_Global)
        result += QLatin1Char('g');
    if (flags & CompiledData::RegExp::RegExp_IgnoreCase)
        result += QLatin1Char('i');
    if (flags & CompiledData::RegExp::RegExp_Multiline)
        result += QLatin1Char('m');
    if (flags & CompiledData::RegExp::RegExp_Unicode)
        result += QLatin1Char('u');
    if (flags & CompiledData::RegExp::RegExp_Sticky)
        result += QLatin1Char('y');
    return result;
}

Heap::RegExp *RegExp::create(ExecutionEngine* engine, const QString& pattern, uint flags)
{
    RegExpCacheKey key(pattern, flags);

    RegExpCache *cache = engine->regExpCache;
    if (!cache)
        cache = engine->regExpCache = new RegExpCache;

    QV4::WeakValue &cachedValue = (*cache)[key];
    if (QV4::RegExp *result = cachedValue.as<RegExp>())
        return result->d();

    Scope scope(engine);
    Scoped<RegExp> result(scope, engine->memoryManager->alloc<RegExp>(engine, pattern, flags));

    result->d()->cache = cache;
    cachedValue.set(engine, result);

    return result->d();
}

void Heap::RegExp::init(ExecutionEngine *engine, const QString &pattern, uint flags)
{
    Base::init();
    this->pattern = new QString(pattern);
    this->flags = flags;

    valid = false;

    JSC::Yarr::ErrorCode error = JSC::Yarr::ErrorCode::NoError;
    JSC::Yarr::YarrPattern yarrPattern(WTF::String(pattern), jscFlags(flags), error);
    if (error != JSC::Yarr::ErrorCode::NoError)
        return;
    subPatternCount = yarrPattern.m_numSubpatterns;
#if ENABLE(YARR_JIT)
    if (!yarrPattern.m_containsBackreferences && engine->canJIT()) {
        jitCode = new JSC::Yarr::YarrCodeBlock;
        JSC::VM *vm = static_cast<JSC::VM *>(engine);
        JSC::Yarr::jitCompile(yarrPattern, JSC::Yarr::Char16, vm, *jitCode);
    }
#else
    Q_UNUSED(engine)
#endif
    if (hasValidJITCode()) {
        valid = true;
        return;
    }
    byteCode = JSC::Yarr::byteCompile(yarrPattern, internalClass->engine->bumperPointerAllocator).release();
    if (byteCode)
        valid = true;
}

void Heap::RegExp::destroy()
{
    if (cache) {
        RegExpCacheKey key(this);
        cache->remove(key);
    }
#if ENABLE(YARR_JIT)
    delete jitCode;
#endif
    delete byteCode;
    delete pattern;
    Base::destroy();
}
