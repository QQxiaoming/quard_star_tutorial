/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
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

#include "qgstreamervideooverlay_p.h"

#include <QtGui/qguiapplication.h>
#include "qgstutils_p.h"

#if !GST_CHECK_VERSION(1,0,0)
#include <gst/interfaces/xoverlay.h>
#else
#include <gst/video/videooverlay.h>
#endif

QT_BEGIN_NAMESPACE

struct ElementMap
{
   const char *qtPlatform;
   const char *gstreamerElement;
};

// Ordered by descending priority
static const ElementMap elementMap[] =
{
    { "xcb", "vaapisink" },
    { "xcb", "xvimagesink" },
    { "xcb", "ximagesink" }
};

class QGstreamerSinkProperties
{
public:
    virtual ~QGstreamerSinkProperties()
    {
    }

    virtual bool hasShowPrerollFrame() const = 0;
    virtual void reset() = 0;
    virtual int brightness() const = 0;
    virtual bool setBrightness(int brightness) = 0;
    virtual int contrast() const = 0;
    virtual bool setContrast(int contrast) = 0;
    virtual int hue() const = 0;
    virtual bool setHue(int hue) = 0;
    virtual int saturation() const = 0;
    virtual bool setSaturation(int saturation) = 0;
    virtual Qt::AspectRatioMode aspectRatioMode() const = 0;
    virtual void setAspectRatioMode(Qt::AspectRatioMode mode) = 0;
};

class QXVImageSinkProperties : public QGstreamerSinkProperties
{
public:
    QXVImageSinkProperties(GstElement *sink)
        : m_videoSink(sink)
    {
        m_hasForceAspectRatio = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "force-aspect-ratio");
        m_hasBrightness = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "brightness");
        m_hasContrast = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "contrast");
        m_hasHue = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "hue");
        m_hasSaturation = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "saturation");
        m_hasShowPrerollFrame = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "show-preroll-frame");
    }

    bool hasShowPrerollFrame() const override
    {
        return m_hasShowPrerollFrame;
    }

    void reset() override
    {
        setAspectRatioMode(m_aspectRatioMode);
        setBrightness(m_brightness);
        setContrast(m_contrast);
        setHue(m_hue);
        setSaturation(m_saturation);
    }

    int brightness() const override
    {
        int brightness = 0;
        if (m_hasBrightness)
            g_object_get(G_OBJECT(m_videoSink), "brightness", &brightness, NULL);

        return brightness / 10;
    }

    bool setBrightness(int brightness) override
    {
        m_brightness = brightness;
        if (m_hasBrightness)
            g_object_set(G_OBJECT(m_videoSink), "brightness", brightness * 10, NULL);

        return m_hasBrightness;
    }

    int contrast() const override
    {
        int contrast = 0;
        if (m_hasContrast)
            g_object_get(G_OBJECT(m_videoSink), "contrast", &contrast, NULL);

        return contrast / 10;
    }

    bool setContrast(int contrast) override
    {
        m_contrast = contrast;
        if (m_hasContrast)
            g_object_set(G_OBJECT(m_videoSink), "contrast", contrast * 10, NULL);

        return m_hasContrast;
    }

    int hue() const override
    {
        int hue = 0;
        if (m_hasHue)
            g_object_get(G_OBJECT(m_videoSink), "hue", &hue, NULL);

        return hue / 10;
    }

    bool setHue(int hue) override
    {
        m_hue = hue;
        if (m_hasHue)
            g_object_set(G_OBJECT(m_videoSink), "hue", hue * 10, NULL);

        return m_hasHue;
    }

    int saturation() const override
    {
        int saturation = 0;
        if (m_hasSaturation)
            g_object_get(G_OBJECT(m_videoSink), "saturation", &saturation, NULL);

        return saturation / 10;
    }

    bool setSaturation(int saturation) override
    {
        m_saturation = saturation;
        if (m_hasSaturation)
            g_object_set(G_OBJECT(m_videoSink), "saturation", saturation * 10, NULL);

        return m_hasSaturation;
    }

    Qt::AspectRatioMode aspectRatioMode() const override
    {
        Qt::AspectRatioMode mode = Qt::KeepAspectRatio;
        if (m_hasForceAspectRatio) {
            gboolean forceAR = false;
            g_object_get(G_OBJECT(m_videoSink), "force-aspect-ratio", &forceAR, NULL);
            if (!forceAR)
                mode = Qt::IgnoreAspectRatio;
        }

        return mode;
    }

    void setAspectRatioMode(Qt::AspectRatioMode mode) override
    {
        m_aspectRatioMode = mode;
        if (m_hasForceAspectRatio) {
            g_object_set(G_OBJECT(m_videoSink),
                         "force-aspect-ratio",
                         (mode == Qt::KeepAspectRatio),
                         (const char*)NULL);
        }
    }

