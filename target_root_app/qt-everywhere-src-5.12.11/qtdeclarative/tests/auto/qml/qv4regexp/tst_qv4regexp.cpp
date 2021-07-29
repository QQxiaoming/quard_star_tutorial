/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
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
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qtest.h>
#include <QtQml/qjsengine.h>

class tst_qv4regexp : public QObject
{
    Q_OBJECT

private slots:
    void catchJitFail();
};

void tst_qv4regexp::catchJitFail()
{
    QJSEngine engine;
    QJSValue result = engine.evaluate(QLatin1String(
            "var prevString = \" ok\";"
            "var r = /^(\\s*)(([\\)\\]}]?\\s*)*([\\)\\]]\\s*))?;/.exec(prevString);"
            "r === null;"), QLatin1String("regexptest.js"));
    QVERIFY(result.isBool());
    QVERIFY(result.toBool());
}

QTEST_MAIN(tst_qv4regexp)

#include "tst_qv4regexp.moc"
