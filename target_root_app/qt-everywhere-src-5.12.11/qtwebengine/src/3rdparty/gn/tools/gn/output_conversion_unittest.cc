// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/output_conversion.h"

#include <sstream>

#include "tools/gn/err.h"
#include "tools/gn/input_conversion.h"
#include "tools/gn/scope.h"
#include "tools/gn/template.h"
#include "tools/gn/test_with_scheduler.h"
#include "tools/gn/test_with_scope.h"
#include "tools/gn/value.h"
#include "util/test/test.h"

namespace {

// InputConversion needs a global scheduler object.
class OutputConversionTest : public TestWithScheduler {
 public:
  OutputConversionTest() = default;

  const Settings* settings() { return setup_.settings(); }
  Scope* scope() { return setup_.scope(); }

 private:
  TestWithScope setup_;
};

}  // namespace

TEST_F(OutputConversionTest, ListLines) {
  Err err;
  Value output(nullptr, Value::LIST);
  output.list_value().push_back(Value(nullptr, ""));
  output.list_value().push_back(Value(nullptr, "foo"));
  output.list_value().push_back(Value(nullptr, ""));
  output.list_value().push_back(Value(nullptr, "bar"));
  std::ostringstream result;
  ConvertValueToOutput(settings(), output, Value(nullptr, "list lines"), result,
                       &err);

  EXPECT_FALSE(err.has_error());
  EXPECT_EQ("\nfoo\n\nbar\n", result.str());
}

TEST_F(OutputConversionTest, String) {
  Err err;
  Value output(nullptr, "foo bar");
  std::ostringstream result;
  ConvertValueToOutput(settings(), output, Value(nullptr, "string"), result,
                       &err);

  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "foo bar");
}

TEST_F(OutputConversionTest, StringInt) {
  Err err;
  Value output(nullptr, int64_t(6));
  std::ostringstream result;
  ConvertValueToOutput(settings(), output, Value(nullptr, "string"), result,
                       &err);

  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "\"6\"");
}

TEST_F(OutputConversionTest, StringBool) {
  Err err;
  Value output(nullptr, true);
  std::ostringstream result;
  ConvertValueToOutput(settings(), output, Value(nullptr, "string"), result,
                       &err);

  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "\"true\"");
}

TEST_F(OutputConversionTest, StringList) {
  Err err;
  Value output(nullptr, Value::LIST);
  output.list_value().push_back(Value(nullptr, "foo"));
  output.list_value().push_back(Value(nullptr, "bar"));
  output.list_value().push_back(Value(nullptr, int64_t(6)));
  std::ostringstream result;
  ConvertValueToOutput(settings(), output, Value(nullptr, "string"), result,
                       &err);

  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "\"[\"foo\", \"bar\", 6]\"");
}

TEST_F(OutputConversionTest, StringScope) {
  Err err;

  auto new_scope = std::make_unique<Scope>(settings());
  // Add some values to the scope.
  Value value(nullptr, "hello");
  new_scope->SetValue("v", value, nullptr);
  base::StringPiece private_var_name("_private");
  new_scope->SetValue(private_var_name, value, nullptr);

  std::ostringstream result;
  ConvertValueToOutput(settings(), Value(nullptr, std::move(new_scope)),
                       Value(nullptr, "string"), result, &err);
  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "\"{\n  _private = \"hello\"\n  v = \"hello\"\n}\"");
}

TEST_F(OutputConversionTest, ValueString) {
  Err err;
  Value output(nullptr, "foo bar");
  std::ostringstream result;
  ConvertValueToOutput(settings(), output, Value(nullptr, "value"), result,
                       &err);

  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "\"foo bar\"");
}

TEST_F(OutputConversionTest, ValueInt) {
  Err err;
  Value output(nullptr, int64_t(6));
  std::ostringstream result;
  ConvertValueToOutput(settings(), output, Value(nullptr, "value"), result,
                       &err);

  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "6");
}

TEST_F(OutputConversionTest, ValueBool) {
  Err err;
  Value output(nullptr, true);
  std::ostringstream result;
  ConvertValueToOutput(settings(), output, Value(nullptr, "value"), result,
                       &err);

  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "true");
}

TEST_F(OutputConversionTest, ValueList) {
  Err err;
  Value output(nullptr, Value::LIST);
  output.list_value().push_back(Value(nullptr, "foo"));
  output.list_value().push_back(Value(nullptr, "bar"));
  output.list_value().push_back(Value(nullptr, int64_t(6)));
  std::ostringstream result;
  ConvertValueToOutput(settings(), output, Value(nullptr, "value"), result,
                       &err);

  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "[\"foo\", \"bar\", 6]");
}

TEST_F(OutputConversionTest, ValueScope) {
  Err err;

  auto new_scope = std::make_unique<Scope>(settings());
  // Add some values to the scope.
  Value value(nullptr, "hello");
  new_scope->SetValue("v", value, nullptr);
  base::StringPiece private_var_name("_private");
  new_scope->SetValue(private_var_name, value, nullptr);

  std::ostringstream result;
  ConvertValueToOutput(settings(), Value(nullptr, std::move(new_scope)),
                       Value(nullptr, "value"), result, &err);
  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "{\n  _private = \"hello\"\n  v = \"hello\"\n}");
}