protected:

    GstElement *m_videoSink = nullptr;
    bool m_hasForceAspectRatio = false;
    bool m_hasBrightness = false;
    bool m_hasContrast = false;
    bool m_hasHue = false;
    bool m_hasSaturation = false;
    bool m_hasShowPrerollFrame = false;
    Qt::AspectRatioMode m_aspectRatioMode = Qt::KeepAspectRatio;
    int m_brightness = 0;
    int m_contrast = 0;
    int m_hue = 0;
    int m_saturation = 0;
};

class QVaapiSinkProperties : public QXVImageSinkProperties
{
public:
    QVaapiSinkProperties(GstElement *sink)
        : QXVImageSinkProperties(sink)
    {
        // Set default values.
        m_contrast = 1;
        m_saturation = 1;
    }

    int brightness() const override
    {
        gfloat brightness = 0;
        if (m_hasBrightness)
            g_object_get(G_OBJECT(m_videoSink), "brightness", &brightness, NULL);

        return brightness * 100; // [-1,1] -> [-100,100]
    }

    bool setBrightness(int brightness) override
    {
        m_brightness = brightness;
        if (m_hasBrightness) {
            gfloat v = brightness / 100.0; // [-100,100] -> [-1,1]
            g_object_set(G_OBJECT(m_videoSink), "brightness", v, NULL);
        }

        return m_hasBrightness;
    }

    int contrast() const override
    {
        gfloat contrast = 1;
        if (m_hasContrast)
            g_object_get(G_OBJECT(m_videoSink), "contrast", &contrast, NULL);

        return (contrast - 1) * 100; // [0,2] -> [-100,100]
    }

    bool setContrast(int contrast) override
    {
        m_contrast = contrast;
        if (m_hasContrast) {
            gfloat v = (contrast / 100.0) + 1; // [-100,100] -> [0,2]
            g_object_set(G_OBJECT(m_videoSink), "contrast", v, NULL);
        }

        return m_hasContrast;
    }

    int hue() const override
    {
        gfloat hue = 0;
        if (m_hasHue)
            g_object_get(G_OBJECT(m_videoSink), "hue", &hue, NULL);

        return hue / 180 * 100; // [-180,180] -> [-100,100]
    }

    bool setHue(int hue) override
    {
        m_hue = hue;
        if (m_hasHue) {
            gfloat v = hue / 100.0 * 180; // [-100,100] -> [-180,180]
            g_object_set(G_OBJECT(m_videoSink), "hue", v, NULL);
        }

        return m_hasHue;
    }

    int saturation() const override
    {
        gfloat saturation = 1;
        if (m_hasSaturation)
            g_object_get(G_OBJECT(m_videoSink), "saturation", &saturation, NULL);

        return (saturation - 1) * 100; // [0,2] -> [-100,100]
    }

    bool setSaturation(int saturation) override
    {
        m_saturation = saturation;
        if (m_hasSaturation) {
            gfloat v = (saturation / 100.0) + 1; // [-100,100] -> [0,2]
            g_object_set(G_OBJECT(m_videoSink), "saturation", v, NULL);
        }

        return m_hasSaturation;
    }
};

static bool qt_gst_element_is_functioning(GstElement *element)
{
    GstStateChangeReturn ret = gst_element_set_state(element, GST_STATE_READY);
    if (ret == GST_STATE_CHANGE_SUCCESS) {
        gst_element_set_state(element, GST_STATE_NULL);
        return true;
    }

    return false;
}

