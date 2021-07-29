/****************************************************************************
**
** Copyright (C) MyScript. Contact: https://www.myscript.com/about/contact-us/sales-inquiry/
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB). Contact: https://www.qt.io/licensing/
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

#include "myscriptinputmethod_p.h"
#include "myscriptinputmethod_p_p.h"

#include <QtVirtualKeyboard/qvirtualkeyboardinputengine.h>
#include <QtVirtualKeyboard/qvirtualkeyboardinputcontext.h>
#include <QtVirtualKeyboard/qvirtualkeyboardtrace.h>

#include <QLoggingCategory>

#include MYSCRIPT_CERTIFICATE
#include <common/Properties.h>
#include <common/PortabilityDefinitions.h>
#include <voim.h>

#include <thread>
#include <mutex>
#include <chrono>

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

#include <QCryptographicHash>
#include <QThread>

#include <QtCore/qmath.h>
#include <QtCore/QLibraryInfo>
#include <QTextFormat>

#define VERIFY(arg) if (!(arg)) abort();

#define VERIFY2(arg, msg, engine) if (!(arg)) { \
                                    qCCritical(qlcVKMyScript) << msg << strMyScriptError(voGetError(engine)); \
                                    abort(); \
                                 }

#define GESTURE_STRING_RIGHT_TO_LEFT    "\xF3\xB0\x80\x82" // equivalent Unicode is "\U000F0002"
#define GESTURE_STRING_LEFT_TO_RIGHT    "\xF3\xB0\x80\x83" // equivalent Unicode is "\U000F0003"
#define GESTURE_STRING_DOWN_THEN_LEFT   "\xF3\xB0\x80\x84" // equivalent Unicode is "\U000F0004"
#define GESTURE_STRING_DOWN_THEN_RIGHT  "\xF3\xB0\x80\x88" // equivalent Unicode is "\U000F0008"

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

Q_LOGGING_CATEGORY(qlcVKMyScript, "qt.virtualkeyboard.myscript")

typedef enum GESTURE_TYPE {
    GESTURE_TYPE_NONE               = 0,
    GESTURE_TYPE_RIGHT_TO_LEFT      = 0x000F0002,
    GESTURE_TYPE_LEFT_TO_RIGHT      = 0x000F0003,
    GESTURE_TYPE_DOWN_THEN_LEFT     = 0x000F0004,
    GESTURE_TYPE_DOWN_THEN_RIGHT    = 0x000F0008
} GestureType;

typedef struct CANDIDATE_ITEM {
    int candidateIndex;
    QStringList candidates;
} CandidateItem;

static inline QString getVoimErrorMessage(const voimErrorCode code)
{
    QString message;

    switch (code) {
      case VOIM_EC_NO_ERROR:
        message = QString::fromLatin1("no error occurred");
        break;
      case VOIM_EC_INVALID_VALUE:
        message = QString::fromLatin1("a value passed to an API function was not valid");
        break;
      case VOIM_EC_INVALID_INDEX:
        message = QString::fromLatin1("an index passed to an API function was out of range");
        break;
      case VOIM_EC_INVALID_OPERATION:
        message = QString::fromLatin1("the requested operation is not valid regarding the current state of the target object");
        break;
      case VOIM_EC_OUT_OF_MEMORY:
        message = QString::fromLatin1("a memory allocation failure");
        break;
      case VOIM_EC_IO_FAILURE:
        message = QString::fromLatin1("an I/O operation failure");
        break;
      case VOIM_EC_INTERNAL_ERROR:
        message = QString::fromLatin1("an internal error");
        break;
      default:
        message = QString::fromLatin1("unknown error code - ") + QString::number(code);
        break;
    }

    return message;
}

class MyScriptInputMethodPrivate
{
    Q_DECLARE_PUBLIC(MyScriptInputMethod)
public:
    MyScriptInputMethodPrivate(MyScriptInputMethod *q_ptr) :
        q_ptr(q_ptr),
        m_engine(nullptr),
        m_languageManager(nullptr),
        m_recognizer(nullptr),
        m_onManagingResult(false),
        m_isProcessing(false),
        m_commitTimer(0),
        textCase(QVirtualKeyboardInputEngine::TextCase::Lower),
        wordIndex(-1),
        m_itemIndex(-1),
        m_preeditCursorPosition(0)
    {
        initHwrEngine();
    }

    ~MyScriptInputMethodPrivate()
    {
        destroyHwrEngine();
    }

    void setContext(QVirtualKeyboardInputEngine::PatternRecognitionMode patternRecognitionMode,
                    const QVariantMap &traceCaptureDeviceInfo,
                    const QVariantMap &traceScreenInfo)
    {
        Q_UNUSED(patternRecognitionMode);
        Q_UNUSED(traceCaptureDeviceInfo);
        Q_UNUSED(traceScreenInfo);

        qCDebug(qlcVKMyScript) << Q_FUNC_INFO;
    }

    QVirtualKeyboardTrace *traceBegin(
            int traceId, QVirtualKeyboardInputEngine::PatternRecognitionMode patternRecognitionMode,
            const QVariantMap &traceCaptureDeviceInfo, const QVariantMap &traceScreenInfo)
    {
        Q_UNUSED(patternRecognitionMode);
        Q_UNUSED(traceCaptureDeviceInfo);
        Q_UNUSED(traceScreenInfo);

        qCDebug(qlcVKMyScript) << Q_FUNC_INFO << traceId;

        if (!m_isProcessing) {
            Q_Q(MyScriptInputMethod);
            if (!q->inputContext()->preeditText().isEmpty())
                q->inputContext()->commit();
        }

        stopCommitTimer();

        for (int i = 0; i < traceList.size(); i++) {
            traceList[i]->setOpacity(qMax(0.0, 1 - 0.25 * (traceList.size() - i)));
        }

        QVirtualKeyboardTrace *trace = new QVirtualKeyboardTrace();
        traceList.append(trace);

        return trace;
    }

    void traceEnd(QVirtualKeyboardTrace *trace)
    {
        if (trace->isCanceled()) {
            qCDebug(qlcVKMyScript) << Q_FUNC_INFO << "discarded" << trace;
            traceList.removeOne(trace);
            delete trace;
        } else {
            addPointsToTraceGroup(trace);
        }

        if (!traceList.isEmpty() && countActiveTraces() == 0) {
            resetCommitTimer();
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
        qDeleteAll(traceList);
        traceList.clear();
    }

    void handleBackspace()
    {
        Q_Q(MyScriptInputMethod);

        cancelRecognition();

        clearCandidates();
        clearItems();
        q->inputContext()->commit();
    }

    void addPointsToTraceGroup(QVirtualKeyboardTrace *trace)
    {
        const QVariantList sourcePoints = trace->points();

        struct voPoint {
            float x;
            float y;
        };

        std::vector<voPoint> points;
        points.reserve(sourcePoints.size());
        for (const QVariant &p : sourcePoints) {
            const QPointF pt(p.toPointF());
            points.push_back({ (float)pt.x(), (float)pt.y() });
        }

        if (!voim_addStroke(m_engine, m_recognizer, &points.data()->x, sizeof(voPoint), &points.data()->y, sizeof(voPoint), (int)(points.size())) &&
            voim_getError(m_engine) != VOIM_EC_NO_ERROR) {
            qCCritical(qlcVKMyScript) << "voim_addStroke() failed -" << getVoimErrorMessage(voim_getError(m_engine));
            return;
        }
    }

    void clearCandidates(void)
    {
        qCDebug(qlcVKMyScript) << Q_FUNC_INFO;

        wordCandidates.clear();
        word = QString();
        wordIndex = -1;

        updateCandidateView();
    }

    void clearItems(void)
    {
        qCDebug(qlcVKMyScript) << Q_FUNC_INFO;

        m_itemIndex = -1;

        for (int i = 0; i < m_items.count(); i++) {
            CandidateItem *candidateItem = m_items.at(i).second;
            delete candidateItem;
        }

        m_items.clear();
    }

    bool cancelRecognition()
    {
        qCDebug(qlcVKMyScript) << Q_FUNC_INFO;
        Q_ASSERT(m_engine != nullptr && m_recognizer != nullptr);

        if (m_isProcessing)
            stopCommitTimer(); //commit();
        clearTraces();

        if (voim_cancel(m_engine, m_recognizer) &&
            voim_getError(m_engine) != VOIM_EC_NO_ERROR) {
            qCCritical(qlcVKMyScript) << "voim_cancel() failed -" << getVoimErrorMessage(voim_getError(m_engine));
            return false;
        }

        m_isProcessing = false;
        return !traceList.isEmpty();
    }

    void resetCommitTimer(void)
    {
        Q_Q(MyScriptInputMethod);

        stopCommitTimer();
        m_commitTimer = q->startTimer(1500);
    }

    void stopCommitTimer(void)
    {
        if (m_commitTimer) {
            Q_Q(MyScriptInputMethod);
            q->killTimer(m_commitTimer);
            m_commitTimer = 0;
        }
    }

    void initHwrEngine(void)
    {
        if (!createEngine())
            return;

        if (!createLanguageManager()) {
            voim_destroyEngine(m_engine);
            m_engine = nullptr;

            return;
        }

        if (!createRecognizer()) {
            voim_destroyLanguageManager(m_engine, m_languageManager);
            voim_destroyEngine(m_engine);

            m_languageManager = nullptr;
            m_engine = nullptr;

            return;
        }

        if (!voim_setNotificationCallback(m_engine, m_recognizer, notificationCallback, this)) {
            qCCritical(qlcVKMyScript) << "voim_setNotificationCallback() failed -" << getVoimErrorMessage(voim_getError(m_engine));
            return;
        }

        m_threadController.reset(new MyScriptRecognizeController(this, m_engine, m_recognizer));
    }

    bool createEngine(void)
    {
        qCDebug(qlcVKMyScript) << Q_FUNC_INFO;

        /*
         * NOTE: you must use the certificate provided by MyScript to use a MyScript product.
         *       It is described in MyCertificate.c and MyCertificate.h
         */
        const voCertificate *certificate = &myCertificate;
        voimProperty *properties = nullptr;

        QString imLibrary = QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/" + MYSCRIPT_VOIM_NAME;
        properties = Properties_put(properties, "com.myscript.im.library", imLibrary.toStdString().c_str());
        if (!properties) {
            qCCritical(qlcVKMyScript) << "failed to define property " << "com.myscript.im.library" << " with value " << imLibrary;
            return false;
        }

        QString engineLibrary = QLibraryInfo::location(QLibraryInfo::BinariesPath) + "/" + MYSCRIPT_ENGINE_NAME;
        properties = Properties_put(properties, "com.myscript.engine.library", engineLibrary.toStdString().c_str());
        if (!properties) {
            qCCritical(qlcVKMyScript) << "failed to define property " << "com.myscript.engine.library" << " with value " << engineLibrary;
            return false;
        }

        QString propertyFile = QLatin1String("/Engine.properties");
        propertyFile = QLibraryInfo::location(QLibraryInfo::DataPath) + "/" + MYSCRIPT_VOIM_PROPERTY_PATH + propertyFile;

        if (!checkFile(propertyFile)) {
            qCCritical(qlcVKMyScript) << "failed to open Engine Property file " << propertyFile;
            return false;
        }

        m_engine = voim_createEngine(certificate, propertyFile.toStdString().c_str(), properties);
        if (!m_engine) {
            qCCritical(qlcVKMyScript) << "voim_createEngine() failed";
            return false;
        }

        return true;
    }

    bool createLanguageManager(void)
    {
        qCDebug(qlcVKMyScript) << Q_FUNC_INFO;

        voimProperty *properties = nullptr;

        QString languageConf = QLibraryInfo::location(QLibraryInfo::DataPath) + "/" + MYSCRIPT_LANGUAGE_CONF_PATH;
        properties = Properties_put(properties, "com.myscript.im.languageSearchPath", languageConf.toStdString().c_str());
        if (!properties) {
            qCCritical(qlcVKMyScript) << "failed to define property " << "com.myscript.im.languageSearchPath" << " with value " << languageConf;
            return false;
        }

        properties = Properties_put(properties, "com.myscript.im.languageManifestSuffix", ".conf");
        if (!properties) {
            qCCritical(qlcVKMyScript) << "failed to define property " << "com.myscript.im.languageManifestSuffix" << " with value" << " \".conf\"";
            return false;
        }

        QString propertyFile = QLatin1String("/LanguageManager.properties");
        propertyFile = QLibraryInfo::location(QLibraryInfo::DataPath) + "/" + MYSCRIPT_VOIM_PROPERTY_PATH + propertyFile;

        if (!checkFile(propertyFile)) {
            qCCritical(qlcVKMyScript) << "failed to open LanguageManager Property file " << propertyFile;
            return false;
        }

        m_languageManager = voim_createLanguageManager(m_engine, propertyFile.toStdString().c_str(), properties);
        if (!m_languageManager) {
            qCCritical(qlcVKMyScript) << "voim_createLanguageManager() failed -" << getVoimErrorMessage(voim_getError(m_engine));
            return false;
        }

        voim_refreshLanguageList(m_engine, m_languageManager);
        if (voim_getError(m_engine) != VOIM_EC_NO_ERROR) {
            qCCritical(qlcVKMyScript) << "voim_refreshLanguageList failed -" << getVoimErrorMessage(voim_getError(m_engine));
        }
        Properties_destroy(properties);

        return true;
    }

    bool createRecognizer(void)
    {
        qCDebug(qlcVKMyScript) << Q_FUNC_INFO;

        QString propertyFile = QLatin1String("/Recognizer.properties");
        propertyFile = QLibraryInfo::location(QLibraryInfo::DataPath) + "/" + MYSCRIPT_VOIM_PROPERTY_PATH + propertyFile;

        if (!checkFile(propertyFile)) {
            qCCritical(qlcVKMyScript) << "failed to open Recognizer Property file " << propertyFile;
            return false;
        }

        m_recognizer = voim_createRecognizer(m_engine, m_languageManager, propertyFile.toStdString().c_str(), NULL);
        if (!m_recognizer) {
            qCCritical(qlcVKMyScript) << "voim_createRecognizer() failed -" << getVoimErrorMessage(voim_getError(m_engine));
            return false;
        }

        return true;
    }

    QString getLanguageName(const QString &locale)
    {
        if (locale.startsWith(QLatin1String("ar"))) // (language == "ar_EG" || language == "ar_AR")
            return QLatin1String("ar");
        else if (locale.startsWith(QLatin1String("fa"))) // (language == "fa_FA")
            return QLatin1String("fa_IR");
        else if (locale.startsWith(QLatin1String("nb")))
            return QLatin1String("no_NO");
        else if (locale.startsWith(QLatin1String("sr")))
            return QLatin1String("sr_Cyrl_RS");
        else
          return locale;
    }

    QString getModeName(Qt::InputMethodHints inputMethodHints)
    {
        /*
         * Qt::InputMethodHints flags description
         *
         *  - Qt::ImhNone                     :0x00000000
         *  - Qt::ImhHiddenText               :0x00000001
         *  - Qt::ImhSensitiveData            :0x00000002
         *  - Qt::ImhNoAutoUppercase          :0x00000004
         *  - Qt::ImhPreferNumbers            :0x00000008
         *  - Qt::ImhPreferUppercase          :0x00000010
         *  - Qt::ImhPreferLowercase          :0x00000020
         *  - Qt::ImhNoPredictiveText         :0x00000040
         *  - Qt::ImhDate                     :0x00000080
         *  - Qt::ImhTime                     :0x00000100
         *  - Qt::ImhPreferLatin              :0x00000200
         *  - Qt::ImhMultiLine                :0x00000400
         *  - Qt::ImhExclusiveInputMask       :0xffff0000
         *      - Qt::ImhDigitsOnly               :0x00010000
         *      - Qt::ImhFormattedNumbersOnly     :0x00020000
         *      - Qt::ImhUppercaseOnly            :0x00040000
         *      - Qt::ImhLowercaseOnly            :0x00080000
         *      - Qt::ImhDialableCharactersOnly   :0x00100000
         *      - Qt::ImhEmailCharactersOnly      :0x00200000
         *      - Qt::ImhUrlCharactersOnly        :0x00400000
         *      - Qt::ImhLatinOnly                :0x00800000
         */

        if (inputMethodHints & Qt::ImhDigitsOnly)
            return QLatin1String("number-superimposed");

        if (inputMethodHints & Qt::ImhFormattedNumbersOnly)
            return QLatin1String("number-superimposed"); // "number-superimposed" is not correctly matched with Qt::ImhFormattedNumbersOnly
                                                         // temporary linked to "number-superimposed", need to improve it later on

        if (inputMethodHints & Qt::ImhDialableCharactersOnly)
            return QLatin1String("phone_number-superimposed");

        if (inputMethodHints & Qt::ImhEmailCharactersOnly)
            return QLatin1String("email-superimposed");

        if (inputMethodHints & Qt::ImhUrlCharactersOnly)
            return QLatin1String("uri-superimposed");

        return QLatin1String("text-superimposed");
    }

    bool setMode(const QString &locale, Qt::InputMethodHints inputMethodHints)
    {
        if (locale == m_locale && inputMethodHints == m_inputMethodHints)
            return false;

        m_locale = locale;
        m_inputMethodHints = inputMethodHints;

        qCDebug(qlcVKMyScript) << Q_FUNC_INFO << locale;
        Q_ASSERT(m_engine != nullptr && m_recognizer != nullptr);

        QString language = getLanguageName(m_locale);
        QString mode = getModeName(m_inputMethodHints);

        if (!voim_setMode(m_engine, m_recognizer, language.toStdString().c_str(), mode.toStdString().c_str()) &&
            voim_getError(m_engine) != VOIM_EC_NO_ERROR) {
            qCCritical(qlcVKMyScript) << "voim_setMode() failed -" << getVoimErrorMessage(voim_getError(m_engine));
            return false;
        }

        return true;
    }

    bool commit(void)
    {
        qCDebug(qlcVKMyScript) << Q_FUNC_INFO;
        Q_ASSERT(m_engine != nullptr && m_recognizer != nullptr);

        if (!voim_commit(m_engine, m_recognizer) &&
            voim_getError(m_engine) != VOIM_EC_NO_ERROR) {
            qCCritical(qlcVKMyScript) << "voim_commit() failed -" << getVoimErrorMessage(voim_getError(m_engine));
            return false;
        }

        stopCommitTimer();

        return true;
    }

    void destroyHwrEngine(void)
    {
        qCDebug(qlcVKMyScript) << Q_FUNC_INFO;

        cancelRecognition();

        if (m_engine) {
            if (m_recognizer)
                voim_destroyRecognizer(m_engine, m_recognizer);

            if (m_languageManager)
                voim_destroyLanguageManager(m_engine, m_languageManager);

            voim_destroyEngine(m_engine);
        }

        m_threadController.reset();
    }

    bool checkFile(QString filename)
    {
        if (QFile::exists(filename)) {
            return true;
        } else {
            return false;
        }
    }

    /******************************************************************************
     * The notificationCallback handles events raised by
     * the worker thread of a recognizer
     *
     * Parameters:
     *  - engine          : the VOIM engine
     *  - recognizer      : the VOIM recognizer that sends the event.
     *  - eventType       : the type of the event.
     *  - eventParameters : the parameters associated with the event, if any.
     *  - userParam       : the userParameter that passed to
     *                      voim_setNotificationCallback
     *****************************************************************************/
    static void notificationCallback(voimEngine *engine,
                                     voimRecognizer *recognizer,
                                     voimEventType eventType,
                                     const void *eventParameters,
                                     void *userParam)
    {
        if (userParam) {
            MyScriptInputMethodPrivate *pParent = static_cast<MyScriptInputMethodPrivate *>(userParam);
            pParent->onNotify(engine, recognizer, eventType, eventParameters);
        }
    }

    /******************************************************************************
     * Event types of VOIM handwriting notification callback
     *
     *  - VOIM_EVENT_ON_SET_MODE
     *    :Occurs when a mode change request has been taken into account.
     *  - VOIM_EVENT_ON_SET_POSITION_AND_SCALE_INDICATOR
     *    :Occurs when a guide line change request has been taken into account.
     *  - VOIM_EVENT_ON_SET_USER_DICTIONARY
     *    :Occurs when a user dictionary has been set to the recognizer.
     *  - VOIM_EVENT_ON_ADD_STROKE
     *    :Occurs when a new digital ink stroke has been added to
     *     the recognizer and received by the worker thread.
     *  - VOIM_EVENT_ON_RECOGNITION_START
     *    :Occurs when a recognition process starts.
     *  - VOIM_EVENT_ON_RECOGNITION_PROGRESS
     *    :Occurs during a recognition process.
     *  - VOIM_EVENT_ON_RECOGNITION_END
     *    :Occurs when a recognition process ends.
     *  - VOIM_EVENT_ON_COMMIT
     *    :Occurs when a commit request has been taken into account.
     *  - VOIM_EVENT_ON_CANCEL
     *    :Occurs when a cancel request has been taken into account.
     *  - VOIM_EVENT_ON_FLOW_SYNC
     *    :Occurs when a flow sync command is received by
     *     the recognizer worker thread.
     *  - VOIM_EVENT_ON_NEW_INPUT_ITEM
     *    :Occurs when a new input item command is received by
     *     the recognizer worker thread.
     *  - VOIM_EVENT_ON_ADD_STRING
     *    :Occurs when a new digital text string has been added
     *     to the recognizer and received by the worker thread.
     *****************************************************************************/
    void onNotify(voimEngine *engine, voimRecognizer *recognizer,
                  voimEventType eventType, const void *eventParameters)
    {
      Q_UNUSED(recognizer);

      switch (eventType) {

      case VOIM_EVENT_ON_SET_MODE:
        {
           /*
           *  voimSetModeParameters
           *
           *    voimLanguage *language    - The language that contains the handwriting mode
           *    int           modeIndex   - The index of the mode in the language
           *    bool          successful  - True if the operation was successful
           */
          const voimSetModeParameters *p = (const voimSetModeParameters *)eventParameters;

          if (p->successful) {
              QString language = QString::fromUtf8(voim_getLanguageName(engine, p->language));
              QString mode = QString::fromUtf8(voim_getLanguageModeNameAt(engine, p->language, p->modeIndex));

              qCDebug(qlcVKMyScript) << "a mode" << language << "/" << mode <<
                                        "has been set, ready to receive strokes";
          } else {
              qCCritical(qlcVKMyScript) << "failed to set mode";
          }
        }
        break;

      case VOIM_EVENT_ON_SET_POSITION_AND_SCALE_INDICATOR:
        {
          /*
           * voimSetPositionAndScaleIndicatorParameters
           *
           *    float baselinePosition  - The baseline position
           *    float xHeight           - The height of small letter x
           *    float lineSpacing       - The line spacing
           *    bool  successful        - True if the operation was successful
           */
          const voimSetPositionAndScaleIndicatorParameters *p = (const voimSetPositionAndScaleIndicatorParameters *)eventParameters;

          if (p->successful) {
              qCDebug(qlcVKMyScript) << "baselinePosition \"" << p->baselinePosition << "\", " <<
                                        "xHeight \"" << p->xHeight << "\", " <<
                                        "lineSpacing \"" << p->lineSpacing << "\" have been set";
          } else {
              qCCritical(qlcVKMyScript) << "failed to set position and scale indicator";
          }
        }
        break;

      case VOIM_EVENT_ON_SET_USER_DICTIONARY:
        {
          /*
           * voimSetUserDictionaryParameters
           *
           *    voimDictionary *dictionary  - The dictionary
           *    bool            successful  - True if the operation was successful
           */
          const voimSetUserDictionaryParameters *p = (const voimSetUserDictionaryParameters *)eventParameters;

          if (p->successful)
              qCDebug(qlcVKMyScript) << "user dictionary has been set";
          else
              qCCritical(qlcVKMyScript) << "failed to set user dictionary";
        }
        break;

      case VOIM_EVENT_ON_ADD_STROKE:
        {
          /*
           * voimAddStrokeParameters
           *
           *    int  sessionIndex  - The index of the recognition session
           *    int  strokeIndex   - The index of the stroke in the recognition session
           *    bool successful    - True if the operation was successful
           */
          const voimAddStrokeParameters *p = (const voimAddStrokeParameters *)eventParameters;

          if (p->successful) {
              qCDebug(qlcVKMyScript) << "a stroke with sessionIndex \"" << p->sessionIndex << "\", " <<
                                        "strokeIndex \"" << p->strokeIndex << "\" has been added";
          } else {
              qCCritical(qlcVKMyScript) << "failed to add stroke";
          }
        }
        break;

      case VOIM_EVENT_ON_RECOGNITION_START:
        {
          /*
           * voimRecognitionStartParameters
           *
           *    int  firstStrokeIndex  - The index of the first new stroke to be recognized
           *    int  sessionIndex      - The index of the recognition session
           *    int  strokeCount       - The number of new strokes to be recognized
           */
          const voimRecognitionStartParameters *p = (const voimRecognitionStartParameters *)eventParameters;

          qCDebug(qlcVKMyScript) << "recognition started at sessionIndex \"" << p->sessionIndex << "\", " <<
                                    "firstStrokeIndex \"" << p->firstStrokeIndex << "\", " <<
                                    "strokeCount \"" << p->strokeCount << "\"";

          m_isProcessing = true;
        }
        break;

      case VOIM_EVENT_ON_RECOGNITION_PROGRESS:
        {
          /*
           * voimRecognitionProgressParameters
           *
           *    int  amountDone  - The current amount of work done
           *    int  amountToDo  - The current amount of work to do
           */
          const voimRecognitionProgressParameters *p = (const voimRecognitionProgressParameters *)eventParameters;

          if (p->amountDone == p->amountToDo)
              qCDebug(qlcVKMyScript) << "progress recognition, " << p->amountDone << "/" << p->amountToDo;
        }
        break;

      case VOIM_EVENT_ON_RECOGNITION_END:
        {
          /*
           * voimRecognitionEndParameters
           *
           *    bool successful  - True if the operation was successful
           */
          const voimRecognitionEndParameters *p = (const voimRecognitionEndParameters *)eventParameters;

          if (p->successful) {
              qCDebug(qlcVKMyScript) << "recognition has been ended";

              if (!m_onManagingResult) {
                  m_onManagingResult = true;
                  if (m_threadController) {
                      m_threadController->emitRecognitionEnded();
                  }
              }
          } else {
              qCCritical(qlcVKMyScript) << "failed to finish recognition";
          }
        }
        break;

      case VOIM_EVENT_ON_COMMIT:
        {
          /*
           * voimCommitParameters
           *
           *    bool successful  - True if the operation was successful
           */
          const voimCommitParameters *p = (const voimCommitParameters *)eventParameters;

          if (p->successful) {
              qCDebug(qlcVKMyScript) << "recognition has been committed";

              if (m_threadController) {
                  m_threadController->emitRecognitionCommitted();
              }
          } else {
              qCCritical(qlcVKMyScript) << "failed to commit recognition";
          }
        }
        break;

      case VOIM_EVENT_ON_CANCEL:
        {
          /*
           * voimCancelParameters
           *
           *    bool successful  - True if the operation was successful
           */
          const voimCancelParameters *p = (const voimCancelParameters *)eventParameters;

          if (p->successful)
              qCDebug(qlcVKMyScript) << "recognition has been canceled";
          else
              qCCritical(qlcVKMyScript) << "failed to cancel recognition";
        }
        break;

      case VOIM_EVENT_ON_FLOW_SYNC:
        {
          /*
           * voimFlowSyncParameters
           *
           *    int  intValue  - The integer value that was passed to the voim_flowSync() function
           */
          const voimFlowSyncParameters *p = (const voimFlowSyncParameters *)eventParameters;

          qCDebug(qlcVKMyScript) << "voim_flowSync() has been called with value \"" << p->intValue << "\"";
        }
        break;

      case VOIM_EVENT_ON_NEW_INPUT_ITEM:
        {
          /*
           * voimNewInputItemParameters
           *
           *    bool successful  - True if the operation was successful
           */
          const voimNewInputItemParameters *p = (const voimNewInputItemParameters *)eventParameters;

          if (p->successful)
              qCDebug(qlcVKMyScript) << "new recognition session being created while remaining in the current session";
          else
              qCCritical(qlcVKMyScript) << "failed to create new recogniiton session";
        }
        break;

      case VOIM_EVENT_ON_ADD_STRING:
        {
          /*
           * [Note that the parameters are same as that of the VOIM_EVENT_ON_ADD_STROKE event
           *  because references to it in the result work the same (using stroke index
           *  for string and point index for its characters)]
           *
           * voimAddStrokeParameters
           *
           *    int  sessionIndex  - The index of the recognition session
           *    int  strokeIndex   - The index of the stroke in the recognition session
           *    bool successful    - True if the operation was successful
           */
          const voimAddStrokeParameters *p = (const voimAddStrokeParameters *)eventParameters;

          if (p->successful) {
              qCDebug(qlcVKMyScript) << "a stroke with sessionIndex \"" << p->sessionIndex << "\", " <<
                                        "strokeIndex \"" << p->strokeIndex << "\" has been added";
          } else {
              qCCritical(qlcVKMyScript) << "failed to add string";
          }
        }
        break;

      default:
        break;
      }
    }

    void updateCandidateView(void)
    {
        Q_Q(MyScriptInputMethod);

        emit q->selectionListChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList);
        emit q->selectionListActiveItemChanged(QVirtualKeyboardSelectionListModel::Type::WordCandidateList, wordIndex);
    }

    void updatePreeditTextCursor(int cursorPosition)
    {
        Q_Q(MyScriptInputMethod);

        QVirtualKeyboardInputContext *ic = q->inputContext();
        if (!ic)
            return;

        qCDebug(qlcVKMyScript) << Q_FUNC_INFO;
#ifdef SENSITIVE_DEBUG
        qCDebug(qlcVKMyScript) << "preeditText:" << ic->preeditText();
#endif

        bool isItemChanged = false;
        int lastPosition = 0;
        QVector<std::pair<int, CandidateItem *>>::const_iterator iter;

        for (iter = m_items.cbegin(); iter != m_items.cend(); iter++) {
            int itemIndex = iter->first;
            CandidateItem *candidateItem = iter->second;
            int candidateIndex = candidateItem->candidateIndex;
            QString candidate = candidateItem->candidates.at(candidateIndex);

            lastPosition += candidate.length();
            if (candidate != " " && cursorPosition <= lastPosition) {
                m_itemStartPosition = lastPosition - candidate.length();
                m_itemLength = candidate.length();

                if (m_itemIndex != itemIndex) {
                    clearCandidates();
                    word = candidate;
                    wordIndex = candidateIndex;
                    wordCandidates = candidateItem->candidates;

                    m_itemIndex = itemIndex;
                    isItemChanged = true;
                }

                break;
            }
        }

        if (isItemChanged)
            updateCandidateView();
    }

    MyScriptInputMethod *q_ptr;

    QScopedPointer<MyScriptRecognizeController> m_threadController;

    voimEngine *m_engine;
    voimLanguageManager *m_languageManager;
    voimRecognizer *m_recognizer;
    bool m_onManagingResult;
    bool m_isProcessing;
    int m_commitTimer;

    QList<QVirtualKeyboardTrace *> traceList;

    QVirtualKeyboardInputEngine::TextCase textCase;
    QStringList wordCandidates;
    QString word;
    int wordIndex;

    int m_itemIndex;
    int m_itemStartPosition;
    int m_itemLength;
    QVector<std::pair<int, CandidateItem *>> m_items;

    QString m_locale;
    Qt::InputMethodHints m_inputMethodHints;

    int m_preeditCursorPosition;
};

