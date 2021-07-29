// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/compile_commands_writer.h"

#include <memory>
#include <sstream>
#include <utility>

#include "tools/gn/config.h"
#include "tools/gn/ninja_target_command_util.h"
#include "tools/gn/scheduler.h"
#include "tools/gn/target.h"
#include "tools/gn/test_with_scheduler.h"
#include "tools/gn/test_with_scope.h"
#include "util/build_config.h"
#include "util/test/test.h"

namespace {

// InputConversion needs a global scheduler object.
class CompileCommandsTest : public TestWithScheduler {
 public:
  CompileCommandsTest() = default;

  const BuildSettings* build_settings() { return setup_.build_settings(); }
  const Settings* settings() { return setup_.settings(); }
  const TestWithScope& setup() { return setup_; }
  const Toolchain* toolchain() { return setup_.toolchain(); }

 private:
  TestWithScope setup_;
};

}  // namespace

TEST_F(CompileCommandsTest, SourceSet) {
  Err err;

  std::vector<const Target*> targets;
  Target target(settings(), Label(SourceDir("//foo/"), "bar"));
  target.set_output_type(Target::SOURCE_SET);
  target.visibility().SetPublic();
  target.sources().push_back(SourceFile("//foo/input1.cc"));
  target.sources().push_back(SourceFile("//foo/input2.cc"));
  // Also test object files, which should be just passed through to the
  // dependents to link.
  target.sources().push_back(SourceFile("//foo/input3.o"));
  target.sources().push_back(SourceFile("//foo/input4.obj"));
  target.SetToolchain(toolchain());
  ASSERT_TRUE(target.OnResolved(&err));
  targets.push_back(&target);

  // Source set itself.
  {
    std::string out;
    CompileCommandsWriter writer;
    writer.RenderJSON(build_settings(), targets, &out);

#if defined(OS_WIN)
    const char expected[] =
        "[\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input1.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input1.cc     -o  "
        "obj/foo/bar.input1.o\"\r\n"
        "  },\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input2.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input2.cc     -o  "
        "obj/foo/bar.input2.o\"\r\n"
        "  }\r\n"
        "]\r\n";
#else
    const char expected[] =
        "[\n"
        "  {\n"
        "    \"file\": \"../../foo/input1.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input1.cc     -o  "
        "obj/foo/bar.input1.o\"\n"
        "  },\n"
        "  {\n"
        "    \"file\": \"../../foo/input2.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input2.cc     -o  "
        "obj/foo/bar.input2.o\"\n"
        "  }\n"
        "]\n";
#endif
    EXPECT_EQ(expected, out);
  }

  // A shared library that depends on the source set.
  Target shlib_target(settings(), Label(SourceDir("//foo/"), "shlib"));
  shlib_target.sources().push_back(SourceFile("//foo/input3.cc"));
  shlib_target.set_output_type(Target::SHARED_LIBRARY);
  shlib_target.public_deps().push_back(LabelTargetPair(&target));
  shlib_target.SetToolchain(toolchain());
  ASSERT_TRUE(shlib_target.OnResolved(&err));
  targets.push_back(&shlib_target);

  {
    std::string out;
    CompileCommandsWriter writer;
    writer.RenderJSON(build_settings(), targets, &out);

#if defined(OS_WIN)
    const char expected[] =
        "[\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input1.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input1.cc     -o  "
        "obj/foo/bar.input1.o\"\r\n"
        "  },\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input2.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input2.cc     -o  "
        "obj/foo/bar.input2.o\"\r\n"
        "  },\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input3.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input3.cc     -o  "
        "obj/foo/libshlib.input3.o\"\r\n"
        "  }\r\n"
        "]\r\n";
#else
    const char expected[] =
        "[\n"
        "  {\n"
        "    \"file\": \"../../foo/input1.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input1.cc     -o  "
        "obj/foo/bar.input1.o\"\n"
        "  },\n"
        "  {\n"
        "    \"file\": \"../../foo/input2.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input2.cc     -o  "
        "obj/foo/bar.input2.o\"\n"
        "  },\n"
        "  {\n"
        "    \"file\": \"../../foo/input3.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input3.cc     -o  "
        "obj/foo/libshlib.input3.o\"\n"
        "  }\n"
        "]\n";
#endif
    EXPECT_EQ(expected, out);
  }

  // A static library that depends on the source set (should not link it).
  Target stlib_target(settings(), Label(SourceDir("//foo/"), "stlib"));
  stlib_target.sources().push_back(SourceFile("//foo/input4.cc"));
  stlib_target.set_output_type(Target::STATIC_LIBRARY);
  stlib_target.public_deps().push_back(LabelTargetPair(&target));
  stlib_target.SetToolchain(toolchain());
  ASSERT_TRUE(stlib_target.OnResolved(&err));
  targets.push_back(&stlib_target);

  {
    std::string out;
    CompileCommandsWriter writer;
    writer.RenderJSON(build_settings(), targets, &out);

#if defined(OS_WIN)
    const char expected[] =
        "[\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input1.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input1.cc     -o  "
        "obj/foo/bar.input1.o\"\r\n"
        "  },\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input2.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input2.cc     -o  "
        "obj/foo/bar.input2.o\"\r\n"
        "  },\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input3.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input3.cc     -o  "
        "obj/foo/libshlib.input3.o\"\r\n"
        "  },\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input4.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input4.cc     -o  "
        "obj/foo/libstlib.input4.o\"\r\n"
        "  }\r\n"
        "]\r\n";
#else
    const char expected[] =
        "[\n"
        "  {\n"
        "    \"file\": \"../../foo/input1.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input1.cc     -o  "
        "obj/foo/bar.input1.o\"\n"
        "  },\n"
        "  {\n"
        "    \"file\": \"../../foo/input2.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input2.cc     -o  "
        "obj/foo/bar.input2.o\"\n"
        "  },\n"
        "  {\n"
        "    \"file\": \"../../foo/input3.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input3.cc     -o  "
        "obj/foo/libshlib.input3.o\"\n"
        "  },\n"
        "  {\n"
        "    \"file\": \"../../foo/input4.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input4.cc     -o  "
        "obj/foo/libstlib.input4.o\"\n"
        "  }\n"
        "]\n";
#endif
    EXPECT_EQ(expected, out);
  }
}

