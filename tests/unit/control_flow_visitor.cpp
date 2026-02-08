// Fixpoint: Powerful static analysis, simplified.
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

#include "helpers.hpp"

using namespace ia;

namespace
{
  struct CFGStats
  {
    int blocks_entered = 0;
    int jumps = 0;
    int conditions = 0;
  };

  class TestCFGVisitor : public fixpoint::ControlFlowVisitor
  {
    Arc<CFGStats> m_stats;

public:
    TestCFGVisitor(Arc<CFGStats> s) : m_stats(s)
    {
    }

    [[nodiscard]] auto get_matcher() const -> fixpoint::DeclarationMatcher override
    {
      return fixpoint::ast::functionDecl(fixpoint::ast::isDefinition());
    }

    auto on_enter_block(const fixpoint::CFGBlock *) -> void override
    {
      m_stats->blocks_entered++;
    }

    auto on_exit_block(const fixpoint::CFGBlock *) -> void override
    {
    }

    auto on_conditional_branch(const fixpoint::Stmt *, const fixpoint::CFGBlock *, const fixpoint::CFGBlock *)
        -> void override
    {
      m_stats->conditions++;
    }

    auto on_loop_decision(const fixpoint::Stmt *, const fixpoint::CFGBlock *, const fixpoint::CFGBlock *)
        -> void override
    {
    }

    auto on_switch_branch(const fixpoint::SwitchStmt *, const fixpoint::CFGBlock::succ_const_range &) -> void override
    {
    }

    auto on_simple_jump(const fixpoint::Stmt *, const fixpoint::CFGBlock *) -> void override
    {
      m_stats->jumps++;
    }
  };
} // namespace

IAT_BEGIN_BLOCK(Core, ControlFlowVisitor)

auto test_if_branch() -> bool
{
  const std::string code = R"(
        void func() {
            if (true) { int x = 1; } else { int x = 2; }
        }
    )";
  auto stats = std::make_shared<CFGStats>();
  IAT_CHECK(run_test_on_code(code, TestCFGVisitor(stats)));

  IAT_CHECK(stats->blocks_entered >= 3);
  IAT_CHECK(stats->conditions >= 1);
  return true;
}

IAT_BEGIN_TEST_LIST()
IAT_ADD_TEST(test_if_branch);
IAT_END_TEST_LIST()

IAT_END_BLOCK()

IAT_REGISTER_ENTRY(Core, ControlFlowVisitor)