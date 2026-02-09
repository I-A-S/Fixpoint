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

#include <fixpoint/control_flow_visitor.hpp>

namespace ia::fixpoint
{
  auto ControlFlowVisitor::run(Ref<MatchResult> result) -> void
  {
    const auto *const func = result.Nodes.getNodeAs<FunctionDecl>("decl");
    if (!func)
      return;

    const SourceLocation loc = func->getLocation();
    if (result.SourceManager->isInSystemHeader(loc) || !result.SourceManager->isInMainFile(loc) || !loc.isValid())
      return;

    auto *const ctx = result.Context;
    if (!func || !ctx || !func->hasBody())
      return;

    m_last_match_result = &result;

    Mut<clang::CFG::BuildOptions> cfg_opts;
    cfg_opts.PruneTriviallyFalseEdges = true;
    cfg_opts.AddImplicitDtors = true;
    cfg_opts.setAllAlwaysAdd();
    cfg_opts.AddInitializers = true;

    const auto cfg = clang::CFG::buildCFG(func, func->getBody(), ctx, cfg_opts);
    if (!cfg)
      return;

    for (const auto *const block : *cfg)
    {
      if (!block)
        continue;

      on_enter_block(block);

      for (const auto &element : *block)
      {
        if (const auto cfg_init = element.getAs<CFGInitializer>())
          on_initializer(cfg_init->getInitializer());
        else if (const auto cfg_dtor = element.getAs<CFGImplicitDtor>())
          on_implicit_dtor(&*cfg_dtor);
        else if (const auto cfg_stmt = element.getAs<clang::CFGStmt>())
          on_statement(cfg_stmt->getStmt());
      }

      const Stmt *const terminator = block->getTerminatorStmt();

      if (!terminator && block->succ_size() == 1)
      {
        if (*block->succ_begin())
          on_simple_jump(nullptr, *block->succ_begin());
      }
      else if (terminator)
      {
        switch (terminator->getStmtClass())
        {
        case Stmt::IfStmtClass:
        case Stmt::ConditionalOperatorClass:
        case Stmt::BinaryOperatorClass: {
          if (block->succ_size() == 2)
          {
            const CFGBlock *const false_blk = *(block->succ_begin());
            const CFGBlock *const true_blk = *(block->succ_begin() + 1);
            on_conditional_branch(terminator, true_blk, false_blk);
          }
          break;
        }

        case Stmt::WhileStmtClass:
        case Stmt::DoStmtClass:
        case Stmt::ForStmtClass:
        case Stmt::CXXForRangeStmtClass: {
          if (block->succ_size() == 2)
          {
            const CFGBlock *const exit_blk = *(block->succ_begin());
            const CFGBlock *const body_blk = *(block->succ_begin() + 1);
            on_loop_decision(terminator, body_blk, exit_blk);
          }
          break;
        }

        case Stmt::SwitchStmtClass: {
          if (const auto *const sw = llvm_cast<SwitchStmt>(terminator))
          {
            on_switch_branch(sw, block->succs());
          }
          break;
        }

        case Stmt::GotoStmtClass:
        case Stmt::BreakStmtClass:
        case Stmt::ContinueStmtClass: {
          if (block->succ_size() == 1)
          {
            on_simple_jump(terminator, *(block->succ_begin()));
          }
          break;
        }

        default: {
          if (block->succ_size() == 2)
          {
            const CFGBlock *const b1 = *(block->succ_begin());
            const CFGBlock *const b2 = *(block->succ_begin() + 1);
            on_conditional_branch(terminator, b2, b1);
          }
          break;
        }
        }
      }

      on_exit_block(block);
    }
  }
} // namespace ia::fixpoint