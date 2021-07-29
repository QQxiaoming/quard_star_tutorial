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

#include "render_widget_host_view_qt.h"

#include "browser_accessibility_manager_qt.h"
#include "chromium_overrides.h"
#include "common/qt_messages.h"
#include "compositor.h"
#include "qtwebenginecoreglobal_p.h"
#include "render_widget_host_view_qt_delegate.h"
#include "type_conversion.h"
#include "web_contents_adapter_client.h"
#include "web_event_factory.h"

#include "components/viz/common/surfaces/frame_sink_id_allocator.h"
#include "content/browser/accessibility/browser_accessibility_state_impl.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/browser/frame_host/frame_tree.h"
#include "content/browser/renderer_host/render_view_host_impl.h"
#include "content/common/content_switches_internal.h"
#include "content/browser/renderer_host/render_widget_host_input_event_router.h"
#include "content/common/cursors/webcursor.h"
#include "content/common/input_messages.h"
#include "third_party/skia/include/core/SkColor.h"
#include "third_party/blink/public/platform/web_cursor_info.h"
#include "ui/events/blink/blink_event_util.h"
#include "ui/events/event.h"
#include "ui/events/gesture_detection/gesture_provider_config_helper.h"
#include "ui/events/gesture_detection/motion_event.h"
#include "ui/gfx/geometry/size_conversions.h"

#if defined(USE_OZONE)
#include "ui/base/clipboard/scoped_clipboard_writer.h"
#endif

#if defined(USE_AURA)
#include "ui/base/cursor/cursor.h"
#include "ui/base/cursor/cursors_aura.h"
#include "ui/base/resource/resource_bundle.h"
#endif

#include <private/qguiapplication_p.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatformintegration.h>
#include <QEvent>
#include <QFocusEvent>
#include <QGuiApplication>
#include <QInputMethodEvent>
#include <QLoggingCategory>
#include <QTextFormat>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QScreen>
#include <QStyleHints>
#include <QVariant>
#include <QWheelEvent>
#include <QWindow>
#include <QtGui/private/qinputcontrol_p.h>
#include <QtGui/qaccessible.h>

namespace QtWebEngineCore {

enum ImStateFlags {
    TextInputStateUpdated = 1 << 0,
    TextSelectionUpdated = 1 << 1,
    TextSelectionBoundsUpdated = 1 << 2,
    TextSelectionFlags = TextSelectionUpdated | TextSelectionBoundsUpdated,
    AllFlags = TextInputStateUpdated | TextSelectionUpdated | TextSelectionBoundsUpdated
};

static inline ui::LatencyInfo CreateLatencyInfo(const blink::WebInputEvent& event) {
  ui::LatencyInfo latency_info;
  // The latency number should only be added if the timestamp is valid.
  if (!event.TimeStamp().is_null()) {
    latency_info.AddLatencyNumberWithTimestamp(
        ui::INPUT_EVENT_LATENCY_ORIGINAL_COMPONENT,
        event.TimeStamp(),
        1);
  }
  return latency_info;
}

static inline Qt::InputMethodHints toQtInputMethodHints(ui::TextInputType inputType)
{
    switch (inputType) {
    case ui::TEXT_INPUT_TYPE_TEXT:
        return Qt::ImhPreferLowercase;
    case ui::TEXT_INPUT_TYPE_SEARCH:
        return Qt::ImhPreferLowercase | Qt::ImhNoAutoUppercase;
    case ui::TEXT_INPUT_TYPE_PASSWORD:
        return Qt::ImhSensitiveData | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase | Qt::ImhHiddenText;
    case ui::TEXT_INPUT_TYPE_EMAIL:
        return Qt::ImhEmailCharactersOnly;
    case ui::TEXT_INPUT_TYPE_NUMBER:
        return Qt::ImhFormattedNumbersOnly;
    case ui::TEXT_INPUT_TYPE_TELEPHONE:
        return Qt::ImhDialableCharactersOnly;
    case ui::TEXT_INPUT_TYPE_URL:
        return Qt::ImhUrlCharactersOnly | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase;
    case ui::TEXT_INPUT_TYPE_DATE_TIME:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_LOCAL:
    case ui::TEXT_INPUT_TYPE_DATE_TIME_FIELD:
        return Qt::ImhDate | Qt::ImhTime;
    case ui::TEXT_INPUT_TYPE_DATE:
    case ui::TEXT_INPUT_TYPE_MONTH:
    case ui::TEXT_INPUT_TYPE_WEEK:
        return Qt::ImhDate;
    case ui::TEXT_INPUT_TYPE_TIME:
        return Qt::ImhTime;
    case ui::TEXT_INPUT_TYPE_TEXT_AREA:
    case ui::TEXT_INPUT_TYPE_CONTENT_EDITABLE:
        return Qt::ImhMultiLine | Qt::ImhPreferLowercase;
    default:
        return Qt::ImhNone;
    }
}

static inline int firstAvailableId(const QMap<int, int> &map)
{
    ui::BitSet32 usedIds;
    QMap<int, int>::const_iterator end = map.end();
    for (QMap<int, int>::const_iterator it = map.begin(); it != end; ++it)
        usedIds.mark_bit(it.value());
    return usedIds.first_unmarked_bit();
}

static inline ui::GestureProvider::Config QtGestureProviderConfig() {
    ui::GestureProvider::Config config = ui::GetGestureProviderConfig(ui::GestureProviderConfigType::CURRENT_PLATFORM);
    // Causes an assert in CreateWebGestureEventFromGestureEventData and we don't need them in Qt.
    config.gesture_begin_end_types_enabled = false;
    config.gesture_detector_config.swipe_enabled = false;
    config.gesture_detector_config.two_finger_tap_enabled = false;
    return config;
}

static inline bool compareTouchPoints(const QTouchEvent::TouchPoint &lhs, const QTouchEvent::TouchPoint &rhs)
{
    // TouchPointPressed < TouchPointMoved < TouchPointReleased
    return lhs.state() < rhs.state();
}

static inline bool isCommonTextEditShortcut(const QKeyEvent *ke)
{
    return QInputControl::isCommonTextEditShortcut(ke);
}

static uint32_t s_eventId = 0;
class MotionEventQt : public ui::MotionEvent {
public:
    MotionEventQt(const QList<QTouchEvent::TouchPoint> &touchPoints, const base::TimeTicks &eventTime, Action action, const Qt::KeyboardModifiers modifiers, float dpiScale, int index = -1)
        : touchPoints(touchPoints)
        , eventTime(eventTime)
        , action(action)
        , eventId(++s_eventId)
        , flags(flagsFromModifiers(modifiers))
        , index(index)
        , dpiScale(dpiScale)
    {
        // ACTION_DOWN and ACTION_UP must be accesssed through pointer_index 0
        Q_ASSERT((action != Action::DOWN && action != Action::UP) || index == 0);
    }

    uint32_t GetUniqueEventId() const override { return eventId; }
    Action GetAction() const override { return action; }
    int GetActionIndex() const override { return index; }
    size_t GetPointerCount() const override { return touchPoints.size(); }
    int GetPointerId(size_t pointer_index) const override { return touchPoints.at(pointer_index).id(); }
    float GetX(size_t pointer_index) const override { return touchPoints.at(pointer_index).pos().x() / dpiScale; }
    float GetY(size_t pointer_index) const override { return touchPoints.at(pointer_index).pos().y() / dpiScale; }
    float GetRawX(size_t pointer_index) const override { return touchPoints.at(pointer_index).screenPos().x(); }
    float GetRawY(size_t pointer_index) const override { return touchPoints.at(pointer_index).screenPos().y(); }
    float GetTouchMajor(size_t pointer_index) const override
    {
        QRectF touchRect = touchPoints.at(pointer_index).rect();
        return std::max(touchRect.height(), touchRect.width());
    }
    float GetTouchMinor(size_t pointer_index) const override
    {
        QRectF touchRect = touchPoints.at(pointer_index).rect();
        return std::min(touchRect.height(), touchRect.width());
    }
    float GetOrientation(size_t pointer_index) const override
    {
        return 0;
    }
    int GetFlags() const override { return flags; }
    float GetPressure(size_t pointer_index) const override { return touchPoints.at(pointer_index).pressure(); }
    float GetTiltX(size_t pointer_index) const override { return 0; }
    float GetTiltY(size_t pointer_index) const override { return 0; }
    base::TimeTicks GetEventTime() const override { return eventTime; }

