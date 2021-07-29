/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Virtual Keyboard module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "t9writeinputmethod_p.h"
#include <QtVirtualKeyboard/qvirtualkeyboardinputengine.h>
#include <QtVirtualKeyboard/qvirtualkeyboardinputcontext.h>
#include <QtVirtualKeyboard/qvirtualkeyboardtrace.h>
#include "t9writeworker_p.h"
#include <QLoggingCategory>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QTime>
#include <QMetaEnum>
#include <QtVirtualKeyboard/private/handwritinggesturerecognizer_p.h>
#ifdef QT_VIRTUALKEYBOARD_RECORD_TRACE_INPUT
#include <QtVirtualKeyboard/private/unipentrace_p.h>
#include <QStandardPaths>
#endif

#include "decumaStatus.h"
#include "decumaSymbolCategories.h"
#include "decumaLanguages.h"
#include "xxt9wOem.h"

/*  Set to 1 to enable T9 Write log.

    The log is routed to qDebug() and it can be enabled for troubleshooting
    and when reporting issues. The log must not to be enabled in production
    build.
*/
#define QT_VIRTUALKEYBOARD_T9WRITE_LOG 0

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

Q_LOGGING_CATEGORY(lcT9Write, "qt.virtualkeyboard.t9write")

class T9WriteCaseFormatter
{
public:
    T9WriteCaseFormatter() :
        preferLowercase(false)
    {
    }

    void clear()
    {
        textCaseList.clear();
    }

    void ensureLength(int length, QVirtualKeyboardInputEngine::TextCase textCase)
    {
        if (length <= 0) {
            textCaseList.clear();
            return;
        }
        while (length < textCaseList.length())
            textCaseList.removeLast();
        while (length > textCaseList.length())
            textCaseList.append(textCase);
    }

    QString formatString(const QString &str) const
    {
        QString result;
        QVirtualKeyboardInputEngine::TextCase textCase = QVirtualKeyboardInputEngine::TextCase::Lower;
        for (int i = 0; i < str.length(); ++i) {
            if (i < textCaseList.length())
                textCase = textCaseList.at(i);
            result.append(textCase == QVirtualKeyboardInputEngine::TextCase::Upper ? str.at(i).toUpper() : (preferLowercase ? str.at(i).toLower() : str.at(i)));
        }
        return result;
    }

    bool preferLowercase;

private:
    QList<QVirtualKeyboardInputEngine::TextCase> textCaseList;
};

class T9WriteInputMethodPrivate
{
    Q_DECLARE_PUBLIC(T9WriteInputMethod)
public:
    T9WriteInputMethodPrivate(T9WriteInputMethod *q_ptr) :
        q_ptr(q_ptr),
        cjk(false),
        engineMode(T9WriteInputMethod::EngineMode::Uninitialized),
        defaultHwrDbPath(QLatin1String(":/QtQuick/VirtualKeyboard/T9Write/data/")),
        defaultDictionaryDbPath(defaultHwrDbPath),
        traceListHardLimit(32),
        dictionaryLock(QMutex::Recursive),
        attachedDictionary(nullptr),
        resultId(0),
        lastResultId(0),
        resultTimer(0),
        decumaSession(nullptr),
        activeWordIndex(-1),
        arcAdditionStarted(false),
        ignoreUpdate(false),
        textCase(QVirtualKeyboardInputEngine::TextCase::Lower)
#ifdef QT_VIRTUALKEYBOARD_RECORD_TRACE_INPUT
        , unipenTrace()
#endif
    {
    }

    static void *decumaMalloc(size_t size, void *pPrivate)
    {
        Q_UNUSED(pPrivate)
        return malloc(size);
    }

    static void *decumaCalloc(size_t elements, size_t size, void *pPrivate)
    {
        Q_UNUSED(pPrivate)
        return calloc(elements, size);
    }

    static void decumaFree(void *ptr, void *pPrivate)
    {
        Q_UNUSED(pPrivate)
        free(ptr);
    }

#if QT_VIRTUALKEYBOARD_T9WRITE_LOG
    static void decumaLogString(void *pUserData, const char *pLogString, DECUMA_UINT32 nLogStringLength)
    {
        static QMutex s_logMutex;
        static QByteArray s_logString;
        Q_UNUSED(pUserData)
        QMutexLocker guard(&s_logMutex);
        s_logString.append(pLogString, nLogStringLength);
        if (s_logString.endsWith('\n')) {
            while (s_logString.endsWith('\n'))
                s_logString.chop(1);
            qDebug() << (const char *)s_logString.constData();
            s_logString.clear();
        }
    }
#endif

    static const char *engineModeToString(T9WriteInputMethod::EngineMode mode)
    {
        return QMetaEnum::fromType<T9WriteInputMethod::EngineMode>().key(static_cast<int>(mode));
    }

    bool initEngine(T9WriteInputMethod::EngineMode newEngineMode)
    {
        if (engineMode == newEngineMode)
            return engineMode != T9WriteInputMethod::EngineMode::Uninitialized;

        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::initEngine()" << engineModeToString(newEngineMode);

        if (decumaSession)
            exitEngine();

        if (newEngineMode == T9WriteInputMethod::EngineMode::Uninitialized)
            return false;

        switch (newEngineMode) {
        case T9WriteInputMethod::EngineMode::Alphabetic:
        case T9WriteInputMethod::EngineMode::Arabic:
        case T9WriteInputMethod::EngineMode::Hebrew:
        case T9WriteInputMethod::EngineMode::Thai:
            cjk = false;
            break;
        case T9WriteInputMethod::EngineMode::SimplifiedChinese:
        case T9WriteInputMethod::EngineMode::TraditionalChinese:
        case T9WriteInputMethod::EngineMode::HongKongChinese:
        case T9WriteInputMethod::EngineMode::Japanese:
        case T9WriteInputMethod::EngineMode::Korean:
            cjk = true;
            break;
        default:
            Q_ASSERT(0 && "Invalid T9WriteInputMethod::EngineMode!");
            return false;
        }
        engineMode = newEngineMode;

        memset(&sessionSettings, 0, sizeof(sessionSettings));

        QString hwrDb = findHwrDb(engineMode, defaultHwrDbPath);
        hwrDbFile.setFileName(hwrDb);
        if (!hwrDbFile.open(QIODevice::ReadOnly)) {
            qCCritical(lcT9Write) << "Could not open HWR database" << hwrDb;
            exitEngine();
            return false;
        }

        sessionSettings.pStaticDB = (DECUMA_STATIC_DB_PTR)hwrDbFile.map(0, hwrDbFile.size(), QFile::NoOptions);
        if (!sessionSettings.pStaticDB) {
            qCCritical(lcT9Write) << "Could not read HWR database" << hwrDb;
            exitEngine();
            return false;
        }

        symbolCategories.append(DECUMA_CATEGORY_ANSI);
        languageCategories.append(DECUMA_LANG_EN);

        sessionSettings.recognitionMode = mcrMode;
        sessionSettings.writingDirection = unknownWriting;
        sessionSettings.charSet.pSymbolCategories = symbolCategories.data();
        sessionSettings.charSet.nSymbolCategories = symbolCategories.size();
        sessionSettings.charSet.pLanguages = languageCategories.data();
        sessionSettings.charSet.nLanguages = languageCategories.size();

        session = QByteArray(DECUMA_API(GetSessionSize)(), 0);
        decumaSession = (DECUMA_SESSION *)(!session.isEmpty() ? session.data() : nullptr);

        DECUMA_STATUS status = DECUMA_API(BeginSession)(decumaSession, &sessionSettings, &memFuncs);
        Q_ASSERT(status == decumaNoError);
        if (status != decumaNoError) {
            qCCritical(lcT9Write) << "Could not initialize engine" << status;
            exitEngine();
            return false;
        }

#if QT_VIRTUALKEYBOARD_T9WRITE_LOG
        DECUMA_API(StartLogging)(decumaSession, 0, decumaLogString);
#endif

        worker.reset(new T9WriteWorker(decumaSession, cjk));
        worker->start();

        Q_Q(T9WriteInputMethod);
        processResultConnection = QObject::connect(q, &T9WriteInputMethod::resultListChanged, q, &T9WriteInputMethod::processResult, Qt::QueuedConnection);

        return true;
    }

    void exitEngine()
    {
        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::exitEngine()";

        if (processResultConnection)
            QObject::disconnect(processResultConnection);

        worker.reset();

        if (sessionSettings.pStaticDB) {
            hwrDbFile.unmap((uchar *)sessionSettings.pStaticDB);
            hwrDbFile.close();
        }

        if (attachedDictionary) {
            detachDictionary(attachedDictionary);
            attachedDictionary.reset();
        }
        loadedDictionary.reset();

        if (decumaSession) {
#if QT_VIRTUALKEYBOARD_T9WRITE_LOG
            DECUMA_API(StopLogging)(decumaSession);
#endif
            DECUMA_API(EndSession)(decumaSession);
            decumaSession = nullptr;
            session.clear();
        }

        memset(&sessionSettings, 0, sizeof(sessionSettings));

        symbolCategories.clear();
        languageCategories.clear();

        engineMode = T9WriteInputMethod::EngineMode::Uninitialized;
        cjk = false;
    }

    QString findHwrDb(T9WriteInputMethod::EngineMode mode, const QString &dir) const
    {
        QString hwrDbPath(dir);
        switch (mode) {
        case T9WriteInputMethod::EngineMode::Alphabetic:
#if T9WRITEAPIMAJORVERNUM >= 21
            hwrDbPath.append(QLatin1String("hwrDB_le.bin"));
#else
            hwrDbPath.append(QLatin1String("_databas_le.bin"));
#endif
            break;
        case T9WriteInputMethod::EngineMode::Arabic:
#if T9WRITEAPIMAJORVERNUM >= 21
            hwrDbPath.append(QLatin1String("arabic/hwrDB_le.bin"));
#else
            hwrDbPath.append(QLatin1String("arabic/_databas_le.bin"));
#endif
            break;
        case T9WriteInputMethod::EngineMode::Hebrew:
#if T9WRITEAPIMAJORVERNUM >= 21
            hwrDbPath.append(QLatin1String("hebrew/hwrDB_le.bin"));
#else
            hwrDbPath.append(QLatin1String("hebrew/_databas_le.bin"));
#endif
            break;
        case T9WriteInputMethod::EngineMode::Thai:
#if T9WRITEAPIMAJORVERNUM >= 21
            hwrDbPath.append(QLatin1String("thai/hwrDB_le.bin"));
#else
            hwrDbPath.append(QLatin1String("thai/_databas_le.bin"));
#endif
            break;
        case T9WriteInputMethod::EngineMode::SimplifiedChinese:
            hwrDbPath.append(QLatin1String("cjk_S_gb18030_le.hdb"));
            break;
        case T9WriteInputMethod::EngineMode::TraditionalChinese:
            hwrDbPath.append(QLatin1String("cjk_T_std_le.hdb"));
            break;
        case T9WriteInputMethod::EngineMode::HongKongChinese:
            hwrDbPath.append(QLatin1String("cjk_HK_std_le.hdb"));
            break;
        case T9WriteInputMethod::EngineMode::Japanese:
            hwrDbPath.append(QLatin1String("cjk_J_std_le.hdb"));
            break;
        case T9WriteInputMethod::EngineMode::Korean:
            hwrDbPath.append(QLatin1String("cjk_K_mkt_le.hdb"));
            break;
        default:
            return QString();
        }
        if (!QFileInfo::exists(hwrDbPath)) {
            qCCritical(lcT9Write) << "Could not find HWR database for" << engineModeToString(mode);
            return QString();
        }
        return hwrDbPath;
    }

