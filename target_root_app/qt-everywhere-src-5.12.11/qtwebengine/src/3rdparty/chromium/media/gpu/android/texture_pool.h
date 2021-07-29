// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_GPU_ANDROID_TEXTURE_POOL_H_
#define MEDIA_GPU_ANDROID_TEXTURE_POOL_H_

#include "base/containers/flat_set.h"
#include "base/containers/unique_ptr_adapters.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread_checker.h"
#include "gpu/ipc/service/command_buffer_stub.h"
#include "media/gpu/media_gpu_export.h"

namespace gpu {
namespace gles2 {
class AbstractTexture;
}  // namespace gles2
}  // namespace gpu

namespace media {

class CommandBufferHelper;

// Owns Textures that are used to hold decoded video frames.  Allows them to
// outlive the decoder that created them, since decoders are torn down when the
// pipeline is suspended, but decoded frames can be on-screen indefinitely.
// TODO(tmathmeyer): Convert this into a pool.  Right now, we just constantly
// add new textures and remove them.
class MEDIA_GPU_EXPORT TexturePool : public base::RefCounted<TexturePool> {
 public:
  TexturePool(scoped_refptr<CommandBufferHelper> helper);

  // Add a new texture into the pool.  This may only be done before |stub_| is
  // destroyed.  When |stub_| is destroyed, we will destroy any textures that
  // are in the pool.
  //
  // Note that if we were really a pool this would mean "add |texture| into the
  // pool of available textures".  There would be some other call to allocate
  // a texture from the pool.
  void AddTexture(std::unique_ptr<gpu::gles2::AbstractTexture> texture);

  // Release a texture back into the pool.  |texture| must have been added to
  // the pool previously, and not released.  Otherwise, this is undefined.
  // Note: since we don't actually pool things, this just forgets |texture|.
  // It's okay if this is called after we've lost |stub_|.  If |sync_token| is
  // not null, then we'll wait for that token before taking any action.
  void ReleaseTexture(gpu::gles2::AbstractTexture* texture,
                      const gpu::SyncToken& sync_token);

 protected:
  virtual ~TexturePool();

  // Called after a sync token has been released, to free |texture|.
  void OnSyncTokenReleased(gpu::gles2::AbstractTexture* texture);

  // Called when |stub_| notifies us that the underlying stub will be destroyed.
  void OnWillDestroyStub(bool have_context);

 private:
  friend class base::RefCounted<TexturePool>;
  THREAD_CHECKER(thread_checker_);

  scoped_refptr<CommandBufferHelper> helper_;

  base::flat_set<std::unique_ptr<gpu::gles2::AbstractTexture>,
                 base::UniquePtrComparator>
      pool_;

  base::WeakPtrFactory<TexturePool> weak_factory_;
};

}  // namespace media

#endif  // MEDIA_GPU_ANDROID_TEXTURE_POOL_H_