    size_t GetHistorySize() const override { return 0; }
    base::TimeTicks GetHistoricalEventTime(size_t historical_index) const override { return base::TimeTicks(); }
    float GetHistoricalTouchMajor(size_t pointer_index, size_t historical_index) const override { return 0; }
    float GetHistoricalX(size_t pointer_index, size_t historical_index) const override { return 0; }
    float GetHistoricalY(size_t pointer_index, size_t historical_index) const override { return 0; }
    ToolType GetToolType(size_t pointer_index) const override { return ui::MotionEvent::ToolType::FINGER; }
    int GetButtonState() const override { return 0; }

private:
    QList<QTouchEvent::TouchPoint> touchPoints;
    base::TimeTicks eventTime;
    Action action;
    const uint32_t eventId;
    int flags;
    int index;
    float dpiScale;
};

bool isAccessibilityEnabled() {
    // On Linux accessibility is disabled by default due to performance issues,
    // and can be re-enabled by setting the QTWEBENGINE_ENABLE_LINUX_ACCESSIBILITY environment
    // variable. For details, see QTBUG-59922.
#ifdef Q_OS_LINUX
    static bool accessibility_enabled
            = qEnvironmentVariableIsSet("QTWEBENGINE_ENABLE_LINUX_ACCESSIBILITY");
#else
    const bool accessibility_enabled = true;
#endif
    return accessibility_enabled;
}

RenderWidgetHostViewQt::RenderWidgetHostViewQt(content::RenderWidgetHost *widget)
    : content::RenderWidgetHostViewBase::RenderWidgetHostViewBase(widget)
    , m_gestureProvider(QtGestureProviderConfig(), this)
    , m_sendMotionActionDown(false)
    , m_touchMotionStarted(false)
    , m_compositor(new Compositor(this))
    , m_loadVisuallyCommittedState(NotCommitted)
    , m_adapterClient(0)
    , m_imeInProgress(false)
    , m_receivedEmptyImeEvent(false)
    , m_imState(0)
    , m_anchorPositionWithinSelection(-1)
    , m_cursorPositionWithinSelection(-1)
    , m_cursorPosition(0)
    , m_emptyPreviousSelection(true)
    , m_wheelAckPending(false)
    , m_pendingResize(false)
    , m_mouseWheelPhaseHandler(this)
    // This frame-sink id is based on what RenderWidgetHostViewChildFrame does:
    , m_frameSinkId(base::checked_cast<uint32_t>(widget->GetProcess()->GetID()),
                    base::checked_cast<uint32_t>(widget->GetRoutingID()))
{
    host()->SetView(this);
#ifndef QT_NO_ACCESSIBILITY
    if (isAccessibilityEnabled()) {
        QAccessible::installActivationObserver(this);
        if (QAccessible::isActive())
            content::BrowserAccessibilityStateImpl::GetInstance()->EnableAccessibility();
    }
#endif // QT_NO_ACCESSIBILITY

    if (GetTextInputManager())
        GetTextInputManager()->AddObserver(this);

    const QPlatformInputContext *context = QGuiApplicationPrivate::platformIntegration()->inputContext();
    m_imeHasHiddenTextCapability = context && context->hasCapability(QPlatformInputContext::HiddenTextCapability);

    m_localSurfaceId = m_localSurfaceIdAllocator.GenerateId();

    if (host()->delegate() && host()->delegate()->GetInputEventRouter())
        host()->delegate()->GetInputEventRouter()->AddFrameSinkIdOwner(GetFrameSinkId(), this);
}

RenderWidgetHostViewQt::~RenderWidgetHostViewQt()
{
    QObject::disconnect(m_adapterClientDestroyedConnection);
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::removeActivationObserver(this);
#endif // QT_NO_ACCESSIBILITY

    if (text_input_manager_)
        text_input_manager_->RemoveObserver(this);
}

void RenderWidgetHostViewQt::setDelegate(RenderWidgetHostViewQtDelegate* delegate)
{
    m_delegate.reset(delegate);
    m_compositor->setViewDelegate(delegate);
}

void RenderWidgetHostViewQt::setAdapterClient(WebContentsAdapterClient *adapterClient)
{
    Q_ASSERT(!m_adapterClient);

    m_adapterClient = adapterClient;
    QObject::disconnect(m_adapterClientDestroyedConnection);
    m_adapterClientDestroyedConnection = QObject::connect(adapterClient->holdingQObject(),
                                                          &QObject::destroyed, [this] {
                                                            m_adapterClient = nullptr; });
}

void RenderWidgetHostViewQt::InitAsChild(gfx::NativeView)
{
}

void RenderWidgetHostViewQt::InitAsPopup(content::RenderWidgetHostView*, const gfx::Rect& rect)
{
    m_delegate->initAsPopup(toQt(rect));
}

void RenderWidgetHostViewQt::InitAsFullscreen(content::RenderWidgetHostView*)
{
}

void RenderWidgetHostViewQt::SetSize(const gfx::Size& size)
{
    int width = size.width();
    int height = size.height();

    m_delegate->resize(width,height);
}

void RenderWidgetHostViewQt::SetBounds(const gfx::Rect& screenRect)
{
    // This is called when webkit has sent us a Move message.
    if (IsPopup())
        m_delegate->move(toQt(screenRect.origin()));
    SetSize(screenRect.size());
}

gfx::Size RenderWidgetHostViewQt::GetCompositorViewportPixelSize() const
{
    if (!m_delegate || !m_delegate->window() || !m_delegate->window()->screen())
        return gfx::Size();

    const QScreen* screen = m_delegate->window()->screen();
    gfx::SizeF size = toGfx(m_delegate->screenRect().size());
    return gfx::ToCeiledSize(gfx::ScaleSize(size, screen->devicePixelRatio()));
}

gfx::NativeView RenderWidgetHostViewQt::GetNativeView() const
{
    // gfx::NativeView is a typedef to a platform specific view
    // pointer (HWND, NSView*, GtkWidget*) and other ports use
    // this function in the renderer_host layer when setting up
    // the view hierarchy and for generating snapshots in tests.
    // Since we manage the view hierarchy in Qt its value hasn't
    // been meaningful.
    return gfx::NativeView();
}

gfx::NativeViewAccessible RenderWidgetHostViewQt::GetNativeViewAccessible()
{
    return 0;
}

content::BrowserAccessibilityManager* RenderWidgetHostViewQt::CreateBrowserAccessibilityManager(content::BrowserAccessibilityDelegate* delegate, bool for_root_frame)
{
    Q_UNUSED(for_root_frame); // FIXME
#ifndef QT_NO_ACCESSIBILITY
    return new content::BrowserAccessibilityManagerQt(
        m_adapterClient->accessibilityParentObject(),
        content::BrowserAccessibilityManagerQt::GetEmptyDocument(),
        delegate);
#else
    return 0;
#endif // QT_NO_ACCESSIBILITY
}

// Set focus to the associated View component.
void RenderWidgetHostViewQt::Focus()
{
    if (!IsPopup())
        m_delegate->setKeyboardFocus();
    host()->Focus();
}

bool RenderWidgetHostViewQt::HasFocus() const
{
    return m_delegate->hasKeyboardFocus();
}

bool RenderWidgetHostViewQt::IsSurfaceAvailableForCopy() const
{
    return true;
}

void RenderWidgetHostViewQt::CopyFromSurface(const gfx::Rect &src_rect,
                                             const gfx::Size &output_size,
                                             base::OnceCallback<void(const SkBitmap &)> callback)
{
    QImage image;
    if (m_delegate->copySurface(toQt(src_rect), toQt(output_size), image))
        std::move(callback).Run(toSkBitmap(image));
    else
        std::move(callback).Run(SkBitmap());
}

void RenderWidgetHostViewQt::Show()
{
    m_delegate->show();
}

void RenderWidgetHostViewQt::Hide()
{
    m_delegate->hide();
}

bool RenderWidgetHostViewQt::IsShowing()
{
    return m_delegate->isVisible();
}

// Retrieve the bounds of the View, in screen coordinates.
gfx::Rect RenderWidgetHostViewQt::GetViewBounds() const
{
    QRectF p = m_delegate->contentsRect();
    float s = dpiScale();
    gfx::Point p1(floor(p.x() / s), floor(p.y() / s));
    gfx::Point p2(ceil(p.right() /s), ceil(p.bottom() / s));
    return gfx::BoundingRect(p1, p2);
}

void RenderWidgetHostViewQt::UpdateBackgroundColor()
{
    auto color = GetBackgroundColor();
    if (color) {
        m_delegate->setClearColor(toQt(*color));
        host()->Send(new RenderViewObserverQt_SetBackgroundColor(host()->GetRoutingID(), *color));
    }
}

// Return value indicates whether the mouse is locked successfully or not.
bool RenderWidgetHostViewQt::LockMouse()
{
    m_previousMousePosition = QCursor::pos();
    m_delegate->lockMouse();
    qApp->setOverrideCursor(Qt::BlankCursor);
    return true;
}

void RenderWidgetHostViewQt::UnlockMouse()
{
    m_delegate->unlockMouse();
    qApp->restoreOverrideCursor();
    host()->LostMouseLock();
}

void RenderWidgetHostViewQt::UpdateCursor(const content::WebCursor &webCursor)
{
    DisplayCursor(webCursor);
}

void RenderWidgetHostViewQt::DisplayCursor(const content::WebCursor &webCursor)
{
    content::CursorInfo cursorInfo;
    webCursor.GetCursorInfo(&cursorInfo);
    Qt::CursorShape shape = Qt::ArrowCursor;
#if defined(USE_AURA)
    ui::CursorType auraType = ui::CursorType::kNull;
#endif
    switch (cursorInfo.type) {
    case blink::WebCursorInfo::kTypePointer:
        shape = Qt::ArrowCursor;
        break;
    case blink::WebCursorInfo::kTypeCross:
        shape = Qt::CrossCursor;
        break;
    case blink::WebCursorInfo::kTypeHand:
        shape = Qt::PointingHandCursor;
        break;
    case blink::WebCursorInfo::kTypeIBeam:
        shape = Qt::IBeamCursor;
        break;
    case blink::WebCursorInfo::kTypeWait:
        shape = Qt::WaitCursor;
        break;
    case blink::WebCursorInfo::kTypeHelp:
        shape = Qt::WhatsThisCursor;
        break;
    case blink::WebCursorInfo::kTypeEastResize:
    case blink::WebCursorInfo::kTypeWestResize:
    case blink::WebCursorInfo::kTypeEastWestResize:
    case blink::WebCursorInfo::kTypeEastPanning:
    case blink::WebCursorInfo::kTypeWestPanning:
        shape = Qt::SizeHorCursor;
        break;
    case blink::WebCursorInfo::kTypeNorthResize:
    case blink::WebCursorInfo::kTypeSouthResize:
    case blink::WebCursorInfo::kTypeNorthSouthResize:
    case blink::WebCursorInfo::kTypeNorthPanning:
    case blink::WebCursorInfo::kTypeSouthPanning:
        shape = Qt::SizeVerCursor;
        break;
    case blink::WebCursorInfo::kTypeNorthEastResize:
    case blink::WebCursorInfo::kTypeSouthWestResize:
    case blink::WebCursorInfo::kTypeNorthEastSouthWestResize:
    case blink::WebCursorInfo::kTypeNorthEastPanning:
    case blink::WebCursorInfo::kTypeSouthWestPanning:
        shape = Qt::SizeBDiagCursor;
        break;
    case blink::WebCursorInfo::kTypeNorthWestResize:
    case blink::WebCursorInfo::kTypeSouthEastResize:
    case blink::WebCursorInfo::kTypeNorthWestSouthEastResize:
    case blink::WebCursorInfo::kTypeNorthWestPanning:
    case blink::WebCursorInfo::kTypeSouthEastPanning:
        shape = Qt::SizeFDiagCursor;
        break;
    case blink::WebCursorInfo::kTypeColumnResize:
        shape = Qt::SplitHCursor;
        break;
    case blink::WebCursorInfo::kTypeRowResize:
        shape = Qt::SplitVCursor;
        break;
    case blink::WebCursorInfo::kTypeMiddlePanning:
    case blink::WebCursorInfo::kTypeMove:
        shape = Qt::SizeAllCursor;
        break;
    case blink::WebCursorInfo::kTypeProgress:
        shape = Qt::BusyCursor;
        break;
    case blink::WebCursorInfo::kTypeCopy:
        shape = Qt::DragCopyCursor;
        break;
    case blink::WebCursorInfo::kTypeAlias:
        shape = Qt::DragLinkCursor;
        break;
#if defined(USE_AURA)
    case blink::WebCursorInfo::kTypeVerticalText:
        auraType = ui::CursorType::kVerticalText;
        break;
    case blink::WebCursorInfo::kTypeCell:
        auraType = ui::CursorType::kCell;
        break;
    case blink::WebCursorInfo::kTypeContextMenu:
        auraType = ui::CursorType::kContextMenu;
        break;
    case blink::WebCursorInfo::kTypeZoomIn:
        auraType = ui::CursorType::kZoomIn;
        break;
    case blink::WebCursorInfo::kTypeZoomOut:
        auraType = ui::CursorType::kZoomOut;
        break;
#else
    case blink::WebCursorInfo::kTypeVerticalText:
    case blink::WebCursorInfo::kTypeCell:
    case blink::WebCursorInfo::kTypeContextMenu:
    case blink::WebCursorInfo::kTypeZoomIn:
    case blink::WebCursorInfo::kTypeZoomOut:
        // FIXME: Support on OS X
        break;
#endif
    case blink::WebCursorInfo::kTypeNoDrop:
    case blink::WebCursorInfo::kTypeNotAllowed:
        shape = Qt::ForbiddenCursor;
        break;
    case blink::WebCursorInfo::kTypeNone:
        shape = Qt::BlankCursor;
        break;
    case blink::WebCursorInfo::kTypeGrab:
        shape = Qt::OpenHandCursor;
        break;
    case blink::WebCursorInfo::kTypeGrabbing:
        shape = Qt::ClosedHandCursor;
        break;
    case blink::WebCursorInfo::kTypeCustom:
        if (cursorInfo.custom_image.colorType() == SkColorType::kN32_SkColorType) {
            QImage cursor = toQImage(cursorInfo.custom_image, QImage::Format_ARGB32);
            m_delegate->updateCursor(QCursor(QPixmap::fromImage(cursor), cursorInfo.hotspot.x(), cursorInfo.hotspot.y()));
            return;
        }
        break;
    }
#if defined(USE_AURA)
    if (auraType != ui::CursorType::kNull) {
        QWindow *window = m_delegate->window();
        qreal windowDpr = window ? window->devicePixelRatio() : 1.0f;
        int resourceId;
        gfx::Point hotspot;
        // GetCursorDataFor only knows hotspots for 1x and 2x cursor images, in physical pixels.
        qreal hotspotDpr = windowDpr <= 1.0f ? 1.0f : 2.0f;
        if (ui::GetCursorDataFor(ui::CursorSize::kNormal, auraType, hotspotDpr, &resourceId, &hotspot)) {
            if (const gfx::ImageSkia *imageSkia = ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(resourceId)) {
                QImage imageQt = toQImage(imageSkia->GetRepresentation(windowDpr));

                // Convert hotspot coordinates into device-independent pixels.
                qreal hotX = hotspot.x() / hotspotDpr;
                qreal hotY = hotspot.y() / hotspotDpr;

#if defined(Q_OS_LINUX)
                // QTBUG-68571: On Linux (xcb, wayland, eglfs), hotspot coordinates must be in physical pixels.
                qreal imageDpr = imageQt.devicePixelRatio();
                hotX *= imageDpr;
                hotY *= imageDpr;
#endif

                m_delegate->updateCursor(QCursor(QPixmap::fromImage(std::move(imageQt)), qRound(hotX), qRound(hotY)));
                return;
            }
        }
    }
#endif
    m_delegate->updateCursor(QCursor(shape));
}

void RenderWidgetHostViewQt::SetIsLoading(bool)
{
    // We use WebContentsDelegateQt::LoadingStateChanged to notify about loading state.
}

void RenderWidgetHostViewQt::ImeCancelComposition()
{
    qApp->inputMethod()->reset();
}

void RenderWidgetHostViewQt::ImeCompositionRangeChanged(const gfx::Range&, const std::vector<gfx::Rect>&)
{
    // FIXME: not implemented?
    QT_NOT_YET_IMPLEMENTED
}

void RenderWidgetHostViewQt::RenderProcessGone(base::TerminationStatus terminationStatus,
                                               int exitCode)
{
    if (m_adapterClient) {
        m_adapterClient->renderProcessTerminated(
                    m_adapterClient->renderProcessExitStatus(terminationStatus),
                    exitCode);
    }
    Destroy();
}

void RenderWidgetHostViewQt::Destroy()
{
    delete this;
}

void RenderWidgetHostViewQt::SetTooltipText(const base::string16 &tooltip_text)
{
    DisplayTooltipText(tooltip_text);
}

void RenderWidgetHostViewQt::DisplayTooltipText(const base::string16 &tooltip_text)
{
    if (m_adapterClient)
        m_adapterClient->setToolTip(toQt(tooltip_text));
}

void RenderWidgetHostViewQt::DidCreateNewRendererCompositorFrameSink(viz::mojom::CompositorFrameSinkClient *frameSink)
{
    m_compositor->setFrameSinkClient(frameSink);
}

void RenderWidgetHostViewQt::SubmitCompositorFrame(const viz::LocalSurfaceId &local_surface_id, viz::CompositorFrame frame, base::Optional<viz::HitTestRegionList>)
{
    bool scrollOffsetChanged = (m_lastScrollOffset != frame.metadata.root_scroll_offset);
    bool contentsSizeChanged = (m_lastContentsSize != frame.metadata.root_layer_size);
    m_lastScrollOffset = frame.metadata.root_scroll_offset;
    m_lastContentsSize = frame.metadata.root_layer_size;

    // Force to process swap messages
    uint32_t frame_token = frame.metadata.frame_token;
    if (frame_token)
        OnFrameTokenChangedForView(frame_token);

    // Support experimental.viewport.devicePixelRatio, see GetScreenInfo implementation below.
    float dpiScale = this->dpiScale();
    if (dpiScale != 0 && dpiScale != 1)
        frame.metadata.device_scale_factor /= dpiScale;

    m_compositor->submitFrame(std::move(frame));

    if (m_loadVisuallyCommittedState == NotCommitted) {
        m_loadVisuallyCommittedState = DidFirstCompositorFrameSwap;
    } else if (m_loadVisuallyCommittedState == DidFirstVisuallyNonEmptyPaint) {
        m_adapterClient->loadVisuallyCommitted();
        m_loadVisuallyCommittedState = NotCommitted;
    }

    if (scrollOffsetChanged)
        m_adapterClient->updateScrollPosition(toQt(m_lastScrollOffset));
    if (contentsSizeChanged)
        m_adapterClient->updateContentsSize(toQt(m_lastContentsSize));

    if (m_pendingResize && host()) {
        if (host()->SynchronizeVisualProperties())
            m_pendingResize = false;
    }
}

void RenderWidgetHostViewQt::GetScreenInfo(content::ScreenInfo *results) const
{
    QWindow *window = m_delegate->window();
    if (!window)
        return;
    GetScreenInfoFromNativeWindow(window, results);

    // Support experimental.viewport.devicePixelRatio
    results->device_scale_factor *= dpiScale();
}

gfx::Rect RenderWidgetHostViewQt::GetBoundsInRootWindow()
{
    if (!m_delegate->window())
        return gfx::Rect();

    QRect r = m_delegate->window()->frameGeometry();
    return gfx::Rect(r.x(), r.y(), r.width(), r.height());
}

void RenderWidgetHostViewQt::ClearCompositorFrame()
{
}

void RenderWidgetHostViewQt::OnUpdateTextInputStateCalled(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view, bool did_update_state)
{
    Q_UNUSED(text_input_manager);
    Q_UNUSED(updated_view);
    Q_UNUSED(did_update_state);

    const content::TextInputState *state = text_input_manager_->GetTextInputState();
    if (!state) {
        m_delegate->inputMethodStateChanged(false /*editorVisible*/, false /*passwordInput*/);
        m_delegate->setInputMethodHints(Qt::ImhNone);
        return;
    }

    ui::TextInputType type = getTextInputType();
    m_delegate->setInputMethodHints(toQtInputMethodHints(getTextInputType()) | Qt::ImhNoPredictiveText | Qt::ImhNoTextHandles | Qt::ImhNoEditMenu);

    m_surroundingText = QString::fromStdString(state->value);
    // Remove IME composition text from the surrounding text
    if (state->composition_start != -1 && state->composition_end != -1)
        m_surroundingText.remove(state->composition_start, state->composition_end - state->composition_start);

    // In case of text selection, the update is expected in RenderWidgetHostViewQt::selectionChanged().
    if (GetSelectedText().empty()) {
        // At this point it is unknown whether the text input state has been updated due to a text selection.
        // Keep the cursor position updated for cursor movements too.
        m_cursorPosition = state->selection_start;
        m_delegate->inputMethodStateChanged(type != ui::TEXT_INPUT_TYPE_NONE, type == ui::TEXT_INPUT_TYPE_PASSWORD);
    }

    if (m_imState & ImStateFlags::TextInputStateUpdated) {
        m_imState = ImStateFlags::TextInputStateUpdated;
        return;
    }

    // Ignore selection change triggered by ime composition unless it clears an actual text selection
    if (state->composition_start != -1 && m_emptyPreviousSelection) {
        m_imState = 0;
        return;
    }

    m_imState |= ImStateFlags::TextInputStateUpdated;
    if (m_imState == ImStateFlags::AllFlags)
        selectionChanged();
}

void RenderWidgetHostViewQt::OnSelectionBoundsChanged(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view)
{
    Q_UNUSED(text_input_manager);
    Q_UNUSED(updated_view);

    m_imState |= ImStateFlags::TextSelectionBoundsUpdated;
    if (m_imState == ImStateFlags::AllFlags
            || (m_imState == ImStateFlags::TextSelectionFlags && getTextInputType() == ui::TEXT_INPUT_TYPE_NONE)) {
        selectionChanged();
    }
}

void RenderWidgetHostViewQt::OnTextSelectionChanged(content::TextInputManager *text_input_manager, RenderWidgetHostViewBase *updated_view)
{
    Q_UNUSED(text_input_manager);
    Q_UNUSED(updated_view);

    const content::TextInputManager::TextSelection *selection = GetTextInputManager()->GetTextSelection(updated_view);
    if (!selection)
        return;

#if defined(USE_OZONE)
    if (!selection->selected_text().empty() && selection->user_initiated()) {
        // Set the CLIPBOARD_TYPE_SELECTION to the ui::Clipboard.
        ui::ScopedClipboardWriter clipboard_writer(ui::CLIPBOARD_TYPE_SELECTION);
        clipboard_writer.WriteText(selection->selected_text());
    }
#endif // defined(USE_OZONE)

    m_imState |= ImStateFlags::TextSelectionUpdated;
    if (m_imState == ImStateFlags::AllFlags
            || (m_imState == ImStateFlags::TextSelectionFlags && getTextInputType() == ui::TEXT_INPUT_TYPE_NONE)) {
        selectionChanged();
    }
}

void RenderWidgetHostViewQt::selectionChanged()
{
    // Reset input manager state
    m_imState = 0;
    ui::TextInputType type = getTextInputType();

    // Handle text selection out of an input field
    if (type == ui::TEXT_INPUT_TYPE_NONE) {
        if (GetSelectedText().empty() && m_emptyPreviousSelection)
            return;

        // Reset position values to emit selectionChanged signal when clearing text selection
        // by clicking into an input field. These values are intended to be used by inputMethodQuery
        // so they are not expected to be valid when selection is out of an input field.
        m_anchorPositionWithinSelection = -1;
        m_cursorPositionWithinSelection = -1;

        m_emptyPreviousSelection = GetSelectedText().empty();
        m_adapterClient->selectionChanged();
        return;
    }

    if (GetSelectedText().empty()) {
        // RenderWidgetHostViewQt::OnUpdateTextInputStateCalled() does not update the cursor position
        // if the selection is cleared because TextInputState changes before the TextSelection change.
        Q_ASSERT(text_input_manager_->GetTextInputState());
        m_cursorPosition = text_input_manager_->GetTextInputState()->selection_start;
        m_delegate->inputMethodStateChanged(true /*editorVisible*/, type == ui::TEXT_INPUT_TYPE_PASSWORD);

        m_anchorPositionWithinSelection = m_cursorPosition;
        m_cursorPositionWithinSelection = m_cursorPosition;

        if (!m_emptyPreviousSelection) {
            m_emptyPreviousSelection = true;
            m_adapterClient->selectionChanged();
        }

        return;
    }

    const content::TextInputManager::TextSelection *selection = text_input_manager_->GetTextSelection();
    if (!selection)
        return;

    if (!selection->range().IsValid())
        return;

    int newAnchorPositionWithinSelection = 0;
    int newCursorPositionWithinSelection = 0;

    if (text_input_manager_->GetSelectionRegion()->anchor.type() == gfx::SelectionBound::RIGHT) {
        newAnchorPositionWithinSelection = selection->range().GetMax() - selection->offset();
        newCursorPositionWithinSelection = selection->range().GetMin() - selection->offset();
    } else {
        newAnchorPositionWithinSelection = selection->range().GetMin() - selection->offset();
        newCursorPositionWithinSelection = selection->range().GetMax() - selection->offset();
    }

    if (m_anchorPositionWithinSelection == newAnchorPositionWithinSelection && m_cursorPositionWithinSelection == newCursorPositionWithinSelection)
        return;

    m_anchorPositionWithinSelection = newAnchorPositionWithinSelection;
    m_cursorPositionWithinSelection = newCursorPositionWithinSelection;

    if (!selection->selected_text().empty())
        m_cursorPosition = newCursorPositionWithinSelection;

    m_emptyPreviousSelection = selection->selected_text().empty();
    m_delegate->inputMethodStateChanged(true /*editorVisible*/, type == ui::TEXT_INPUT_TYPE_PASSWORD);
    m_adapterClient->selectionChanged();
}

void RenderWidgetHostViewQt::OnGestureEvent(const ui::GestureEventData& gesture)
{
    if ((gesture.type() == ui::ET_GESTURE_PINCH_BEGIN
         || gesture.type() == ui::ET_GESTURE_PINCH_UPDATE
         || gesture.type() == ui::ET_GESTURE_PINCH_END)
        && !content::IsPinchToZoomEnabled()) {
        return;
    }

    host()->ForwardGestureEvent(ui::CreateWebGestureEventFromGestureEventData(gesture));
}

viz::ScopedSurfaceIdAllocator RenderWidgetHostViewQt::DidUpdateVisualProperties(const cc::RenderFrameMetadata &metadata)
{
    base::OnceCallback<void()> allocation_task =
        base::BindOnce(&RenderWidgetHostViewQt::OnDidUpdateVisualPropertiesComplete,
                       base::Unretained(this), metadata);
    return viz::ScopedSurfaceIdAllocator(std::move(allocation_task));
}

void RenderWidgetHostViewQt::OnDidUpdateVisualPropertiesComplete(const cc::RenderFrameMetadata &metadata)
{
    if (metadata.local_surface_id)
        m_localSurfaceIdAllocator.UpdateFromChild(*metadata.local_surface_id);

    m_localSurfaceId = m_localSurfaceIdAllocator.GenerateId();
    host()->SendScreenRects();
    if (m_pendingResize) {
        if (host()->SynchronizeVisualProperties())
            m_pendingResize = false;
    }
}

QSGNode *RenderWidgetHostViewQt::updatePaintNode(QSGNode *oldNode)
{
    return m_compositor->updatePaintNode(oldNode);
}

void RenderWidgetHostViewQt::notifyResize()
{
    m_pendingResize = true;
    if (host()->SynchronizeVisualProperties())
        m_pendingResize = false;
}

void RenderWidgetHostViewQt::notifyShown()
{
    host()->WasShown(false);
}

void RenderWidgetHostViewQt::notifyHidden()
{
    host()->WasHidden();
}

void RenderWidgetHostViewQt::windowBoundsChanged()
{
    host()->SendScreenRects();
    if (m_delegate && m_delegate->window())
        host()->NotifyScreenInfoChanged();
}

void RenderWidgetHostViewQt::windowChanged()
{
    if (m_delegate && m_delegate->window())
        host()->NotifyScreenInfoChanged();
}

bool RenderWidgetHostViewQt::forwardEvent(QEvent *event)
{
    Q_ASSERT(host()->GetView());

    switch (event->type()) {
    case QEvent::ShortcutOverride: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        auto acceptKeyOutOfInputField = [](QKeyEvent *keyEvent) -> bool {
#ifdef Q_OS_MACOS
            // Check if a shortcut is registered for this key sequence.
            QKeySequence sequence = QKeySequence (
                         (keyEvent->modifiers() | keyEvent->key()) &
                         ~(Qt::KeypadModifier | Qt::GroupSwitchModifier));
            if (QGuiApplicationPrivate::instance()->shortcutMap.hasShortcutForKeySequence(sequence))
                return false;

            // The following shortcuts are handled out of input field too but
            // disabled on macOS to let the blinking menu handling to the
            // embedder application (see kKeyboardCodeKeyDownEntries in
            // third_party/WebKit/Source/core/editing/EditingBehavior.cpp).
            // Let them pass on macOS to generate the corresponding edit command.
            return keyEvent->matches(QKeySequence::Copy)
                    || keyEvent->matches(QKeySequence::Paste)
                    || keyEvent->matches(QKeySequence::Cut)
                    || keyEvent->matches(QKeySequence::SelectAll);
#else
            return false;
#endif
        };

        if (!inputMethodQuery(Qt::ImEnabled).toBool() && !acceptKeyOutOfInputField(keyEvent))
            return false;

        Q_ASSERT(m_editCommand.empty());
        if (WebEventFactory::getEditCommand(keyEvent, &m_editCommand)
                || isCommonTextEditShortcut(keyEvent)) {
            event->accept();
            return true;
        }

        return false;
    }
    case QEvent::MouseButtonPress:
        Focus();
        Q_FALLTHROUGH();
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
        // Skip second MouseMove event when a window is being adopted, so that Chromium
        // can properly handle further move events.
        // Also make sure the adapter client exists to prevent a null pointer dereference,
        // because it's possible for a QWebEnginePagePrivate (adapter) instance to be destroyed,
        // and then the OS (observed on Windows) might still send mouse move events to a still
        // existing popup RWHVQDW instance.
        if (m_adapterClient && m_adapterClient->isBeingAdopted())
            return false;
        handleMouseEvent(static_cast<QMouseEvent*>(event));
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        handleKeyEvent(static_cast<QKeyEvent*>(event));
        break;
    case QEvent::Wheel:
        handleWheelEvent(static_cast<QWheelEvent*>(event));
        break;
    case QEvent::TouchBegin:
        Focus();
        Q_FALLTHROUGH();
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        handleTouchEvent(static_cast<QTouchEvent*>(event));
        break;
#if QT_CONFIG(tabletevent)
    case QEvent::TabletPress:
        Focus();
        Q_FALLTHROUGH();
    case QEvent::TabletRelease:
    case QEvent::TabletMove:
        handleTabletEvent(static_cast<QTabletEvent*>(event));
        break;
#endif
#ifndef QT_NO_GESTURES
    case QEvent::NativeGesture:
        handleGestureEvent(static_cast<QNativeGestureEvent *>(event));
        break;
#endif // QT_NO_GESTURES
    case QEvent::HoverMove:
        handleHoverEvent(static_cast<QHoverEvent*>(event));
        break;
    case QEvent::FocusIn:
    case QEvent::FocusOut:
        handleFocusEvent(static_cast<QFocusEvent*>(event));
        break;
    case QEvent::InputMethod:
        handleInputMethodEvent(static_cast<QInputMethodEvent*>(event));
        break;
    case QEvent::InputMethodQuery:
        handleInputMethodQueryEvent(static_cast<QInputMethodQueryEvent*>(event));
        break;
    case QEvent::HoverLeave:
    case QEvent::Leave:
        host()->ForwardMouseEvent(WebEventFactory::toWebMouseEvent(event));
        break;
    default:
        return false;
    }
    return true;
}

