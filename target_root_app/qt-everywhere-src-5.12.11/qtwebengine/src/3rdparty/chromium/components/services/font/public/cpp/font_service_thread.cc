// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/services/font/public/cpp/font_service_thread.h"

#include <utility>

#include "base/bind.h"
#include "base/files/file.h"
#include "base/synchronization/waitable_event.h"
#include "components/services/font/public/cpp/mapped_font_file.h"

namespace font_service {
namespace internal {

namespace {
const char kFontThreadName[] = "Font_Proxy_Thread";
}  // namespace

FontServiceThread::FontServiceThread(mojom::FontServicePtr font_service)
    : base::Thread(kFontThreadName),
      font_service_info_(font_service.PassInterface()),
      weak_factory_(this) {
  DETACH_FROM_THREAD(thread_checker_);
  Start();
}

bool FontServiceThread::MatchFamilyName(
    const char family_name[],
    SkFontStyle requested_style,
    SkFontConfigInterface::FontIdentity* out_font_identity,
    SkString* out_family_name,
    SkFontStyle* out_style) {
  DCHECK_NE(GetThreadId(), base::PlatformThread::CurrentId());

  bool out_valid = false;
  // This proxies to the other thread, which proxies to mojo. Only on the reply
  // from mojo do we return from this.
  base::WaitableEvent done_event;
  task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&FontServiceThread::MatchFamilyNameImpl, this, &done_event,
                     family_name, requested_style, &out_valid,
                     out_font_identity, out_family_name, out_style));
  done_event.Wait();

  return out_valid;
}

bool FontServiceThread::FallbackFontForCharacter(
    uint32_t character,
    std::string locale,
    font_service::mojom::FontIdentityPtr* out_font_identity,
    std::string* out_family_name,
    bool* out_is_bold,
    bool* out_is_italic) {
  DCHECK_NE(GetThreadId(), base::PlatformThread::CurrentId());
  bool out_valid = false;
  base::WaitableEvent done_event;
  task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&FontServiceThread::FallbackFontForCharacterImpl, this,
                     &done_event, character, std::move(locale), &out_valid,
                     out_font_identity, out_family_name, out_is_bold,
                     out_is_italic));
  done_event.Wait();

  return out_valid;
}

bool FontServiceThread::FontRenderStyleForStrike(
    std::string family,
    uint32_t size,
    bool is_italic,
    bool is_bold,
    float device_scale_factor,
    font_service::mojom::FontRenderStylePtr* out_font_render_style) {
  DCHECK_NE(GetThreadId(), base::PlatformThread::CurrentId());
  bool out_valid = false;
  base::WaitableEvent done_event;
  task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&FontServiceThread::FontRenderStyleForStrikeImpl, this,
                     &done_event, family, size, is_italic, is_bold,
                     device_scale_factor, &out_valid, out_font_render_style));
  done_event.Wait();
  return out_valid;
}

void FontServiceThread::MatchFontWithFallback(
    std::string family,
    bool is_bold,
    bool is_italic,
    uint32_t charset,
    uint32_t fallback_family_type,
    base::File* out_font_file_handle) {
  DCHECK_NE(GetThreadId(), base::PlatformThread::CurrentId());
  base::WaitableEvent done_event;
  task_runner()->PostTask(
      FROM_HERE,
      base::BindOnce(&FontServiceThread::MatchFontWithFallbackImpl, this,
                     &done_event, std::move(family), is_bold, is_italic,
                     charset, fallback_family_type, out_font_file_handle));
  done_event.Wait();
}

scoped_refptr<MappedFontFile> FontServiceThread::OpenStream(
    const SkFontConfigInterface::FontIdentity& identity) {
  DCHECK_NE(GetThreadId(), base::PlatformThread::CurrentId());

  base::File stream_file;
  // This proxies to the other thread, which proxies to mojo. Only on the
  // reply from mojo do we return from this.
  base::WaitableEvent done_event;
  task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&FontServiceThread::OpenStreamImpl, this,
                                &done_event, &stream_file, identity.fID));
  done_event.Wait();

  if (!stream_file.IsValid()) {
    // The font-service may have been killed.
    return nullptr;
  }

  // Converts the file to out internal type.
  scoped_refptr<MappedFontFile> mapped_font_file =
      new MappedFontFile(identity.fID);
  if (!mapped_font_file->Initialize(std::move(stream_file)))
    return nullptr;

  return mapped_font_file;
}

FontServiceThread::~FontServiceThread() {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  Stop();
}

void FontServiceThread::MatchFamilyNameImpl(
    base::WaitableEvent* done_event,
    const char family_name[],
    SkFontStyle requested_style,
    bool* out_valid,
    SkFontConfigInterface::FontIdentity* out_font_identity,
    SkString* out_family_name,
    SkFontStyle* out_style) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (font_service_.encountered_error()) {
    *out_valid = false;
    done_event->Signal();
    return;
  }

  mojom::TypefaceStylePtr style(mojom::TypefaceStyle::New());
  style->weight = requested_style.weight();
  style->width = requested_style.width();
  style->slant = static_cast<mojom::TypefaceSlant>(requested_style.slant());

  const char* arg_family = family_name ? family_name : "";

  pending_waitable_events_.insert(done_event);
  font_service_->MatchFamilyName(
      arg_family, std::move(style),
      base::BindOnce(&FontServiceThread::OnMatchFamilyNameComplete, this,
                     done_event, out_valid, out_font_identity, out_family_name,
                     out_style));
}