/*!
    \class QtVirtualKeyboard::MyScriptInputMethod
    \internal
*/

MyScriptInputMethod::MyScriptInputMethod(QObject *parent) :
    QVirtualKeyboardAbstractInputMethod(parent),
    d_ptr(new MyScriptInputMethodPrivate(this))
{
    connect(this, SIGNAL(preeditTextChanged(QString, bool, int, int, int)), this, SLOT(setPreeditText(QString, bool, int, int, int)));
    connect(this, SIGNAL(gestureDetected(int, int)), this, SLOT(doGestureAction(int, int)));
}

MyScriptInputMethod::~MyScriptInputMethod()
{

}

QList<QVirtualKeyboardInputEngine::InputMode> MyScriptInputMethod::inputModes(const QString &locale)
{
    Q_UNUSED(locale);
    return QList<QVirtualKeyboardInputEngine::InputMode>()
            << QVirtualKeyboardInputEngine::InputMode::Latin;
}

bool MyScriptInputMethod::setInputMode(const QString &locale, QVirtualKeyboardInputEngine::InputMode inputMode)
{
    Q_UNUSED(inputMode);
    Q_D(MyScriptInputMethod);
    QVirtualKeyboardInputContext *ic = inputContext();
    if (d->setMode(locale, ic->inputMethodHints())) {
        d->m_locale = locale;
        return true;
    }
    return false;
}