QVariant RenderWidgetHostViewQt::inputMethodQuery(Qt::InputMethodQuery query)
{
    switch (query) {
    case Qt::ImEnabled: {
        ui::TextInputType type = getTextInputType();
        bool editorVisible = type != ui::TEXT_INPUT_TYPE_NONE;
        // IME manager should disable composition on input fields with ImhHiddenText hint if supported
        if (m_imeHasHiddenTextCapability)
            return QVariant(editorVisible);

        bool passwordInput = type == ui::TEXT_INPUT_TYPE_PASSWORD;
        return QVariant(editorVisible && !passwordInput);
    }
    case Qt::ImFont:
        // TODO: Implement this
        return QVariant();
    case Qt::ImCursorRectangle: {
        if (text_input_manager_) {
            if (auto *region = text_input_manager_->GetSelectionRegion()) {
                if (region->focus.GetHeight() > 0) {
                    gfx::Rect caretRect = gfx::RectBetweenSelectionBounds(region->anchor, region->focus);
                    if (caretRect.width() == 0)
                        caretRect.set_width(1); // IME API on Windows expects a width > 0
                    return toQt(caretRect);
                }
            }
        }
        return QVariant();
    }
    case Qt::ImCursorPosition:
        return m_cursorPosition;
    case Qt::ImAnchorPosition:
        return GetSelectedText().empty() ? m_cursorPosition : m_anchorPositionWithinSelection;
    case Qt::ImSurroundingText:
        return m_surroundingText;
    case Qt::ImCurrentSelection:
        return toQt(GetSelectedText());
    case Qt::ImMaximumTextLength:
        // TODO: Implement this
        return QVariant(); // No limit.
    case Qt::ImHints:
        return int(toQtInputMethodHints(getTextInputType()) | Qt::ImhNoPredictiveText | Qt::ImhNoTextHandles | Qt::ImhNoEditMenu);
    default:
        return QVariant();
    }
}

