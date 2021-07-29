/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef BUILD_CONFIG_QT
#define BUILD_CONFIG_QT

#include <QtCore/qglobal.h>
#include <QtWebEngineCore/qtwebenginecore-config.h>
#include <QtWebEngineCore/private/qtwebenginecore-config_p.h>

#include "printing/buildflags/buildflags.h"
#include "components/spellcheck/spellcheck_buildflags.h"
#include "media/media_buildflags.h"
#include "ppapi/buildflags/buildflags.h"

// This is just config sanity check
#if QT_CONFIG(webengine_printing_and_pdf)
#if !BUILDFLAG(ENABLE_PRINTING) || !BUILDFLAG(ENABLE_PRINT_PREVIEW)
#error Config sanity check for webengine_printing_and_pdf failed
#endif
#else
#if BUILDFLAG(ENABLE_PRINTING) || BUILDFLAG(ENABLE_PRINT_PREVIEW)
#error Config sanity check for ENABLE_PRINTING, ENABLE_PRINT_PREVIEW failed
#endif
#endif

#if QT_CONFIG(webengine_spellchecker)
#if !BUILDFLAG(ENABLE_SPELLCHECK)
#error Config sanity check for webengine_spellchecker failed
#endif
#else
#if BUILDFLAG(ENABLE_SPELLCHECK)
#error Config sanity check for ENABLE_SPELLCHECK failed
#endif
#endif

#if QT_CONFIG(webengine_pepper_plugins)
#if !BUILDFLAG(ENABLE_PLUGINS)
#error Config sanity check for webengine_pepper_plugins failed
#endif
#else
#if BUILDFLAG(ENABLE_PLUGINS)
#error Config sanity check for ENABLE_PLUGINS failed
#endif
#endif

#if QT_CONFIG(webengine_webrtc)
#if !BUILDFLAG(ENABLE_WEBRTC)
#error Config sanity check for webengine_webrtc failed
#endif
#else
#if BUILDFLAG(ENABLE_WEBRTC)
#error Config sanity check for ENABLE_WEBRTC failed
#endif
#endif

#if QT_CONFIG(webengine_native_spellchecker)
#if !BUILDFLAG(USE_BROWSER_SPELLCHECKER)
#error Config sanity check for webengine_native_spellchecker failed
#endif
#else
#if BUILDFLAG(USE_BROWSER_SPELLCHECKER)
#error Config sanity check for USE_BROWSER_SPELLCHECKER failed
#endif
#endif

#endif
