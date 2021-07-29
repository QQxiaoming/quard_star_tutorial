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

#include "web_engine_context.h"

#include <math.h>

#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include "base/threading/thread_restrictions.h"
#include "cc/base/switches.h"
#if QT_CONFIG(webengine_printing_and_pdf)
#include "chrome/browser/printing/print_job_manager.h"
#endif
#include "components/viz/common/features.h"
#include "components/web_cache/browser/web_cache_manager.h"
#include "content/browser/devtools/devtools_http_handler.h"
#include "content/browser/gpu/gpu_main_thread_factory.h"
#include "content/browser/renderer_host/render_process_host_impl.h"
#include "content/browser/utility_process_host.h"
#include "content/gpu/in_process_gpu_thread.h"
#include "content/public/app/content_main.h"
#include "content/public/app/content_main_runner.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/browser/plugin_service.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/common/content_features.h"
#include "content/public/common/content_paths.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/renderer/in_process_renderer_thread.h"
#include "content/utility/in_process_utility_thread.h"
#include "gpu/command_buffer/service/gpu_switches.h"
#include "gpu/ipc/host/gpu_switches.h"
#include "media/audio/audio_manager.h"
#include "media/base/media_switches.h"
#include "mojo/core/embedder/embedder.h"
#include "net/base/port_util.h"
#include "ppapi/buildflags/buildflags.h"
#include "services/service_manager/sandbox/switches.h"
#include "services/resource_coordinator/public/cpp/resource_coordinator_features.h"
#include "ui/events/event_switches.h"
#include "ui/native_theme/native_theme_features.h"
#include "ui/gl/gl_switches.h"
#if defined(OS_WIN)
#include "sandbox/win/src/sandbox_types.h"
#include "content/public/app/sandbox_helper_win.h"
#endif // OS_WIN

#include "api/qwebengineurlscheme.h"
#include "profile_adapter.h"
#include "content_browser_client_qt.h"
#include "content_client_qt.h"
#include "content_main_delegate_qt.h"
#include "devtools_manager_delegate_qt.h"
#include "media_capture_devices_dispatcher.h"
#include "net/webui_controller_factory_qt.h"
#include "type_conversion.h"
#include "ozone/gl_context_qt.h"
#include "web_engine_library_info.h"

#include <QFileInfo>
#include <QGuiApplication>
#include <QOffscreenSurface>
#ifndef QT_NO_OPENGL
# include <QOpenGLContext>
#endif
#include <QQuickWindow>
#include <QStringList>
#include <QSurfaceFormat>
#include <QVector>
#include <qpa/qplatformnativeinterface.h>

using namespace QtWebEngineCore;

#ifndef QT_NO_OPENGL
QT_BEGIN_NAMESPACE
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();
QT_END_NAMESPACE
#endif

namespace {

#ifndef QT_NO_OPENGL
bool usingANGLE()
{
#if defined(Q_OS_WIN)
    if (qt_gl_global_share_context())
        return qt_gl_global_share_context()->isOpenGLES();
    return QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES;
#else
    return false;
#endif
}

bool usingQtQuick2DRenderer()
{
    const QStringList args = QGuiApplication::arguments();
    QString device;
    for (int index = 0; index < args.count(); ++index) {
        if (args.at(index).startsWith(QLatin1String("--device="))) {
            device = args.at(index).mid(9);
            break;
        }
    }

    if (device.isEmpty())
        device = QQuickWindow::sceneGraphBackend();
    if (device.isEmpty())
        device = QString::fromLocal8Bit(qgetenv("QT_QUICK_BACKEND"));
    if (device.isEmpty())
        device = QString::fromLocal8Bit(qgetenv("QMLSCENE_DEVICE"));
    if (device.isEmpty())
        device = QLatin1String("default");

    // Anything other than the default OpenGL device will need to render in 2D mode.
    return device != QLatin1String("default");
}
#endif //QT_NO_OPENGL
#if QT_CONFIG(webengine_pepper_plugins)
void dummyGetPluginCallback(const std::vector<content::WebPluginInfo>&)
{
}
#endif

} // namespace