void RenderWidgetHostViewQt::closePopup()
{
    // We notify the popup to be closed by telling it that it lost focus. WebKit does the rest
    // (hiding the widget and automatic memory cleanup via
    // RenderWidget::CloseWidgetSoon() -> RenderWidgetHostImpl::ShutdownAndDestroyWidget(true).
    host()->SetActive(false);
    host()->LostFocus();
}

void RenderWidgetHostViewQt::ProcessAckedTouchEvent(const content::TouchEventWithLatencyInfo &touch, content::InputEventAckState ack_result) {
    Q_UNUSED(touch);
    const bool eventConsumed = ack_result == content::INPUT_EVENT_ACK_STATE_CONSUMED;
    m_gestureProvider.OnTouchEventAck(touch.event.unique_touch_event_id, eventConsumed, /*fixme: ?? */false);
}

void RenderWidgetHostViewQt::processMotionEvent(const ui::MotionEvent &motionEvent)
{
    auto result = m_gestureProvider.OnTouchEvent(motionEvent);
    if (!result.succeeded)
        return;

    blink::WebTouchEvent touchEvent = ui::CreateWebTouchEventFromMotionEvent(motionEvent,
                                                                             result.moved_beyond_slop_region,
                                                                             false /*hovering, FIXME ?*/);
    host()->ForwardTouchEventWithLatencyInfo(touchEvent, CreateLatencyInfo(touchEvent));
}

