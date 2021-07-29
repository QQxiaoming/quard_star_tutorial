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

#ifndef T9WRITEDICTIONARY_P_H
#define T9WRITEDICTIONARY_P_H

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

#include <QtGlobal>
#include <QFile>
#include "t9write_p.h"

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

class T9WriteDictionary
{
    Q_DISABLE_COPY(T9WriteDictionary)
public:
    explicit T9WriteDictionary(DECUMA_SESSION *decumaSession, const DECUMA_MEM_FUNCTIONS &memFuncs, bool cjk);
    ~T9WriteDictionary();

    bool load(const QString &fileName);
    bool convert(const DECUMA_SRC_DICTIONARY_INFO &dictionaryInfo);

    QString fileName() const;
    const void *data() const;
    qint64 size() const;
    bool isConverted() const;

private:
    QFile file;
    DECUMA_SESSION *decumaSession;
    const DECUMA_MEM_FUNCTIONS &memFuncs;
    bool cjk;
    void *sourceData;
    qint64 sourceSize;
    void *convertedData;
    DECUMA_UINT32 convertedSize;
};

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE

#endif // T9WRITEDICTIONARY_P_H