TEST_F(OutputConversionTest, JSON) {
  Err err;
  auto new_scope = std::make_unique<Scope>(settings());
  // Add some values to the scope.
  Value a_value(nullptr, "foo");
  new_scope->SetValue("a", a_value, nullptr);
  Value b_value(nullptr, int64_t(6));
  new_scope->SetValue("b", b_value, nullptr);

  auto c_scope = std::make_unique<Scope>(settings());
  Value e_value(nullptr, Value::LIST);
  e_value.list_value().push_back(Value(nullptr, "bar"));

  auto e_value_scope = std::make_unique<Scope>(settings());
  Value f_value(nullptr, "baz");
  e_value_scope->SetValue("f", f_value, nullptr);
  e_value.list_value().push_back(Value(nullptr, std::move(e_value_scope)));

  c_scope->SetValue("e", e_value, nullptr);

  new_scope->SetValue("c", Value(nullptr, std::move(c_scope)), nullptr);

  std::string expected(R"*({
  "a": "foo",
  "b": 6,
  "c": {
    "e": [
      "bar",
      {
        "f": "baz"
      }
    ]
  }
})*");
  std::ostringstream result;
  ConvertValueToOutput(settings(), Value(nullptr, std::move(new_scope)),
                       Value(nullptr, "json"), result, &err);
  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), expected);
}

TEST_F(OutputConversionTest, ValueEmpty) {
  Err err;
  std::ostringstream result;
  ConvertValueToOutput(settings(), Value(), Value(nullptr, ""), result, &err);
  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "<void>");
}

TEST_F(OutputConversionTest, DefaultValue) {
  Err err;
  Value output(nullptr, "foo bar");
  std::ostringstream result;
  ConvertValueToOutput(settings(), output, Value(nullptr, ""), result, &err);

  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(result.str(), "foo bar");
}

TEST_F(OutputConversionTest, DefaultListLines) {
  Err err;
  Value output(nullptr, Value::LIST);
  output.list_value().push_back(Value(nullptr, ""));
  output.list_value().push_back(Value(nullptr, "foo"));
  output.list_value().push_back(Value(nullptr, ""));
  output.list_value().push_back(Value(nullptr, "bar"));
  std::ostringstream result;
  ConvertValueToOutput(settings(), output, Value(nullptr, ""), result, &err);

  EXPECT_FALSE(err.has_error());
  EXPECT_EQ("\nfoo\n\nbar\n", result.str());
}

TEST_F(OutputConversionTest, ReverseString) {
  Err err;
  std::string input("foo bar");
  Value result = ConvertInputToValue(settings(), input, nullptr,
                                     Value(nullptr, "string"), &err);
  EXPECT_FALSE(err.has_error());

  std::ostringstream reverse;
  ConvertValueToOutput(settings(), result, Value(nullptr, "string"), reverse,
                       &err);
  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(reverse.str(), input)
      << "actual: " << reverse.str() << "expected:" << input;
}

TEST_F(OutputConversionTest, ReverseListLines) {
  Err err;
  std::string input("\nfoo\nbar\n\n");
  Value result = ConvertInputToValue(settings(), input, nullptr,
                                     Value(nullptr, "list lines"), &err);
  EXPECT_FALSE(err.has_error());

  std::ostringstream reverse;
  ConvertValueToOutput(settings(), result, Value(nullptr, "list lines"),
                       reverse, &err);
  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(reverse.str(), input)
      << "actual: " << reverse.str() << "expected:" << input;
}

TEST_F(OutputConversionTest, ReverseValueString) {
  Err err;
  std::string input("\"str\"");
  Value result = ConvertInputToValue(settings(), input, nullptr,
                                     Value(nullptr, "value"), &err);
  EXPECT_FALSE(err.has_error());

  std::ostringstream reverse;
  ConvertValueToOutput(settings(), result, Value(nullptr, "value"), reverse,
                       &err);
  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(reverse.str(), input)
      << "actual: " << reverse.str() << "expected:" << input;
}

TEST_F(OutputConversionTest, ReverseValueInt) {
  Err err;
  std::string input("6");
  Value result = ConvertInputToValue(settings(), input, nullptr,
                                     Value(nullptr, "value"), &err);
  EXPECT_FALSE(err.has_error());

  std::ostringstream reverse;
  ConvertValueToOutput(settings(), result, Value(nullptr, "value"), reverse,
                       &err);
  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(reverse.str(), input)
      << "actual: " << reverse.str() << "expected:" << input;
}

TEST_F(OutputConversionTest, ReverseValueList) {
  Err err;
  std::string input("[\"a\", 5]");
  Value result = ConvertInputToValue(settings(), input, nullptr,
                                     Value(nullptr, "value"), &err);
  EXPECT_FALSE(err.has_error());

  std::ostringstream reverse;
  ConvertValueToOutput(settings(), result, Value(nullptr, "value"), reverse,
                       &err);
  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(reverse.str(), input)
      << "actual: " << reverse.str() << "expected:" << input;
}

TEST_F(OutputConversionTest, ReverseValueDict) {
  Err err;
  std::string input("  a = 5\n  b = \"foo\"\n  c = 7\n");
  Value result = ConvertInputToValue(settings(), input, nullptr,
                                     Value(nullptr, "scope"), &err);
  EXPECT_FALSE(err.has_error());

  std::ostringstream reverse;
  ConvertValueToOutput(settings(), result, Value(nullptr, "scope"), reverse,
                       &err);
  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(reverse.str(), input)
      << "actual: " << reverse.str() << "expected:" << input;
}

TEST_F(OutputConversionTest, ReverseValueEmpty) {
  Err err;
  Value result = ConvertInputToValue(settings(), "", nullptr,
                                     Value(nullptr, "value"), &err);
  EXPECT_FALSE(err.has_error());

  std::ostringstream reverse;
  ConvertValueToOutput(settings(), result, Value(nullptr, "value"), reverse,
                       &err);
  EXPECT_FALSE(err.has_error());
  EXPECT_EQ(reverse.str(), "");
}