QList<QTouchEvent::TouchPoint> RenderWidgetHostViewQt::mapTouchPointIds(const QList<QTouchEvent::TouchPoint> &inputPoints)
{
    QList<QTouchEvent::TouchPoint> outputPoints = inputPoints;
    for (int i = 0; i < outputPoints.size(); ++i) {
        QTouchEvent::TouchPoint &point = outputPoints[i];

        int qtId = point.id();
        QMap<int, int>::const_iterator it = m_touchIdMapping.find(qtId);
        if (it == m_touchIdMapping.end())
            it = m_touchIdMapping.insert(qtId, firstAvailableId(m_touchIdMapping));
        point.setId(it.value());

        if (point.state() == Qt::TouchPointReleased)
            m_touchIdMapping.remove(qtId);
    }

    return outputPoints;
}

float RenderWidgetHostViewQt::dpiScale() const
{
    return m_adapterClient ? m_adapterClient->dpiScale() : 1.0;
}

bool RenderWidgetHostViewQt::IsPopup() const
{
    return popup_type_ != blink::kWebPopupTypeNone;
}

void RenderWidgetHostViewQt::handleMouseEvent(QMouseEvent* event)
{
    // Don't forward mouse events synthesized by the system, which are caused by genuine touch
    // events. Chromium would then process for e.g. a mouse click handler twice, once due to the
    // system synthesized mouse event, and another time due to a touch-to-gesture-to-mouse
    // transformation done by Chromium.
    if (event->source() == Qt::MouseEventSynthesizedBySystem)
        return;
    handlePointerEvent<QMouseEvent>(event);
}