bool MyScriptInputMethod::setTextCase(QVirtualKeyboardInputEngine::TextCase textCase)
{
    Q_D(MyScriptInputMethod);
    d->textCase = textCase;
    return true;
}

bool MyScriptInputMethod::keyEvent(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(text);
    Q_UNUSED(modifiers);

    Q_D(MyScriptInputMethod);
    switch (key) {
    case Qt::Key_Backspace:
        d->handleBackspace();
        break;
    default:
        d->cancelRecognition();
        if (inputContext())
            inputContext()->commit();
        break;
    }
    return false;
}

void MyScriptInputMethod::reset()
{
    QVirtualKeyboardInputContext *ic = inputContext();
    if (ic) {
        Q_D(MyScriptInputMethod);
        d->clearCandidates();
        ic->commit();
    }
}

void MyScriptInputMethod::update()
{
    Q_D(MyScriptInputMethod);
    if (d->m_isProcessing)
        d->cancelRecognition();

    reset();
}

QList<QVirtualKeyboardSelectionListModel::Type> MyScriptInputMethod::selectionLists()
{
    return QList<QVirtualKeyboardSelectionListModel::Type>() << QVirtualKeyboardSelectionListModel::Type::WordCandidateList;
}

int MyScriptInputMethod::selectionListItemCount(QVirtualKeyboardSelectionListModel::Type type)
{
    Q_D(MyScriptInputMethod);

    if (type != QVirtualKeyboardSelectionListModel::Type::WordCandidateList)
        return 0;

    return d->wordCandidates.count();
}

