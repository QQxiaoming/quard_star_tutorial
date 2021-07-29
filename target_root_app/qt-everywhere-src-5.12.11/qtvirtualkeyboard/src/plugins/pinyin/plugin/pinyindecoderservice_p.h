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

#ifndef PINYINDECODERSERVICE_P_H
#define PINYINDECODERSERVICE_P_H

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

#include <QObject>

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

class PinyinDecoderService : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PinyinDecoderService)
    explicit PinyinDecoderService(QObject *parent = nullptr);

public:
    ~PinyinDecoderService();

    static PinyinDecoderService *getInstance();

    bool init();
    void setUserDictionary(bool enabled);
    bool isUserDictionaryEnabled() const;
    void setLimits(int maxSpelling, int maxHzsLen);
    int search(const QString &spelling);
    int deleteSearch(int pos, bool isPosInSpellingId, bool clearFixedInThisStep);
    void resetSearch();
    QString pinyinString(bool decoded);
    int pinyinStringLength(bool decoded);
    QVector<int> spellingStartPositions();
    QString candidateAt(int index);
    QList<QString> fetchCandidates(int index, int count, int sentFixedLen);
    int chooceCandidate(int index);
    int cancelLastChoice();
    int fixedLength();
    void flushCache();
    QList<QString> predictionList(const QString &history);

private:
    static QScopedPointer<PinyinDecoderService> _instance;
    bool initDone;
};

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE

#endif // PINYINDECODERSERVICE_P_H