void RenderWidgetHostViewQt::handleKeyEvent(QKeyEvent *ev)
{
    if (IsMouseLocked() && ev->key() == Qt::Key_Escape && ev->type() == QEvent::KeyRelease)
        UnlockMouse();

    if (m_receivedEmptyImeEvent) {
        // IME composition was not finished with a valid commit string.
        // We're getting the composition result in a key event.
        if (ev->key() != 0) {
            // The key event is not a result of an IME composition. Cancel IME.
            host()->ImeCancelComposition();
            m_receivedEmptyImeEvent = false;
        } else {
            if (ev->type() == QEvent::KeyRelease) {
                host()->ImeCommitText(toString16(ev->text()),
                                      std::vector<ui::ImeTextSpan>(),
                                      gfx::Range::InvalidRange(),
                                      0);
                m_receivedEmptyImeEvent = false;
                m_imeInProgress = false;
            }
            return;
        }
    }

    content::NativeWebKeyboardEvent webEvent = WebEventFactory::toWebKeyboardEvent(ev);
    if (webEvent.GetType() == blink::WebInputEvent::kRawKeyDown && !m_editCommand.empty()) {
        ui::LatencyInfo latency;
        latency.set_source_event_type(ui::SourceEventType::KEY_PRESS);
        content::EditCommands commands;
        commands.emplace_back(m_editCommand, "");
        m_editCommand.clear();
        host()->ForwardKeyboardEventWithCommands(webEvent, latency, &commands, nullptr);
        return;
    }

    bool keyDownTextInsertion = webEvent.GetType() == blink::WebInputEvent::kRawKeyDown && webEvent.text[0];
    webEvent.skip_in_browser = keyDownTextInsertion;
    host()->ForwardKeyboardEvent(webEvent);

    if (keyDownTextInsertion) {
        // Blink won't consume the RawKeyDown, but rather the Char event in this case.
        // The RawKeyDown is skipped on the way back (see above).
        // The same os_event will be set on both NativeWebKeyboardEvents.
        webEvent.skip_in_browser = false;
        webEvent.SetType(blink::WebInputEvent::kChar);
        host()->ForwardKeyboardEvent(webEvent);
    }
}

