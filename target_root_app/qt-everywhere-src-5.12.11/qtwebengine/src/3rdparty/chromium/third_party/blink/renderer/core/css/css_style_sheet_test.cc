// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/css/css_style_sheet.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/renderer/bindings/core/v8/media_list_or_string.h"
#include "third_party/blink/renderer/bindings/core/v8/script_function.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise.h"
#include "third_party/blink/renderer/bindings/core/v8/script_value.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_binding_for_testing.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_css_style_sheet.h"
#include "third_party/blink/renderer/core/css/css_rule_list.h"
#include "third_party/blink/renderer/core/css/css_style_sheet_init.h"
#include "third_party/blink/renderer/core/css/media_list.h"
#include "third_party/blink/renderer/core/testing/page_test_base.h"

namespace blink {

class CSSStyleSheetTest : public PageTestBase {
 protected:
  void SetUp() override {
    PageTestBase::SetUp();
    RuntimeEnabledFeatures::SetConstructableStylesheetsEnabled(true);
  }

  class FunctionForTest : public ScriptFunction {
   public:
    static v8::Local<v8::Function> CreateFunction(ScriptState* script_state,
                                                  ScriptValue* output) {
      FunctionForTest* self = new FunctionForTest(script_state, output);
      return self->BindToV8Function();
    }

   private:
    FunctionForTest(ScriptState* script_state, ScriptValue* output)
        : ScriptFunction(script_state), output_(output) {}

    ScriptValue Call(ScriptValue value) override {
      DCHECK(!value.IsEmpty());
      *output_ = value;
      return value;
    }