    QString findDictionary(const QString &dir, const QLocale &locale, DECUMA_SRC_DICTIONARY_TYPE &srcType)
    {
        srcType = numberOfSrcDictionaryTypes;

        QStringList languageCountry = locale.name().split(QLatin1String("_"));
        if (languageCountry.length() != 2)
            return QString();
        const QString language = languageCountry[0].toUpper();

        QString dictionary;
        QDirIterator it(dir, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            QString fileEntry = it.next();
            const QFileInfo fileInfo(fileEntry);

            if (fileInfo.isDir())
                continue;

            const QString fileName(fileInfo.fileName());
            if (!fileName.startsWith(language) &&
                    !fileName.startsWith(QLatin1String("zzEval_") + language))
                continue;

            if (fileEntry.endsWith(QLatin1String(".ldb"))) {
#if T9WRITEAPIMAJORVERNUM >= 20
                qCCritical(lcT9Write) << "Incompatible dictionary" << fileEntry;
                continue;
#else
                srcType = decumaXT9LDB;
#endif
            } else if (fileEntry.endsWith(QLatin1String(".phd"))) {
#if T9WRITEAPIMAJORVERNUM >= 20
                srcType = decumaPortableHWRDictionary;
#else
                qCCritical(lcT9Write) << "Incompatible dictionary" << fileEntry;
                continue;
#endif
            } else {
                qCCritical(lcT9Write) << "Incompatible dictionary" << fileEntry;
                continue;
            }

            dictionary = fileEntry;
            break;
        }

        return dictionary;
    }

    bool attachDictionary(const QSharedPointer<T9WriteDictionary> &dictionary)
    {
        QMutexLocker dictionaryGuard(&dictionaryLock);
        Q_ASSERT(decumaSession != nullptr);
        Q_ASSERT(dictionary != nullptr);
        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::attachDictionary():" << dictionary->fileName();
#if T9WRITEAPIMAJORVERNUM >= 20
        DECUMA_STATUS status = DECUMA_API(AttachDictionary)(decumaSession, dictionary->data(), dictionary->size());
#else
        DECUMA_STATUS status = DECUMA_API(AttachConvertedDictionary)(decumaSession, dictionary->data());
#endif
        return status == decumaNoError;
    }

    void detachDictionary(const QSharedPointer<T9WriteDictionary> &dictionary)
    {
        QMutexLocker dictionaryGuard(&dictionaryLock);
        if (!dictionary)
            return;

        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::detachDictionary():" << dictionary->fileName();

        Q_ASSERT(decumaSession != nullptr);
        DECUMA_STATUS status = DECUMA_API(DetachDictionary)(decumaSession, dictionary->data());
        Q_UNUSED(status)
        Q_ASSERT(status == decumaNoError);
    }

    bool setInputMode(const QLocale &locale, QVirtualKeyboardInputEngine::InputMode inputMode)
    {
        Q_Q(T9WriteInputMethod);
        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::setInputMode():" << locale << inputMode;

        finishRecognition();

        DECUMA_UINT32 language = mapToDecumaLanguage(locale, inputMode);
        if (language == DECUMA_LANG_GSMDEFAULT) {
            qCCritical(lcT9Write) << "Language is not supported" << locale.name();
            return false;
        }

        if (!initEngine(mapLocaleToEngineMode(locale, language)))
            return false;

        int isLanguageSupported = 0;
        DECUMA_API(DatabaseIsLanguageSupported)(sessionSettings.pStaticDB, language, &isLanguageSupported);
        if (!isLanguageSupported) {
            qCCritical(lcT9Write) << "Language is not supported" << locale.name();
            return false;
        }

        bool languageChanged = languageCategories.isEmpty() || languageCategories.first() != language;
        languageCategories.clear();
        languageCategories.append(language);

        // Add English as secondary language for non-latin languages.
        // T9 Write requires it for punctuation and latin symbols if
        // included in the symbol categories.
        if (locale.script() != QLocale::LatinScript)
            languageCategories.append(DECUMA_LANG_EN);

        if (!updateSymbolCategories(language, locale, inputMode))
            return false;
        updateRecognitionMode(language, locale, inputMode);
        updateDictionary(language, locale, languageChanged);
        static const QList<DECUMA_UINT32> rtlLanguages = QList<DECUMA_UINT32>()
                << DECUMA_LANG_AR << DECUMA_LANG_IW << DECUMA_LANG_FA << DECUMA_LANG_UR;
        sessionSettings.writingDirection = rtlLanguages.contains(language) ? rightToLeft : leftToRight;

        // Enable multi-threaded recognition if available.
#ifdef DECUMA_USE_MULTI_THREAD
        // Note: This feature requires T9 Write v8.0.0 or later,
        //       and feature enabled in the SDK.
        sessionSettings.nMaxThreads = qMax(QThread::idealThreadCount(), 0);
#endif

        qCDebug(lcT9Write) << " -> language categories:" << languageCategories;
        qCDebug(lcT9Write) << " -> symbol categories:" << symbolCategories;
        qCDebug(lcT9Write) << " -> recognition mode:" << sessionSettings.recognitionMode;

        // Change session settings
        sessionSettings.charSet.pSymbolCategories = symbolCategories.data();
        sessionSettings.charSet.nSymbolCategories = symbolCategories.size();
        sessionSettings.charSet.pLanguages = languageCategories.data();
        sessionSettings.charSet.nLanguages = languageCategories.size();
        DECUMA_STATUS status = DECUMA_API(ChangeSessionSettings)(decumaSession, &sessionSettings);
        Q_ASSERT(status == decumaNoError);

        caseFormatter.preferLowercase = q->inputContext()->inputMethodHints().testFlag(Qt::ImhPreferLowercase);

        return status == decumaNoError;
    }

    T9WriteInputMethod::EngineMode mapLocaleToEngineMode(const QLocale &locale, DECUMA_UINT32 language = 0)
    {
#ifdef HAVE_T9WRITE_CJK
        switch (locale.language()) {
        case QLocale::Chinese: {
            if (locale.script() == QLocale::TraditionalChineseScript)
                return locale.country() == QLocale::HongKong ? T9WriteInputMethod::EngineMode::HongKongChinese : T9WriteInputMethod::EngineMode::TraditionalChinese;
            return T9WriteInputMethod::EngineMode::SimplifiedChinese;
            break;
        }
        case QLocale::Japanese:
            return T9WriteInputMethod::EngineMode::Japanese;
            break;
        case QLocale::Korean:
            return T9WriteInputMethod::EngineMode::Korean;
        default:
            break;
        }
#else
        Q_UNUSED(locale)
        Q_UNUSED(language)
#endif

#ifdef HAVE_T9WRITE_ALPHABETIC
        switch (locale.script()) {
        case QLocale::ArabicScript:
            return T9WriteInputMethod::EngineMode::Arabic;
        case QLocale::HebrewScript:
            return T9WriteInputMethod::EngineMode::Hebrew;
        case QLocale::ThaiScript:
            return language == DECUMA_LANG_EN ? T9WriteInputMethod::EngineMode::Alphabetic
                                              : T9WriteInputMethod::EngineMode::Thai;
        default:
            return T9WriteInputMethod::EngineMode::Alphabetic;
        }
#else
        return T9WriteInputMethod::EngineMode::Uninitialized;
#endif
    }