void RenderWidgetHostViewQt::handleInputMethodEvent(QInputMethodEvent *ev)
{
    // Reset input manager state
    m_imState = 0;

    if (!host())
        return;

    QString commitString = ev->commitString();
    QString preeditString = ev->preeditString();

    int cursorPositionInPreeditString = -1;
    gfx::Range selectionRange = gfx::Range::InvalidRange();

    const QList<QInputMethodEvent::Attribute> &attributes = ev->attributes();
    std::vector<ui::ImeTextSpan> underlines;
    bool hasSelection = false;

    for (const auto &attribute : attributes) {
        switch (attribute.type) {
        case QInputMethodEvent::TextFormat: {
            if (preeditString.isEmpty())
                break;

            int start = qMin(attribute.start, (attribute.start + attribute.length));
            int end = qMax(attribute.start, (attribute.start + attribute.length));

            // Blink does not support negative position values. Adjust start and end positions
            // to non-negative values.
            if (start < 0) {
                start = 0;
                end = qMax(0, start + end);
            }

            underlines.push_back(ui::ImeTextSpan(ui::ImeTextSpan::Type::kComposition, start, end, ui::ImeTextSpan::Thickness::kThin, SK_ColorTRANSPARENT));

            QTextCharFormat format = qvariant_cast<QTextFormat>(attribute.value).toCharFormat();
            if (format.underlineStyle() != QTextCharFormat::NoUnderline)
                underlines.back().underline_color = toSk(format.underlineColor());

            break;
        }
        case QInputMethodEvent::Cursor:
            // Always set the position of the cursor, even if it's marked invisible by Qt, otherwise
            // there is no way the user will know which part of the composition string will be
            // changed, when performing an IME-specific action (like selecting a different word
            // suggestion).
            cursorPositionInPreeditString = attribute.start;
            break;
        case QInputMethodEvent::Selection:
            hasSelection = true;

            // Cancel IME composition
            if (preeditString.isEmpty() && attribute.start + attribute.length == 0) {
                selectionRange.set_start(0);
                selectionRange.set_end(0);
                break;
            }

            selectionRange.set_start(qMin(attribute.start, (attribute.start + attribute.length)));
            selectionRange.set_end(qMax(attribute.start, (attribute.start + attribute.length)));
            break;
        default:
            break;
        }
    }

    if (!selectionRange.IsValid()) {
        // We did not receive a valid selection range, hence the range is going to mark the
        // cursor position.
        int newCursorPosition =
                (cursorPositionInPreeditString < 0) ? preeditString.length()
                                                    : cursorPositionInPreeditString;
        selectionRange.set_start(newCursorPosition);
        selectionRange.set_end(newCursorPosition);
    }

    if (hasSelection) {
        content::RenderFrameHostImpl *frameHost = static_cast<content::RenderFrameHostImpl *>(getFocusedFrameHost());
        if (frameHost)
            frameHost->GetFrameInputHandler()->SetEditableSelectionOffsets(selectionRange.start(), selectionRange.end());
    }

    int replacementLength = ev->replacementLength();
    gfx::Range replacementRange = gfx::Range::InvalidRange();

    if (replacementLength > 0)
    {
        int replacementStart = ev->replacementStart() < 0 ? m_cursorPosition + ev->replacementStart() : ev->replacementStart();
        if (replacementStart >= 0 && replacementStart < m_surroundingText.length())
            replacementRange = gfx::Range(replacementStart, replacementStart + replacementLength);
    }

    // There are so-far two known cases, when an empty QInputMethodEvent is received.
    // First one happens when backspace is used to remove the last character in the pre-edit
    // string, thus signaling the end of the composition.
    // The second one happens (on Windows) when a Korean char gets composed, but instead of
    // the event having a commit string, both strings are empty, and the actual char is received
    // as a QKeyEvent after the QInputMethodEvent is processed.
    // In lieu of the second case, we can't simply cancel the composition on an empty event,
    // and then add the Korean char when QKeyEvent is received, because that leads to text
    // flickering in the textarea (or any other element).
    // Instead we postpone the processing of the empty QInputMethodEvent by posting it
    // to the same focused object, and cancelling the composition on the next event loop tick.
    if (commitString.isEmpty() && preeditString.isEmpty() && replacementLength == 0) {
        if (!m_receivedEmptyImeEvent && m_imeInProgress && !hasSelection) {
            m_receivedEmptyImeEvent = true;
            QInputMethodEvent *eventCopy = new QInputMethodEvent(*ev);
            QGuiApplication::postEvent(qApp->focusObject(), eventCopy);
        } else {
            m_receivedEmptyImeEvent = false;
            if (m_imeInProgress) {
                m_imeInProgress = false;
                host()->ImeCancelComposition();
            }
        }

        return;
    }

    m_receivedEmptyImeEvent = false;

    // Finish compostion: insert or erase text.
    if (!commitString.isEmpty() || replacementLength > 0) {
        host()->ImeCommitText(toString16(commitString),
                              underlines,
                              replacementRange,
                              0);
        m_imeInProgress = false;
    }

    // Update or start new composition.
    // Be aware of that, we might get a commit string and a pre-edit string in a single event and
    // this means a new composition.
    if (!preeditString.isEmpty()) {
        host()->ImeSetComposition(toString16(preeditString),
                                  underlines,
                                  replacementRange,
                                  selectionRange.start(),
                                  selectionRange.end());
        m_imeInProgress = true;
    }
}

void RenderWidgetHostViewQt::handleInputMethodQueryEvent(QInputMethodQueryEvent *ev)
{
    Qt::InputMethodQueries queries = ev->queries();
    for (uint i = 0; i < 32; ++i) {
        Qt::InputMethodQuery query = (Qt::InputMethodQuery)(int)(queries & (1<<i));
        if (query) {
            QVariant v = inputMethodQuery(query);
            ev->setValue(query, v);
        }
    }
    ev->accept();
}

#ifndef QT_NO_ACCESSIBILITY
void RenderWidgetHostViewQt::accessibilityActiveChanged(bool active)
{
    if (active)
        content::BrowserAccessibilityStateImpl::GetInstance()->EnableAccessibility();
    else
        content::BrowserAccessibilityStateImpl::GetInstance()->DisableAccessibility();
}
#endif // QT_NO_ACCESSIBILITY

void RenderWidgetHostViewQt::handleWheelEvent(QWheelEvent *ev)
{
    if (!m_wheelAckPending) {
        Q_ASSERT(m_pendingWheelEvents.isEmpty());
        blink::WebMouseWheelEvent webEvent = WebEventFactory::toWebWheelEvent(ev, dpiScale());
        m_wheelAckPending = (webEvent.phase != blink::WebMouseWheelEvent::kPhaseEnded);
        m_mouseWheelPhaseHandler.AddPhaseIfNeededAndScheduleEndEvent(webEvent, false);
        host()->ForwardWheelEvent(webEvent);
        return;
    }
    if (!m_pendingWheelEvents.isEmpty()) {
        // Try to combine with this wheel event with the last pending one.
        if (WebEventFactory::coalesceWebWheelEvent(m_pendingWheelEvents.last(), ev, dpiScale()))
            return;
    }
    m_pendingWheelEvents.append(WebEventFactory::toWebWheelEvent(ev, dpiScale()));
}

void RenderWidgetHostViewQt::WheelEventAck(const blink::WebMouseWheelEvent &event, content::InputEventAckState /*ack_result*/)
{
    if (event.phase == blink::WebMouseWheelEvent::kPhaseEnded)
        return;
    Q_ASSERT(m_wheelAckPending);
    m_wheelAckPending = false;
    while (!m_pendingWheelEvents.isEmpty() && !m_wheelAckPending) {
        blink::WebMouseWheelEvent webEvent = m_pendingWheelEvents.takeFirst();
        m_wheelAckPending = (webEvent.phase != blink::WebMouseWheelEvent::kPhaseEnded);
        m_mouseWheelPhaseHandler.AddPhaseIfNeededAndScheduleEndEvent(webEvent, false);
        host()->ForwardWheelEvent(webEvent);
    }
    // TODO: We could forward unhandled wheelevents to our parent.
}

content::MouseWheelPhaseHandler *RenderWidgetHostViewQt::GetMouseWheelPhaseHandler()
{
    return &m_mouseWheelPhaseHandler;
}

void RenderWidgetHostViewQt::clearPreviousTouchMotionState()
{
    m_previousTouchPoints.clear();
    m_touchMotionStarted = false;
}

#ifndef QT_NO_GESTURES
void RenderWidgetHostViewQt::handleGestureEvent(QNativeGestureEvent *ev)
{
    const Qt::NativeGestureType type = ev->gestureType();
    // These are the only supported gestures by Chromium so far.
    if (type == Qt::ZoomNativeGesture || type == Qt::SmartZoomNativeGesture) {
        host()->ForwardGestureEvent(WebEventFactory::toWebGestureEvent(
                                        ev,
                                        static_cast<double>(dpiScale())));
    }
}
#endif

Q_DECLARE_LOGGING_CATEGORY(QWEBENGINE_TOUCH_HANDLING);
Q_LOGGING_CATEGORY(QWEBENGINE_TOUCH_HANDLING, "qt.webengine.touch");