QVariant MyScriptInputMethod::selectionListData(QVirtualKeyboardSelectionListModel::Type type, int index, QVirtualKeyboardSelectionListModel::Role role)
{
    Q_D(MyScriptInputMethod);

    if (type != QVirtualKeyboardSelectionListModel::Type::WordCandidateList)
        return QVariant();

    switch (role) {
    case QVirtualKeyboardSelectionListModel::Role::Display:
        return QVariant(d->wordCandidates.at(index));
    case QVirtualKeyboardSelectionListModel::Role::WordCompletionLength:
    {
        const QString wordCandidate(d->wordCandidates.at(index));
        int wordCompletionLength = wordCandidate.length() - d->word.length();
        return QVariant((wordCompletionLength > 0 && wordCandidate.startsWith(d->word)) ? wordCompletionLength : 0);
    }
    default:
        return QVirtualKeyboardAbstractInputMethod::selectionListData(type, index, role);
    }
}

void MyScriptInputMethod::selectionListItemSelected(QVirtualKeyboardSelectionListModel::Type type, int index)
{
    Q_D(MyScriptInputMethod);

    if (d->m_isProcessing)
        return;

    QVirtualKeyboardInputContext *ic = inputContext();
    if (!ic)
        return;

    qCDebug(qlcVKMyScript) << Q_FUNC_INFO;

    QVirtualKeyboardAbstractInputMethod::selectionListItemSelected(type, index);

    int itemIndex = d->m_itemIndex;
    CandidateItem *candidateItem = d->m_items.at(itemIndex).second;

    QString candidate = candidateItem->candidates.at(index);
    candidateItem->candidateIndex = index;

    QString label = ic->preeditText();
    label.replace(d->m_itemStartPosition, d->m_itemLength, candidate);
    setPreeditText(label, true, d->m_preeditCursorPosition, d->m_itemStartPosition, candidate.length());

    d->updatePreeditTextCursor(d->m_preeditCursorPosition);
}