    DECUMA_UINT32 mapToDecumaLanguage(const QLocale &locale, QVirtualKeyboardInputEngine::InputMode inputMode)
    {
        static const QLocale::Language maxLanguage = QLocale::Vietnamese;
        static const DECUMA_UINT32 languageMap[maxLanguage + 1] = {
            DECUMA_LANG_GSMDEFAULT,         // AnyLanguage = 0
            DECUMA_LANG_GSMDEFAULT,         // C = 1
            DECUMA_LANG_GSMDEFAULT,         // Abkhazian = 2
            DECUMA_LANG_GSMDEFAULT,         // Oromo = 3
            DECUMA_LANG_GSMDEFAULT,         // Afar = 4
            DECUMA_LANG_AF,                 // Afrikaans = 5
            DECUMA_LANG_SQ,                 // Albanian = 6
            DECUMA_LANG_GSMDEFAULT,         // Amharic = 7
            DECUMA_LANG_AR,                 // Arabic = 8
            DECUMA_LANG_GSMDEFAULT,         // Armenian = 9
            DECUMA_LANG_GSMDEFAULT,         // Assamese = 10
            DECUMA_LANG_GSMDEFAULT,         // Aymara = 11
            DECUMA_LANG_AZ,                 // Azerbaijani = 12
            DECUMA_LANG_GSMDEFAULT,         // Bashkir = 13
            DECUMA_LANG_EU,                 // Basque = 14
            DECUMA_LANG_BN,                 // Bengali = 15
            DECUMA_LANG_GSMDEFAULT,         // Dzongkha = 16
            DECUMA_LANG_GSMDEFAULT,         // Bihari = 17
            DECUMA_LANG_GSMDEFAULT,         // Bislama = 18
            DECUMA_LANG_GSMDEFAULT,         // Breton = 19
            DECUMA_LANG_BG,                 // Bulgarian = 20
            DECUMA_LANG_GSMDEFAULT,         // Burmese = 21
            DECUMA_LANG_BE,                 // Belarusian = 22
            DECUMA_LANG_KM,                 // Khmer = 23
            DECUMA_LANG_CA,                 // Catalan = 24
            DECUMA_LANG_PRC,                // Chinese = 25
            DECUMA_LANG_GSMDEFAULT,         // Corsican = 26
            DECUMA_LANG_HR,                 // Croatian = 27
            DECUMA_LANG_CS,                 // Czech = 28
            DECUMA_LANG_DA,                 // Danish = 29
            DECUMA_LANG_NL,                 // Dutch = 30
            DECUMA_LANG_EN,                 // English = 31
            DECUMA_LANG_GSMDEFAULT,         // Esperanto = 32
            DECUMA_LANG_ET,                 // Estonian = 33
            DECUMA_LANG_GSMDEFAULT,         // Faroese = 34
            DECUMA_LANG_GSMDEFAULT,         // Fijian = 35
            DECUMA_LANG_FI,                 // Finnish = 36
            DECUMA_LANG_FR,                 // French = 37
            DECUMA_LANG_GSMDEFAULT,         // WesternFrisian = 38
            DECUMA_LANG_GSMDEFAULT,         // Gaelic = 39
            DECUMA_LANG_GL,                 // Galician = 40
            DECUMA_LANG_GSMDEFAULT,         // Georgian = 41
            DECUMA_LANG_DE,                 // German = 42
            DECUMA_LANG_EL,                 // Greek = 43
            DECUMA_LANG_GSMDEFAULT,         // Greenlandic = 44
            DECUMA_LANG_GSMDEFAULT,         // Guarani = 45
            DECUMA_LANG_GU,                 // Gujarati = 46
            DECUMA_LANG_HA,                 // Hausa = 47
            DECUMA_LANG_IW,                 // Hebrew = 48
            DECUMA_LANG_HI,                 // Hindi = 49
            DECUMA_LANG_HU,                 // Hungarian = 50
            DECUMA_LANG_IS,                 // Icelandic = 51
            DECUMA_LANG_IN,                 // Indonesian = 52
            DECUMA_LANG_GSMDEFAULT,         // Interlingua = 53
            DECUMA_LANG_GSMDEFAULT,         // Interlingue = 54
            DECUMA_LANG_GSMDEFAULT,         // Inuktitut = 55
            DECUMA_LANG_GSMDEFAULT,         // Inupiak = 56
            DECUMA_LANG_GSMDEFAULT,         // Irish = 57
            DECUMA_LANG_IT,                 // Italian = 58
            DECUMA_LANG_JP,                 // Japanese = 59
            DECUMA_LANG_GSMDEFAULT,         // Javanese = 60
            DECUMA_LANG_KN,                 // Kannada = 61
            DECUMA_LANG_GSMDEFAULT,         // Kashmiri = 62
            DECUMA_LANG_KK,                 // Kazakh = 63
            DECUMA_LANG_GSMDEFAULT,         // Kinyarwanda = 64
            DECUMA_LANG_KY,                 // Kirghiz = 65
            DECUMA_LANG_KO,                 // Korean = 66
            DECUMA_LANG_GSMDEFAULT,         // Kurdish = 67
            DECUMA_LANG_GSMDEFAULT,         // Rundi = 68
            DECUMA_LANG_GSMDEFAULT,         // Lao = 69
            DECUMA_LANG_GSMDEFAULT,         // Latin = 70
            DECUMA_LANG_LV,                 // Latvian = 71
            DECUMA_LANG_GSMDEFAULT,         // Lingala = 72
            DECUMA_LANG_LT,                 // Lithuanian = 73
            DECUMA_LANG_MK,                 // Macedonian = 74
            DECUMA_LANG_GSMDEFAULT,         // Malagasy = 75
            DECUMA_LANG_MS,                 // Malay = 76
            DECUMA_LANG_ML,                 // Malayalam = 77
            DECUMA_LANG_GSMDEFAULT,         // Maltese = 78
            DECUMA_LANG_GSMDEFAULT,         // Maori = 79
            DECUMA_LANG_MR,                 // Marathi = 80
            DECUMA_LANG_GSMDEFAULT,         // Marshallese = 81
            DECUMA_LANG_MN,                 // Mongolian = 82
            DECUMA_LANG_GSMDEFAULT,         // NauruLanguage = 83
            DECUMA_LANG_GSMDEFAULT,         // Nepali = 84
            DECUMA_LANG_NO,                 // NorwegianBokmal = 85
            DECUMA_LANG_GSMDEFAULT,         // Occitan = 86
            DECUMA_LANG_GSMDEFAULT,         // Oriya = 87
            DECUMA_LANG_GSMDEFAULT,         // Pashto = 88
            DECUMA_LANG_FA,                 // Persian = 89
            DECUMA_LANG_PL,                 // Polish = 90
            DECUMA_LANG_PT,                 // Portuguese = 91
            DECUMA_LANG_PA,                 // Punjabi = 92
            DECUMA_LANG_GSMDEFAULT,         // Quechua = 93
            DECUMA_LANG_GSMDEFAULT,         // Romansh = 94
            DECUMA_LANG_RO,                 // Romanian = 95
            DECUMA_LANG_RU,                 // Russian = 96
            DECUMA_LANG_GSMDEFAULT,         // Samoan = 97
            DECUMA_LANG_GSMDEFAULT,         // Sango = 98
            DECUMA_LANG_GSMDEFAULT,         // Sanskrit = 99
            DECUMA_LANG_SRCY,               // Serbian = 100
            DECUMA_LANG_GSMDEFAULT,         // Ossetic = 101
            DECUMA_LANG_ST,                 // SouthernSotho = 102
            DECUMA_LANG_GSMDEFAULT,         // Tswana = 103
            DECUMA_LANG_GSMDEFAULT,         // Shona = 104
            DECUMA_LANG_GSMDEFAULT,         // Sindhi = 105
            DECUMA_LANG_SI,                 // Sinhala = 106
            DECUMA_LANG_GSMDEFAULT,         // Swati = 107
            DECUMA_LANG_SK,                 // Slovak = 108
            DECUMA_LANG_SL,                 // Slovenian = 109
            DECUMA_LANG_GSMDEFAULT,         // Somali = 110
            DECUMA_LANG_ES,                 // Spanish = 111
            DECUMA_LANG_GSMDEFAULT,         // Sundanese = 112
            DECUMA_LANG_SW,                 // Swahili = 113
            DECUMA_LANG_SV,                 // Swedish = 114
            DECUMA_LANG_GSMDEFAULT,         // Sardinian = 115
            DECUMA_LANG_TG,                 // Tajik = 116
            DECUMA_LANG_TA,                 // Tamil = 117
            DECUMA_LANG_GSMDEFAULT,         // Tatar = 118
            DECUMA_LANG_TE,                 // Telugu = 119
            DECUMA_LANG_TH,                 // Thai = 120
            DECUMA_LANG_GSMDEFAULT,         // Tibetan = 121
            DECUMA_LANG_GSMDEFAULT,         // Tigrinya = 122
            DECUMA_LANG_GSMDEFAULT,         // Tongan = 123
            DECUMA_LANG_GSMDEFAULT,         // Tsonga = 124
            DECUMA_LANG_TR,                 // Turkish = 125
            DECUMA_LANG_GSMDEFAULT,         // Turkmen = 126
            DECUMA_LANG_GSMDEFAULT,         // Tahitian = 127
            DECUMA_LANG_GSMDEFAULT,         // Uighur = 128
            DECUMA_LANG_UK,                 // Ukrainian = 129
            DECUMA_LANG_UR,                 // Urdu = 130
            DECUMA_LANG_UZ,                 // Uzbek = 131
            DECUMA_LANG_VI                  // Vietnamese = 132
        };

        int localeLanguage = locale.language();
        if (locale.language() > maxLanguage)
            return DECUMA_LANG_GSMDEFAULT;

        DECUMA_UINT32 language = languageMap[localeLanguage];
        if (language == DECUMA_LANG_PRC) {
            if (inputMode != QVirtualKeyboardInputEngine::InputMode::ChineseHandwriting)
                language = DECUMA_LANG_EN;
            else if (locale.script() == QLocale::TraditionalChineseScript)
                language = (locale.country() == QLocale::HongKong) ? DECUMA_LANG_HK : DECUMA_LANG_TW;
        } else if (language == DECUMA_LANG_JP) {
            if (inputMode != QVirtualKeyboardInputEngine::InputMode::JapaneseHandwriting)
                language = DECUMA_LANG_EN;
        } else if (language == DECUMA_LANG_KO) {
            if (inputMode != QVirtualKeyboardInputEngine::InputMode::KoreanHandwriting)
                language = DECUMA_LANG_EN;
        } else if (language == DECUMA_LANG_SRCY) {
            if (inputMode != QVirtualKeyboardInputEngine::InputMode::Cyrillic)
                language = DECUMA_LANG_SRLA;
        } else if (language == DECUMA_LANG_AR || language == DECUMA_LANG_FA) {
            if (inputMode != QVirtualKeyboardInputEngine::InputMode::Arabic && inputMode != QVirtualKeyboardInputEngine::InputMode::Numeric)
                language = DECUMA_LANG_EN;
        } else if (language == DECUMA_LANG_IW) {
            if (inputMode != QVirtualKeyboardInputEngine::InputMode::Hebrew)
                language = DECUMA_LANG_EN;
        } else if (language == DECUMA_LANG_TH) {
            if (inputMode != QVirtualKeyboardInputEngine::InputMode::Thai)
                language = DECUMA_LANG_EN;
        }

        return language;
    }

    void updateRecognitionMode(DECUMA_UINT32 language, const QLocale &locale,
                               QVirtualKeyboardInputEngine::InputMode inputMode)
    {
        Q_Q(T9WriteInputMethod);
        Q_UNUSED(language)
        Q_UNUSED(locale)

        // Select recognition mode
        // Note: MCR mode is preferred, as it does not require recognition
        //       timer and provides better user experience.
        sessionSettings.recognitionMode = mcrMode;

        // T9 Write Alphabetic v8.0.0 supports UCR mode for specific languages
#if T9WRITEAPIMAJORVERNUM >= 21
        if (!cjk) {
            switch (inputMode) {
            case QVirtualKeyboardInputEngine::InputMode::Latin:
                switch (language) {
                case DECUMA_LANG_EN:
                case DECUMA_LANG_FR:
                case DECUMA_LANG_IT:
                case DECUMA_LANG_DE:
                case DECUMA_LANG_ES:
                    sessionSettings.recognitionMode = ucrMode;
                    break;
                default:
                    break;
                }
                break;
            case QVirtualKeyboardInputEngine::InputMode::Arabic:
                sessionSettings.recognitionMode = ucrMode;
                break;
            default:
                break;
            }
        }
#endif

        // Use scrMode with hidden text or with no predictive mode
        if (inputMode != QVirtualKeyboardInputEngine::InputMode::ChineseHandwriting &&
                inputMode != QVirtualKeyboardInputEngine::InputMode::JapaneseHandwriting &&
                inputMode != QVirtualKeyboardInputEngine::InputMode::KoreanHandwriting) {
            const Qt::InputMethodHints inputMethodHints = q->inputContext()->inputMethodHints();
            if (inputMethodHints.testFlag(Qt::ImhHiddenText) || inputMethodHints.testFlag(Qt::ImhNoPredictiveText))
                sessionSettings.recognitionMode = scrMode;
        }
    }

