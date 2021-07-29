# Mojo For Chromium Developers

## Overview

This document contains the minimum amount of information needed for a developer
to start using Mojo in Chromium. For more detailed documentation on the C++
bindings, see [this link](/mojo/public/cpp/bindings/README.md).

## Terminology

A **message pipe** is a pair of **endpoints**. Each endpoint has a queue of
incoming messages, and writing a message to one endpoint effectively enqueues
that message on the other endpoint. Message pipes are thus bidirectional.

A **mojom** file describes **interfaces** which describe strongly typed message
structures, similar to proto files.

Given a **mojom interface** and a **message pipe**, the two **message pipes**
can be given the labels **InterfacePtr** and **Binding**. This now describes a
strongly typed **message pipe** which transports messages described by the
**mojom interface**. The **InterfacePtr** is the **endpoint** which "sends"
messages, and the **Binding** "receives" messages. Note that the **message
pipe** itself is still bidirectional, and it's possible for a message to have a
response callback, which the **InterfacePtr** would receive.

Another way to think of this is that an **InterfacePtr** is capable of making
remote calls on an implementation of the mojom interface associated with the
**Binding**.

The **Binding** itself is just glue that wires the endpoint's message queue up
to some implementation of the interface provided by the developer.

## Example

Let's apply this to Chrome. Let's say we want to send a "Ping" message from a
Browser to a Renderer. First we need to define the mojom interface.

```
module example.mojom;
interface PingResponder {
  // Receives a "Ping" and responds with a random integer.
  Ping() => (int random);
};
```

Now let's make a MessagePipe.
```cpp
example::mojom::PingResponderPtr ping_responder;
example::mojom::PingResponderRequest request = mojo::MakeRequest(&ping_responder);
```

In this example, ```ping_responder``` is the **InterfacePtr**, and ```request```
is an **InterfaceRequest**, which is a **Binding** precursor that will shortly
be turned into a **Binding**. Now we can send our Ping message.

```cpp
auto callback = base::Bind(&OnPong);
ping_responder->Ping(callback);
```

Important aside: If we want to receive the the response, we must keep the object
```ping_responder``` alive. After all, it's just a wrapper around a **Message
Pipe endpoint**, if it were to go away, there'd be nothing left to receive the
response.

We're done! Of course, if everything were this easy, this document wouldn't need
to exist. We've taken the hard problem of sending a message from the Browser to
a Renderer, and transformed it into a problem where we just need to take the
```request``` object, pass it to the Renderer, turn it into a **Binding**, and
implement the interface.

In Chrome, processes host services, and the services themselves are connected to
a Service Manager via **message pipes**. It's easy to pass ```request``` to the
appropriate Renderer using the Service Manager, but this requires explicitly
declaring our intentions via manifest files. For this example, we'll use the
content_browser service [manifest
file](https://cs.chromium.org/chromium/src/content/public/app/mojo/content_browser_manifest.json)
and the content_renderer service [manifest
file](https://cs.chromium.org/chromium/src/content/public/app/mojo/content_renderer_manifest.json).

```js
content_renderer_manifest.json:
...
  "interface_provider_specs": {
    "service_manager:connector": {
      "provides": {
        "cool_ping_feature": [
          "example::mojom::PingResponder"
        ]
      },
    },
...
```

```js
content_browser_manifest.json:
...
 "interface_provider_specs": {
    "service_manager:connector": {
      "requires": {
        "content_renderer": [ "cool_ping_feature" ],
      },
    },
  },
...
```

These changes indicate that the content_renderer service provides the interface
PingResponder, under the **capability** named "cool_ping_feature". And the
content_browser services intends to use this feature.
```content::BindInterface``` is a helper function that takes ```request``` and
sends it to the renderer process via the Service Manager.

```cpp
content::RenderProcessHost* host = GetRenderProcessHost();
content::BindInterface(host, std::move(request));
```

Putting this all together for the browser process:
```cpp
example::mojom::PingResponderPtr ping_responder;  // Make sure to keep this alive! Otherwise the response will never be received.
example::mojom::PingResponderRequest request = mojo::MakeRequest(&ping_responder);
ping_responder->Ping(base::BindOnce(&OnPong));
content::RenderProcessHost* host = GetRenderProcessHost();
content::BindInterface(host, std::move(request));
```

In the Renderer process, we need to write an implementation for PingResponder,
and ensure that a **Binding** is created using the transported ```request```. In a
standalone Mojo service, this would require us to implement
```service_manager::Service::OnBindInterface()```. In Chrome, this is abstracted
behind ```content::ConnectionFilters``` and
```service_manager::BinderRegistry```. This is typically done in
```RenderThreadImpl::Init```.

```cpp
class PingResponderImpl : mojom::PingResponder {
  void BindToInterface(example::mojom::PingResponderRequest request) {
    binding_.reset(
      new mojo::Binding<mojom::MemlogClient>(this, std::move(request)));
  }
  void Ping(PingCallback callback) { std::move(callback).Run(4); }
  std::unique_ptr<mojo::Binding<mojom::PingResponder>> binding_;
};

RenderThreadImpl::Init() {
...
  this->ping_responder = std::make_unique<PingResponderImpl>();
  auto registry = base::MakeUnique<service_manager::BinderRegistry>();

  // This makes the assumption that |this->ping_responder| will outlive |registry|.
  registry->AddInterface(base::Bind(&PingResponderImpl::BindToInterface), base::Unretained(this->ping_responder.get()));

  GetServiceManagerConnection()->AddConnectionFilter(
      base::MakeUnique<SimpleConnectionFilter>(std::move(registry)));
...
```