QList<QVirtualKeyboardInputEngine::PatternRecognitionMode> MyScriptInputMethod::patternRecognitionModes() const
{
    return QList<QVirtualKeyboardInputEngine::PatternRecognitionMode>()
            << QVirtualKeyboardInputEngine::PatternRecognitionMode::Handwriting;
}

QVirtualKeyboardTrace *MyScriptInputMethod::traceBegin(
        int traceId, QVirtualKeyboardInputEngine::PatternRecognitionMode patternRecognitionMode,
        const QVariantMap &traceCaptureDeviceInfo, const QVariantMap &traceScreenInfo)
{
    Q_D(MyScriptInputMethod);

    return d->traceBegin(traceId, patternRecognitionMode, traceCaptureDeviceInfo, traceScreenInfo);
}

bool MyScriptInputMethod::traceEnd(QVirtualKeyboardTrace *trace)
{
    Q_D(MyScriptInputMethod);

    d->traceEnd(trace);
    return true;
}

bool MyScriptInputMethod::clickPreeditText(int cursorPosition)
{
    Q_D(MyScriptInputMethod);

    if (d->m_isProcessing)
        return true;

    QVirtualKeyboardInputContext *ic = inputContext();
    if (ic) {
        setPreeditText(ic->preeditText(), true, cursorPosition);
    }

    return true;
}

