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
  namespace utils
  {
    [[nodiscard]] auto get_loc_str_path_and_line(Ref<FullSourceLoc> loc) -> String;
    [[nodiscard]] auto get_decl_str_start_and_end_cols(const VarDecl *decl) -> String;
    [[nodiscard]] auto get_decl_str_start_and_end_cols(const FunctionDecl *decl) -> String;
    [[nodiscard]] auto get_decl_str_start_and_end_cols(const CXXRecordDecl *decl) -> String;
    [[nodiscard]] auto get_ref_str_start_and_end_cols(const fixpoint::DeclRefExpr *ref) -> String;

    [[nodiscard]] auto fits_in_register(const VarDecl *decl) -> bool;
    [[nodiscard]] auto is_cheap_to_copy(const VarDecl *decl) -> bool;
    [[nodiscard]] auto is_std_class(QualType type, const char *class_name) -> bool;
    [[nodiscard]] auto is_std_call(const CallExpr *call, const char *function_name) -> bool;

    [[nodiscard]] inline auto is_string_view(QualType type) -> bool
    {
      return is_std_class(type, "basic_string_view");
    }

    [[nodiscard]] inline auto is_memory_span(QualType type) -> bool
    {
      return is_std_class(type, "span");
    }

    [[nodiscard]] inline auto is_std_function(QualType type) -> bool
    {
      return is_std_class(type, "function");
    }
  } // namespace utils
} // namespace ia::fixpoint