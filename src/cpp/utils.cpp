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

#include <fixpoint/utils.hpp>

namespace ia::fixpoint::utils
{
  [[nodiscard]] auto get_loc_str_path_and_line(Ref<FullSourceLoc> loc) -> String
  {
    Mut<std::error_code> ec;
    const clang::FileEntry *const file_entry = loc.getFileEntry();
    const auto file_path = file_entry ? file_entry->tryGetRealPathName().str() : String();
    const auto display_path = std::filesystem::canonical(file_path, ec);
    return std::format("{}:{}", ec ? file_path : display_path.string(), loc.getSpellingLineNumber());
  }

  [[nodiscard]] auto get_decl_str_start_and_end_cols(const VarDecl *decl) -> String
  {
    Mut<u32> start_col{0}, end_col{0};

    if (const auto tsi = decl->getTypeSourceInfo())
    {
      const auto loc = tsi->getTypeLoc().getBeginLoc();
      if (loc.isValid())
      {
        start_col = decl->getASTContext().getSourceManager().getSpellingColumnNumber(loc);
      }
    }

    {
      const auto loc = decl->getEndLoc();
      if (loc.isValid())
      {
        end_col = decl->getASTContext().getSourceManager().getSpellingColumnNumber(loc);
      }
    }

    return std::format("{}:{}", start_col, end_col);
  }

  [[nodiscard]] auto get_decl_str_start_and_end_cols(const FunctionDecl *decl) -> String
  {
    AU_UNUSED(decl);
    return ""; // [IATODO]: IMPL not needed rn
  }

  [[nodiscard]] auto get_decl_str_start_and_end_cols(const CXXRecordDecl *decl) -> String
  {
    AU_UNUSED(decl);
    return ""; // [IATODO]: IMPL not needed rn
  }

  [[nodiscard]] auto fits_in_register(const VarDecl *decl) -> bool
  {
    if (!decl)
      return false;

    clang::ASTContext &ctx = decl->getASTContext();
    const QualType t = decl->getType();

    if (t.isNull() || t->isIncompleteType())
      return false;

    if (t->isReferenceType())
      return true;

    return ctx.getTypeSize(t) <= 64;
  }

  [[nodiscard]] auto is_cheap_to_copy(const VarDecl *decl) -> bool
  {
    if (!decl)
      return false;

    const QualType t = decl->getType();

    if (fits_in_register(decl))
      return true;

    if (is_string_view(t) || is_memory_span(t))
      return true;

    clang::ASTContext &ctx = decl->getASTContext();
    if (t.isTriviallyCopyableType(ctx) && ctx.getTypeSize(t) <= 128)
      return true;

    return false;
  }

  [[nodiscard]] auto is_std_class(QualType type, const char *class_name) -> bool
  {
    if (type.isNull())
      return false;

    const QualType base = type.getNonReferenceType();

    Mut<const CXXRecordDecl *> decl = base->getAsCXXRecordDecl();

    if (!decl)
    {
      if (const auto *const tpl = base->getAs<clang::TemplateSpecializationType>())
      {
        if (const auto *const tmpl_decl = tpl->getTemplateName().getAsTemplateDecl())
        {
          decl = dyn_cast_or_null<CXXRecordDecl>(tmpl_decl->getTemplatedDecl());
        }
      }
    }

    if (!decl)
      return false;

    return decl->isInStdNamespace() && decl->getIdentifier() && decl->getIdentifier()->isStr(class_name);
  }

  [[nodiscard]] auto is_std_call(const CallExpr *call, const char *function_name) -> bool
  {
    if (!call)
      return false;

    const FunctionDecl *const func = call->getDirectCallee();
    if (!func)
      return false;

    const clang::IdentifierInfo *const ident = func->getIdentifier();
    if (!ident || !ident->isStr(function_name))
      return false;

    if (!func->isInStdNamespace())
      return false;

    return true;
  }
} // namespace ia::fixpoint::utils