namespace QtWebEngineCore {

bool usingSoftwareDynamicGL()
{
    if (QCoreApplication::testAttribute(Qt::AA_UseSoftwareOpenGL))
        return true;
#if defined(Q_OS_WIN) && !defined(QT_NO_OPENGL)
    HMODULE handle = static_cast<HMODULE>(QOpenGLContext::openGLModuleHandle());
    wchar_t path[MAX_PATH];
    DWORD size = GetModuleFileName(handle, path, MAX_PATH);
    QFileInfo openGLModule(QString::fromWCharArray(path, size));
    return openGLModule.fileName() == QLatin1String("opengl32sw.dll");
#else
    return false;
#endif
}

scoped_refptr<QtWebEngineCore::WebEngineContext> WebEngineContext::m_handle;
bool WebEngineContext::m_destroyed = false;

void WebEngineContext::destroyProfileAdapter()
{
    if (content::RenderProcessHost::run_renderer_in_process()) {
        Q_ASSERT(m_profileAdapters.count() == 1);
        // this might be default profile
        m_defaultProfileAdapter.release();
        delete m_profileAdapters.first();
    }
}

void WebEngineContext::addProfileAdapter(ProfileAdapter *profileAdapter)
{
    Q_ASSERT(!m_profileAdapters.contains(profileAdapter));
    const QString path = profileAdapter->dataPath();
    if (!path.isEmpty()) {
        for (auto profileAdapter : m_profileAdapters) {
            if (profileAdapter->dataPath() == path) {
                // QTBUG-66068
                qWarning("Using the same data path for profile, may corrupt the data.");
                break;
            }
        }
    }

    if (content::RenderProcessHost::run_renderer_in_process() &&
            !m_profileAdapters.isEmpty()) {
        qFatal("Single mode supports only single profile.");
    }
    m_profileAdapters.append(profileAdapter);
}

void WebEngineContext::removeProfileAdapter(ProfileAdapter *profileAdapter)
{
    m_profileAdapters.removeAll(profileAdapter);
}

void WebEngineContext::destroy()
{
    if (m_devtoolsServer)
        m_devtoolsServer->stop();
    base::MessagePump::Delegate *delegate =
            static_cast<base::MessageLoop *>(m_runLoop->delegate_);
    // Flush the UI message loop before quitting.
    while (delegate->DoWork()) { }

#if QT_CONFIG(webengine_printing_and_pdf)
    // Kill print job manager early as it has a content::NotificationRegistrar
    m_printJobManager.reset();
#endif

    // Delete the global object and thus custom profiles
    // In case of single process ~RenderProcessHostImpl (there is only one instance)
    // is called expliclty by BrowserMainLoop::ShutdownThreadsAndCleanUp and requires browser context.
    // therefore delete browser context on PostMainMessageLoopRun.
    if (!content::RenderProcessHost::run_renderer_in_process()) {
        m_defaultProfileAdapter.reset();
        m_globalQObject.reset();
        while (m_profileAdapters.count())
            delete m_profileAdapters.first();
    } else {
        m_globalQObject.reset();
    }

    // Handle any events posted by browser-context shutdown.
    // This should deliver all nessesery calls of DeleteSoon from PostTask
    while (delegate->DoWork()) { }

    GLContextHelper::destroy();
    m_devtoolsServer.reset();
    m_runLoop->AfterRun();

    // Destroy the main runner, this stops main message loop
    m_browserRunner.reset();

    // Destroying content-runner will force Chromium at_exit calls to run, and
    // reap child processes.
    m_contentRunner.reset();

    // Drop the false reference.
    m_handle->Release();
}

WebEngineContext::~WebEngineContext()
{
    // WebEngineContext::destroy() must be called before we are deleted
    Q_ASSERT(!m_globalQObject);
    Q_ASSERT(!m_devtoolsServer);
    Q_ASSERT(!m_browserRunner);
    Q_ASSERT(m_profileAdapters.isEmpty());
}

WebEngineContext *WebEngineContext::current()
{
    if (m_destroyed)
        return nullptr;
    if (!m_handle.get()) {
        m_handle = new WebEngineContext();
        // Make sure that we ramp down Chromium before QApplication destroys its X connection, etc.
        qAddPostRoutine(WebEngineContext::destroyContextPostRoutine);
        // Add a false reference so there is no race between unreferencing m_handle and a global QApplication.
        m_handle->AddRef();
    }
    return m_handle.get();
}

ProfileAdapter *WebEngineContext::createDefaultProfileAdapter()
{
    Q_ASSERT(!m_destroyed);
    if (!m_defaultProfileAdapter)
        m_defaultProfileAdapter.reset(new ProfileAdapter(QStringLiteral("Default")));
    return m_defaultProfileAdapter.get();
}

ProfileAdapter *WebEngineContext::defaultProfileAdapter()
{
    return m_defaultProfileAdapter.get();
}

QObject *WebEngineContext::globalQObject()
{
    return m_globalQObject.get();
}

void WebEngineContext::destroyContextPostRoutine()
{
    // Destroy WebEngineContext before its static pointer is zeroed and destructor called.
    // Before destroying MessageLoop via destroying BrowserMainRunner destructor
    // WebEngineContext's pointer is used.
   m_handle->destroy();
#if !defined(NDEBUG)
    if (!m_handle->HasOneRef())
        qWarning("WebEngineContext leaked on exit, likely due to leaked WebEngine View or Page");
#endif
    m_handle = nullptr;
    m_destroyed = true;
}

#ifndef CHROMIUM_VERSION
#error Chromium version should be defined at gyp-time. Something must have gone wrong
#define CHROMIUM_VERSION // This is solely to keep Qt Creator happy.
#endif

const static char kChromiumFlagsEnv[] = "QTWEBENGINE_CHROMIUM_FLAGS";
const static char kDisableSandboxEnv[] = "QTWEBENGINE_DISABLE_SANDBOX";

static void appendToFeatureSwitch(base::CommandLine *commandLine, const char *featureSwitch, const char *feature)
{
    if (!commandLine->HasSwitch(featureSwitch)) {
        commandLine->AppendSwitchASCII(featureSwitch, feature);
    } else {
        std::string featureList = commandLine->GetSwitchValueASCII(featureSwitch);
        featureList = featureList + "," + feature;
        commandLine->AppendSwitchASCII(featureSwitch, featureList);
    }
}

WebEngineContext::WebEngineContext()
    : m_mainDelegate(new ContentMainDelegateQt)
    , m_globalQObject(new QObject())
{
    base::TaskScheduler::Create("Browser");
    m_contentRunner.reset(content::ContentMainRunner::Create());
    m_browserRunner.reset(content::BrowserMainRunner::Create());
#ifdef Q_OS_LINUX
    // Call qputenv before BrowserMainRunnerImpl::Initialize is called.
    // http://crbug.com/245466
    qputenv("force_s3tc_enable", "true");
#endif

    if (QWebEngineUrlScheme::schemeByName(QByteArrayLiteral("qrc")) == QWebEngineUrlScheme()) {
        // User might have registered "qrc" already with different options.
        QWebEngineUrlScheme qrcScheme(QByteArrayLiteral("qrc"));
        qrcScheme.setFlags(QWebEngineUrlScheme::SecureScheme
                           | QWebEngineUrlScheme::LocalAccessAllowed
                           | QWebEngineUrlScheme::ViewSourceAllowed);
        QWebEngineUrlScheme::registerScheme(qrcScheme);
    }

    QWebEngineUrlScheme::lockSchemes();

    // Allow us to inject javascript like any webview toolkit.
    content::RenderFrameHost::AllowInjectingJavaScriptForAndroidWebView();

    base::CommandLine::CreateEmpty();
    base::CommandLine* parsedCommandLine = base::CommandLine::ForCurrentProcess();
    QStringList appArgs = QCoreApplication::arguments();
    if (qEnvironmentVariableIsSet(kChromiumFlagsEnv)) {
        appArgs = appArgs.mid(0, 1); // Take application name and drop the rest
        appArgs.append(QString::fromLocal8Bit(qgetenv(kChromiumFlagsEnv)).split(' '));
    }

    bool enableWebGLSoftwareRendering =
            appArgs.removeAll(QStringLiteral("--enable-webgl-software-rendering"));

    bool useEmbeddedSwitches = false;
#if defined(QTWEBENGINE_EMBEDDED_SWITCHES)
    useEmbeddedSwitches = !appArgs.removeAll(QStringLiteral("--disable-embedded-switches"));
#else
    useEmbeddedSwitches = appArgs.removeAll(QStringLiteral("--enable-embedded-switches"));
#endif
    base::CommandLine::StringVector argv;
    argv.resize(appArgs.size());
#if defined(Q_OS_WIN)
    for (int i = 0; i < appArgs.size(); ++i)
        argv[i] = toString16(appArgs[i]);
#else
    for (int i = 0; i < appArgs.size(); ++i)
        argv[i] = appArgs[i].toStdString();
#endif
    parsedCommandLine->InitFromArgv(argv);

    parsedCommandLine->AppendSwitchPath(switches::kBrowserSubprocessPath, WebEngineLibraryInfo::getPath(content::CHILD_PROCESS_EXE));

    // Enable sandboxing on OS X and Linux (Desktop / Embedded) by default.
    bool disable_sandbox = qEnvironmentVariableIsSet(kDisableSandboxEnv);
    if (!disable_sandbox) {
#if defined(Q_OS_WIN)
        parsedCommandLine->AppendSwitch(service_manager::switches::kNoSandbox);
#elif defined(Q_OS_LINUX)
        parsedCommandLine->AppendSwitch(service_manager::switches::kDisableSetuidSandbox);
#endif
    } else {
        parsedCommandLine->AppendSwitch(service_manager::switches::kNoSandbox);
        qInfo() << "Sandboxing disabled by user.";
    }

    parsedCommandLine->AppendSwitch(switches::kEnableThreadedCompositing);
    // These are currently only default on OS X, and we don't support them:
    parsedCommandLine->AppendSwitch(switches::kDisableZeroCopy);
    parsedCommandLine->AppendSwitch(switches::kDisableGpuMemoryBufferCompositorResources);

    // Enabled on OS X and Linux but currently not working. It worked in 5.7 on OS X.
    parsedCommandLine->AppendSwitch(switches::kDisableGpuMemoryBufferVideoFrames);

#if defined(Q_OS_MACOS)
    // Accelerated decoding currently does not work on macOS due to issues with OpenGL Rectangle
    // texture support. See QTBUG-60002.
    parsedCommandLine->AppendSwitch(switches::kDisableAcceleratedVideoDecode);
    // Same problem with Pepper using OpenGL images.
    parsedCommandLine->AppendSwitch(switches::kDisablePepper3DImageChromium);
    // Same problem with select popups.
    parsedCommandLine->AppendSwitch(switches::kDisableNativeGpuMemoryBuffers);
    // SandboxV2 doesn't currently work for us
    appendToFeatureSwitch(parsedCommandLine, switches::kDisableFeatures, features::kMacV2Sandbox.name);
#endif

#if defined(Q_OS_WIN)
    // This switch is used in Chromium's gl_context_wgl.cc file to determine whether to create
    // an OpenGL Core Profile context. If the switch is not set, it would always try to create a
    // Core Profile context, even if Qt uses a legacy profile, which causes
    // "Could not share GL contexts" warnings, because it's not possible to share between Core and
    // legacy profiles.
    // Given that Core profile is not currently supported on Windows anyway, pass this switch to
    // get rid of the warnings.
    parsedCommandLine->AppendSwitch(switches::kDisableES3GLContext);
#endif
    // Needed to allow navigations within pages that were set using setHtml(). One example is
    // tst_QWebEnginePage::acceptNavigationRequest.
    // This is deprecated behavior, and will be removed in a future Chromium version, as per
    // upstream Chromium commit ba52f56207a4b9d70b34880fbff2352e71a06422.
    appendToFeatureSwitch(parsedCommandLine, switches::kEnableFeatures, features::kAllowContentInitiatedDataUrlNavigations.name);
    // Surface synchronization breaks our current graphics integration (since 65)
    appendToFeatureSwitch(parsedCommandLine, switches::kDisableFeatures, features::kEnableSurfaceSynchronization.name);
    // The video-capture service is not functioning at this moment (since 69)
    appendToFeatureSwitch(parsedCommandLine, switches::kDisableFeatures, features::kMojoVideoCapture.name);
    // We do not yet support the internal video capture API.
    appendToFeatureSwitch(parsedCommandLine, switches::kDisableFeatures, features::kUseVideoCaptureApiForDevToolsSnapshots.name);
    // Qt 5.12 only: The modern media controls are not yet good enough in 69-based,
    // so we stick to the old style
    appendToFeatureSwitch(parsedCommandLine, switches::kDisableFeatures, media::kUseModernMediaControls.name);

    if (useEmbeddedSwitches) {
        // embedded switches are based on the switches for Android, see content/browser/android/content_startup_flags.cc
        appendToFeatureSwitch(parsedCommandLine, switches::kEnableFeatures, features::kOverlayScrollbar.name);
        if (!parsedCommandLine->HasSwitch(switches::kDisablePinch))
            parsedCommandLine->AppendSwitch(switches::kEnablePinch);
        parsedCommandLine->AppendSwitch(switches::kEnableViewport);
        parsedCommandLine->AppendSwitch(switches::kMainFrameResizesAreOrientationChanges);
        parsedCommandLine->AppendSwitch(cc::switches::kDisableCompositedAntialiasing);
    }
    base::FeatureList::InitializeInstance(
        parsedCommandLine->GetSwitchValueASCII(switches::kEnableFeatures),
        parsedCommandLine->GetSwitchValueASCII(switches::kDisableFeatures));

    GLContextHelper::initialize();

    const char *glType = 0;
#ifndef QT_NO_OPENGL

    bool tryGL =
            !usingANGLE()
            && (!usingSoftwareDynamicGL()
                // If user requested WebGL support instead of using Skia rendering to
                // bitmaps, use software rendering via software OpenGL. This might be less
                // performant, but at least provides WebGL support.
                || enableWebGLSoftwareRendering
                )
            && !usingQtQuick2DRenderer();

    if (tryGL) {
        if (qt_gl_global_share_context() && qt_gl_global_share_context()->isValid()) {
            // If the native handle is QEGLNativeContext try to use GL ES/2, if there is no native handle
            // assume we are using wayland and try GL ES/2, and finally Ozone demands GL ES/2 too.
            if (qt_gl_global_share_context()->nativeHandle().isNull()
                || !strcmp(qt_gl_global_share_context()->nativeHandle().typeName(), "QEGLNativeContext"))
            {
                if (qt_gl_global_share_context()->isOpenGLES()) {
                    glType = gl::kGLImplementationEGLName;
                } else {
                    QOpenGLContext context;
                    QSurfaceFormat format;

                    format.setRenderableType(QSurfaceFormat::OpenGL);
                    format.setVersion(2, 0);

                    context.setFormat(format);
                    context.setShareContext(qt_gl_global_share_context());
                    if (context.create()) {
                        QOffscreenSurface surface;

                        surface.setFormat(format);
                        surface.create();

                        if (context.makeCurrent(&surface)) {
                            if (context.hasExtension("GL_ARB_ES2_compatibility"))
                                glType = gl::kGLImplementationEGLName;

                            context.doneCurrent();
                        }

                        surface.destroy();
                    }
                }
            } else {
                if (!qt_gl_global_share_context()->isOpenGLES()) {
                    // Default to Desktop non-Core profile OpenGL.
                    glType = gl::kGLImplementationDesktopName;

                    // Check if Core profile was requested and is supported.
                    QSurfaceFormat globalSharedFormat = qt_gl_global_share_context()->format();
                    if (globalSharedFormat.profile() == QSurfaceFormat::CoreProfile) {
#ifdef Q_OS_MACOS
                        glType = gl::kGLImplementationCoreProfileName;
#else
                        qWarning("An OpenGL Core Profile was requested, but it is not supported "
                                 "on the current platform. Falling back to a non-Core profile. "
                                 "Note that this might cause rendering issues.");
#endif
                    }
                }
            }
            if (qt_gl_global_share_context()->format().profile() == QSurfaceFormat::CompatibilityProfile)
                parsedCommandLine->AppendSwitch(switches::kCreateDefaultGLContext);
        } else {
            qWarning("WebEngineContext used before QtWebEngine::initialize() or OpenGL context creation failed.");
        }
    }
#endif

    if (glType) {
        parsedCommandLine->AppendSwitchASCII(switches::kUseGL, glType);
        parsedCommandLine->AppendSwitch(switches::kInProcessGPU);
        if (enableWebGLSoftwareRendering) {
            parsedCommandLine->AppendSwitch(switches::kDisableGpuRasterization);
            parsedCommandLine->AppendSwitch(switches::kIgnoreGpuBlacklist);
        }
    } else {
        parsedCommandLine->AppendSwitch(switches::kDisableGpu);
    }

    content::UtilityProcessHost::RegisterUtilityMainThreadFactory(content::CreateInProcessUtilityThread);
    content::RenderProcessHostImpl::RegisterRendererMainThreadFactory(content::CreateInProcessRendererThread);
    content::RegisterGpuMainThreadFactory(content::CreateInProcessGpuThread);

    mojo::core::Init();

    content::ContentMainParams contentMainParams(m_mainDelegate.get());
#if defined(OS_WIN)
    sandbox::SandboxInterfaceInfo sandbox_info = {0};
    content::InitializeSandboxInfo(&sandbox_info);
    contentMainParams.sandbox_info = &sandbox_info;
#endif
    m_contentRunner->Initialize(contentMainParams);
    m_browserRunner->Initialize(content::MainFunctionParams(*base::CommandLine::ForCurrentProcess()));

    // Once the MessageLoop has been created, attach a top-level RunLoop.
    m_runLoop.reset(new base::RunLoop);
    m_runLoop->BeforeRun();

    m_devtoolsServer.reset(new DevToolsServerQt());
    m_devtoolsServer->start();
    // Force the initialization of MediaCaptureDevicesDispatcher on the UI
    // thread to avoid a thread check assertion in its constructor when it
    // first gets referenced on the IO thread.
    MediaCaptureDevicesDispatcher::GetInstance();

    // Initialize WebCacheManager here to ensure its subscription to render process creation events.
    web_cache::WebCacheManager::GetInstance();

    base::ThreadRestrictions::SetIOAllowed(true);

    if (parsedCommandLine->HasSwitch(switches::kExplicitlyAllowedPorts)) {
        std::string allowedPorts = parsedCommandLine->GetSwitchValueASCII(switches::kExplicitlyAllowedPorts);
        net::SetExplicitlyAllowedPorts(allowedPorts);
    }

#if defined(OS_LINUX)
    media::AudioManager::SetGlobalAppName(QCoreApplication::applicationName().toStdString());
#endif

#if QT_CONFIG(webengine_pepper_plugins)
    // Creating pepper plugins from the page (which calls PluginService::GetPluginInfoArray)
    // might fail unless the page queried the list of available plugins at least once
    // (which ends up calling PluginService::GetPlugins). Since the plugins list can only
    // be created from the FILE thread, and that GetPluginInfoArray is synchronous, it
    // can't loads plugins synchronously from the IO thread to serve the render process' request
    // and we need to make sure that it happened beforehand.
    content::PluginService::GetInstance()->GetPlugins(base::Bind(&dummyGetPluginCallback));
#endif

#if QT_CONFIG(webengine_printing_and_pdf)
    m_printJobManager.reset(new printing::PrintJobManager());
#endif

    content::WebUIControllerFactory::RegisterFactory(WebUIControllerFactoryQt::GetInstance());
}

#if QT_CONFIG(webengine_printing_and_pdf)
printing::PrintJobManager* WebEngineContext::getPrintJobManager()
{
    return m_printJobManager.get();
}
#endif
} // namespace