static GstElement *findBestVideoSink()
{
    GstElement *choice = 0;
    QString platform = QGuiApplication::platformName();

    // We need a native window ID to use the GstVideoOverlay interface.
    // Bail out if the Qt platform plugin in use cannot provide a sensible WId.
    if (platform != QLatin1String("xcb"))
        return 0;

    // First, try some known video sinks, depending on the Qt platform plugin in use.
    for (quint32 i = 0; i < (sizeof(elementMap) / sizeof(ElementMap)); ++i) {
        if (platform == QLatin1String(elementMap[i].qtPlatform)
                && (choice = gst_element_factory_make(elementMap[i].gstreamerElement, NULL))) {

            if (qt_gst_element_is_functioning(choice))
                return choice;

            gst_object_unref(choice);
            choice = 0;
        }
    }

    // If none of the known video sinks are available, try to find one that implements the
    // GstVideoOverlay interface and has autoplugging rank.
    GList *list = qt_gst_video_sinks();
    for (GList *item = list; item != NULL; item = item->next) {
        GstElementFactory *f = GST_ELEMENT_FACTORY(item->data);

        if (!gst_element_factory_has_interface(f, QT_GSTREAMER_VIDEOOVERLAY_INTERFACE_NAME))
            continue;

        if (GstElement *el = gst_element_factory_create(f, NULL)) {
            if (qt_gst_element_is_functioning(el)) {
                choice = el;
                break;
            }

            gst_object_unref(el);
        }
    }

    gst_plugin_feature_list_free(list);

    return choice;
}

QGstreamerVideoOverlay::QGstreamerVideoOverlay(QObject *parent, const QByteArray &elementName)
    : QObject(parent)
    , QGstreamerBufferProbe(QGstreamerBufferProbe::ProbeCaps)
{
    GstElement *sink = nullptr;
    if (!elementName.isEmpty())
        sink = gst_element_factory_make(elementName.constData(), NULL);
    else
        sink = findBestVideoSink();

    setVideoSink(sink);
}

QGstreamerVideoOverlay::~QGstreamerVideoOverlay()
{
    if (m_videoSink) {
        delete m_sinkProperties;
        GstPad *pad = gst_element_get_static_pad(m_videoSink, "sink");
        removeProbeFromPad(pad);
        gst_object_unref(GST_OBJECT(pad));
        gst_object_unref(GST_OBJECT(m_videoSink));
    }
}

GstElement *QGstreamerVideoOverlay::videoSink() const
{
    return m_videoSink;
}

void QGstreamerVideoOverlay::setVideoSink(GstElement *sink)
{
    if (!sink)
        return;

    if (m_videoSink)
        gst_object_unref(GST_OBJECT(m_videoSink));

    m_videoSink = sink;
    qt_gst_object_ref_sink(GST_OBJECT(m_videoSink));

    GstPad *pad = gst_element_get_static_pad(m_videoSink, "sink");
    addProbeToPad(pad);
    gst_object_unref(GST_OBJECT(pad));

    QString sinkName(QLatin1String(GST_OBJECT_NAME(sink)));
    bool isVaapi = sinkName.startsWith(QLatin1String("vaapisink"));
    delete m_sinkProperties;
    m_sinkProperties = isVaapi ? new QVaapiSinkProperties(sink) : new QXVImageSinkProperties(sink);

    if (m_sinkProperties->hasShowPrerollFrame())
        g_signal_connect(m_videoSink, "notify::show-preroll-frame",
                         G_CALLBACK(showPrerollFrameChanged), this);
}

QSize QGstreamerVideoOverlay::nativeVideoSize() const
{
    return m_nativeVideoSize;
}

void QGstreamerVideoOverlay::setWindowHandle(WId id)
{
    m_windowId = id;

    if (isActive())
        setWindowHandle_helper(id);
}

void QGstreamerVideoOverlay::setWindowHandle_helper(WId id)
{
#if GST_CHECK_VERSION(1,0,0)
    if (m_videoSink && GST_IS_VIDEO_OVERLAY(m_videoSink)) {
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(m_videoSink), id);
#else
    if (m_videoSink && GST_IS_X_OVERLAY(m_videoSink)) {
# if GST_CHECK_VERSION(0,10,31)
        gst_x_overlay_set_window_handle(GST_X_OVERLAY(m_videoSink), id);
# else
        gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(m_videoSink), id);
# endif
#endif

        // Properties need to be reset when changing the winId.
        m_sinkProperties->reset();
    }
}

void QGstreamerVideoOverlay::expose()
{
    if (!isActive())
        return;

#if !GST_CHECK_VERSION(1,0,0)
    if (m_videoSink && GST_IS_X_OVERLAY(m_videoSink))
        gst_x_overlay_expose(GST_X_OVERLAY(m_videoSink));
#else
    if (m_videoSink && GST_IS_VIDEO_OVERLAY(m_videoSink)) {
        gst_video_overlay_expose(GST_VIDEO_OVERLAY(m_videoSink));
    }
#endif
}

