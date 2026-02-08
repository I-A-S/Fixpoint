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

#include <fixpoint/options.hpp>

namespace ia::fixpoint
{
  auto Options::create(Ref<String> name, i32 argc, const char **argv) -> Result<Options>
  {
    Mut<llvm::cl::OptionCategory> category(std::format("{} options", name));
    auto expected_parser = mut(clang::tooling::CommonOptionsParser::create(argc, argv, category));
    if (!expected_parser)
      return fail("Failed to create options from arguments: {}", llvm::toString(expected_parser.takeError()));

    return Options{std::move(expected_parser.get()), name};
  }

  Options::Options(ForwardRef<clang::tooling::CommonOptionsParser> cop, Ref<String> name)
      : m_name(name), m_cop(std::move(cop))
  {
  }
} // namespace ia::fixpoint