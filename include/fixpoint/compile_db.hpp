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

#include <fixpoint/options.hpp>

namespace ia::fixpoint
{
  class CompileDB : public clang::tooling::CompilationDatabase
  {
public:
    static auto create(MutRef<Options> options) -> Result<CompileDB>;

    ~CompileDB() = default;

public:
    [[nodiscard]] auto getCompileCommands(Mut<LLVM_StringRef> file_path) const -> Vec<CompileCommand> override
    {
      auto cmds = mut(m_base.getCompileCommands(file_path));

      if (cmds.size() > 1)
        cmds.resize(1);

      return cmds;
    }

    [[nodiscard]] auto getAllFiles() const -> Vec<String> override
    {
      return m_base.getAllFiles();
    }

    [[nodiscard]] auto getAllCompileCommands() const -> Vec<CompileCommand> override
    {
      return m_base.getAllCompileCommands();
    }

private:
    const clang::tooling::CompilationDatabase &m_base;

    CompileDB(Ref<clang::tooling::CompilationDatabase> db);
  };
} // namespace ia::fixpoint