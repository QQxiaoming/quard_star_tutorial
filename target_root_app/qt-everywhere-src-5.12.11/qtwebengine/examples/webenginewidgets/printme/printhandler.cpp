/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "printhandler.h"
#include <QEventLoop>
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>
#include <QPrintPreviewDialog>
#include <QWebEnginePage>

PrintHandler::PrintHandler(QObject *parent)
    : QObject(parent)
{

}

void PrintHandler::setPage(QWebEnginePage *page)
{
    Q_ASSERT(!m_page);
    m_page = page;
    connect(m_page, &QWebEnginePage::printRequested, this, &PrintHandler::printPreview);
}

void PrintHandler::print()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, m_page->view());
    if (dialog.exec() != QDialog::Accepted)
        return;
    printDocument(&printer);
}

void PrintHandler::printDocument(QPrinter *printer)
{
    QEventLoop loop;
    bool result;
    auto printPreview = [&](bool success) { result = success; loop.quit(); };
    m_page->print(printer, std::move(printPreview));
    loop.exec();
    if (!result) {
        QPainter painter;
        if (painter.begin(printer)) {
            QFont font = painter.font();
            font.setPixelSize(20);
            painter.setFont(font);
            painter.drawText(QPointF(10,25),
                             QStringLiteral("Could not generate print preview."));

            painter.end();
        }
    }
}

void PrintHandler::printPreview()
{
    if (!m_page)
        return;
    if (m_inPrintPreview)
        return;
    m_inPrintPreview = true;
    QPrinter printer;
    QPrintPreviewDialog preview(&printer, m_page->view());
    connect(&preview, &QPrintPreviewDialog::paintRequested,
            this, &PrintHandler::printDocument);
    preview.exec();
    m_inPrintPreview = false;
}
