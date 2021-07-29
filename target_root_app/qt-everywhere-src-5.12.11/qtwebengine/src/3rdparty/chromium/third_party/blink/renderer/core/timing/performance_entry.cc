/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "third_party/blink/renderer/core/timing/performance_entry.h"

#include "base/atomic_sequence_num.h"
#include "third_party/blink/renderer/bindings/core/v8/script_value.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_object_builder.h"

namespace blink {

namespace {
static base::AtomicSequenceNumber index_seq;
}

PerformanceEntry::PerformanceEntry(const String& name,
                                   double start_time,
                                   double finish_time)
    : duration_(finish_time - start_time),
      name_(name),
      start_time_(start_time),
      index_(index_seq.GetNext()) {}

PerformanceEntry::~PerformanceEntry() = default;

String PerformanceEntry::name() const {
  return name_;
}

DOMHighResTimeStamp PerformanceEntry::startTime() const {
  return start_time_;
}

DOMHighResTimeStamp PerformanceEntry::duration() const {
  return duration_;
}

const AtomicString& PerformanceEntry::CompositeKeyword() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ThreadSpecific<AtomicString>, composite, ());
  if (!composite.IsSet())
    *composite = "composite";
  return *composite;
}

const AtomicString& PerformanceEntry::EventKeyword() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ThreadSpecific<AtomicString>, event, ());
  if (!event.IsSet())
    *event = "event";
  return *event;
}

const AtomicString& PerformanceEntry::FirstInputKeyword() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ThreadSpecific<AtomicString>, firstInput, ());
  if (!firstInput.IsSet())
    *firstInput = "firstInput";
  return *firstInput;
}

const AtomicString& PerformanceEntry::LongtaskKeyword() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ThreadSpecific<AtomicString>, longtask, ());
  if (!longtask.IsSet())
    *longtask = "longtask";
  return *longtask;
}

const AtomicString& PerformanceEntry::MarkKeyword() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ThreadSpecific<AtomicString>, mark, ());
  if (!mark.IsSet())
    *mark = "mark";
  return *mark;
}

const AtomicString& PerformanceEntry::MeasureKeyword() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ThreadSpecific<AtomicString>, measure, ());
  if (!measure.IsSet())
    *measure = "measure";
  return *measure;
}

const AtomicString& PerformanceEntry::NavigationKeyword() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ThreadSpecific<AtomicString>, navigation, ());
  if (!navigation.IsSet())
    *navigation = "navigation";
  return *navigation;
}

const AtomicString& PerformanceEntry::PaintKeyword() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ThreadSpecific<AtomicString>, paint, ());
  if (!paint.IsSet())
    *paint = "paint";
  return *paint;
}

const AtomicString& PerformanceEntry::RenderKeyword() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ThreadSpecific<AtomicString>, render, ());
  if (!render.IsSet())
    *render = "render";
  return *render;
}

const AtomicString& PerformanceEntry::ResourceKeyword() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ThreadSpecific<AtomicString>, resource, ());
  if (!resource.IsSet())
    *resource = "resource";
  return *resource;
}

const AtomicString& PerformanceEntry::TaskattributionKeyword() {
  DEFINE_THREAD_SAFE_STATIC_LOCAL(ThreadSpecific<AtomicString>, taskattribution,
                                  ());
  if (!taskattribution.IsSet())
    *taskattribution = "taskattribution";
  return *taskattribution;
}

PerformanceEntry::EntryType PerformanceEntry::ToEntryTypeEnum(
    const AtomicString& entry_type) {
  if (entry_type == CompositeKeyword())
    return kComposite;
  if (entry_type == LongtaskKeyword())
    return kLongTask;
  if (entry_type == MarkKeyword())
    return kMark;
  if (entry_type == MeasureKeyword())
    return kMeasure;
  if (entry_type == RenderKeyword())
    return kRender;
  if (entry_type == ResourceKeyword())
    return kResource;
  if (entry_type == NavigationKeyword())
    return kNavigation;
  if (entry_type == TaskattributionKeyword())
    return kTaskAttribution;
  if (entry_type == PaintKeyword())
    return kPaint;
  if (entry_type == EventKeyword())
    return kEvent;
  if (entry_type == FirstInputKeyword())
    return kFirstInput;
  return kInvalid;
}

ScriptValue PerformanceEntry::toJSONForBinding(
    ScriptState* script_state) const {
  V8ObjectBuilder result(script_state);
  BuildJSONValue(result);
  return result.GetScriptValue();
}

void PerformanceEntry::BuildJSONValue(V8ObjectBuilder& builder) const {
  builder.AddString("name", name());
  builder.AddString("entryType", entryType());
  builder.AddNumber("startTime", startTime());
  builder.AddNumber("duration", duration());
}

}  // namespace blink
