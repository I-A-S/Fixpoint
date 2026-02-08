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
  class VarFinder : public fixpoint::DeclPolice
  {
public:
    std::vector<std::string> found_vars;

    [[nodiscard]] auto get_matcher() const -> fixpoint::DeclarationMatcher override
    {
      return fixpoint::ast::varDecl(fixpoint::ast::unless(fixpoint::ast::isImplicit())).bind("var");
    }

    auto police(Ref<fixpoint::MatchFinder::MatchResult> result, const fixpoint::Decl *decl,
                Ref<fixpoint::SourceLocation> loc) -> void override
    {
      AU_UNUSED(result);
      AU_UNUSED(loc);

      if (const auto var = fixpoint::llvm_cast<const fixpoint::VarDecl>(decl))
        found_vars.push_back(var->getNameAsString());
    }
  };
} // namespace

IAT_BEGIN_BLOCK(Core, DeclPolice)

auto test_var_decl() -> bool
{
  const std::string code = R"(
    void f() {
        int x = 10;
        double y = 20.0;
    }
  )";

  VarFinder finder;
  bool success = run_test_on_code(code, std::move(finder));

  return success;
}

auto test_capture_vars() -> bool
{
  struct SharedState
  {
    std::vector<std::string> vars;
  };

  auto state = std::make_shared<SharedState>();

  class SharedFinder : public fixpoint::DeclPolice
  {
    Arc<SharedState> m_state;

public:
    SharedFinder(Arc<SharedState> s) : m_state(s)
    {
    }

    [[nodiscard]] auto get_matcher() const -> fixpoint::DeclarationMatcher override
    {
      return fixpoint::ast::varDecl(fixpoint::ast::unless(fixpoint::ast::isImplicit()));
    }

    auto police(Ref<fixpoint::MatchFinder::MatchResult>, const fixpoint::Decl *d, Ref<fixpoint::SourceLocation>)
        -> void override
    {
      if (const auto v = fixpoint::llvm_cast<const fixpoint::VarDecl>(d))
      {
        m_state->vars.push_back(v->getNameAsString());
      }
    }
  };

  const std::string code = "int global_var; void f() { int local_var; }";
  IAT_CHECK(run_test_on_code(code, SharedFinder(state)));

  IAT_CHECK_EQ(state->vars.size(), 2u);
  bool has_global = std::ranges::find(state->vars, "global_var") != state->vars.end();
  bool has_local = std::ranges::find(state->vars, "local_var") != state->vars.end();

  IAT_CHECK(has_global);
  IAT_CHECK(has_local);

  return true;
}

IAT_BEGIN_TEST_LIST()
IAT_ADD_TEST(test_var_decl);
IAT_ADD_TEST(test_capture_vars);
IAT_END_TEST_LIST()

IAT_END_BLOCK()

IAT_REGISTER_ENTRY(Core, DeclPolice)