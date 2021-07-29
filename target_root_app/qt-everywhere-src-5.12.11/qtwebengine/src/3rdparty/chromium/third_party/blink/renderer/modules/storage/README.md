# `blink/renderer/modules/storage`

This diretory contains the renderer side implementation of the DOM Storage API.
This API is defined in the
[HTML Spec](https://html.spec.whatwg.org/multipage/webstorage.html)'s section on
Web Storage.

The browser side code for this lives in `content/browser/dom_storage/`.

## Class structure

`StorageArea` implements the WebIDL `Storage` interface. Instances of this class
will in the future hold a reference to a shared `CachedStorageArea` instance.
All `StorageArea` instances representing the same area will use the same
`CachedStorageArea` instance. Today instead each `StorageArea` owns a separate
`WebStorageArea` instance, implemented by the content layer.

`CachedStorageArea` will be responsible for communicating with the browser
process over the `StorageArea` mojom interface. It will also cache all the data
for a area in a `StorageAreaMap` instance, and will dispatch events it receives
to the relevant `StorageArea` and `InspectorDOMStorageAgent` instances.

`StorageAreaMap` represents the in-memory version of the data in a particular
storage area. It will be owned by a instance of the (yet-to-be-written)
`CachedStorageArea` class.