void FontServiceThread::OnMatchFamilyNameComplete(
    base::WaitableEvent* done_event,
    bool* out_valid,
    SkFontConfigInterface::FontIdentity* out_font_identity,
    SkString* out_family_name,
    SkFontStyle* out_style,
    mojom::FontIdentityPtr font_identity,
    const std::string& family_name,
    mojom::TypefaceStylePtr style) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  pending_waitable_events_.erase(done_event);

  *out_valid = !font_identity.is_null();
  if (font_identity) {
    out_font_identity->fID = font_identity->id;
    out_font_identity->fTTCIndex = font_identity->ttc_index;
    out_font_identity->fString = font_identity->str_representation.data();
    // TODO(erg): fStyle isn't set. This is rather odd, however it matches the
    // behaviour of the current Linux IPC version.

    *out_family_name = family_name.data();
    *out_style = SkFontStyle(style->weight, style->width,
                             static_cast<SkFontStyle::Slant>(style->slant));
  }

  done_event->Signal();
}

void FontServiceThread::OpenStreamImpl(base::WaitableEvent* done_event,
                                       base::File* output_file,
                                       const uint32_t id_number) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  if (font_service_.encountered_error()) {
    done_event->Signal();
    return;
  }

  pending_waitable_events_.insert(done_event);
  font_service_->OpenStream(
      id_number, base::BindOnce(&FontServiceThread::OnOpenStreamComplete, this,
                                done_event, output_file));
}

void FontServiceThread::OnOpenStreamComplete(base::WaitableEvent* done_event,
                                             base::File* output_file,
                                             base::File file) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  pending_waitable_events_.erase(done_event);
  *output_file = std::move(file);
  done_event->Signal();
}

void FontServiceThread::FallbackFontForCharacterImpl(
    base::WaitableEvent* done_event,
    uint32_t character,
    std::string locale,
    bool* out_valid,
    font_service::mojom::FontIdentityPtr* out_font_identity,
    std::string* out_family_name,
    bool* out_is_bold,
    bool* out_is_italic) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (font_service_.encountered_error()) {
    *out_valid = false;
    done_event->Signal();
    return;
  }

  pending_waitable_events_.insert(done_event);
  font_service_->FallbackFontForCharacter(
      character, std::move(locale),
      base::BindOnce(&FontServiceThread::OnFallbackFontForCharacterComplete,
                     this, done_event, out_valid, out_font_identity,
                     out_family_name, out_is_bold, out_is_italic));
}

void FontServiceThread::OnFallbackFontForCharacterComplete(
    base::WaitableEvent* done_event,
    bool* out_valid,
    font_service::mojom::FontIdentityPtr* out_font_identity,
    std::string* out_family_name,
    bool* out_is_bold,
    bool* out_is_italic,
    mojom::FontIdentityPtr font_identity,
    const std::string& family_name,
    bool is_bold,
    bool is_italic) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  pending_waitable_events_.erase(done_event);

  *out_valid = !font_identity.is_null();
  if (font_identity) {
    *out_font_identity = std::move(font_identity);
    *out_family_name = family_name.data();
    *out_is_bold = is_bold;
    *out_is_italic = is_italic;
  }
  done_event->Signal();
}

void FontServiceThread::FontRenderStyleForStrikeImpl(
    base::WaitableEvent* done_event,
    std::string family,
    uint32_t size,
    bool is_italic,
    bool is_bold,
    float device_scale_factor,
    bool* out_valid,
    mojom::FontRenderStylePtr* out_font_render_style) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);

  if (font_service_.encountered_error()) {
    *out_valid = false;
    done_event->Signal();
    return;
  }

  pending_waitable_events_.insert(done_event);
  font_service_->FontRenderStyleForStrike(
      std::move(family), size, is_italic, is_bold, device_scale_factor,
      base::BindOnce(&FontServiceThread::OnFontRenderStyleForStrikeComplete,
                     this, done_event, out_valid, out_font_render_style));
}

void FontServiceThread::OnFontRenderStyleForStrikeComplete(
    base::WaitableEvent* done_event,
    bool* out_valid,
    mojom::FontRenderStylePtr* out_font_render_style,
    mojom::FontRenderStylePtr font_render_style) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  pending_waitable_events_.erase(done_event);

  *out_valid = !font_render_style.is_null();
  if (font_render_style) {
    *out_font_render_style = std::move(font_render_style);
  }
  done_event->Signal();
}

void FontServiceThread::MatchFontWithFallbackImpl(
    base::WaitableEvent* done_event,
    std::string family,
    bool is_bold,
    bool is_italic,
    uint32_t charset,
    uint32_t fallback_family_type,
    base::File* out_font_file_handle) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  *out_font_file_handle = base::File();
  if (font_service_.encountered_error()) {
    done_event->Signal();
    return;
  }
  pending_waitable_events_.insert(done_event);
  font_service_->MatchFontWithFallback(
      family, is_bold, is_italic, charset, fallback_family_type,
      base::BindOnce(&FontServiceThread::OnMatchFontWithFallbackComplete, this,
                     done_event, out_font_file_handle));
}

void FontServiceThread::OnMatchFontWithFallbackComplete(
    base::WaitableEvent* done_event,
    base::File* out_font_file_handle,
    base::File file) {
  DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
  pending_waitable_events_.erase(done_event);

  *out_font_file_handle = std::move(file);
  done_event->Signal();
}

void FontServiceThread::OnFontServiceConnectionError() {
  std::set<base::WaitableEvent*> events;
  events.swap(pending_waitable_events_);
  for (base::WaitableEvent* event : events)
    event->Signal();
}

void FontServiceThread::Init() {
  font_service_.Bind(std::move(font_service_info_));
  font_service_.set_connection_error_handler(
      base::BindOnce(&FontServiceThread::OnFontServiceConnectionError,
                     weak_factory_.GetWeakPtr()));
}

void FontServiceThread::CleanUp() {
  font_service_.reset();
}

}  // namespace internal
}  // namespace font_service