TEST_F(CompileCommandsTest, EscapeDefines) {
  Err err;

  std::vector<const Target*> targets;
  TestTarget target(setup(), "//foo:bar", Target::STATIC_LIBRARY);
  target.sources().push_back(SourceFile("//foo/input.cc"));
  target.config_values().defines().push_back("BOOL_DEF");
  target.config_values().defines().push_back("INT_DEF=123");
  target.config_values().defines().push_back("STR_DEF=\"ABCD-1\"");
  ASSERT_TRUE(target.OnResolved(&err));
  targets.push_back(&target);

  std::string out;
  CompileCommandsWriter writer;
  writer.RenderJSON(build_settings(), targets, &out);

  const char expected[] =
      "-DBOOL_DEF -DINT_DEF=123 -DSTR_DEF=\\\\\\\"ABCD-1\\\\\\\"";
  EXPECT_TRUE(out.find(expected) != std::string::npos);
}

TEST_F(CompileCommandsTest, WinPrecompiledHeaders) {
  Err err;

  // This setup's toolchain does not have precompiled headers defined.
  // A precompiled header toolchain.
  Settings pch_settings(build_settings(), "withpch/");
  Toolchain pch_toolchain(&pch_settings,
                          Label(SourceDir("//toolchain/"), "withpch"));
  pch_settings.set_toolchain_label(pch_toolchain.label());
  pch_settings.set_default_toolchain_label(toolchain()->label());

  // Declare a C++ compiler that supports PCH.
  std::unique_ptr<Tool> cxx_tool = std::make_unique<Tool>();
  TestWithScope::SetCommandForTool(
      "c++ {{source}} {{cflags}} {{cflags_cc}} {{defines}} {{include_dirs}} "
      "-o {{output}}",
      cxx_tool.get());
  cxx_tool->set_outputs(SubstitutionList::MakeForTest(
      "{{source_out_dir}}/{{target_output_name}}.{{source_name_part}}.o"));
  cxx_tool->set_precompiled_header_type(Tool::PCH_MSVC);
  pch_toolchain.SetTool(Toolchain::TYPE_CXX, std::move(cxx_tool));

  // Add a C compiler as well.
  std::unique_ptr<Tool> cc_tool = std::make_unique<Tool>();
  TestWithScope::SetCommandForTool(
      "cc {{source}} {{cflags}} {{cflags_c}} {{defines}} {{include_dirs}} "
      "-o {{output}}",
      cc_tool.get());
  cc_tool->set_outputs(SubstitutionList::MakeForTest(
      "{{source_out_dir}}/{{target_output_name}}.{{source_name_part}}.o"));
  cc_tool->set_precompiled_header_type(Tool::PCH_MSVC);
  pch_toolchain.SetTool(Toolchain::TYPE_CC, std::move(cc_tool));
  pch_toolchain.ToolchainSetupComplete();

  // This target doesn't specify precompiled headers.
  {
    std::vector<const Target*> targets;
    Target no_pch_target(&pch_settings,
                         Label(SourceDir("//foo/"), "no_pch_target"));
    no_pch_target.set_output_type(Target::SOURCE_SET);
    no_pch_target.visibility().SetPublic();
    no_pch_target.sources().push_back(SourceFile("//foo/input1.cc"));
    no_pch_target.sources().push_back(SourceFile("//foo/input2.c"));
    no_pch_target.config_values().cflags_c().push_back("-std=c99");
    no_pch_target.SetToolchain(&pch_toolchain);
    ASSERT_TRUE(no_pch_target.OnResolved(&err));
    targets.push_back(&no_pch_target);

    std::string out;
    CompileCommandsWriter writer;
    writer.RenderJSON(build_settings(), targets, &out);

#if defined(OS_WIN)
    const char no_pch_expected[] =
        "[\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input1.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input1.cc     -o  "
        "withpch/obj/foo/no_pch_target.input1.o\"\r\n"
        "  },\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input2.c\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"cc ../../foo/input2.c   -std=c99   -o  "
        "withpch/obj/foo/no_pch_target.input2.o\"\r\n"
        "  }\r\n"
        "]\r\n";
#else
    const char no_pch_expected[] =
        "[\n"
        "  {\n"
        "    \"file\": \"../../foo/input1.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input1.cc     -o  "
        "withpch/obj/foo/no_pch_target.input1.o\"\n"
        "  },\n"
        "  {\n"
        "    \"file\": \"../../foo/input2.c\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"cc ../../foo/input2.c   -std=c99   -o  "
        "withpch/obj/foo/no_pch_target.input2.o\"\n"
        "  }\n"
        "]\n";
#endif
    EXPECT_EQ(no_pch_expected, out);
  }

  // This target specifies PCH.
  {
    std::vector<const Target*> targets;
    Target pch_target(&pch_settings, Label(SourceDir("//foo/"), "pch_target"));
    pch_target.config_values().set_precompiled_header("build/precompile.h");
    pch_target.config_values().set_precompiled_source(
        SourceFile("//build/precompile.cc"));
    pch_target.set_output_type(Target::SOURCE_SET);
    pch_target.visibility().SetPublic();
    pch_target.sources().push_back(SourceFile("//foo/input1.cc"));
    pch_target.sources().push_back(SourceFile("//foo/input2.c"));
    pch_target.SetToolchain(&pch_toolchain);
    ASSERT_TRUE(pch_target.OnResolved(&err));
    targets.push_back(&pch_target);

    std::string out;
    CompileCommandsWriter writer;
    writer.RenderJSON(build_settings(), targets, &out);

#if defined(OS_WIN)
    const char pch_win_expected[] =
        "[\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input1.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input1.cc   "
        "/Fpwithpch/obj/foo/pch_target_cc.pch /Yubuild/precompile.h   -o  "
        "withpch/obj/foo/pch_target.input1.o\"\r\n"
        "  },\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input2.c\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"cc ../../foo/input2.c   "
        "/Fpwithpch/obj/foo/pch_target_c.pch /Yubuild/precompile.h   -o  "
        "withpch/obj/foo/pch_target.input2.o\"\r\n"
        "  }\r\n"
        "]\r\n";
#else
    const char pch_win_expected[] =
        "[\n"
        "  {\n"
        "    \"file\": \"../../foo/input1.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input1.cc   "
        "/Fpwithpch/obj/foo/pch_target_cc.pch /Yubuild/precompile.h   -o  "
        "withpch/obj/foo/pch_target.input1.o\"\n"
        "  },\n"
        "  {\n"
        "    \"file\": \"../../foo/input2.c\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"cc ../../foo/input2.c   "
        "/Fpwithpch/obj/foo/pch_target_c.pch /Yubuild/precompile.h   -o  "
        "withpch/obj/foo/pch_target.input2.o\"\n"
        "  }\n"
        "]\n";
#endif
    EXPECT_EQ(pch_win_expected, out);
  }
}

