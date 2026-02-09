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
  class DeclPolice : public IWorkloadTask
  {
public:
    virtual auto police(const Decl *decl, Ref<SourceLocation> loc) -> void = 0;

public:
    inline auto run(Ref<MatchResult> result) -> void override;

protected:
    auto get_match_result() -> const MatchResult *
    {
      return m_last_match_result;
    }

private:
    const MatchResult *m_last_match_result{};
  };

  auto DeclPolice::run(Ref<MatchFinder::MatchResult> result) -> void
  {
    const auto *decl = result.Nodes.getNodeAs<Decl>("decl");
    if (!decl)
      return;

    SourceLocation loc = decl->getLocation();

    if (result.SourceManager->isInSystemHeader(loc) || !result.SourceManager->isInMainFile(loc) || !loc.isValid())
      return;

    m_last_match_result = &result;

    police(decl, loc);
  }
} // namespace ia::fixpoint