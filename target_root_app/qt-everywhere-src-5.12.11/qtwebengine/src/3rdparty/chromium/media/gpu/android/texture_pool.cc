// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/gpu/android/texture_pool.h"

#include "gpu/command_buffer/service/abstract_texture.h"
#include "gpu/command_buffer/service/texture_manager.h"
#include "media/gpu/command_buffer_helper.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/scoped_make_current.h"

using gpu::gles2::AbstractTexture;

namespace media {

TexturePool::TexturePool(scoped_refptr<CommandBufferHelper> helper)
    : helper_(std::move(helper)), weak_factory_(this) {
  if (helper_) {
    helper_->SetWillDestroyStubCB(base::BindOnce(
        &TexturePool::OnWillDestroyStub, weak_factory_.GetWeakPtr()));
  }
}

TexturePool::~TexturePool() {
  // We'll drop all textures from the pool, if any.  It's okay if we don't have
  // a current context, since AbstractTexture handles it.
}

void TexturePool::AddTexture(std::unique_ptr<AbstractTexture> texture) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  DCHECK(pool_.find(texture.get()) == pool_.end());
  // Don't permit additions after we've lost the stub.
  // TODO(liberato): consider making this fail gracefully.  However, nobody
  // should be doing this, so for now it's a DCHECK.
  DCHECK(helper_);
  pool_.insert(std::move(texture));
}

void TexturePool::ReleaseTexture(AbstractTexture* texture,
                                 const gpu::SyncToken& sync_token) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  // If we don't have a sync token, or if we have no stub, then just finish.
  if (!sync_token.HasData() || !helper_) {
    OnSyncTokenReleased(texture);
    return;
  }

  // We keep a strong ref to |this| in the callback, so that we are guaranteed
  // to receive it.  It's common for the last ref to us to be our caller, as
  // a callback.  We need to stick around a bit longer than that if there's a
  // sync token.  Plus, we're required to keep |helper_| around while a wait is
  // still pending.
  helper_->WaitForSyncToken(
      sync_token, base::BindOnce(&TexturePool::OnSyncTokenReleased,
                                 scoped_refptr<TexturePool>(this), texture));
}

void TexturePool::OnSyncTokenReleased(AbstractTexture* texture) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  auto iter = pool_.find(texture);
  DCHECK(iter != pool_.end());

  // Drop the texture.  This is safe without the context being current.  It's
  // also safe if the stub has been destroyed.

  pool_.erase(iter);
}

void TexturePool::OnWillDestroyStub(bool have_context) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  DCHECK(helper_);
  // TODO(liberato): Should we drop all unrendered codec buffers here?  It seems
  // like a good idea, just to release the resources.  However, they won't block
  // decoding, since decoding requires the stub too.  More generally, it might
  // be worthwhile to have a callback on AbstractTexture that's called when it
  // transitions to not owning a texture.

  // Note that we don't have to do anything with |pool_|, since AbstractTextures
  // can outlive the stub that created them.  They just don't have a texture.
  helper_ = nullptr;
}

}  // namespace media
