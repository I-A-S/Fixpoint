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
  template<typename Derived> class ASTVisitor : public IWorkloadTask, public clang::RecursiveASTVisitor<Derived>
  {
public:
    [[nodiscard]] auto get_matcher() const -> DeclarationMatcher override
    {
      return ast::translationUnitDecl();
    }

    auto run(Ref<MatchFinder::MatchResult> result) -> void override
    {
      const auto *node = result.Nodes.getNodeAs<Decl>("decl");
      if (!node)
        return;

      m_last_match_result = &result;

      static_cast<Derived *>(this)->TraverseDecl(const_cast<Decl *>(node));
    }

    [[nodiscard]] auto should_visit_implicit_code() const -> bool
    {
      return false;
    }

protected:
    auto get_match_result() -> const MatchResult *
    {
      return m_last_match_result;
    }

private:
    const MatchResult *m_last_match_result{};
  };
} // namespace ia::fixpoint