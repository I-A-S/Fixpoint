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

#pragma once

#include <fixpoint/utils.hpp>
#include <fixpoint/compile_db.hpp>

#include <fixpoint/decl_police.hpp>
#include <fixpoint/data_flow_solver.hpp>
#include <fixpoint/control_flow_visitor.hpp>

namespace ia::fixpoint
{
  class Workload
  {
public:
    template<typename Task> auto add_task() -> void;

public:
    auto add_task(ForwardRef<Box<IWorkloadTask>> task) -> void
    {
      m_tasks.emplace_back(std::move(task));
    }

    [[nodiscard]] auto get_tasks() const -> Ref<Vec<Box<IWorkloadTask>>>
    {
      return m_tasks;
    }

private:
    Vec<Box<IWorkloadTask>> m_tasks;
  };

  class Tool
  {
public:
    using SyntaxErrorHandlerT = std::function<i32(Ref<Diagnostic> diagnostics, DiagnosticPrinter *printer)>;

public:
    static auto create(MutRef<Options> options, Ref<CompileDB> compile_db) -> Result<Box<Tool>>;

    ~Tool() = default;

public:
    auto run(Ref<Workload> workload) -> Result<void>;

    static auto set_syntax_error_handler(SyntaxErrorHandlerT handler) -> void
    {
      s_syntax_error_handler = handler;
    }

    static auto get_syntax_error_handler() -> SyntaxErrorHandlerT
    {
      return s_syntax_error_handler;
    }

private:
    clang::tooling::ClangTool m_clang_tool;
    static SyntaxErrorHandlerT s_syntax_error_handler;

protected:
    Tool(ForwardRef<clang::tooling::ClangTool> ct);
  };

  template<typename Task> auto Workload::add_task() -> void
  {
    add_task(make_box<Task>());
  }
} // namespace ia::fixpoint