void RenderWidgetHostViewQt::handleTouchEvent(QTouchEvent *ev)
{
    // On macOS instead of handling touch events, we use the OS provided QNativeGestureEvents.
#ifdef Q_OS_MACOS
    if (ev->spontaneous()) {
        return;
    } else {
        qCWarning(QWEBENGINE_TOUCH_HANDLING)
            << "Sending simulated touch events to Chromium does not work properly on macOS. "
               "Consider using QNativeGestureEvents or QMouseEvents.";
    }
#endif

    // Chromium expects the touch event timestamps to be comparable to base::TimeTicks::Now().
    // Most importantly we also have to preserve the relative time distance between events.
    // Calculate a delta between event timestamps and Now() on the first received event, and
    // apply this delta to all successive events. This delta is most likely smaller than it
    // should by calculating it here but this will hopefully cause less than one frame of delay.
    base::TimeTicks eventTimestamp = base::TimeTicks() + base::TimeDelta::FromMilliseconds(ev->timestamp());
    if (m_eventsToNowDelta == base::TimeDelta())
        m_eventsToNowDelta = base::TimeTicks::Now() - eventTimestamp;
    eventTimestamp += m_eventsToNowDelta;

    QList<QTouchEvent::TouchPoint> touchPoints = mapTouchPointIds(ev->touchPoints());

    switch (ev->type()) {
    case QEvent::TouchBegin:
        m_sendMotionActionDown = true;
        m_touchMotionStarted = true;
        break;
    case QEvent::TouchUpdate:
        m_touchMotionStarted = true;
        break;
    case QEvent::TouchCancel:
    {
        // Don't process a TouchCancel event if no motion was started beforehand, or if there are
        // no touch points in the current event or in the previously processed event.
        if (!m_touchMotionStarted || (touchPoints.isEmpty() && m_previousTouchPoints.isEmpty())) {
            clearPreviousTouchMotionState();
            return;
        }

        // Use last saved touch points for the cancel event, to get rid of a QList assert,
        // because Chromium expects a MotionEvent::ACTION_CANCEL instance to contain at least
        // one touch point, whereas a QTouchCancel may not contain any touch points at all.
        if (touchPoints.isEmpty())
            touchPoints = m_previousTouchPoints;
        clearPreviousTouchMotionState();
        MotionEventQt cancelEvent(touchPoints, eventTimestamp, ui::MotionEvent::Action::CANCEL,
                                  ev->modifiers(), dpiScale());
        processMotionEvent(cancelEvent);
        return;
    }
    case QEvent::TouchEnd:
        clearPreviousTouchMotionState();
        break;
    default:
        break;
    }

    if (m_imeInProgress && ev->type() == QEvent::TouchBegin) {
        m_imeInProgress = false;
        // Tell input method to commit the pre-edit string entered so far, and finish the
        // composition operation.
#ifdef Q_OS_WIN
        // Yes the function name is counter-intuitive, but commit isn't actually implemented
        // by the Windows QPA, and reset does exactly what is necessary in this case.
        qApp->inputMethod()->reset();
#else
        qApp->inputMethod()->commit();
#endif
    }

    // Make sure that ACTION_POINTER_DOWN is delivered before ACTION_MOVE,
    // and ACTION_MOVE before ACTION_POINTER_UP.
    std::sort(touchPoints.begin(), touchPoints.end(), compareTouchPoints);

    m_previousTouchPoints = touchPoints;
    for (int i = 0; i < touchPoints.size(); ++i) {
        ui::MotionEvent::Action action;
        switch (touchPoints[i].state()) {
        case Qt::TouchPointPressed:
            if (m_sendMotionActionDown) {
                action = ui::MotionEvent::Action::DOWN;
                m_sendMotionActionDown = false;
            } else {
                action = ui::MotionEvent::Action::POINTER_DOWN;
            }
            break;
        case Qt::TouchPointMoved:
            action = ui::MotionEvent::Action::MOVE;
            break;
        case Qt::TouchPointReleased:
            action = touchPoints.size() > 1 ? ui::MotionEvent::Action::POINTER_UP :
                                              ui::MotionEvent::Action::UP;
            break;
        default:
            // Ignore Qt::TouchPointStationary
            continue;
        }

        MotionEventQt motionEvent(touchPoints, eventTimestamp, action, ev->modifiers(), dpiScale(),
                                  i);
        processMotionEvent(motionEvent);
    }
}

#if QT_CONFIG(tabletevent)
void RenderWidgetHostViewQt::handleTabletEvent(QTabletEvent *event)
{
    handlePointerEvent<QTabletEvent>(event);
}
#endif

template<class T>
void RenderWidgetHostViewQt::handlePointerEvent(T *event)
{
    // Currently WebMouseEvent is a subclass of WebPointerProperties, so basically
    // tablet events are mouse events with extra properties.
    blink::WebMouseEvent webEvent = WebEventFactory::toWebMouseEvent(event, dpiScale());
    if ((webEvent.GetType() == blink::WebInputEvent::kMouseDown || webEvent.GetType() == blink::WebInputEvent::kMouseUp)
            && webEvent.button == blink::WebMouseEvent::Button::kNoButton) {
        // Blink can only handle the 3 main mouse-buttons and may assert when processing mouse-down for no button.
        return;
    }

    if (webEvent.GetType() == blink::WebInputEvent::kMouseDown) {
        if (event->button() != m_clickHelper.lastPressButton
            || (event->timestamp() - m_clickHelper.lastPressTimestamp > static_cast<ulong>(qGuiApp->styleHints()->mouseDoubleClickInterval()))
            || (event->pos() - m_clickHelper.lastPressPosition).manhattanLength() > qGuiApp->styleHints()->startDragDistance()
            || m_clickHelper.clickCounter >= 3)
            m_clickHelper.clickCounter = 0;

        m_clickHelper.lastPressTimestamp = event->timestamp();
        webEvent.click_count = ++m_clickHelper.clickCounter;
        m_clickHelper.lastPressButton = event->button();
        m_clickHelper.lastPressPosition = QPointF(event->pos()).toPoint();
    }

    webEvent.movement_x = event->globalX() - m_previousMousePosition.x();
    webEvent.movement_y = event->globalY() - m_previousMousePosition.y();

    if (IsMouseLocked())
        QCursor::setPos(m_previousMousePosition);
    else
        m_previousMousePosition = event->globalPos();

    if (m_imeInProgress && webEvent.GetType() == blink::WebInputEvent::kMouseDown) {
        m_imeInProgress = false;
        // Tell input method to commit the pre-edit string entered so far, and finish the
        // composition operation.
#ifdef Q_OS_WIN
        // Yes the function name is counter-intuitive, but commit isn't actually implemented
        // by the Windows QPA, and reset does exactly what is necessary in this case.
        qApp->inputMethod()->reset();
#else
        qApp->inputMethod()->commit();
#endif
    }

    host()->ForwardMouseEvent(webEvent);
}

void RenderWidgetHostViewQt::handleHoverEvent(QHoverEvent *ev)
{
    host()->ForwardMouseEvent(WebEventFactory::toWebMouseEvent(ev, dpiScale()));
}

void RenderWidgetHostViewQt::handleFocusEvent(QFocusEvent *ev)
{
    if (ev->gotFocus()) {
        host()->GotFocus();
        host()->SetActive(true);
        content::RenderViewHostImpl *viewHost = content::RenderViewHostImpl::From(host());
        Q_ASSERT(viewHost);
        if (ev->reason() == Qt::TabFocusReason)
            viewHost->SetInitialFocus(false);
        else if (ev->reason() == Qt::BacktabFocusReason)
            viewHost->SetInitialFocus(true);
        ev->accept();
    } else if (ev->lostFocus()) {
        host()->SetActive(false);
        host()->LostFocus();
        ev->accept();
    }
}

void RenderWidgetHostViewQt::SetNeedsBeginFrames(bool needs_begin_frames)
{
    m_compositor->setNeedsBeginFrames(needs_begin_frames);
}

void RenderWidgetHostViewQt::OnBeginFrame(base::TimeTicks frame_time)
{
    host()->ProgressFlingIfNeeded(frame_time);
}

content::RenderFrameHost *RenderWidgetHostViewQt::getFocusedFrameHost()
{
    content::RenderViewHostImpl *viewHost = content::RenderViewHostImpl::From(host());
    if (!viewHost)
        return nullptr;

    content::FrameTreeNode *focusedFrame = viewHost->GetDelegate()->GetFrameTree()->GetFocusedFrame();
    if (!focusedFrame)
        return nullptr;

    return focusedFrame->current_frame_host();
}

ui::TextInputType RenderWidgetHostViewQt::getTextInputType() const
{
    if (text_input_manager_ && text_input_manager_->GetTextInputState())
        return text_input_manager_->GetTextInputState()->type;

    return ui::TEXT_INPUT_TYPE_NONE;
}

void RenderWidgetHostViewQt::SetWantsAnimateOnlyBeginFrames()
{
}

viz::SurfaceId RenderWidgetHostViewQt::GetCurrentSurfaceId() const
{
    return viz::SurfaceId();
}

const viz::FrameSinkId &RenderWidgetHostViewQt::GetFrameSinkId() const
{
    return m_frameSinkId;
}

const viz::LocalSurfaceId &RenderWidgetHostViewQt::GetLocalSurfaceId() const
{
    return m_localSurfaceId;
}

void RenderWidgetHostViewQt::TakeFallbackContentFrom(content::RenderWidgetHostView *view)
{
    DCHECK(!static_cast<RenderWidgetHostViewBase*>(view)->IsRenderWidgetHostViewChildFrame());
    DCHECK(!static_cast<RenderWidgetHostViewBase*>(view)->IsRenderWidgetHostViewGuest());
    base::Optional<SkColor> color = view->GetBackgroundColor();
    if (color)
        SetBackgroundColor(*color);
}

void RenderWidgetHostViewQt::EnsureSurfaceSynchronizedForLayoutTest()
{
    ++m_latestCaptureSequenceNumber;
    if (host())
        host()->SynchronizeVisualProperties();
}

uint32_t RenderWidgetHostViewQt::GetCaptureSequenceNumber() const
{
    return m_latestCaptureSequenceNumber;
}

} // namespace QtWebEngineCore
