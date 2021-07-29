/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "mainwindow.h"
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>

CookieDialog::CookieDialog(const QNetworkCookie &cookie, QWidget *parent): QDialog(parent)
{
    setupUi(this);
    m_nameLineEdit->setText(cookie.name());
    m_domainLineEdit->setText(cookie.domain());
    m_valueLineEdit->setText(cookie.value());
    m_pathLineEdit->setText(cookie.path());
    m_dateEdit->setDate(cookie.expirationDate().date());
    m_isSecureComboBox->addItem(cookie.isSecure() ? tr("yes") : tr("no"));
    m_isHttpOnlyComboBox->addItem(cookie.isHttpOnly() ? tr("yes") : tr("no"));
    m_addButton->setVisible(false);
    m_cancelButton->setText(tr("Close"));
}

CookieDialog::CookieDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    m_nameLineEdit->setReadOnly(false);
    m_domainLineEdit->setReadOnly(false);
    m_valueLineEdit->setReadOnly(false);
    m_pathLineEdit->setReadOnly(false);
    m_dateEdit->setReadOnly(false);
    m_dateEdit->setDate(QDateTime::currentDateTime().addYears(1).date());
    m_isSecureComboBox->addItem(tr("no"));
    m_isSecureComboBox->addItem(tr("yes"));
    m_isHttpOnlyComboBox->addItem(tr("no"));
    m_isHttpOnlyComboBox->addItem(tr("yes"));
}

QNetworkCookie CookieDialog::cookie()
{
    QNetworkCookie cookie;
    cookie.setDomain(m_domainLineEdit->text());
    cookie.setName(m_nameLineEdit->text().toLatin1());
    cookie.setValue(m_valueLineEdit->text().toLatin1());
    cookie.setExpirationDate(QDateTime(m_dateEdit->date()));
    cookie.setPath(m_pathLineEdit->text());
    cookie.setSecure(m_isSecureComboBox->currentText() == tr("yes"));
    cookie.setHttpOnly(m_isHttpOnlyComboBox->currentText() == tr("yes"));
    return cookie;
}

CookieWidget::CookieWidget(const QNetworkCookie &cookie, QWidget *parent): QWidget(parent)
{
    setupUi(this);
    setAutoFillBackground(true);
    m_nameLabel->setText(cookie.name());
    m_domainLabel->setText(cookie.domain());
    connect(m_viewButton, &QPushButton::clicked, this, &CookieWidget::viewClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &CookieWidget::deleteClicked);
}

void CookieWidget::setHighlighted(bool enabled)
{
    QPalette p = palette();
    p.setColor(backgroundRole(), enabled ? QColor(0xF0, 0xF8, 0xFF) : Qt::white);
    setPalette(p);
}

MainWindow::MainWindow(const QUrl &url) :
    QMainWindow(),
    m_store(nullptr),
    m_layout(new QVBoxLayout)
{
    setupUi(this);
    m_urlLineEdit->setText(url.toString());

    m_layout->addItem(new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    QWidget *w = new QWidget();
    QPalette p = w->palette();
    p.setColor(widget->backgroundRole(), Qt::white);
    w->setPalette(p);
    w->setLayout(m_layout);

    m_scrollArea->setWidget(w);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(m_urlButton, &QPushButton::clicked, this, &MainWindow::handleUrlClicked);
    connect(m_deleteAllButton, &QPushButton::clicked, this, &MainWindow::handleDeleteAllClicked);
    connect(m_newButton, &QPushButton::clicked, this, &MainWindow::handleNewClicked);

    m_store = m_webview->page()->profile()->cookieStore();
    connect(m_store, &QWebEngineCookieStore::cookieAdded, this, &MainWindow::handleCookieAdded);
    m_store->loadAllCookies();
    m_webview->load(url);
}

bool MainWindow::containsCookie(const QNetworkCookie &cookie)
{
    for (auto c: m_cookies) {
        if (c.hasSameIdentifier(cookie))
            return true;
    }
    return false;
}

void MainWindow::handleCookieAdded(const QNetworkCookie &cookie)
{
    // only new cookies
    if (containsCookie(cookie))
        return;

    CookieWidget *widget = new CookieWidget(cookie);
    widget->setHighlighted(m_cookies.count() % 2);
    m_cookies.append(cookie);
    m_layout->insertWidget(0,widget);

    connect(widget, &CookieWidget::deleteClicked, [this, cookie, widget]() {
        m_store->deleteCookie(cookie);
        delete widget;
        m_cookies.removeOne(cookie);
        for (int i = 0; i < m_layout->count() - 1; i++) {
            // fix background colors
            auto widget = qobject_cast<CookieWidget*>(m_layout->itemAt(i)->widget());
            widget->setHighlighted(i % 2);
        }
    });

    connect(widget, &CookieWidget::viewClicked, [cookie]() {
        CookieDialog dialog(cookie);
        dialog.exec();
    });
}

void MainWindow::handleDeleteAllClicked()
{
    m_store->deleteAllCookies();
    for (int i = m_layout->count() - 1; i >= 0; i--)
        delete m_layout->itemAt(i)->widget();
    m_cookies.clear();
}

void MainWindow::handleNewClicked()
{
    CookieDialog dialog;
    if (dialog.exec() == QDialog::Accepted)
        m_store->setCookie(dialog.cookie());
}

void MainWindow::handleUrlClicked()
{
    m_webview->load(QUrl::fromUserInput(m_urlLineEdit->text()));
}
