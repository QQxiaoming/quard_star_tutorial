/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWebView module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebviewfactory_p.h"
#include "qwebviewplugin_p.h"
#include <private/qfactoryloader_p.h>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QWebViewPluginInterface_iid, QLatin1String("/webview")))

static QString getPluginName()
{
    static const QString name = !qEnvironmentVariableIsEmpty("QT_WEBVIEW_PLUGIN")
                                ? QString::fromLatin1(qgetenv("QT_WEBVIEW_PLUGIN"))
#ifdef Q_OS_MACOS
                                : QStringLiteral("webengine");
#else
                                : QStringLiteral("native");
#endif // Q_OS_MACOS
    return name;
}

class QNullWebView : public QAbstractWebView
{
public:
    void setParentView(QObject *view) override { Q_UNUSED(view); }
    QObject *parentView() const override { return nullptr; }
    void setGeometry(const QRect &geometry) override { Q_UNUSED(geometry); }
    void setVisibility(QWindow::Visibility visibility) override { Q_UNUSED(visibility); }
    void setVisible(bool visible) override { Q_UNUSED(visible); }

    QUrl url() const override { return QUrl(); }
    void setUrl(const QUrl &url) override { Q_UNUSED(url); }
    bool canGoBack() const override { return false; }
    bool canGoForward() const override { return false; }
    QString title() const override { return QString(); }
    int loadProgress() const override { return 0; }
    bool isLoading() const override { return false; }
    void goBack() override { }
    void goForward() override { }
    void stop() override { }
    void reload() override { }
    void loadHtml(const QString &html, const QUrl &baseUrl) override
    { Q_UNUSED(html); Q_UNUSED(baseUrl); }
    void runJavaScriptPrivate(const QString &script, int callbackId) override
    { Q_UNUSED(script); Q_UNUSED(callbackId); }
};

QAbstractWebView *QWebViewFactory::createWebView()
{
    QAbstractWebView *wv = nullptr;
    QWebViewPlugin *plugin = getPlugin();
    if (plugin)
        wv = plugin->create(QStringLiteral("webview"));

    if (!wv || !plugin) {
        qWarning("No WebView plug-in found!");
        wv = new QNullWebView;
    }

    return wv;
}

bool QWebViewFactory::requiresExtraInitializationSteps()
{
    const QString pluginName = getPluginName();
    const int index = pluginName.isEmpty() ? 0 : qMax<int>(0, loader->indexOf(pluginName));

    const auto metaDataList = loader->metaData();
    if (metaDataList.isEmpty())
        return false;

    const auto &pluginMetaData = metaDataList.at(index);
    const auto iid = pluginMetaData.value(QLatin1String("IID"));
    Q_ASSERT(iid == QJsonValue(QLatin1String(QWebViewPluginInterface_iid)));
    const auto metaDataObject = pluginMetaData.value(QLatin1String("MetaData")).toObject();
    const auto it = metaDataObject.find(QLatin1String("RequiresInit"));
    if (it != pluginMetaData.constEnd())
        return it->isBool() ? it->toBool() : false;

    return false;
}

QWebViewPlugin *QWebViewFactory::getPlugin()
{
    // Plugin loading logic:
    // 1. Get plugin name - plugin name is either user specified or "native"
    //    - Exception: macOS, which will default to using "webengine" until the native plugin is matured.
    // 2. If neither a user specified or "default" plugin exists, then the first available is used.
    const QString pluginName = getPluginName();
    const int index = pluginName.isEmpty() ? 0 : qMax<int>(0, loader->indexOf(pluginName));
    return qobject_cast<QWebViewPlugin *>(loader->instance(index));
}

QT_END_NAMESPACE