    bool updateSymbolCategories(DECUMA_UINT32 language, const QLocale &locale,
                                QVirtualKeyboardInputEngine::InputMode inputMode)
    {
        // Handle CJK in separate method
        if (cjk)
            return updateSymbolCategoriesCjk(language, locale, inputMode);

        symbolCategories.clear();

        // Choose the symbol categories by input mode, script and input method hints
        bool leftToRightGestures = true;
        Q_Q(T9WriteInputMethod);
        const Qt::InputMethodHints inputMethodHints = q->inputContext()->inputMethodHints();
        switch (inputMode) {
        case QVirtualKeyboardInputEngine::InputMode::Latin:
            if (inputMethodHints.testFlag(Qt::ImhEmailCharactersOnly)) {
                symbolCategories.append(DECUMA_CATEGORY_EMAIL);
            } else if (inputMethodHints.testFlag(Qt::ImhUrlCharactersOnly)) {
                symbolCategories.append(DECUMA_CATEGORY_URL);
            } else {
                if (language == DECUMA_LANG_EN || language == DECUMA_LANG_NL ||
                        language == DECUMA_LANG_MS || language == DECUMA_LANG_IN)
                    symbolCategories.append(DECUMA_CATEGORY_ANSI);
                else
                    symbolCategories.append(DECUMA_CATEGORY_ISO8859_1);
                symbolCategories.append(DECUMA_CATEGORY_DIGIT);
                symbolCategories.append(DECUMA_CATEGORY_BASIC_PUNCTUATIONS);
                symbolCategories.append(DECUMA_CATEGORY_CONTRACTION_MARK);
                if (language == DECUMA_LANG_ES)
                    symbolCategories.append(DECUMA_CATEGORY_SPANISH_PUNCTUATIONS);
                else if (language == DECUMA_LANG_VI)
                    symbolCategories.append(DECUMA_CATEGORY_VIETNAMESE_SUPPLEMENTS);
            }
            break;

        case QVirtualKeyboardInputEngine::InputMode::Numeric:
            if (language == DECUMA_LANG_AR || language == DECUMA_LANG_FA) {
                symbolCategories.append(DECUMA_CATEGORY_ARABIC_NUM_MODE);
                symbolCategories.append(DECUMA_CATEGORY_ARABIC_GESTURES);
                leftToRightGestures = false;
                break;
            }
            symbolCategories.append(DECUMA_CATEGORY_DIGIT);
            if (!inputMethodHints.testFlag(Qt::ImhDigitsOnly))
                symbolCategories.append(DECUMA_CATEGORY_NUM_SUP);
            break;

        case QVirtualKeyboardInputEngine::InputMode::Dialable:
            symbolCategories.append(DECUMA_CATEGORY_PHONE_NUMBER);
            break;

        case QVirtualKeyboardInputEngine::InputMode::Greek:
            symbolCategories.append(DECUMA_CATEGORY_GREEK);
            symbolCategories.append(DECUMA_CATEGORY_QUEST_EXCL_MARK_PUNCTUATIONS);
            symbolCategories.append(DECUMA_CATEGORY_PERIOD_COMMA_PUNCTUATIONS);
            symbolCategories.append(DECUMA_CATEGORY_COLON_PUNCTUATIONS);
            symbolCategories.append(DECUMA_CATEGORY_CONTRACTION_MARK);
            symbolCategories.append(DECUMA_CATEGORY_CONTRACTION_MARK);
            break;

        case QVirtualKeyboardInputEngine::InputMode::Cyrillic:
            symbolCategories.append(DECUMA_CATEGORY_CYRILLIC);
            symbolCategories.append(DECUMA_CATEGORY_QUEST_EXCL_MARK_PUNCTUATIONS);
            symbolCategories.append(DECUMA_CATEGORY_PERIOD_COMMA_PUNCTUATIONS);
            // Ukrainian needs contraction mark, but not Russian or Bulgarian
            if (language == DECUMA_LANG_UK)
                symbolCategories.append(DECUMA_CATEGORY_CONTRACTION_MARK);
            break;

        case QVirtualKeyboardInputEngine::InputMode::Arabic:
            symbolCategories.append(DECUMA_CATEGORY_ARABIC_ISOLATED_LETTER_MODE);
            symbolCategories.append(DECUMA_CATEGORY_ARABIC_GESTURES);
            leftToRightGestures = false;
            break;

        case QVirtualKeyboardInputEngine::InputMode::Hebrew:
            symbolCategories.append(DECUMA_CATEGORY_HEBREW_GL_HEBREW_CURSIVE_MODE);
            symbolCategories.append(DECUMA_CATEGORY_HEBREW_GL_HEBREW_LETTERSYMBOLS);
            symbolCategories.append(DECUMA_CATEGORY_HEBREW_SHEQEL);
            symbolCategories.append(DECUMA_CATEGORY_ARABIC_GESTURES);
            leftToRightGestures = false;
            break;

        case QVirtualKeyboardInputEngine::InputMode::Thai:
            symbolCategories.append(DECUMA_CATEGORY_THAI_BASE);
            symbolCategories.append(DECUMA_CATEGORY_THAI_NON_BASE);
            break;

        default:
            qCCritical(lcT9Write)  << "Invalid input mode" << inputMode;
            return false;
        }

        if (leftToRightGestures) {
            symbolCategories.append(DECUMA_CATEGORY_BACKSPACE_STROKE);
            symbolCategories.append(DECUMA_CATEGORY_RETURN_STROKE);
            symbolCategories.append(DECUMA_CATEGORY_WHITESPACE_STROKE);
        }

        return true;
    }

    bool updateSymbolCategoriesCjk(DECUMA_UINT32 language, const QLocale &locale,
                                   QVirtualKeyboardInputEngine::InputMode inputMode)
    {
        Q_ASSERT(cjk);

        symbolCategories.clear();

        switch (inputMode) {
        case QVirtualKeyboardInputEngine::InputMode::Latin:
            symbolCategories.append(DECUMA_CATEGORY_ANSI);
            symbolCategories.append(DECUMA_CATEGORY_CJK_SYMBOL);
            symbolCategories.append(DECUMA_CATEGORY_PUNCTUATIONS);
            break;

        case QVirtualKeyboardInputEngine::InputMode::Numeric:
            symbolCategories.append(DECUMA_CATEGORY_DIGIT);
            symbolCategories.append(DECUMA_CATEGORY_CJK_SYMBOL);
            symbolCategories.append(DECUMA_CATEGORY_PUNCTUATIONS);
            break;

        case QVirtualKeyboardInputEngine::InputMode::Dialable:
            symbolCategories.append(DECUMA_CATEGORY_DIGIT);
            symbolCategories.append(DECUMA_CATEGORY_CJK_SYMBOL);
            break;

        case QVirtualKeyboardInputEngine::InputMode::ChineseHandwriting:
            switch (locale.script()) {
            case QLocale::SimplifiedChineseScript:
                symbolCategories.append(DECUMA_CATEGORY_GB2312_A);
                symbolCategories.append(DECUMA_CATEGORY_GB2312_B_CHARS_ONLY);
                symbolCategories.append(DECUMA_CATEGORY_GBK_3);
                symbolCategories.append(DECUMA_CATEGORY_GBK_4);
                symbolCategories.append(DECUMA_CATEGORY_CJK_SYMBOL);
                symbolCategories.append(DECUMA_CATEGORY_CJK_GENERAL_PUNCTUATIONS);
                symbolCategories.append(DECUMA_CATEGORY_PUNCTUATIONS);
                break;

            case QLocale::TraditionalChineseScript:
                symbolCategories.append(DECUMA_CATEGORY_BIGFIVE);
                if (language == DECUMA_LANG_HK)
                    symbolCategories.append(DECUMA_CATEGORY_HKSCS_CHARS_ONLY);
                symbolCategories.append(DECUMA_CATEGORY_CJK_SYMBOL);
                symbolCategories.append(DECUMA_CATEGORY_CJK_GENERAL_PUNCTUATIONS);
                symbolCategories.append(DECUMA_CATEGORY_PUNCTUATIONS);
                break;

            default:
                qCCritical(lcT9Write)  << "Invalid locale" << locale << "for" << engineModeToString(engineMode);
                return false;
            }
            break;

        case QVirtualKeyboardInputEngine::InputMode::JapaneseHandwriting:
            symbolCategories.append(DECUMA_CATEGORY_JIS_LEVEL_1);
            symbolCategories.append(DECUMA_CATEGORY_JIS_LEVEL_2);
            symbolCategories.append(DECUMA_CATEGORY_HIRAGANA);
            symbolCategories.append(DECUMA_CATEGORY_KATAKANA);
            symbolCategories.append(DECUMA_CATEGORY_HIRAGANASMALL);
            symbolCategories.append(DECUMA_CATEGORY_KATAKANASMALL);
            symbolCategories.append(DECUMA_CATEGORY_CJK_SYMBOL);
            symbolCategories.append(DECUMA_CATEGORY_CJK_GENERAL_PUNCTUATIONS);
            symbolCategories.append(DECUMA_CATEGORY_PUNCTUATIONS);
            break;

        case QVirtualKeyboardInputEngine::InputMode::KoreanHandwriting:
            symbolCategories.append(DECUMA_CATEGORY_HANGUL_1001_A);
            symbolCategories.append(DECUMA_CATEGORY_HANGUL_1001_B);
            symbolCategories.append(DECUMA_CATEGORY_CJK_SYMBOL);
            symbolCategories.append(DECUMA_CATEGORY_CJK_GENERAL_PUNCTUATIONS);
            symbolCategories.append(DECUMA_CATEGORY_PUNCTUATIONS);
            break;

        default:
            return false;
        }

        return true;
    }

    void updateDictionary(DECUMA_UINT32 language, const QLocale &locale, bool languageChanged)
    {
        Q_Q(T9WriteInputMethod);

        /*  The dictionary is loaded in the background thread. Once the loading is
            complete the dictionary will be attached to the current session. The
            attachment happens in the worker thread context, thus the direct
            connection for the signal handler and the mutex protecting the
            converted dictionary for concurrent access.
            The loading operation is blocking for the main thread only if the
            user starts handwriting input before the operation is complete.
        */
        QMutexLocker dictionaryGuard(&dictionaryLock);

        // Detach previous dictionary if the language is being changed
        // or the recognizer mode is single-character mode
        const Qt::InputMethodHints inputMethodHints = q->inputContext()->inputMethodHints();
        if ((languageChanged || inputMethodHints.testFlag(Qt::ImhNoPredictiveText) || sessionSettings.recognitionMode == scrMode) && attachedDictionary) {
            detachDictionary(attachedDictionary);
            attachedDictionary.reset();
        }

        // Check if a dictionary needs to be loaded
        if (languageChanged || !loadedDictionary) {
            loadedDictionary.reset();

            DECUMA_SRC_DICTIONARY_INFO dictionaryInfo;
            memset(&dictionaryInfo, 0, sizeof(dictionaryInfo));

            QList<QLocale> decumaLocales;
            decumaLocales.append(locale);

            // CJK: No dictionary for latin input
            if (cjk && language == DECUMA_LANG_EN)
                decumaLocales.clear();

            dictionaryFileName.clear();
            QLocale decumaLocale;
            for (QLocale tryLocale : decumaLocales) {
                dictionaryFileName = findDictionary(defaultDictionaryDbPath, tryLocale, dictionaryInfo.srcType);
                if (!dictionaryFileName.isEmpty()) {
                    decumaLocale = tryLocale;
                    break;
                }
            }
            if (!dictionaryFileName.isEmpty()) {
                if (dictionaryTask.isNull() || dictionaryTask->dictionaryFileName != dictionaryFileName) {
                    qCDebug(lcT9Write) << " -> load dictionary:" << dictionaryFileName;

                    bool convertDictionary = true;
#if defined(HAVE_T9WRITE_CJK) && T9WRITEAPIMAJORVERNUM >= 20
                    // Chinese dictionary cannot be converted (PHD)
                    if (dictionaryInfo.srcType == decumaPortableHWRDictionary && decumaLocale.language() == QLocale::Chinese)
                        convertDictionary = false;
#endif

                    QSharedPointer<T9WriteDictionary> newDictionary(new T9WriteDictionary(decumaSession, memFuncs, cjk));
                    dictionaryTask.reset(new T9WriteDictionaryTask(newDictionary, dictionaryFileName, convertDictionary, dictionaryInfo));

                    QObject::connect(dictionaryTask.data(), &T9WriteDictionaryTask::completed,
                                     q, &T9WriteInputMethod::dictionaryLoadCompleted, Qt::DirectConnection);
                    worker->addTask(dictionaryTask);
                }
            }
        }

        // Attach existing dictionary, if available
        if (sessionSettings.recognitionMode != scrMode && !inputMethodHints.testFlag(Qt::ImhNoPredictiveText) &&
                loadedDictionary && !attachedDictionary) {
            if (attachDictionary(loadedDictionary))
                attachedDictionary = loadedDictionary;
        }
    }