void MyScriptInputMethod::timerEvent(QTimerEvent *timerEvent)
{
    Q_D(MyScriptInputMethod);

    if (timerEvent->timerId() == d->m_commitTimer) {
        d->commit();
    }
}

void MyScriptInputMethod::setPreeditText(QString label, bool isCommitted, int cursorPosition, int highlightStart, int highlightLength)
{
    Q_D(MyScriptInputMethod);
    qCDebug(qlcVKMyScript) << Q_FUNC_INFO;

    QList<QInputMethodEvent::Attribute> attributes;
    QBrush foreground = isCommitted ? QBrush(Qt::black) : QBrush(Qt::blue);
    QBrush backgroundNormal = QBrush(Qt::white);
    QBrush backgroundHighlight = QBrush(QColor(0x66, 0xCD, 0xAA));

    QTextCharFormat textFormat;
    textFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, label.length(), textFormat));

    int highlightEnd = highlightStart + highlightLength;
    if (highlightLength > 0 && highlightStart <= label.length() && highlightEnd <= label.length()) {

        if (highlightStart > 0) {
            QTextCharFormat normal;
            normal.setBackground(backgroundNormal);
            normal.setForeground(foreground);
            attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, highlightStart, normal));
        }

        QTextCharFormat highlight;
        highlight.setBackground(backgroundHighlight);
        highlight.setForeground(foreground);
        attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, highlightStart, highlightLength, highlight));

        if (highlightEnd < label.length()) {
            QTextCharFormat normal;
            int highlightLength = label.length() - highlightEnd;
            normal.setBackground(backgroundNormal);
            normal.setForeground(foreground);
            attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, highlightEnd, highlightLength, normal));
        }
    } else {

        QTextCharFormat normal;
        normal.setBackground(backgroundNormal);
        normal.setForeground(foreground);
        attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, label.length(), normal));
    }

    d->m_preeditCursorPosition = (cursorPosition != -1) ? cursorPosition : label.length();
    attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, d->m_preeditCursorPosition, 1, QVariant()));

    inputContext()->setPreeditText(label, attributes);
    if (isCommitted) {
        Q_D(MyScriptInputMethod);
        d->updatePreeditTextCursor(d->m_preeditCursorPosition);
    }
}