    ScriptValue* output_;
  };
};

TEST_F(CSSStyleSheetTest, ConstructorWithoutRuntimeFlagThrowsException) {
  DummyExceptionStateForTesting exception_state;
  RuntimeEnabledFeatures::SetConstructableStylesheetsEnabled(false);
  EXPECT_EQ(CSSStyleSheet::Create(GetDocument(), exception_state), nullptr);
  ASSERT_TRUE(exception_state.HadException());
}

TEST_F(CSSStyleSheetTest, CSSStyleSheetConstructionWithEmptyCSSStyleSheetInit) {
  DummyExceptionStateForTesting exception_state;
  CSSStyleSheet* sheet = CSSStyleSheet::Create(GetDocument(), exception_state);
  ASSERT_FALSE(exception_state.HadException());
  EXPECT_TRUE(sheet->href().IsNull());
  EXPECT_EQ(sheet->parentStyleSheet(), nullptr);
  EXPECT_EQ(sheet->ownerNode(), nullptr);
  EXPECT_EQ(sheet->ownerRule(), nullptr);
  EXPECT_EQ(sheet->media()->length(), 0U);
  EXPECT_EQ(sheet->title(), StringImpl::empty_);
  EXPECT_FALSE(sheet->AlternateFromConstructor());
  EXPECT_FALSE(sheet->disabled());
  EXPECT_EQ(sheet->cssRules(exception_state)->length(), 0U);
  ASSERT_FALSE(exception_state.HadException());
}

TEST_F(CSSStyleSheetTest,
       CSSStyleSheetConstructionWithNonEmptyCSSStyleSheetInit) {
  DummyExceptionStateForTesting exception_state;
  CSSStyleSheetInit init;
  init.setMedia(MediaListOrString::FromString("screen, print"));
  init.setTitle("test");
  init.setAlternate(true);
  init.setDisabled(true);
  CSSStyleSheet* sheet =
      CSSStyleSheet::Create(GetDocument(), init, exception_state);
  ASSERT_FALSE(exception_state.HadException());
  EXPECT_TRUE(sheet->href().IsNull());
  EXPECT_EQ(sheet->parentStyleSheet(), nullptr);
  EXPECT_EQ(sheet->ownerNode(), nullptr);
  EXPECT_EQ(sheet->ownerRule(), nullptr);
  EXPECT_EQ(sheet->media()->length(), 2U);
  EXPECT_EQ(sheet->media()->mediaText(), init.media().GetAsString());
  EXPECT_EQ(sheet->title(), init.title());
  EXPECT_TRUE(sheet->AlternateFromConstructor());
  EXPECT_TRUE(sheet->disabled());
  EXPECT_EQ(sheet->cssRules(exception_state)->length(), 0U);
  ASSERT_FALSE(exception_state.HadException());
}

TEST_F(CSSStyleSheetTest,
       CreateCSSStyleSheetWithEmptyCSSStyleSheetInitAndText) {
  V8TestingScope scope;
  DummyExceptionStateForTesting exception_state;
  ScriptPromise promise = GetDocument().createCSSStyleSheet(
      scope.GetScriptState(), "", CSSStyleSheetInit(), exception_state);
  EXPECT_FALSE(promise.IsEmpty());
  ASSERT_FALSE(exception_state.HadException());
  ScriptValue on_fulfilled, on_rejected;
  promise.Then(
      FunctionForTest::CreateFunction(scope.GetScriptState(), &on_fulfilled),
      FunctionForTest::CreateFunction(scope.GetScriptState(), &on_rejected));

  v8::MicrotasksScope::PerformCheckpoint(scope.GetIsolate());
  CSSStyleSheet* sheet = V8CSSStyleSheet::ToImplWithTypeCheck(
      scope.GetIsolate(), on_fulfilled.V8Value());
  EXPECT_TRUE(sheet->href().IsNull());
  EXPECT_EQ(sheet->parentStyleSheet(), nullptr);
  EXPECT_EQ(sheet->ownerNode(), nullptr);
  EXPECT_EQ(sheet->ownerRule(), nullptr);
  EXPECT_EQ(sheet->media()->length(), 0U);
  EXPECT_EQ(sheet->title(), StringImpl::empty_);
  EXPECT_FALSE(sheet->AlternateFromConstructor());
  EXPECT_FALSE(sheet->disabled());
  EXPECT_EQ(sheet->cssRules(exception_state)->length(), 0U);
  ASSERT_FALSE(exception_state.HadException());
}

TEST_F(CSSStyleSheetTest,
       CreateCSSStyleSheetWithNonEmptyCSSStyleSheetInitAndText) {
  String styleText[2] = {".red { color: red; }",
                         ".red + span + span { color: red; }"};
  CSSStyleSheetInit init;
  init.setMedia(MediaListOrString::FromString("screen, print"));
  init.setTitle("test");
  init.setAlternate(true);
  init.setDisabled(true);
  V8TestingScope scope;
  DummyExceptionStateForTesting exception_state;
  ScriptPromise promise = GetDocument().createCSSStyleSheet(
      scope.GetScriptState(), styleText[0] + styleText[1], init,
      exception_state);
  EXPECT_FALSE(promise.IsEmpty());
  ASSERT_FALSE(exception_state.HadException());
  ScriptValue on_fulfilled, on_rejected;
  promise.Then(
      FunctionForTest::CreateFunction(scope.GetScriptState(), &on_fulfilled),
      FunctionForTest::CreateFunction(scope.GetScriptState(), &on_rejected));

  v8::MicrotasksScope::PerformCheckpoint(scope.GetIsolate());
  CSSStyleSheet* sheet = V8CSSStyleSheet::ToImplWithTypeCheck(
      scope.GetIsolate(), on_fulfilled.V8Value());

  EXPECT_TRUE(sheet->href().IsNull());
  EXPECT_EQ(sheet->parentStyleSheet(), nullptr);
  EXPECT_EQ(sheet->ownerNode(), nullptr);
  EXPECT_EQ(sheet->ownerRule(), nullptr);
  EXPECT_EQ(sheet->media()->length(), 2U);
  EXPECT_EQ(sheet->media()->mediaText(), init.media().GetAsString());
  EXPECT_EQ(sheet->title(), init.title());
  EXPECT_TRUE(sheet->AlternateFromConstructor());
  EXPECT_TRUE(sheet->disabled());
  EXPECT_EQ(sheet->cssRules(exception_state)->length(), 2U);
  EXPECT_EQ(sheet->cssRules(exception_state)->item(0)->cssText(), styleText[0]);
  EXPECT_EQ(sheet->cssRules(exception_state)->item(1)->cssText(), styleText[1]);
  ASSERT_FALSE(exception_state.HadException());
}

}  // namespace blink
