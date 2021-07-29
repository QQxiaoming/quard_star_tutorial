/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:FDL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Free Documentation License Usage
** Alternatively, this file may be used under the terms of the GNU Free
** Documentation License version 1.3 as published by the Free Software
** Foundation and appearing in the file included in the packaging of
** this file. Please review the following information to ensure
** the GNU Free Documentation License version 1.3 requirements
** will be met: https://www.gnu.org/licenses/fdl-1.3.html.
** $QT_END_LICENSE$
**
****************************************************************************/

void wrapInFunction()
{

//! [0]
    view->page()->history();
//! [0]


//! [1]
    view->page()->settings();
//! [1]


//! [2]
    view->triggerPageAction(QWebEnginePage::Copy);
//! [2]


//! [3]
    view->page()->triggerAction(QWebEnginePage::Stop);
//! [3]


//! [4]
    view->page()->triggerAction(QWebEnginePage::Back);
//! [4]


//! [5]
    view->page()->triggerAction(QWebEnginePage::Forward);
//! [5]

//! [6]
    view->page()->settings();
//! [6]
}

