/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#ifndef LIPIINPUTMETHOD_P_H
#define LIPIINPUTMETHOD_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifdef QT_HUNSPELLINPUTMETHOD_LIB
#include <QtHunspellInputMethod/private/hunspellinputmethod_p.h>
#define LipiInputMethodBase HunspellInputMethod
#else
#include <QtVirtualKeyboard/qvirtualkeyboardabstractinputmethod.h>
#define LipiInputMethodBase QVirtualKeyboardAbstractInputMethod
#endif

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

class LipiInputMethodPrivate;

class LipiInputMethod : public LipiInputMethodBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LipiInputMethod)
public:
    explicit LipiInputMethod(QObject *parent = nullptr);
    ~LipiInputMethod();

    QList<QVirtualKeyboardInputEngine::InputMode> inputModes(const QString &locale);
    bool setInputMode(const QString &locale, QVirtualKeyboardInputEngine::InputMode inputMode);
    bool setTextCase(QVirtualKeyboardInputEngine::TextCase textCase);

    bool keyEvent(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers);

    void reset();
    void update();

    void selectionListItemSelected(QVirtualKeyboardSelectionListModel::Type type, int index);

    QList<QVirtualKeyboardInputEngine::PatternRecognitionMode> patternRecognitionModes() const;
    QVirtualKeyboardTrace *traceBegin(
            int traceId, QVirtualKeyboardInputEngine::PatternRecognitionMode patternRecognitionMode,
            const QVariantMap &traceCaptureDeviceInfo, const QVariantMap &traceScreenInfo);
    bool traceEnd(QVirtualKeyboardTrace *trace);

protected:
    void timerEvent(QTimerEvent *timerEvent);

protected slots:
    void resultsAvailable(const QVariantList &resultList);

#ifndef QT_HUNSPELLINPUTMETHOD_LIB
private:
    QScopedPointer<LipiInputMethodPrivate> d_ptr;
#endif
};

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE

#endif
