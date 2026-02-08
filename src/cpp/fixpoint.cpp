// Fixpoint: Powerful C++ Static Analysis, Simplified.
// Copyright (C) 2026 IAS (ias@iasoft.dev)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fixpoint/fixpoint.hpp>

namespace ia::fixpoint
{
  Mut<Tool::SyntaxErrorHandlerT> Tool::s_syntax_error_handler = [](Ref<Diagnostic> diagnostics,
                                                                   DiagnosticPrinter *printer) -> i32 {
    llvm::errs() << "\033[1;31mNon-compliant C++ source detected:\033[0m\n";
    printer->HandleDiagnostic(clang::DiagnosticsEngine::Level::Error, diagnostics);
    return -1;
  };

  class StrictDiagnosticConsumer : public clang::DiagnosticConsumer
  {
public:
    StrictDiagnosticConsumer(clang::DiagnosticsEngine *engine = nullptr)
    {
      AU_UNUSED(engine);

      m_printer = make_box<DiagnosticPrinter>(llvm::errs(), m_options);
    }

    void BeginSourceFile(const clang::LangOptions &lo, const clang::Preprocessor *pp) override
    {
      m_printer->BeginSourceFile(lo, pp);
    }

    void EndSourceFile() override
    {
      m_printer->EndSourceFile();
    }

    void HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic &info) override
    {
      if (level < clang::DiagnosticsEngine::Error)
        return;

      m_has_error = true;

      const auto exit_code = Tool::get_syntax_error_handler()(info, m_printer.get());
      if (exit_code)
        exit(exit_code);
    }

    [[nodiscard]] auto has_error() const -> bool
    {
      return m_has_error;
    }

private:
    clang::DiagnosticOptions m_options{};
    Box<DiagnosticPrinter> m_printer;
    bool m_has_error{false};
  };

  static auto get_clang_resource_dir() -> String &
  {
    static thread_local Mut<String> result;

    if (!result.empty())
      return result;

#if defined(_WIN32)
    FILE *const pipe = _popen("clang -print-resource-dir", "r");
#else
    FILE *const pipe = popen("clang -print-resource-dir", "r");
#endif
    if (!pipe)
      return result;

    Mut<char> buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
      result += buffer;

#if defined(_WIN32)
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
      result.pop_back();

    return result;
  }
} // namespace ia::fixpoint

namespace ia::fixpoint
{
  auto Tool::create(MutRef<Options> options, Ref<CompileDB> compile_db) -> Result<Box<Tool>>
  {
    return make_box_protected<Tool>(clang::tooling::ClangTool(compile_db, options.get_cop().getSourcePathList()));
  }

  Tool::Tool(ForwardRef<clang::tooling::ClangTool> ct) : m_clang_tool(std::move(ct))
  {
    const auto &resource_dir = get_clang_resource_dir();

    m_clang_tool.appendArgumentsAdjuster([&](const clang::tooling::CommandLineArguments &args, LLVM_StringRef) {
      clang::tooling::CommandLineArguments new_args;

      if (!args.empty())
        new_args.push_back(args[0]);

      if (!resource_dir.empty())
      {
        new_args.push_back("-resource-dir");
        new_args.push_back(resource_dir);
      }

      for (size_t i = 1; i < args.size(); ++i)
      {
        LLVM_StringRef arg = args[i];

        if (arg == "-Xclang" && i + 1 < args.size())
        {
          LLVM_StringRef next_arg = args[i + 1];
          bool should_strip_next = false;

          if (next_arg == "-include-pch" || next_arg == "-pch-is-pch")
          {
            should_strip_next = true;
          }
          else if (next_arg == "-include")
          {
            size_t file_idx = i + 2;
            if (file_idx < args.size() && args[file_idx] == "-Xclang")
              file_idx++;

            if (file_idx < args.size() && (args[file_idx].find("cmake_pch") != String::npos))
            {
              should_strip_next = true;
            }
          }

          if (should_strip_next)
          {
            continue;
          }
        }

        if (arg == "-include-pch" || arg == "-pch-is-pch")
        {
          if (arg == "-include-pch" && i + 1 < args.size())
            i++;
          continue;
        }

        if (arg == "-include")
        {
          size_t next_idx = i + 1;
          if (next_idx < args.size() && args[next_idx] == "-Xclang")
            next_idx++;

          if (next_idx < args.size() && (args[next_idx].find("cmake_pch") != String::npos))
          {
            i = next_idx;
            continue;
          }
        }

        if (arg.contains("cmake_pch") && (arg.ends_with(".hxx") || arg.ends_with(".pch")))
        {
          continue;
        }

        new_args.push_back(std::string(arg));
      }
      return new_args;
    });

    m_clang_tool.setDiagnosticConsumer(new StrictDiagnosticConsumer());
  }

  auto Tool::run(Ref<Workload> workload) -> Result<void>
  {
    for (auto &task : workload.get_tasks())
    {
      MatchFinder finder;
      finder.addMatcher(clang::ast_matchers::traverse(clang::TK_IgnoreUnlessSpelledInSource,
                                                      clang::ast_matchers::decl(task->get_matcher()).bind("decl")),
                        task.get());
      const auto result = m_clang_tool.run(clang::tooling::newFrontendActionFactory(&finder).get());
      if (result != 0)
        return fail("ClangTool run failed with error code: {}", result);
    }

    return {};
  }
} // namespace ia::fixpoint