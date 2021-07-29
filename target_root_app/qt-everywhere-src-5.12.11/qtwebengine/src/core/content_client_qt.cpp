/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "content_client_qt.h"

#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/version.h"
#include "content/public/common/cdm_info.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/user_agent.h"
#include "media/base/media_switches.h"
#include "media/base/video_codecs.h"
#include "media/media_buildflags.h"
#include "ui/base/layout.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"

#include "net/qrc_protocol_handler_qt.h"
#include "type_conversion.h"

#include <QCoreApplication>
#include <QFile>
#include <QLibraryInfo>
#include <QString>

#if BUILDFLAG(ENABLE_LIBRARY_CDMS)
#include "media/cdm/cdm_paths.h"  // nogncheck
#include "third_party/widevine/cdm/widevine_cdm_common.h"
#include "widevine_cdm_version.h"  // In SHARED_INTERMEDIATE_DIR.
#define WIDEVINE_CDM_AVAILABLE
#if defined(WIDEVINE_CDM_AVAILABLE) && !defined(WIDEVINE_CDM_IS_COMPONENT)
#define WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT
namespace switches {
const char kCdmWidevinePath[] = "widevine-path";
}
// File name of the CDM on different platforms.
const char kWidevineCdmFileName[] =
#if defined(OS_MACOSX)
    "widevinecdm.plugin";
#elif defined(OS_WIN)
    "widevinecdm.dll";
#else  // OS_LINUX, etc.
    "libwidevinecdm.so";
#endif
#endif

static QString webenginePluginsPath()
{
    // Look for plugins in /plugins/webengine or application dir.
    static bool initialized = false;
    static QString potentialPluginsPath = QLibraryInfo::location(QLibraryInfo::PluginsPath) % QLatin1String("/webengine");
    if (!initialized) {
        initialized = true;
        if (!QFileInfo::exists(potentialPluginsPath))
            potentialPluginsPath = QCoreApplication::applicationDirPath();
    }
    return potentialPluginsPath;
}
#endif  // BUILDFLAG(ENABLE_LIBRARY_CDMS)


#if defined(Q_OS_WIN)
#include <shlobj.h>
static QString getLocalAppDataDir()
{
    QString result;
    wchar_t path[MAX_PATH];
    if (SHGetSpecialFolderPath(0, path, CSIDL_LOCAL_APPDATA, FALSE))
        result = QDir::fromNativeSeparators(QString::fromWCharArray(path));
    return result;
}
#endif

#if QT_CONFIG(webengine_pepper_plugins)

// The plugin logic is based on chrome/common/chrome_content_client.cc:
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "content/public/common/pepper_plugin_info.h"
#include "ppapi/shared_impl/ppapi_permissions.h"

static const int32_t kPepperFlashPermissions = ppapi::PERMISSION_DEV |
                                               ppapi::PERMISSION_PRIVATE |
                                               ppapi::PERMISSION_BYPASS_USER_GESTURE |
                                               ppapi::PERMISSION_FLASH |
                                               ppapi::PERMISSION_SOCKET;

namespace switches {
const char kPpapiFlashPath[]    = "ppapi-flash-path";
const char kPpapiFlashVersion[] = "ppapi-flash-version";
const char kPpapiWidevinePath[] = "ppapi-widevine-path";
}

static QString ppapiPluginsPath()
{
    // Look for plugins in /plugins/ppapi or application dir.
    static bool initialized = false;
    static QString potentialPluginsPath = QLibraryInfo::location(QLibraryInfo::PluginsPath) % QLatin1String("/ppapi");
    if (!initialized) {
        initialized = true;
        if (!QFileInfo::exists(potentialPluginsPath))
            potentialPluginsPath = QCoreApplication::applicationDirPath();
    }
    return potentialPluginsPath;
}