    QByteArray getContext(QVirtualKeyboardInputEngine::PatternRecognitionMode patternRecognitionMode,
                          const QVariantMap &traceCaptureDeviceInfo,
                          const QVariantMap &traceScreenInfo) const
    {
        QCryptographicHash hash(QCryptographicHash::Md5);

        hash.addData((const char *)&patternRecognitionMode, sizeof(patternRecognitionMode));

        QByteArray mapData;
        QDataStream ds(&mapData, QIODevice::WriteOnly);
        ds << traceCaptureDeviceInfo;
        ds << traceScreenInfo;
        hash.addData(mapData);

        return hash.result();
    }

    void setContext(QVirtualKeyboardInputEngine::PatternRecognitionMode patternRecognitionMode,
                    const QVariantMap &traceCaptureDeviceInfo,
                    const QVariantMap &traceScreenInfo,
                    const QByteArray &context)
    {
        Q_UNUSED(patternRecognitionMode)
        if (context == currentContext)
            return;
        currentContext = context;

        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::setContext():" << QLatin1String((context.toHex()));

        // Finish recognition, but preserve current input
        Q_Q(T9WriteInputMethod);
        QString preeditText = q->inputContext()->preeditText();
        // WA:  T9Write CJK may crash in some cases with long stringStart.
        //      Therefore we don't restore the current input in this mode.
        bool preserveCurrentInput = !preeditText.isEmpty() && !cjk;
        T9WriteCaseFormatter oldCaseFormatter(caseFormatter);
        finishRecognition(!preserveCurrentInput);

        if (preserveCurrentInput) {
            caseFormatter = oldCaseFormatter;
            stringStart = preeditText;
            wordCandidates.append(preeditText);
            activeWordIndex = 0;
            emit q->selectionListChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
            emit q->selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList, activeWordIndex);
        }

        const int dpi = traceCaptureDeviceInfo.value(QLatin1String("dpi"), 96).toInt();
        static const int INSTANT_GESTURE_WIDTH_THRESHOLD_MM = 25;
        static const int INSTANT_GESTURE_HEIGHT_THRESHOLD_MM = 25;
        instantGestureSettings.widthThreshold = INSTANT_GESTURE_WIDTH_THRESHOLD_MM / 25.4 * dpi;
        instantGestureSettings.heightThreshold = INSTANT_GESTURE_HEIGHT_THRESHOLD_MM / 25.4 * dpi;

        gestureRecognizer.setDpi(dpi);

        QVariantList horizontalRulers(traceScreenInfo.value(QLatin1String("horizontalRulers"), QVariantList()).toList());
        if (horizontalRulers.count() > 2) {
            sessionSettings.baseline = horizontalRulers.last().toInt();
            sessionSettings.helpline = 0;
            sessionSettings.topline = horizontalRulers.first().toInt();
            sessionSettings.supportLineSet = baselineAndTopline;
            sessionSettings.UIInputGuide = supportlines;
        } else if (horizontalRulers.count() == 2) {
            sessionSettings.baseline = horizontalRulers.last().toInt();
            sessionSettings.helpline = horizontalRulers.first().toInt();
            sessionSettings.topline = 0;
            sessionSettings.supportLineSet = baselineAndHelpline;
            sessionSettings.UIInputGuide = supportlines;
        } else {
            sessionSettings.baseline = 0;
            sessionSettings.helpline = 0;
            sessionSettings.topline = 0;
            sessionSettings.supportLineSet = baselineAndHelpline;
            sessionSettings.UIInputGuide = none;
        }

        DECUMA_STATUS status = DECUMA_API(ChangeSessionSettings)(decumaSession, &sessionSettings);
        Q_ASSERT(status == decumaNoError);
    }

    QVirtualKeyboardTrace *traceBegin(
            int traceId, QVirtualKeyboardInputEngine::PatternRecognitionMode patternRecognitionMode,
            const QVariantMap &traceCaptureDeviceInfo, const QVariantMap &traceScreenInfo)
    {
        if (!worker)
            return nullptr;

        // The result id follows the trace id so that the (previous)
        // results completed during the handwriting can be rejected.
        resultId = traceId;

        stopResultTimer();

        // Dictionary must be completed before the arc addition can begin
        if (dictionaryTask) {
            dictionaryTask->wait();
            dictionaryTask.reset();
        }

        // Cancel the current recognition task
        worker->removeAllTasks<T9WriteRecognitionResultsTask>();
        worker->removeAllTasks<T9WriteRecognitionTask>();
        if (recognitionTask) {
            recognitionTask->cancelRecognition();
            recognitionTask.reset();
        }

#ifdef QT_VIRTUALKEYBOARD_RECORD_TRACE_INPUT
        if (!unipenTrace)
            unipenTrace.reset(new UnipenTrace(traceCaptureDeviceInfo, traceScreenInfo));
#endif

        QByteArray context = getContext(patternRecognitionMode, traceCaptureDeviceInfo, traceScreenInfo);
        if (context != currentContext) {
            worker->waitForAllTasks();
            setContext(patternRecognitionMode, traceCaptureDeviceInfo, traceScreenInfo, context);
        }

        DECUMA_STATUS status;

        if (!arcAdditionStarted) {
            worker->waitForAllTasks();
            status = DECUMA_API(BeginArcAddition)(decumaSession);
            Q_ASSERT(status == decumaNoError);
            arcAdditionStarted = true;
        }

        QVirtualKeyboardTrace *trace = new QVirtualKeyboardTrace();
#ifdef QT_VIRTUALKEYBOARD_RECORD_TRACE_INPUT
        trace->setChannels(QStringList(QLatin1String("t")));
#endif
        traceList.append(trace);

        return trace;
    }

    void traceEnd(QVirtualKeyboardTrace *trace)
    {
        if (trace->isCanceled()) {
            traceList.removeOne(trace);
            delete trace;
        } else {
            if (cjk && countActiveTraces() == 0) {
                // For some reason gestures don't seem to work in CJK mode
                // Using our own gesture recognizer as fallback
                if (handleGesture())
                    return;
            }
            worker->addTask(QSharedPointer<T9WriteAddArcTask>(new T9WriteAddArcTask(trace)));
        }
        if (!traceList.isEmpty()) {
            Q_ASSERT(arcAdditionStarted);
            if (countActiveTraces() == 0)
                restartRecognition();
        }
    }

    int countActiveTraces() const
    {
        int count = 0;
        for (QVirtualKeyboardTrace *trace : qAsConst(traceList)) {
            if (!trace->isFinal())
                count++;
        }
        return count;
    }

    void clearTraces()
    {
        worker->waitForAllTasks();
        qDeleteAll(traceList);
        traceList.clear();
    }

    void noteSelected(int index)
    {
        if (wordCandidatesHwrResultIndex.isEmpty())
            return;

        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::noteSelected():" << index;
        Q_ASSERT(index >= 0 && index < wordCandidatesHwrResultIndex.length());
        int resultIndex = wordCandidatesHwrResultIndex[index];
        DECUMA_API(NoteSelectedCandidate)(decumaSession, resultIndex);
    }

    void restartRecognition()
    {
        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::restartRecognition()";

        Q_Q(T9WriteInputMethod);

        worker->removeAllTasks<T9WriteRecognitionResultsTask>();
        if (recognitionTask) {
            recognitionTask->cancelRecognition();
            recognitionTask.reset();
        }

        // Boost dictionary words by default
        BOOST_LEVEL boostLevel = attachedDictionary ? boostDictWords : noBoost;

        // Disable dictionary boost in UCR mode for URL and E-mail input
        // Otherwise it will completely mess input
        const Qt::InputMethodHints inputMethodHints = q->inputContext()->inputMethodHints();
        if (sessionSettings.recognitionMode == ucrMode && (inputMethodHints & (Qt::ImhUrlCharactersOnly | Qt::ImhEmailCharactersOnly)))
            boostLevel = noBoost;

        QSharedPointer<T9WriteRecognitionResult> recognitionResult(new T9WriteRecognitionResult(resultId, 9, 64));
        recognitionTask.reset(new T9WriteRecognitionTask(recognitionResult, instantGestureSettings,
                                                         boostLevel, stringStart));
        worker->addTask(recognitionTask);

        QSharedPointer<T9WriteRecognitionResultsTask> resultsTask(new T9WriteRecognitionResultsTask(recognitionResult));
        q->connect(resultsTask.data(), SIGNAL(resultsAvailable(const QVariantList &)), SLOT(resultsAvailable(const QVariantList &)));
        worker->addTask(resultsTask);

        resetResultTimer();
    }

    void waitForRecognitionResults()
    {
        if (!worker)
            return;

        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::waitForRecognitionResults()";
        worker->waitForAllTasks();
        processResult();
    }

    bool finishRecognition(bool emitSelectionListChanged = true)
    {
        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::finishRecognition()";
        if (!worker)
            return false;

        bool result = !traceList.isEmpty();

        Q_ASSERT(decumaSession != nullptr);

        stopResultTimer();

        worker->removeAllTasks<T9WriteAddArcTask>();
        worker->removeAllTasks<T9WriteRecognitionResultsTask>();
        if (recognitionTask) {
            recognitionTask->cancelRecognition();
            recognitionTask.reset();
            result = true;
        }

        clearTraces();

        if (arcAdditionStarted) {
            DECUMA_API(EndArcAddition)(decumaSession);
            arcAdditionStarted = false;
        }

        if (!wordCandidates.isEmpty()) {
            wordCandidates.clear();
            wordCandidatesHwrResultIndex.clear();
            activeWordIndex = -1;
            if (emitSelectionListChanged) {
                Q_Q(T9WriteInputMethod);
                emit q->selectionListChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
                emit q->selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList, activeWordIndex);
            }
            result = true;
        }

        stringStart.clear();
        scrResult.clear();
        caseFormatter.clear();

#ifdef QT_VIRTUALKEYBOARD_RECORD_TRACE_INPUT
        unipenTrace.reset();