void MyScriptInputMethod::doGestureAction(const int gestureType, const int gestureCount)
{
    qCDebug(qlcVKMyScript) << Q_FUNC_INFO;

    Q_D(MyScriptInputMethod);
    d->cancelRecognition();

    QVirtualKeyboardInputContext *ic = inputContext();
    if (ic) {
        switch (gestureType) {
        case GESTURE_TYPE_LEFT_TO_RIGHT:
            if (d->m_locale.contains("ar")) {
                ic->inputEngine()->virtualKeyClick(Qt::Key_Backspace, QString(), Qt::NoModifier);
            } else {
                for (int i = 0; i < gestureCount; i++)
                    ic->inputEngine()->virtualKeyClick(Qt::Key_Space, QLatin1String(" "), Qt::NoModifier);
            }
            break;

        case GESTURE_TYPE_RIGHT_TO_LEFT:
            if (d->m_locale.contains("ar")) {
                for (int i = 0; i < gestureCount; i++)
                    ic->inputEngine()->virtualKeyClick(Qt::Key_Space, QLatin1String(" "), Qt::NoModifier);
            } else {
                ic->inputEngine()->virtualKeyClick(Qt::Key_Backspace, QString(), Qt::NoModifier);
            }
            break;

        case GESTURE_TYPE_DOWN_THEN_LEFT:
            if (!d->m_locale.contains("ar"))
                ic->inputEngine()->virtualKeyClick(Qt::Key_Return, QString(), Qt::NoModifier);
            break;

        case GESTURE_TYPE_DOWN_THEN_RIGHT:
            if (d->m_locale.contains("ar"))
                ic->inputEngine()->virtualKeyClick(Qt::Key_Return, QString(), Qt::NoModifier);
            break;
        }
    }
}

MyScriptRecognizeWorker::MyScriptRecognizeWorker(voimEngine *engine, voimRecognizer *recognizer)
    : QObject()
    , m_engine(engine)
    , m_recognizer(recognizer)
    , m_resultLabel(QString())
{
}

void MyScriptRecognizeWorker::manageRecognitionEnded() {
    qCDebug(qlcVKMyScript) << "recognition has been ended (thread)";
    manageRecognitionResult(m_engine, m_recognizer, false);
}

void MyScriptRecognizeWorker::manageRecognitionCommitted() {
    qCDebug(qlcVKMyScript) << "recognition has been committed (thread)";
    manageRecognitionResult(m_engine, m_recognizer, true);
}