TEST_F(CompileCommandsTest, GCCPrecompiledHeaders) {
  Err err;

  // This setup's toolchain does not have precompiled headers defined.
  // A precompiled header toolchain.
  Settings pch_settings(build_settings(), "withpch/");
  Toolchain pch_toolchain(&pch_settings,
                          Label(SourceDir("//toolchain/"), "withpch"));
  pch_settings.set_toolchain_label(pch_toolchain.label());
  pch_settings.set_default_toolchain_label(toolchain()->label());

  // Declare a C++ compiler that supports PCH.
  std::unique_ptr<Tool> cxx_tool = std::make_unique<Tool>();
  TestWithScope::SetCommandForTool(
      "c++ {{source}} {{cflags}} {{cflags_cc}} {{defines}} {{include_dirs}} "
      "-o {{output}}",
      cxx_tool.get());
  cxx_tool->set_outputs(SubstitutionList::MakeForTest(
      "{{source_out_dir}}/{{target_output_name}}.{{source_name_part}}.o"));
  cxx_tool->set_precompiled_header_type(Tool::PCH_GCC);
  pch_toolchain.SetTool(Toolchain::TYPE_CXX, std::move(cxx_tool));
  pch_toolchain.ToolchainSetupComplete();

  // Add a C compiler as well.
  std::unique_ptr<Tool> cc_tool = std::make_unique<Tool>();
  TestWithScope::SetCommandForTool(
      "cc {{source}} {{cflags}} {{cflags_c}} {{defines}} {{include_dirs}} "
      "-o {{output}}",
      cc_tool.get());
  cc_tool->set_outputs(SubstitutionList::MakeForTest(
      "{{source_out_dir}}/{{target_output_name}}.{{source_name_part}}.o"));
  cc_tool->set_precompiled_header_type(Tool::PCH_GCC);
  pch_toolchain.SetTool(Toolchain::TYPE_CC, std::move(cc_tool));
  pch_toolchain.ToolchainSetupComplete();

  // This target doesn't specify precompiled headers.
  {
    std::vector<const Target*> targets;
    Target no_pch_target(&pch_settings,
                         Label(SourceDir("//foo/"), "no_pch_target"));
    no_pch_target.set_output_type(Target::SOURCE_SET);
    no_pch_target.visibility().SetPublic();
    no_pch_target.sources().push_back(SourceFile("//foo/input1.cc"));
    no_pch_target.sources().push_back(SourceFile("//foo/input2.c"));
    no_pch_target.config_values().cflags_c().push_back("-std=c99");
    no_pch_target.SetToolchain(&pch_toolchain);
    ASSERT_TRUE(no_pch_target.OnResolved(&err));
    targets.push_back(&no_pch_target);

    std::string out;
    CompileCommandsWriter writer;
    writer.RenderJSON(build_settings(), targets, &out);

#if defined(OS_WIN)
    const char no_pch_expected[] =
        "[\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input1.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input1.cc     -o  "
        "withpch/obj/foo/no_pch_target.input1.o\"\r\n"
        "  },\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input2.c\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"cc ../../foo/input2.c   -std=c99   -o  "
        "withpch/obj/foo/no_pch_target.input2.o\"\r\n"
        "  }\r\n"
        "]\r\n";
#else
    const char no_pch_expected[] =
        "[\n"
        "  {\n"
        "    \"file\": \"../../foo/input1.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input1.cc     -o  "
        "withpch/obj/foo/no_pch_target.input1.o\"\n"
        "  },\n"
        "  {\n"
        "    \"file\": \"../../foo/input2.c\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"cc ../../foo/input2.c   -std=c99   -o  "
        "withpch/obj/foo/no_pch_target.input2.o\"\n"
        "  }\n"
        "]\n";
#endif
    EXPECT_EQ(no_pch_expected, out);
  }

  // This target specifies PCH.
  {
    std::vector<const Target*> targets;
    Target pch_target(&pch_settings, Label(SourceDir("//foo/"), "pch_target"));
    pch_target.config_values().set_precompiled_source(
        SourceFile("//build/precompile.h"));
    pch_target.config_values().cflags_c().push_back("-std=c99");
    pch_target.set_output_type(Target::SOURCE_SET);
    pch_target.visibility().SetPublic();
    pch_target.sources().push_back(SourceFile("//foo/input1.cc"));
    pch_target.sources().push_back(SourceFile("//foo/input2.c"));
    pch_target.SetToolchain(&pch_toolchain);
    ASSERT_TRUE(pch_target.OnResolved(&err));
    targets.push_back(&pch_target);

    std::string out;
    CompileCommandsWriter writer;
    writer.RenderJSON(build_settings(), targets, &out);

#if defined(OS_WIN)
    const char pch_gcc_expected[] =
        "[\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input1.cc\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"c++ ../../foo/input1.cc   -include "
        "withpch/obj/build/pch_target.precompile.h-cc   -o  "
        "withpch/obj/foo/pch_target.input1.o\"\r\n"
        "  },\r\n"
        "  {\r\n"
        "    \"file\": \"../../foo/input2.c\",\r\n"
        "    \"directory\": \"out/Debug\",\r\n"
        "    \"command\": \"cc ../../foo/input2.c   -std=c99 -include "
        "withpch/obj/build/pch_target.precompile.h-c   -o  "
        "withpch/obj/foo/pch_target.input2.o\"\r\n"
        "  }\r\n"
        "]\r\n";
#else
    const char pch_gcc_expected[] =
        "[\n"
        "  {\n"
        "    \"file\": \"../../foo/input1.cc\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"c++ ../../foo/input1.cc   -include "
        "withpch/obj/build/pch_target.precompile.h-cc   -o  "
        "withpch/obj/foo/pch_target.input1.o\"\n"
        "  },\n"
        "  {\n"
        "    \"file\": \"../../foo/input2.c\",\n"
        "    \"directory\": \"out/Debug\",\n"
        "    \"command\": \"cc ../../foo/input2.c   -std=c99 -include "
        "withpch/obj/build/pch_target.precompile.h-c   -o  "
        "withpch/obj/foo/pch_target.input2.o\"\n"
        "  }\n"
        "]\n";
#endif
    EXPECT_EQ(pch_gcc_expected, out);
  }
}

TEST_F(CompileCommandsTest, EscapedFlags) {
  Err err;

  std::vector<const Target*> targets;
  Target target(settings(), Label(SourceDir("//foo/"), "bar"));
  target.set_output_type(Target::SOURCE_SET);
  target.sources().push_back(SourceFile("//foo/input1.c"));
  target.config_values().cflags_c().push_back("-DCONFIG=\"/config\"");
  target.SetToolchain(toolchain());
  ASSERT_TRUE(target.OnResolved(&err));
  targets.push_back(&target);

  std::string out;
  CompileCommandsWriter writer;
  writer.RenderJSON(build_settings(), targets, &out);

#if defined(OS_WIN)
  const char expected[] =
      "[\r\n"
      "  {\r\n"
      "    \"file\": \"../../foo/input1.c\",\r\n"
      "    \"directory\": \"out/Debug\",\r\n"
      "    \"command\": \"cc ../../foo/input1.c   -DCONFIG=\\\"/config\\\"   "
      "-o  obj/foo/bar.input1.o\"\r\n"
      "  }\r\n"
      "]\r\n";
#else
  const char expected[] =
      "[\n"
      "  {\n"
      "    \"file\": \"../../foo/input1.c\",\n"
      "    \"directory\": \"out/Debug\",\n"
      "    \"command\": \"cc ../../foo/input1.c   -DCONFIG=\\\"/config\\\"   "
      "-o  obj/foo/bar.input1.o\"\n"
      "  }\n"
      "]\n";
#endif
  EXPECT_EQ(expected, out);
}