#endif

        return result;
    }

    bool select(int index = -1)
    {
        if (!worker)
            return false;

        if (sessionSettings.recognitionMode != scrMode && wordCandidates.isEmpty()) {
            finishRecognition();
            return false;
        }
        if (sessionSettings.recognitionMode == scrMode && scrResult.isEmpty()) {
            finishRecognition();
            return false;
        }

        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::select():" << index;

        Q_Q(T9WriteInputMethod);
        if (sessionSettings.recognitionMode != scrMode) {
            index = index >= 0 ? index : activeWordIndex;
            noteSelected(index);
            QString finalWord = wordCandidates.at(index);

#ifdef QT_VIRTUALKEYBOARD_RECORD_TRACE_INPUT
            // Record trace
            if (unipenTrace) {
                if (finalWord.length() == 1) {
                    // In recording mode, the text case must match with the current text case
                    QChar ch(finalWord.at(0));
                    if (!ch.isLetter() || (ch.isUpper() == (textCase == QVirtualKeyboardInputEngine::TextCase::Upper))) {
                        QStringList homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
                        if (!homeLocations.isEmpty()) {
                            unipenTrace->setDirectory(QStringLiteral("%1/%2").arg(homeLocations.at(0)).arg(QLatin1String("VIRTUAL_KEYBOARD_TRACES")));
                            unipenTrace->record(traceList);
                            unipenTrace->save(ch.unicode(), 100);
                        }
                    }
                }
            }
#endif

            finishRecognition();
            QChar gesture = T9WriteInputMethodPrivate::mapSymbolToGesture(finalWord.right(1).at(0));
            if (!gesture.isNull())
                finalWord.chop(1);
            q->inputContext()->commit(finalWord);
            applyGesture(gesture);
        } else if (sessionSettings.recognitionMode == scrMode) {
            QString finalWord = scrResult;
            finishRecognition();
            q->inputContext()->inputEngine()->virtualKeyClick((Qt::Key)finalWord.at(0).unicode(), finalWord, Qt::NoModifier);
        }

        return true;
    }

    void resetResultTimer(int interval = 500)
    {
        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::resetResultTimer():" << interval;
        Q_Q(T9WriteInputMethod);
        stopResultTimer();
        resultTimer = q->startTimer(interval);
    }

    void stopResultTimer()
    {
        if (resultTimer) {
            qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::stopResultTimer()";
            Q_Q(T9WriteInputMethod);
            q->killTimer(resultTimer);
            resultTimer = 0;
        }
    }

    void processResult()
    {
        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::processResult()";
        Q_Q(T9WriteInputMethod);
        QVirtualKeyboardInputContext *ic = q->inputContext();
        if (!ic)
            return;

        QStringList newWordCandidates;
        QList<int> newWordCandidatesHwrResultIndex;
        QString resultString;
        QString gesture;
        QVariantList symbolStrokes;
        {
            QMutexLocker resultListGuard(&resultListLock);
            if (resultList.isEmpty())
                return;

            if (resultList.first().toMap()[QLatin1String("resultId")] != resultId) {
                qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::processResult(): resultId mismatch" << resultList.first().toMap()[QLatin1String("resultId")] << "(" << resultId << ")";
                resultList.clear();
                return;
            }
            lastResultId = resultId;

            for (int i = 0; i < resultList.size(); i++) {
                QVariantMap result = resultList.at(i).toMap();
                QString resultChars = result[QLatin1String("chars")].toString();
                if (i == 0) {
                    if (ic->isShiftActive()) {
                        caseFormatter.ensureLength(1, textCase);
                        caseFormatter.ensureLength(resultChars.length(), QVirtualKeyboardInputEngine::TextCase::Lower);
                    } else {
                        caseFormatter.ensureLength(resultChars.length(), textCase);
                    }
                }
                if (!resultChars.isEmpty()) {
                    resultChars = caseFormatter.formatString(resultChars);
                    if (sessionSettings.recognitionMode != scrMode) {
                        newWordCandidates.append(resultChars);
                        newWordCandidatesHwrResultIndex.append(i);
                    }
                }
                if (i == 0) {
                    resultString = resultChars;
                    if (result.contains(QLatin1String("gesture")))
                        gesture = result[QLatin1String("gesture")].toString();
                    if (sessionSettings.recognitionMode != scrMode && result.contains(QLatin1String("symbolStrokes")))
                        symbolStrokes = result[QLatin1String("symbolStrokes")].toList();
                    if (sessionSettings.recognitionMode == scrMode)
                        break;
                } else {
                    // Add a gesture symbol to the secondary candidate
                    if (sessionSettings.recognitionMode != scrMode && result.contains(QLatin1String("gesture"))) {
                        QString gesture2 = result[QLatin1String("gesture")].toString();
                        if (gesture2.length() == 1) {
                            QChar symbol = T9WriteInputMethodPrivate::mapGestureToSymbol(gesture2.at(0).unicode());
                            if (!symbol.isNull()) {
                                // Check for duplicates
                                bool duplicateFound = false;
                                for (const QString &wordCandidate : newWordCandidates) {
                                    duplicateFound = wordCandidate.size() == 1 && wordCandidate.at(0) == symbol;
                                    if (duplicateFound)
                                        break;
                                }
                                if (!duplicateFound) {
                                    if (!resultChars.isEmpty()) {
                                        newWordCandidates.last().append(symbol);
                                    } else {
                                        newWordCandidates.append(symbol);
                                        newWordCandidatesHwrResultIndex.append(i);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            resultList.clear();
        }

        bool wordCandidatesChanged = wordCandidates != newWordCandidates;

#ifndef QT_VIRTUALKEYBOARD_RECORD_TRACE_INPUT
        // Delete trace history
        // Note: We have to be sure there are no background tasks
        //       running since the QVirtualKeyboardTrace objects consumed there.
        if (worker->numberOfPendingTasks() == 0) {

            const QVirtualKeyboardInputEngine::InputMode inputMode = q->inputEngine()->inputMode();
            if (sessionSettings.recognitionMode == mcrMode && !symbolStrokes.isEmpty() &&
                    inputMode != QVirtualKeyboardInputEngine::InputMode::ChineseHandwriting &&
                    inputMode != QVirtualKeyboardInputEngine::InputMode::JapaneseHandwriting &&
                    inputMode != QVirtualKeyboardInputEngine::InputMode::KoreanHandwriting) {
                int activeTraces = symbolStrokes.at(symbolStrokes.count() - 1).toInt();
                if (symbolStrokes.count() > 1)
                    activeTraces += symbolStrokes.at(symbolStrokes.count() - 2).toInt();
                while (activeTraces < traceList.count())
                    delete traceList.takeFirst();
            }

            // Enforce hard limit for number of traces
            if (traceList.count() >= traceListHardLimit) {
                qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::processResult(): Clearing traces (hard limit):" << traceList.count();
                clearTraces();
            }
        }
#endif

        // Find a gesture at the end of the first result
        if (!gesture.isEmpty()) {

            DECUMA_UNICODE gestureSymbol = gesture.at(0).unicode();
            if (!applyGesture(gestureSymbol)) {
                ic->commit(ic->preeditText());
                finishRecognition();
            }

            return;
        }

        if (sessionSettings.recognitionMode != scrMode) {
            ignoreUpdate = true;
            ic->setPreeditText(resultString);
            ignoreUpdate = false;
        } else {
            scrResult = resultString;
        }

        if (wordCandidatesChanged) {
            wordCandidates = newWordCandidates;
            wordCandidatesHwrResultIndex = newWordCandidatesHwrResultIndex;
            activeWordIndex = wordCandidates.isEmpty() ? -1 : 0;
            emit q->selectionListChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
            emit q->selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList, activeWordIndex);
        }

        if (arcAdditionStarted && traceList.isEmpty() && worker->numberOfPendingTasks() == 0) {
            DECUMA_API(EndArcAddition)(decumaSession);
            arcAdditionStarted = false;
        }
    }

    static QChar mapGestureToSymbol(const QChar &gesture)
    {
        switch (gesture.unicode()) {
        case '\r':
            return QChar(0x23CE);
        case ' ':
            return QChar(0x2423);
        default:
            return QChar();
        }
    }

    static QChar mapSymbolToGesture(const QChar &symbol)
    {
        switch (symbol.unicode()) {
        case 0x23CE:
            return QLatin1Char('\r');
        case 0x2423:
            return QLatin1Char(' ');
        default:
            return QChar();
        }
    }

    bool applyGesture(const QChar &gesture)
    {
        Q_Q(T9WriteInputMethod);
        QVirtualKeyboardInputContext *ic = q->inputContext();
        switch (gesture.unicode()) {
        case '\b':
            return ic->inputEngine()->virtualKeyClick(Qt::Key_Backspace, QString(), Qt::NoModifier);
        case '\r':
            return ic->inputEngine()->virtualKeyClick(Qt::Key_Return, QLatin1String("\n"), Qt::NoModifier);
        case ' ':
            return ic->inputEngine()->virtualKeyClick(Qt::Key_Space, QLatin1String(" "), Qt::NoModifier);
        default:
            return false;
        }
    }

    bool handleGesture()
    {
        if (countActiveTraces() > 0)
            return false;

        QVariantMap gesture(gestureRecognizer.recognize(traceList.mid(traceList.length() - 1, 1)));
        if (gesture.isEmpty())
            return false;

        qCDebug(lcT9Write) << "T9WriteInputMethodPrivate::handleGesture():" << gesture;

        if (gesture[QLatin1String("type")].toString() == QLatin1String("swipe")) {

            static const int SWIPE_ANGLE_THRESHOLD = 15;    // degrees +-

            qreal swipeLength = gesture[QLatin1String("length")].toReal();
            if (swipeLength >= instantGestureSettings.widthThreshold) {

                Q_Q(T9WriteInputMethod);
                QVirtualKeyboardInputContext *ic = q->inputContext();
                if (!ic)
                    return false;

                qreal swipeAngle = gesture[QLatin1String("angle_degrees")].toReal();
                int swipeTouchCount = gesture[QLatin1String("touch_count")].toInt();

                // Swipe left
                if (swipeAngle <= 180 + SWIPE_ANGLE_THRESHOLD && swipeAngle >= 180 - SWIPE_ANGLE_THRESHOLD) {
                    if (swipeTouchCount == 1) {
                        // Single swipe: backspace
                        ic->inputEngine()->virtualKeyClick(Qt::Key_Backspace, QString(), Qt::NoModifier);
                        return true;
                    }
                    return false;
                }

                // Swipe right
                const QVirtualKeyboardInputEngine::InputMode inputMode = q->inputEngine()->inputMode();
                if (inputMode != QVirtualKeyboardInputEngine::InputMode::ChineseHandwriting &&
                        inputMode != QVirtualKeyboardInputEngine::InputMode::JapaneseHandwriting &&
                        inputMode != QVirtualKeyboardInputEngine::InputMode::KoreanHandwriting) {
                    if (swipeAngle <= SWIPE_ANGLE_THRESHOLD || swipeAngle >= 360 - SWIPE_ANGLE_THRESHOLD) {
                        if (swipeTouchCount == 1) {
                            // Single swipe: space
                            ic->inputEngine()->virtualKeyClick(Qt::Key_Space, QLatin1String(" "), Qt::NoModifier);
                            return true;
                        }
                        return false;
                    }
                }

                // Swipe up
                if (swipeAngle <= 270 + SWIPE_ANGLE_THRESHOLD && swipeAngle >= 270 - SWIPE_ANGLE_THRESHOLD) {
                    if (swipeTouchCount == 1) {
                        // Single swipe: toggle input mode
                        select();
                        if (!(ic->inputMethodHints() & (Qt::ImhDialableCharactersOnly | Qt::ImhFormattedNumbersOnly | Qt::ImhDigitsOnly))) {
                            QList<int> inputModes = ic->inputEngine()->inputModes();
                            // Filter out duplicate numeric mode (in favor of Numeric)
                            int indexOfNumericInputMode = inputModes.indexOf(static_cast<const int>(QVirtualKeyboardInputEngine::InputMode::Numeric));
                            int indexOfDialableInputMode = inputModes.indexOf(static_cast<const int>(QVirtualKeyboardInputEngine::InputMode::Dialable));
                            if (indexOfNumericInputMode != -1 && indexOfDialableInputMode != -1)
                                inputModes.removeAt(inputMode != QVirtualKeyboardInputEngine::InputMode::Dialable ?
                                            indexOfDialableInputMode :
                                            indexOfNumericInputMode);
                            if (inputModes.count() > 1) {
                                int inputModeIndex = inputModes.indexOf(static_cast<const int>(inputMode)) + 1;
                                if (inputModeIndex >= inputModes.count())
                                    inputModeIndex = 0;
                                ic->inputEngine()->setInputMode(static_cast<QVirtualKeyboardInputEngine::InputMode>(inputModes.at(inputModeIndex)));
                            }
                        }
                        return true;
                    }
                }
            }
        }

        return false;
    }

    bool isValidInputChar(const QChar &c) const
    {
        if (c.isLetterOrNumber())
            return true;
        if (isJoiner(c))
            return true;
        return false;
    }

    bool isJoiner(const QChar &c) const
    {
        if (c.isPunct() || c.isSymbol()) {
            Q_Q(const T9WriteInputMethod);
            QVirtualKeyboardInputContext *ic = q->inputContext();
            if (ic) {
                Qt::InputMethodHints inputMethodHints = ic->inputMethodHints();
                if (inputMethodHints.testFlag(Qt::ImhUrlCharactersOnly) || inputMethodHints.testFlag(Qt::ImhEmailCharactersOnly))
                    return QString(QStringLiteral(":/?#[]@!$&'()*+,;=-_.%")).contains(c);
            }
            ushort unicode = c.unicode();
            if (unicode == Qt::Key_Apostrophe || unicode == Qt::Key_Minus)
                return true;
        }
        return false;
    }

    T9WriteInputMethod *q_ptr;
    static const DECUMA_MEM_FUNCTIONS memFuncs;
    bool cjk;
    T9WriteInputMethod::EngineMode engineMode;
    QByteArray currentContext;
    DECUMA_SESSION_SETTINGS sessionSettings;
    DECUMA_INSTANT_GESTURE_SETTINGS instantGestureSettings;
    QString defaultHwrDbPath;
    QString defaultDictionaryDbPath;
    QFile hwrDbFile;
    QVector<DECUMA_UINT32> languageCategories;
    QVector<DECUMA_UINT32> symbolCategories;
    QScopedPointer<T9WriteWorker> worker;
    QList<QVirtualKeyboardTrace *> traceList;
    int traceListHardLimit;
    QMutex dictionaryLock;
    QString dictionaryFileName;
    QSharedPointer<T9WriteDictionary> loadedDictionary;
    QSharedPointer<T9WriteDictionary> attachedDictionary;
    QSharedPointer<T9WriteDictionaryTask> dictionaryTask;
    QSharedPointer<T9WriteRecognitionTask> recognitionTask;
    QMutex resultListLock;
    QVariantList resultList;
    int resultId;
    int lastResultId;
    int resultTimer;
    QMetaObject::Connection processResultConnection;
    QByteArray session;
    DECUMA_SESSION *decumaSession;
    QStringList wordCandidates;
    QList<int> wordCandidatesHwrResultIndex;
    QString stringStart;
    QString scrResult;
    int activeWordIndex;
    bool arcAdditionStarted;
    bool ignoreUpdate;
    QVirtualKeyboardInputEngine::TextCase textCase;
    T9WriteCaseFormatter caseFormatter;
    HandwritingGestureRecognizer gestureRecognizer;
#ifdef QT_VIRTUALKEYBOARD_RECORD_TRACE_INPUT
    QScopedPointer<UnipenTrace> unipenTrace;
#endif
};

const DECUMA_MEM_FUNCTIONS T9WriteInputMethodPrivate::memFuncs = {
    T9WriteInputMethodPrivate::decumaMalloc,
    T9WriteInputMethodPrivate::decumaCalloc,
    T9WriteInputMethodPrivate::decumaFree,
    nullptr
};

/*!
    \class QtVirtualKeyboard::T9WriteInputMethod
    \internal
*/

T9WriteInputMethod::T9WriteInputMethod(QObject *parent) :
    QVirtualKeyboardAbstractInputMethod(parent),
    d_ptr(new T9WriteInputMethodPrivate(this))
{
}

T9WriteInputMethod::~T9WriteInputMethod()
{
    Q_D(T9WriteInputMethod);
    d->exitEngine();
}

QList<QVirtualKeyboardInputEngine::InputMode> T9WriteInputMethod::inputModes(const QString &locale)
{
    Q_D(T9WriteInputMethod);
    QList<QVirtualKeyboardInputEngine::InputMode> availableInputModes;
    const Qt::InputMethodHints inputMethodHints(inputContext()->inputMethodHints());
    const QLocale loc(locale);
    T9WriteInputMethod::EngineMode mode = d->mapLocaleToEngineMode(loc);

    // Add primary input mode
    switch (mode) {
#ifdef HAVE_T9WRITE_ALPHABETIC
    case T9WriteInputMethod::EngineMode::Alphabetic:
        if (d->findHwrDb(T9WriteInputMethod::EngineMode::Alphabetic, d->defaultHwrDbPath).isEmpty())
            return availableInputModes;
        if (!(inputMethodHints & (Qt::ImhDialableCharactersOnly | Qt::ImhFormattedNumbersOnly | Qt::ImhDigitsOnly | Qt::ImhLatinOnly))) {
            switch (loc.script()) {
            case QLocale::GreekScript:
                availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Greek);
                break;
            case QLocale::CyrillicScript:
                availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Cyrillic);
                break;
            case QLocale::ThaiScript:
                availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Thai);
                break;
            default:
                break;
            }
            availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Latin);
        }
        break;
    case T9WriteInputMethod::EngineMode::Arabic:
        if (d->findHwrDb(T9WriteInputMethod::EngineMode::Arabic, d->defaultHwrDbPath).isEmpty())
            return availableInputModes;
        if (!(inputMethodHints & (Qt::ImhDialableCharactersOnly | Qt::ImhFormattedNumbersOnly | Qt::ImhDigitsOnly | Qt::ImhLatinOnly)))
            availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Arabic);
        break;
    case T9WriteInputMethod::EngineMode::Hebrew:
        if (d->findHwrDb(T9WriteInputMethod::EngineMode::Hebrew, d->defaultHwrDbPath).isEmpty())
            return availableInputModes;
        if (!(inputMethodHints & (Qt::ImhDialableCharactersOnly | Qt::ImhFormattedNumbersOnly | Qt::ImhDigitsOnly | Qt::ImhLatinOnly)))
            availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Hebrew);
        break;
    case T9WriteInputMethod::EngineMode::Thai:
        if (d->findHwrDb(T9WriteInputMethod::EngineMode::Thai, d->defaultHwrDbPath).isEmpty())
            return availableInputModes;
        if (!(inputMethodHints & (Qt::ImhDialableCharactersOnly | Qt::ImhFormattedNumbersOnly | Qt::ImhDigitsOnly | Qt::ImhLatinOnly)))
            availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Thai);
        break;