void MyScriptRecognizeWorker::manageRecognitionResult(voimEngine *engine, voimRecognizer *recognizer, const bool isCommitted)
{
    Q_ASSERT(engine != nullptr && recognizer != nullptr);

    voimResult *result = voim_getResult(engine, recognizer, false, true);
    if (result != NULL) {

        QString resultLabel = "";
        qCDebug(qlcVKMyScript) << ">>> recognition result";

        QStringList wordCandidates;
        QString word;
        int wordIndex = -1;

        int gestureType = GESTURE_TYPE_NONE;
        int gestureCount = 1;

        emit clearItem();

        int itemCount = voim_getItemCount(engine, result);
        for (int itemIndex = 0; itemIndex < itemCount; itemIndex++) {

            wordCandidates.clear();
            word = QString();
            wordIndex = -1;

            qCDebug(qlcVKMyScript) << "    * item #" << itemIndex << " of " << itemCount;

            int candidateCount = voim_getItemCandidateCount(engine, result, itemIndex);
            for (int candidateIndex = 0; candidateIndex < candidateCount; candidateIndex++) {

                int length = voim_getItemCandidateLabel(engine, result, itemIndex, candidateIndex, nullptr, 0, "UTF-8");

                if (length > 0) {

                    std::vector<char> bytes(length + 1);
                    char *temp = bytes.data();

                    voim_getItemCandidateLabel(engine, result, itemIndex, candidateIndex, temp, length, "UTF-8");
#ifdef SENSITIVE_DEBUG
                    float score = voim_getItemCandidateScore(engine, result, itemIndex, candidateIndex);
                    qCDebug(qlcVKMyScript) << "      - candidate #" << candidateIndex << " of " << candidateCount << " :" << temp << "(" << score << ")";
#endif

                    QString label = QString::fromUtf8(temp);

                    if (candidateIndex == 0) {
                        gestureType = isGesture(label);

                        if (gestureType == GESTURE_TYPE_NONE) {
                            resultLabel += label;
                        } else {
                            gestureCount = label.length() / QString::fromStdString(GESTURE_STRING_LEFT_TO_RIGHT).length();
                            break;
                        }
                    }

                    if (isGesture(label) == GESTURE_TYPE_NONE) {
                        if (wordIndex == -1) {
                            word = label;
                            wordIndex = candidateIndex;
                        }

                        wordCandidates << label;
                    }
                }
            }

            if (gestureType != GESTURE_TYPE_NONE)
                break;

            emit newItem(itemIndex, wordIndex, wordCandidates);
        }

        voim_destroyResult(m_engine, result);

        if (gestureType != GESTURE_TYPE_NONE) {
            emit clearTraces();
            emit gestureDetected(gestureType, gestureCount);
        } else {
            m_resultLabel = resultLabel;
            emit preeditChanged(m_resultLabel, false);
            emit newCandidates(wordCandidates, word, wordIndex);
        }

    } else if (result == NULL && voim_getError(m_engine) != VOIM_EC_NO_ERROR) {
        qCCritical(qlcVKMyScript) << "voim_getResult() failed -" << getVoimErrorMessage(voim_getError(m_engine));
    }

    emit recognitionEnded();

    if (isCommitted) {
        emit clearTraces();
        emit preeditChanged(m_resultLabel, true);
        emit recognitionCommitted();
    }
}

int MyScriptRecognizeWorker::isGesture(const QString label)
{
    if (label.length() < 2)
        return GESTURE_TYPE_NONE;

    if (label.contains(QString::fromUtf8(GESTURE_STRING_RIGHT_TO_LEFT)))
        return GESTURE_TYPE_RIGHT_TO_LEFT;

    if (label.contains(QString::fromUtf8(GESTURE_STRING_LEFT_TO_RIGHT)))
        return GESTURE_TYPE_LEFT_TO_RIGHT;

    if (label.contains(QString::fromUtf8(GESTURE_STRING_DOWN_THEN_LEFT)))
        return GESTURE_TYPE_DOWN_THEN_LEFT;

    if (label.contains(QString::fromUtf8(GESTURE_STRING_DOWN_THEN_RIGHT)))
        return GESTURE_TYPE_DOWN_THEN_RIGHT;

    return GESTURE_TYPE_NONE;
}

MyScriptRecognizeController::MyScriptRecognizeController(MyScriptInputMethodPrivate *d_, voimEngine *engine, voimRecognizer *recognizer)
    : QObject()
    , d(d_)
{
    MyScriptRecognizeWorker *worker = new MyScriptRecognizeWorker(engine, recognizer);
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &MyScriptRecognizeController::recognitionEnded, worker, &MyScriptRecognizeWorker::manageRecognitionEnded);
    connect(this, &MyScriptRecognizeController::recognitionCommitted, worker, &MyScriptRecognizeWorker::manageRecognitionCommitted);
    connect(worker, &MyScriptRecognizeWorker::recognitionEnded, this, &MyScriptRecognizeController::handleRecognitionEnded);
    connect(worker, &MyScriptRecognizeWorker::recognitionCommitted, this, &MyScriptRecognizeController::handleRecognitionCommitted);
    connect(worker, &MyScriptRecognizeWorker::gestureDetected, this, &MyScriptRecognizeController::handelGestureDetected);
    connect(worker, &MyScriptRecognizeWorker::preeditChanged, this, &MyScriptRecognizeController::handlePreeditChanged);
    connect(worker, &MyScriptRecognizeWorker::clearItem, this, &MyScriptRecognizeController::handleClearItem);
    connect(worker, &MyScriptRecognizeWorker::newItem, this, &MyScriptRecognizeController::handleNewItem);
    connect(worker, &MyScriptRecognizeWorker::newCandidates, this, &MyScriptRecognizeController::handleNewCandidates);
    connect(worker, &MyScriptRecognizeWorker::clearTraces, this, &MyScriptRecognizeController::handleClearTraces);
    workerThread.start();
}

MyScriptRecognizeController::~MyScriptRecognizeController()
{
    workerThread.quit();
    workerThread.wait();
}

void MyScriptRecognizeController::emitRecognitionEnded()
{
    emit recognitionEnded();
}

void MyScriptRecognizeController::emitRecognitionCommitted()
{
    emit recognitionCommitted();
}

void MyScriptRecognizeController::handleRecognitionEnded()
{
    qCDebug(qlcVKMyScript) << Q_FUNC_INFO;
    d->m_onManagingResult = false;
}

void MyScriptRecognizeController::handleRecognitionCommitted()
{
    qCDebug(qlcVKMyScript) << Q_FUNC_INFO;
    d->m_isProcessing = false;
}

void MyScriptRecognizeController::handelGestureDetected(const int gestureType, const int gestureCount)
{
    qCDebug(qlcVKMyScript) << Q_FUNC_INFO;
    emit d->q_ptr->gestureDetected(gestureType, gestureCount);
}

void MyScriptRecognizeController::handlePreeditChanged(const QString &preedit, const bool isCommitted)
{
    emit d->q_ptr->preeditTextChanged(preedit, isCommitted, -1, 0, 0);
}

void MyScriptRecognizeController::handleClearItem()
{
    d->clearItems();
}

void MyScriptRecognizeController::handleNewItem(const int itemIndex, const int candidateIndex, const QStringList &candidates)
{
    CandidateItem *candidateItem = new CandidateItem;

    candidateItem->candidateIndex = candidateIndex;
    candidateItem->candidates = candidates;

    d->m_items.push_back(std::make_pair(itemIndex, candidateItem));

    d->m_itemIndex = itemIndex;
}

void MyScriptRecognizeController::handleNewCandidates(const QStringList &candidates, const QString &word, int wordIndex)
{
    d->wordCandidates = candidates;
    d->word = word;
    d->wordIndex = wordIndex;

    if (!word.isEmpty() && word != " " && word != "\u00A0") {
        d->updateCandidateView();
    }
}

void MyScriptRecognizeController::handleClearTraces()
{
    d->clearTraces();
}

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE
