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

#include <fixpoint/pch.hpp>

namespace ia::fixpoint
{
  class ControlFlowVisitor : public IWorkloadTask
  {
public:
    virtual auto on_enter_block(const CFGBlock *block) -> void = 0;
    virtual auto on_exit_block(const CFGBlock *block) -> void = 0;

    virtual auto on_conditional_branch(const Stmt *terminator, const CFGBlock *true_path, const CFGBlock *false_path)
        -> void = 0;

    virtual auto on_loop_decision(const Stmt *terminator, const CFGBlock *loop_body, const CFGBlock *loop_exit)
        -> void = 0;

    virtual auto on_switch_branch(const SwitchStmt *terminator, const CFGBlock::succ_const_range &successors)
        -> void = 0;

    virtual auto on_simple_jump(const Stmt *terminator, const CFGBlock *target) -> void = 0;

    virtual auto on_initializer(const CXXCtorInitializer *init) -> void
    {
      AU_UNUSED(init);
    }

    virtual auto on_implicit_dtor(const CFGImplicitDtor *dtor) -> void
    {
      AU_UNUSED(dtor);
    }

    virtual auto on_statement(const Stmt *stmt) -> void
    {
      AU_UNUSED(stmt);
    }

public:
    auto run(Ref<MatchResult> result) -> void override;

protected:
    auto get_match_result() -> const MatchResult *
    {
      return m_last_match_result;
    }

private:
    const MatchResult *m_last_match_result{};
  };
} // namespace ia::fixpoint