#endif
#ifdef HAVE_T9WRITE_CJK
    case T9WriteInputMethod::EngineMode::SimplifiedChinese:
    case T9WriteInputMethod::EngineMode::TraditionalChinese:
    case T9WriteInputMethod::EngineMode::HongKongChinese:
        if (d->findHwrDb(mode, d->defaultHwrDbPath).isEmpty())
            return availableInputModes;
        if (!(inputMethodHints & (Qt::ImhDialableCharactersOnly | Qt::ImhFormattedNumbersOnly | Qt::ImhDigitsOnly | Qt::ImhLatinOnly)))
            availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::ChineseHandwriting);
        break;
    case T9WriteInputMethod::EngineMode::Japanese:
        if (d->findHwrDb(T9WriteInputMethod::EngineMode::Japanese, d->defaultHwrDbPath).isEmpty())
            return availableInputModes;
        if (!(inputMethodHints & (Qt::ImhDialableCharactersOnly | Qt::ImhFormattedNumbersOnly | Qt::ImhDigitsOnly | Qt::ImhLatinOnly)))
            availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::JapaneseHandwriting);
        break;
    case T9WriteInputMethod::EngineMode::Korean:
        if (d->findHwrDb(T9WriteInputMethod::EngineMode::Korean, d->defaultHwrDbPath).isEmpty())
            return availableInputModes;
        if (!(inputMethodHints & (Qt::ImhDialableCharactersOnly | Qt::ImhFormattedNumbersOnly | Qt::ImhDigitsOnly | Qt::ImhLatinOnly)))
            availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::KoreanHandwriting);
        break;
#endif
    default:
        return availableInputModes;
    }

    // Add exclusive input modes
    if (inputMethodHints.testFlag(Qt::ImhDialableCharactersOnly) || inputMethodHints.testFlag(Qt::ImhDigitsOnly)) {
        availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Dialable);
    } else if (inputMethodHints.testFlag(Qt::ImhFormattedNumbersOnly)) {
        availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Numeric);
    } else if (inputMethodHints.testFlag(Qt::ImhLatinOnly)) {
        availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Latin);
    } else {
        // Add other input modes
        Q_ASSERT(!availableInputModes.isEmpty());
        if (!availableInputModes.contains(QVirtualKeyboardInputEngine::InputMode::Latin))
            availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Latin);
        availableInputModes.append(QVirtualKeyboardInputEngine::InputMode::Numeric);
    }

    return availableInputModes;
}

bool T9WriteInputMethod::setInputMode(const QString &locale, QVirtualKeyboardInputEngine::InputMode inputMode)
{
    Q_D(T9WriteInputMethod);
    d->select();
    return d->setInputMode(QLocale(locale), inputMode);
}

bool T9WriteInputMethod::setTextCase(QVirtualKeyboardInputEngine::TextCase textCase)
{
    Q_D(T9WriteInputMethod);
    d->textCase = textCase;
    return true;
}

bool T9WriteInputMethod::keyEvent(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)
    Q_D(T9WriteInputMethod);
    switch (key) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Tab:
    case Qt::Key_Space:
        d->select();
        update();
        break;

    case Qt::Key_Backspace:
        {
            QVirtualKeyboardInputContext *ic = inputContext();
            QString preeditText = ic->preeditText();
            if (preeditText.length() > 1) {
                preeditText.chop(1);
                ic->setPreeditText(preeditText);
                // WA:  T9Write CJK may crash in some cases with long stringStart.
                //      Therefore we commit the current input and finish the recognition.
                if (d->cjk) {
                    d->waitForRecognitionResults();
                    ic->commit();
                    d->finishRecognition();
                    return true;
                }
                d->caseFormatter.ensureLength(preeditText.length(), d->textCase);
                T9WriteCaseFormatter caseFormatter(d->caseFormatter);
                d->finishRecognition(false);
                d->caseFormatter = caseFormatter;
                d->stringStart = preeditText;
                d->wordCandidates.append(preeditText);
                d->activeWordIndex = 0;
                emit selectionListChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
                emit selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList, d->activeWordIndex);
                return true;
            } else {
                bool result = !preeditText.isEmpty();
                if (result)
                    ic->clear();
                else
                    result = !d->scrResult.isEmpty();
                d->finishRecognition();
                return result;
            }
            break;
        }

    default:
        if (d->sessionSettings.recognitionMode != scrMode && text.length() > 0) {
            d->waitForRecognitionResults();
            QVirtualKeyboardInputContext *ic = inputContext();
            QString preeditText = ic->preeditText();
            QChar c = text.at(0);
            bool addToWord = d->isValidInputChar(c) && (!preeditText.isEmpty() || !d->isJoiner(c));
            if (addToWord) {
                preeditText.append(text);
                ic->setPreeditText(preeditText);
                d->caseFormatter.ensureLength(preeditText.length(), d->textCase);
                T9WriteCaseFormatter caseFormatter(d->caseFormatter);
                d->finishRecognition(false);
                d->caseFormatter = caseFormatter;
                d->stringStart = preeditText;
                d->wordCandidates.append(preeditText);
                d->activeWordIndex = 0;
                emit selectionListChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
                emit selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList, d->activeWordIndex);
                return true;
            } else {
                ic->commit();
                d->finishRecognition();
            }
            break;
        } else if (d->sessionSettings.recognitionMode == scrMode) {
            d->finishRecognition();
        }
    }
    return false;
}

