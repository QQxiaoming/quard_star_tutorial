# Overview
At a high level, AR/VR (collectively known as XR) APIs are wrapped in
XRRuntimes.

Some XRRuntimes must live in the browser process, while others must not live in
the browser process.  The ones that cannot live in the browser, are hosted in a
service.

# Renderer <-> Browser interfaces (defined in vr_service.mojom)
VRService - lives in the browser process, corresponds to a single frame.  Root
object to obtain other XR objects.

VRDisplayHost - lives in the browser process.  Allows a client to start a
session (either immersive/exclusive/presenting or magic window).

VRServiceClient - lives in the renderer process.  Is notified when VRDisplays
are connected.

VRDisplayClient - lives in the renderer process.  Is notified when display
settings change.

# Renderer <-> Device interfaces (defined in vr_service.mojom)
These interfaces allow communication betwee an XRRuntime and the renderer
process.  They may live in the browser process or may live in the isolated
service.

## Presentation-related:
Presentation is exclusive access to a headset where a site may display
a stereo view to the user.

VRPresentationProvider - lives in the XRDevice process.  Implements the details
for a presentation session, such as submitting frames to the underlying VR API.

VRSubmitFrameClient - lives in the renderer process.  Is notified when various
rendering events occur, so it can reclaim/reuse textures.

## Magic-window related:
Magic window is a mode where a site may request poses, but renders through the
normal Chrome compositor pipeline.

VRMagicWindowProvider - lives in the XRDevice process.  Provides a way to obtain
poses.

# Browser <-> Device interfaces (defined in isolated_xr_service.mojom)
The XRDevice process may be the browser process or an isolated service for
different devices implementations.  A device provider in the browser will choose
to start the isolated device service when appropriate.

XRRuntime - an abstraction over a XR API.  Lives in the XRDevice process.
Exposes a way for the browser to register for events, and start sessions (Magic
Window or Presentation).

XRSessionController - Lives in the XRDevice process.  Allows the browser to
pause or stop a session (MagicWindow or Presentation).

XRRuntimeEventListener - Lives in the browser process.  Exposes runtime events
to the browser.
