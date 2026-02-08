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
  struct State
  {
    i32 count = 0;

    auto operator==(const State &o) const -> bool
    {
      return count == o.count;
    }
  };

  class SaturatingSolver : public fixpoint::DataFlowSolver<State>
  {
public:
    static constexpr i32 SATURATION_LIMIT = 5;
    Arc<i32> m_max_value_reached;

    SaturatingSolver(Arc<i32> ptr) : m_max_value_reached(ptr)
    {
    }

    auto get_initial_state() -> State override
    {
      return {0};
    }

    auto get_matcher() const -> fixpoint::DeclarationMatcher override
    {
      return fixpoint::ast::functionDecl(fixpoint::ast::isDefinition());
    }

    auto merge(Ref<State> current, Ref<State> incoming) -> State override
    {
      return {std::max(current.count, incoming.count)};
    }

    auto transfer(const fixpoint::Stmt *stmt, MutRef<State> state) -> void override
    {
      AU_UNUSED(stmt);

      if (state.count < SATURATION_LIMIT)
        state.count++;
      *m_max_value_reached = std::max(*m_max_value_reached, state.count);
    }
  };
} // namespace

IAT_BEGIN_BLOCK(Core, DataFlowSolver)

auto test_solver_execution() -> bool
{
  auto result_val = std::make_shared<int>(0);
  const std::string code = R"(
        void simple_linear() {
            int a = 1;
            int b = 2;
        }
    )";

  IAT_CHECK(run_test_on_code(code, SaturatingSolver(result_val)));
  IAT_CHECK(*result_val > 0);

  return true;
}

auto test_solver_convergence() -> bool
{
  const std::string code = R"(
        void loop_func() {
            for(int i=0; i<100; ++i) {
                int x = i;
            }
        }
    )";

  auto result_val = std::make_shared<int>(0);

  bool success = run_test_on_code(code, SaturatingSolver(result_val));

  IAT_CHECK(success);

  IAT_CHECK_EQ(*result_val, 5);

  return true;
}

IAT_BEGIN_TEST_LIST()
IAT_ADD_TEST(test_solver_execution);
IAT_ADD_TEST(test_solver_convergence);
IAT_END_TEST_LIST()

IAT_END_BLOCK()

IAT_REGISTER_ENTRY(Core, DataFlowSolver)