void T9WriteInputMethod::reset()
{
    Q_D(T9WriteInputMethod);
    d->finishRecognition();
    d->setInputMode(QLocale(inputContext()->locale()), inputEngine()->inputMode());
}

void T9WriteInputMethod::update()
{
    Q_D(T9WriteInputMethod);
    if (d->ignoreUpdate)
        return;
    d->select();
}

QList<QVirtualKeyboardSelectionListModel::Type> T9WriteInputMethod::selectionLists()
{
    return QList<QVirtualKeyboardSelectionListModel::Type>() << QVirtualKeyboardSelectionListModel::Type::WordCandidateList;
}

int T9WriteInputMethod::selectionListItemCount(QVirtualKeyboardSelectionListModel::Type type)
{
    Q_UNUSED(type)
    Q_D(T9WriteInputMethod);
    return d->wordCandidates.count();
}

QVariant T9WriteInputMethod::selectionListData(QVirtualKeyboardSelectionListModel::Type type, int index, QVirtualKeyboardSelectionListModel::Role role)
{
    QVariant result;
    Q_D(T9WriteInputMethod);
    switch (role) {
    case QVirtualKeyboardSelectionListModel::Role::Display:
        result = QVariant(d->wordCandidates.at(index));
        break;
    case QVirtualKeyboardSelectionListModel::Role::WordCompletionLength:
        result.setValue(0);
        break;
    default:
        result = QVirtualKeyboardAbstractInputMethod::selectionListData(type, index, role);
        break;
    }
    return result;
}

void T9WriteInputMethod::selectionListItemSelected(QVirtualKeyboardSelectionListModel::Type type, int index)
{
    Q_UNUSED(type)
    Q_D(T9WriteInputMethod);
    d->select(index);
}

QList<QVirtualKeyboardInputEngine::PatternRecognitionMode> T9WriteInputMethod::patternRecognitionModes() const
{
    return QList<QVirtualKeyboardInputEngine::PatternRecognitionMode>()
            << QVirtualKeyboardInputEngine::PatternRecognitionMode::Handwriting;
}

QVirtualKeyboardTrace *T9WriteInputMethod::traceBegin(
        int traceId, QVirtualKeyboardInputEngine::PatternRecognitionMode patternRecognitionMode,
        const QVariantMap &traceCaptureDeviceInfo, const QVariantMap &traceScreenInfo)
{
    Q_D(T9WriteInputMethod);
    return d->traceBegin(traceId, patternRecognitionMode, traceCaptureDeviceInfo, traceScreenInfo);
}

bool T9WriteInputMethod::traceEnd(QVirtualKeyboardTrace *trace)
{
    Q_D(T9WriteInputMethod);
    d->traceEnd(trace);
    return true;
}

bool T9WriteInputMethod::reselect(int cursorPosition, const QVirtualKeyboardInputEngine::ReselectFlags &reselectFlags)
{
    Q_D(T9WriteInputMethod);

    if (d->sessionSettings.recognitionMode == scrMode)
        return false;

    QVirtualKeyboardInputContext *ic = inputContext();
    if (!ic)
        return false;

    const QVirtualKeyboardInputEngine::InputMode inputMode = inputEngine()->inputMode();
    const int maxLength = (inputMode == QVirtualKeyboardInputEngine::InputMode::ChineseHandwriting ||
            inputMode == QVirtualKeyboardInputEngine::InputMode::JapaneseHandwriting ||
            inputMode == QVirtualKeyboardInputEngine::InputMode::KoreanHandwriting) ? 0 : 32;
    const QString surroundingText = ic->surroundingText();
    int replaceFrom = 0;

    if (reselectFlags.testFlag(QVirtualKeyboardInputEngine::ReselectFlag::WordBeforeCursor)) {
        for (int i = cursorPosition - 1; i >= 0 && d->stringStart.length() < maxLength; --i) {
            QChar c = surroundingText.at(i);
            if (!d->isValidInputChar(c))
                break;
            d->stringStart.insert(0, c);
            --replaceFrom;
        }

        while (replaceFrom < 0 && d->isJoiner(d->stringStart.at(0))) {
            d->stringStart.remove(0, 1);
            ++replaceFrom;
        }
    }

    if (reselectFlags.testFlag(QVirtualKeyboardInputEngine::ReselectFlag::WordAtCursor) && replaceFrom == 0) {
        d->stringStart.clear();
        return false;
    }

    if (reselectFlags.testFlag(QVirtualKeyboardInputEngine::ReselectFlag::WordAfterCursor)) {
        for (int i = cursorPosition; i < surroundingText.length() && d->stringStart.length() < maxLength; ++i) {
            QChar c = surroundingText.at(i);
            if (!d->isValidInputChar(c))
                break;
            d->stringStart.append(c);
        }

        while (replaceFrom > -d->stringStart.length()) {
            int lastPos = d->stringStart.length() - 1;
            if (!d->isJoiner(d->stringStart.at(lastPos)))
                break;
            d->stringStart.remove(lastPos, 1);
        }
    }

    if (d->stringStart.isEmpty())
        return false;

    if (reselectFlags.testFlag(QVirtualKeyboardInputEngine::ReselectFlag::WordAtCursor) && replaceFrom == -d->stringStart.length() && d->stringStart.length() < maxLength) {
        d->stringStart.clear();
        return false;
    }

    if (d->isJoiner(d->stringStart.at(0))) {
        d->stringStart.clear();
        return false;
    }

    if (d->isJoiner(d->stringStart.at(d->stringStart.length() - 1))) {
        d->stringStart.clear();
        return false;
    }

    ic->setPreeditText(d->stringStart, QList<QInputMethodEvent::Attribute>(), replaceFrom, d->stringStart.length());
    for (int i = 0; i < d->stringStart.length(); ++i)
        d->caseFormatter.ensureLength(i + 1, d->stringStart.at(i).isUpper() ? QVirtualKeyboardInputEngine::TextCase::Upper : QVirtualKeyboardInputEngine::TextCase::Lower);
    d->wordCandidates.append(d->stringStart);
    d->activeWordIndex = 0;
    emit selectionListChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
    emit selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList, d->activeWordIndex);

    return true;
}

void T9WriteInputMethod::timerEvent(QTimerEvent *timerEvent)
{
    Q_D(T9WriteInputMethod);
    int timerId = timerEvent->timerId();
    qCDebug(lcT9Write) << "T9WriteInputMethod::timerEvent():" << timerId;
    if (timerId == d->resultTimer) {
        d->stopResultTimer();

        // Ignore if the result is not yet available
        if (d->resultId != d->lastResultId) {
            qCDebug(lcT9Write) << "T9WriteInputMethod::timerEvent(): Result not yet available";
            return;
        }

        if (d->sessionSettings.recognitionMode != scrMode) {
#ifndef QT_VIRTUALKEYBOARD_RECORD_TRACE_INPUT
            // Don't clear traces in UCR mode if dictionary is loaded.
            // In UCR mode the whole purpose is to write the word with
            // one or few strokes.
            if (d->sessionSettings.recognitionMode == ucrMode) {
                QMutexLocker dictionaryGuard(&d->dictionaryLock);
                if (d->attachedDictionary)
                    return;
            }

            const QVirtualKeyboardInputEngine::InputMode inputMode = inputEngine()->inputMode();
            if (inputMode != QVirtualKeyboardInputEngine::InputMode::ChineseHandwriting &&
                    inputMode != QVirtualKeyboardInputEngine::InputMode::JapaneseHandwriting &&
                    inputMode != QVirtualKeyboardInputEngine::InputMode::KoreanHandwriting) {
                d->clearTraces();
            }
#endif
        } else {
            d->select();
        }
    }
}

void T9WriteInputMethod::dictionaryLoadCompleted(QSharedPointer<T9WriteDictionary> dictionary)
{
    Q_D(T9WriteInputMethod);
    // Note: This method is called in worker thread context
    QMutexLocker dictionaryGuard(&d->dictionaryLock);

    if (!dictionary)
        return;

    qCDebug(lcT9Write) << "T9WriteInputMethod::dictionaryLoadCompleted():"
                            << dictionary->fileName() << dictionary->data() << dictionary->size();

    QVirtualKeyboardInputContext *ic = inputContext();
    if (ic && dictionary->fileName() == d->dictionaryFileName) {
        d->loadedDictionary = dictionary;
        if (d->sessionSettings.recognitionMode != scrMode &&
                !ic->inputMethodHints().testFlag(Qt::ImhNoPredictiveText) &&
                !d->attachedDictionary) {
            if (d->attachDictionary(d->loadedDictionary))
                d->attachedDictionary = d->loadedDictionary;
        }
    }
}

void T9WriteInputMethod::resultsAvailable(const QVariantList &resultList)
{
#ifdef SENSITIVE_DEBUG
    if (lcT9Write().isDebugEnabled()) {
        qCDebug(lcT9Write) << "T9WriteInputMethod::resultsAvailable():";
        for (int i = 0; i < resultList.size(); i++) {
            QVariantMap result = resultList.at(i).toMap();
            QString resultPrint = QStringLiteral("%1: ").arg(i + 1);
            QString resultChars = result.value(QLatin1String("chars")).toString();
            if (!resultChars.isEmpty())
                resultPrint.append(resultChars);
            if (result.contains(QLatin1String("gesture"))) {
                if (!resultChars.isEmpty())
                    resultPrint.append(QLatin1String(", "));
                QString gesture = result[QLatin1String("gesture")].toString();
                resultPrint.append(QLatin1String("gesture ="));
                for (const QChar &chr : gesture) {
                    resultPrint.append(QString::fromLatin1(" 0x%1").arg(chr.unicode(), 0, 16));
                }
            }
            qCDebug(lcT9Write) << resultPrint.toUtf8().constData();
        }
    }
#endif
    Q_D(T9WriteInputMethod);
    QMutexLocker resultListGuard(&d->resultListLock);
    d->resultList = resultList;
    emit resultListChanged();
}

void T9WriteInputMethod::processResult()
{
    Q_D(T9WriteInputMethod);
    bool resultTimerWasRunning = d->resultTimer != 0;

    d->processResult();

    // Restart the result timer now if it stopped before the results were completed
    if (!resultTimerWasRunning && (!d->scrResult.isEmpty() || !d->wordCandidates.isEmpty()))
        d->resetResultTimer(0);

}

void T9WriteInputMethod::recognitionError(int status)
{
    qCDebug(lcT9Write) << "T9WriteInputMethod::recognitionError():" << status;
    reset();
}

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE
