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

#pragma once

#include <iatest/iatest.hpp>

#include <fixpoint/fixpoint.hpp>

#include <fstream>

namespace ia::fixpoint
{
  template<typename TaskT> auto run_test_on_code(const std::string &code, TaskT &&task) -> bool
  {
    const std::string filename = "temp_fixpoint_test.cpp";
    {
      std::ofstream out(filename);
      out << code;
    }

    const char *argv[] = {"fixpoint_test", filename.c_str(), "--", "-std=c++20"};
    int argc = 4;

    auto options = fixpoint::Options::create("Test", argc, argv);
    if (!options)
      return false;

    auto db = fixpoint::CompileDB::create(*options);
    if (!db)
      return false;

    auto tool = fixpoint::Tool::create(*options, *db);
    if (!tool)
      return false;

    fixpoint::Workload workload;
    workload.add_task(std::make_unique<TaskT>(std::move(task)));

    auto res = (*tool)->run(workload);
    std::filesystem::remove(filename);
    return res.has_value();
  }
} // namespace ia::fixpoint