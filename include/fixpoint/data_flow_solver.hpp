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
  template<typename T>
  concept DataFlowState = std::default_initializable<T> && std::copy_constructible<T> && std::equality_comparable<T>;

  template<DataFlowState StateT> class DataFlowSolver : public IWorkloadTask
  {
public:
    [[nodiscard]] virtual auto merge(Ref<StateT> current_state, Ref<StateT> incoming_state) -> StateT = 0;

    virtual auto transfer(const Stmt *s, MutRef<StateT> state) -> void = 0;

    virtual auto transfer_initializer(const CXXCtorInitializer *init, MutRef<StateT> state) -> void
    {
      AU_UNUSED(init);
      AU_UNUSED(state);
    }

    virtual auto transfer_implicit_dtor(const CFGImplicitDtor *dtor, MutRef<StateT> state) -> void
    {
      AU_UNUSED(dtor);
      AU_UNUSED(state);
    }

    [[nodiscard]] virtual auto get_initial_state() -> StateT = 0;

public:
    auto run(Ref<MatchResult> result) -> void override;

protected:
    auto get_match_result() -> const MatchResult *
    {
      return m_last_match_result;
    }

private:
    auto analyze_function(const FunctionDecl *func, clang::ASTContext *ctx) -> void;

    const MatchResult *m_last_match_result{};
  };

  template<DataFlowState StateT> auto DataFlowSolver<StateT>::run(Ref<MatchResult> result) -> void
  {
    const auto *decl = result.Nodes.getNodeAs<Decl>("decl");
    if (!decl)
      return;

    m_last_match_result = &result;

    auto *ctx = result.Context;

    if (const auto func = llvm_cast<const FunctionDecl>(decl))
      analyze_function(func, ctx);
    else if (const auto record = llvm_cast<const CXXRecordDecl>(decl))
    {
      for (const auto method : record->methods())
        analyze_function(method, ctx);
    }
    else if (const auto tu = llvm_cast<const clang::TranslationUnitDecl>(decl))
    {
      for (const auto *sub_decl : tu->decls())
      {
        if (const auto f = llvm_cast<const FunctionDecl>(sub_decl))
          analyze_function(f, ctx);
      }
    }
  }

  template<DataFlowState StateT>
  auto DataFlowSolver<StateT>::analyze_function(const FunctionDecl *func, clang::ASTContext *ctx) -> void
  {
    if (!func || !func->hasBody())
      return;

    SourceLocation loc = func->getLocation();
    if (ctx->getSourceManager().isInSystemHeader(loc))
      return;

    clang::CFG::BuildOptions cfg_opts;
    cfg_opts.PruneTriviallyFalseEdges = true;
    cfg_opts.AddImplicitDtors = true;
    cfg_opts.AddInitializers = true;
    cfg_opts.setAllAlwaysAdd();

    std::unique_ptr<clang::CFG> cfg = clang::CFG::buildCFG(func, func->getBody(), ctx, cfg_opts);
    if (!cfg)
      return;

    std::vector<StateT> block_in_states(cfg->getNumBlockIDs());

    std::vector<const CFGBlock *> worklist;

    std::vector<bool> in_worklist_set(cfg->getNumBlockIDs(), false);

    const CFGBlock &entry_block = cfg->getEntry();
    unsigned entry_id = entry_block.getBlockID();

    block_in_states[entry_id] = get_initial_state();

    for (const auto *block : *cfg)
    {
      if (block)
      {
        worklist.push_back(block);
        in_worklist_set[block->getBlockID()] = true;
      }
    }

    while (!worklist.empty())
    {
      const CFGBlock *block = worklist.back();
      worklist.pop_back();
      unsigned block_id = block->getBlockID();
      in_worklist_set[block_id] = false;

      StateT current_state = block_in_states[block_id];

      for (const auto &element : *block)
      {
        if (auto cfg_stmt = element.getAs<clang::CFGStmt>())
        {
          transfer(cfg_stmt->getStmt(), current_state);
        }
        else if (auto cfg_init = element.getAs<CFGInitializer>())
        {
          transfer_initializer(cfg_init->getInitializer(), current_state);
        }
        else if (auto cfg_dtor = element.getAs<CFGImplicitDtor>())
        {
          transfer_implicit_dtor(&*cfg_dtor, current_state);
        }
      }

      for (auto it = block->succ_begin(); it != block->succ_end(); ++it)
      {
        const CFGBlock *succ = *it;

        if (!succ)
          continue;

        unsigned succ_id = succ->getBlockID();
        StateT &succ_in_state = block_in_states[succ_id];

        StateT new_succ_state = merge(succ_in_state, current_state);

        if (!(new_succ_state == succ_in_state))
        {
          succ_in_state = std::move(new_succ_state);

          if (!in_worklist_set[succ_id])
          {
            worklist.push_back(succ);
            in_worklist_set[succ_id] = true;
          }
        }
      }
    }
  }
} // namespace ia::fixpoint