content::PepperPluginInfo CreatePepperFlashInfo(const base::FilePath& path, const std::string& version)
{
    content::PepperPluginInfo plugin;

    plugin.is_out_of_process = true;
    plugin.name = content::kFlashPluginName;
    plugin.path = path;
    plugin.permissions = kPepperFlashPermissions;

    std::vector<std::string> flash_version_numbers = base::SplitString(version, ".", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
    if (flash_version_numbers.size() < 1)
        flash_version_numbers.push_back("11");
    else if (flash_version_numbers[0].empty())
        flash_version_numbers[0] = "11";
    if (flash_version_numbers.size() < 2)
        flash_version_numbers.push_back("2");
    if (flash_version_numbers.size() < 3)
        flash_version_numbers.push_back("999");
    if (flash_version_numbers.size() < 4)
        flash_version_numbers.push_back("999");

    // E.g., "Shockwave Flash 10.2 r154":
    plugin.description = plugin.name + " " + flash_version_numbers[0] + "." + flash_version_numbers[1] + " r" + flash_version_numbers[2];
    plugin.version = base::JoinString(flash_version_numbers, ".");
    content::WebPluginMimeType swf_mime_type(content::kFlashPluginSwfMimeType,
                                             content::kFlashPluginSwfExtension,
                                             content::kFlashPluginSwfDescription);
    plugin.mime_types.push_back(swf_mime_type);
    content::WebPluginMimeType spl_mime_type(content::kFlashPluginSplMimeType,
                                             content::kFlashPluginSplExtension,
                                             content::kFlashPluginSplDescription);
    plugin.mime_types.push_back(spl_mime_type);

    return plugin;
}

void AddPepperFlashFromSystem(std::vector<content::PepperPluginInfo>* plugins)
{
    QStringList pluginPaths;
#if defined(Q_OS_WIN)
    QString winDir = QDir::fromNativeSeparators(qgetenv("WINDIR"));
    if (winDir.isEmpty())
        winDir = QString::fromLatin1("C:/Windows");
    QDir pluginDir(winDir + "/System32/Macromed/Flash");
    pluginDir.setFilter(QDir::Files);
    const QFileInfoList infos = pluginDir.entryInfoList(QStringList("pepflashplayer*.dll"));
    for (const QFileInfo &info : infos)
        pluginPaths << info.absoluteFilePath();
    pluginPaths << ppapiPluginsPath() + QStringLiteral("/pepflashplayer.dll");
#endif
#if defined(Q_OS_OSX)
    pluginPaths << "/Library/Internet Plug-Ins/PepperFlashPlayer/PepperFlashPlayer.plugin"; // System path
    QDir potentialDir(QDir::homePath() + "/Library/Application Support/Google/Chrome/PepperFlash");
    if (potentialDir.exists()) {
        QFileInfoList versionDirs = potentialDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
        for (int i = 0; i < versionDirs.size(); ++i) {
            pluginPaths << versionDirs.at(i).absoluteFilePath() + "/PepperFlashPlayer.plugin";
        }
    }
    pluginPaths << ppapiPluginsPath() + QStringLiteral("/PepperFlashPlayer.plugin");
#endif
#if defined(Q_OS_LINUX)
    pluginPaths << "/opt/google/chrome/PepperFlash/libpepflashplayer.so" // Google Chrome
                << "/usr/lib/pepperflashplugin-nonfree/libpepflashplayer.so" // Ubuntu, package pepperflashplugin-nonfree
                << "/usr/lib/adobe-flashplugin/libpepflashplayer.so" // Ubuntu, package adobe-flashplugin
                << "/usr/lib/PepperFlash/libpepflashplayer.so" // Arch
                << "/usr/lib64/chromium/PepperFlash/libpepflashplayer.so"; // OpenSuSE
    pluginPaths << ppapiPluginsPath() + QStringLiteral("/libpepflashplayer.so");
#endif
    std::string flash_version = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(switches::kPpapiFlashVersion);
    for (auto it = pluginPaths.constBegin(); it != pluginPaths.constEnd(); ++it) {
        if (!QFile::exists(*it))
            continue;
        plugins->push_back(CreatePepperFlashInfo(QtWebEngineCore::toFilePath(*it), flash_version));
    }
}

void AddPepperFlashFromCommandLine(std::vector<content::PepperPluginInfo>* plugins)
{
    const base::CommandLine::StringType flash_path = base::CommandLine::ForCurrentProcess()->GetSwitchValueNative(switches::kPpapiFlashPath);
    if (flash_path.empty() || !QFile::exists(QtWebEngineCore::toQt(flash_path)))
        return;

    // Read pepper flash plugin version from command-line. (e.g. 16.0.0.235)
    std::string flash_version = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(switches::kPpapiFlashVersion);
    plugins->push_back(CreatePepperFlashInfo(base::FilePath(flash_path), flash_version));
}

namespace QtWebEngineCore {

void ContentClientQt::AddPepperPlugins(std::vector<content::PepperPluginInfo>* plugins)
{
    AddPepperFlashFromSystem(plugins);
    AddPepperFlashFromCommandLine(plugins);
}

} // namespace QtWebEngineCore
#endif // QT_CONFIG(webengine_pepper_plugins)

namespace QtWebEngineCore {

#if defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)
static bool IsWidevineAvailable(base::FilePath *cdm_path,
                                content::CdmCapability *capability)
{
    QStringList pluginPaths;
    const base::CommandLine::StringType widevine_argument = base::CommandLine::ForCurrentProcess()->GetSwitchValueNative(switches::kCdmWidevinePath);
    if (!widevine_argument.empty())
        pluginPaths << QtWebEngineCore::toQt(widevine_argument);
    else {
        pluginPaths << webenginePluginsPath() + QStringLiteral("/") + QString::fromLatin1(kWidevineCdmFileName);
#if QT_CONFIG(webengine_pepper_plugins)
        pluginPaths << ppapiPluginsPath() + QStringLiteral("/") + QString::fromLatin1(kWidevineCdmFileName);
#endif
#if defined(Q_OS_OSX)
    QDir potentialWidevineDir("/Applications/Google Chrome.app/Contents/Versions");
    if (potentialWidevineDir.exists()) {
        QFileInfoList widevineVersionDirs = potentialWidevineDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot,
                                                                               QDir::Name | QDir::Reversed);
        const QString library = QLatin1String("/Google Chrome Framework.framework/Versions/A/Libraries/"
                                              "WidevineCdm/_platform_specific/mac_x64/libwidevinecdm.dylib");
        for (const QFileInfo &info : widevineVersionDirs)
            pluginPaths << info.absoluteFilePath() + library;
    }

    QDir oldPotentialWidevineDir(QDir::homePath() + "/Library/Application Support/Google/Chrome/WidevineCDM");
    if (oldPotentialWidevineDir.exists()) {
        QFileInfoList widevineVersionDirs = oldPotentialWidevineDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
        for (int i = 0; i < widevineVersionDirs.size(); ++i) {
            QString versionDirPath(widevineVersionDirs.at(i).absoluteFilePath());
            QString potentialWidevinePluginPath = versionDirPath + "/_platform_specific/mac_x64/" + QString::fromLatin1(kWidevineCdmFileName);
            pluginPaths << potentialWidevinePluginPath;
        }
    }
#elif defined(Q_OS_WIN)
    QDir potentialWidevineDir(getLocalAppDataDir() + "/Google/Chrome/User Data/WidevineCDM");
    if (potentialWidevineDir.exists()) {
        QFileInfoList widevineVersionDirs = potentialWidevineDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
        for (int i = 0; i < widevineVersionDirs.size(); ++i) {
            QString versionDirPath(widevineVersionDirs.at(i).absoluteFilePath());
#ifdef WIN64
            QString potentialWidevinePluginPath = versionDirPath + "/_platform_specific/win_x64/" + QString::fromLatin1(kWidevineCdmFileName);
#else
            QString potentialWidevinePluginPath = versionDirPath + "/_platform_specific/win_x86/" + QString::fromLatin1(kWidevineCdmFileName);
#endif
            pluginPaths << potentialWidevinePluginPath;
        }
    }
#elif defined(Q_OS_LINUX)
        pluginPaths << QStringLiteral("/opt/google/chrome/libwidevinecdm.so") // Google Chrome
                    << QStringLiteral("/usr/lib/chromium/libwidevinecdm.so") // Arch
                    << QStringLiteral("/usr/lib/chromium-browser/libwidevinecdm.so") // Ubuntu/neon
                    << QStringLiteral("/usr/lib64/chromium/libwidevinecdm.so"); // OpenSUSE style
#endif
    }

    for (const QString &pluginPath : qAsConst(pluginPaths)) {
        *cdm_path = QtWebEngineCore::toFilePath(pluginPath);
        if (base::PathExists(*cdm_path)) {
            // Add the supported codecs as if they came from the component manifest.
            // This list must match the CDM that is being bundled with Chrome.
            capability->video_codecs.push_back(media::VideoCodec::kCodecVP8);
            capability->video_codecs.push_back(media::VideoCodec::kCodecVP9);
#if BUILDFLAG(USE_PROPRIETARY_CODECS)
            capability->video_codecs.push_back(media::VideoCodec::kCodecH264);
#endif  // BUILDFLAG(USE_PROPRIETARY_CODECS)

            // Add the supported encryption schemes as if they came from the
            // component manifest. This list must match the CDM that is being
            // bundled with Chrome.
            capability->encryption_schemes.insert(media::EncryptionMode::kCenc);
            capability->encryption_schemes.insert(media::EncryptionMode::kCbcs);

            // Temporary session is always supported.
            capability->session_types.insert(media::CdmSessionType::kTemporary);

            return true;
        }
    }
    return false;
}
#endif  // defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)


void ContentClientQt::AddContentDecryptionModules(std::vector<content::CdmInfo> *cdms,
                                                  std::vector<media::CdmHostFilePath> *cdm_host_file_paths)
{
    Q_UNUSED(cdm_host_file_paths);
    if (cdms) {
#if defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)
        base::FilePath cdm_path;
        content::CdmCapability capability;
        if (IsWidevineAvailable(&cdm_path, &capability)) {
            const base::Version version;
            cdms->push_back(content::CdmInfo(kWidevineCdmDisplayName, kWidevineCdmGuid, version, cdm_path,
                                             kWidevineCdmFileSystemId, std::move(capability),
                                             kWidevineKeySystem, false));
        }
#endif  // defined(WIDEVINE_CDM_AVAILABLE_NOT_COMPONENT)

#if BUILDFLAG(ENABLE_LIBRARY_CDMS)
        // Register Clear Key CDM if specified in command line.
        base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
        base::FilePath clear_key_cdm_path = command_line->GetSwitchValuePath(switches::kClearKeyCdmPathForTesting);
        if (!clear_key_cdm_path.empty() && base::PathExists(clear_key_cdm_path)) {
            // TODO(crbug.com/764480): Remove these after we have a central place for
            // External Clear Key (ECK) related information.
            // Normal External Clear Key key system.
            const char kExternalClearKeyKeySystem[] = "org.chromium.externalclearkey";
            // A variant of ECK key system that has a different GUID.
            const char kExternalClearKeyDifferentGuidTestKeySystem[] =
                    "org.chromium.externalclearkey.differentguid";

            // Supported codecs are hard-coded in ExternalClearKeyProperties.
            content::CdmCapability capability(
                {}, {media::EncryptionMode::kCenc, media::EncryptionMode::kCbcs},
                {media::CdmSessionType::kTemporary,
                 media::CdmSessionType::kPersistentLicense},
                {});

            // Register kExternalClearKeyDifferentGuidTestKeySystem first separately.
            // Otherwise, it'll be treated as a sub-key-system of normal
            // kExternalClearKeyKeySystem. See MultipleCdmTypes test in
            // ECKEncryptedMediaTest.
            cdms->push_back(content::CdmInfo(media::kClearKeyCdmDisplayName, media::kClearKeyCdmDifferentGuid,
                                             base::Version("0.1.0.0"), clear_key_cdm_path,
                                             media::kClearKeyCdmFileSystemId, capability,
                                             kExternalClearKeyDifferentGuidTestKeySystem, false));

            // Supported codecs are hard-coded in ExternalClearKeyProperties.
            cdms->push_back(content::CdmInfo(media::kClearKeyCdmDisplayName, media::kClearKeyCdmGuid,
                                             base::Version("0.1.0.0"), clear_key_cdm_path,
                                             media::kClearKeyCdmFileSystemId, capability,
                                             kExternalClearKeyKeySystem, true));
        }
#endif  // BUILDFLAG(ENABLE_LIBRARY_CDMS)
    }
}

std::string ContentClientQt::getUserAgent()
{
    // Mention the Chromium version we're based on to get passed stupid UA-string-based feature detection (several WebRTC demos need this)
    return content::BuildUserAgentFromProduct("QtWebEngine/" QTWEBENGINECORE_VERSION_STR " Chrome/" CHROMIUM_VERSION);
}

base::StringPiece ContentClientQt::GetDataResource(int resource_id, ui::ScaleFactor scale_factor) const {
    return ui::ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(resource_id, scale_factor);
}

base::RefCountedMemory *ContentClientQt::GetDataResourceBytes(int resource_id) const
{
    return ui::ResourceBundle::GetSharedInstance().LoadDataResourceBytes(resource_id);
}

base::string16 ContentClientQt::GetLocalizedString(int message_id) const
{
    return l10n_util::GetStringUTF16(message_id);
}

std::string ContentClientQt::GetProduct() const
{
    QString productName(qApp->applicationName() % '/' % qApp->applicationVersion());
    return productName.toStdString();
}

} // namespace QtWebEngineCore