void QGstreamerVideoOverlay::setRenderRectangle(const QRect &rect)
{
    int x = -1;
    int y = -1;
    int w = -1;
    int h = -1;

    if (!rect.isEmpty()) {
        x = rect.x();
        y = rect.y();
        w = rect.width();
        h = rect.height();
    }

#if GST_CHECK_VERSION(1,0,0)
    if (m_videoSink && GST_IS_VIDEO_OVERLAY(m_videoSink))
        gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(m_videoSink), x, y, w, h);
#elif GST_CHECK_VERSION(0, 10, 29)
    if (m_videoSink && GST_IS_X_OVERLAY(m_videoSink))
        gst_x_overlay_set_render_rectangle(GST_X_OVERLAY(m_videoSink), x, y , w , h);
#else
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(w)
    Q_UNUSED(h)
#endif
}

bool QGstreamerVideoOverlay::processSyncMessage(const QGstreamerMessage &message)
{
    GstMessage* gm = message.rawMessage();

#if !GST_CHECK_VERSION(1,0,0)
    if (gm && (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_ELEMENT) &&
            gst_structure_has_name(gm->structure, "prepare-xwindow-id")) {
#else
      if (gm && (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_ELEMENT) &&
              gst_structure_has_name(gst_message_get_structure(gm), "prepare-window-handle")) {
#endif
        setWindowHandle_helper(m_windowId);
        return true;
    }

    return false;
}

bool QGstreamerVideoOverlay::processBusMessage(const QGstreamerMessage &message)
{
    GstMessage* gm = message.rawMessage();

    if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_STATE_CHANGED &&
            GST_MESSAGE_SRC(gm) == GST_OBJECT_CAST(m_videoSink)) {

        updateIsActive();
    }

    return false;
}

void QGstreamerVideoOverlay::probeCaps(GstCaps *caps)
{
    QSize size = QGstUtils::capsCorrectedResolution(caps);
    if (size != m_nativeVideoSize) {
        m_nativeVideoSize = size;
        emit nativeVideoSizeChanged();
    }
}

bool QGstreamerVideoOverlay::isActive() const
{
    return m_isActive;
}

void QGstreamerVideoOverlay::updateIsActive()
{
    if (!m_videoSink)
        return;

    GstState state = GST_STATE(m_videoSink);
    gboolean showPreroll = true;

    if (m_sinkProperties->hasShowPrerollFrame())
        g_object_get(G_OBJECT(m_videoSink), "show-preroll-frame", &showPreroll, NULL);

    bool newIsActive = (state == GST_STATE_PLAYING || (state == GST_STATE_PAUSED && showPreroll));

    if (newIsActive != m_isActive) {
        m_isActive = newIsActive;
        emit activeChanged();
    }
}

void QGstreamerVideoOverlay::showPrerollFrameChanged(GObject *, GParamSpec *, QGstreamerVideoOverlay *overlay)
{
    overlay->updateIsActive();
}

Qt::AspectRatioMode QGstreamerVideoOverlay::aspectRatioMode() const
{
    return m_sinkProperties->aspectRatioMode();
}

void QGstreamerVideoOverlay::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    m_sinkProperties->setAspectRatioMode(mode);
}

int QGstreamerVideoOverlay::brightness() const
{
    return m_sinkProperties->brightness();
}

void QGstreamerVideoOverlay::setBrightness(int brightness)
{
    if (m_sinkProperties->setBrightness(brightness))
        emit brightnessChanged(brightness);
}

int QGstreamerVideoOverlay::contrast() const
{
    return m_sinkProperties->contrast();
}

void QGstreamerVideoOverlay::setContrast(int contrast)
{
    if (m_sinkProperties->setContrast(contrast))
        emit contrastChanged(contrast);
}

int QGstreamerVideoOverlay::hue() const
{
    return m_sinkProperties->hue();
}

void QGstreamerVideoOverlay::setHue(int hue)
{
    if (m_sinkProperties->setHue(hue))
        emit hueChanged(hue);
}

int QGstreamerVideoOverlay::saturation() const
{
    return m_sinkProperties->saturation();
}

void QGstreamerVideoOverlay::setSaturation(int saturation)
{
    if (m_sinkProperties->setSaturation(saturation))
        emit saturationChanged(saturation);
}

QT_END_NAMESPACE
