# Service workers
[content/browser/service_worker]: /content/browser/service_worker
[content/renderer/service_worker]: /content/renderer/service_worker
[content/renderer/service_worker]: /content/renderer/service_worker
[content/common/service_worker]: /content/common/service_worker
[third_party/blink/common/service_worker]: /third_party/blink/common/service_worker
[third_party/blink/public/common/service_worker]: /third_party/blink/public/common/service_worker
[third_party/blink/public/mojom/service_worker]: /third_party/blink/public/mojom/service_worker
[third_party/blink/public/platform/modules/service_worker]: /third_party/blink/public/platform/modules/service_worker
[third_party/blink/public/web/modules/service_worker]: /third_party/blink/public/web/modules/service_worker
[third_party/blink/renderer/modules/service_worker]: /third_party/blink/renderer/modules/service_worker
[Blink Public API]: /third_party/blink/public
[Onion Soup]: https://docs.google.com/document/d/1K1nO8G9dO9kNSmtVz2gJ2GG9gQOTgm65sJlV3Fga4jE/edit?usp=sharing

This is Chromium's implementation of [service
workers](https://developer.mozilla.org/en-US/docs/Web/API/Service_Worker_API). See the
[Service Worker specification](https://w3c.github.io/ServiceWorker/).

## Directory structure

- [content/browser/service_worker]: Browser process code, including stored
  registration data, the inception of starting a service worker, and controlling
  navigations. The browser process has host objects of most live renderer
  entities that deal with service workers, and the bulk of work is performed by
  these host objects.
- [content/renderer/service_worker]: Renderer process code. This should move to
  third_party/blink per [Onion Soup].
- [content/common/service_worker]: Common process code.
- [third_party/blink/common/service_worker]: Common process code. Contains the
  implementation of third_party/blink/public/common/service_worker.
- [third_party/blink/public/common/service_worker]: Header files for common
  process code that can be used by both inside Blink and outside Blink.
- [third_party/blink/public/mojom/service_worker]: Mojom files for common
  process code that can be used by both Blink and content.
- [third_party/blink/public/platform/modules/service_worker]: [Blink Public API]
  header files. This should be removed per [Onion Soup].
- [third_party/blink/public/web/modules/service_worker]: More [Blink Public API]
  header files. This should be removed per [Onion Soup].
- [third_party/blink/renderer/modules/service_worker]: Renderer process code in
  Blink. This is the closest code to the web-exposed Service Worker API.

## Other documentation

- [Service Worker Security FAQ](/docs/security/service-worker-security-faq.md)
