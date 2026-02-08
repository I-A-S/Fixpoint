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

#include <crux/crux.hpp>

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/Lexer.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Signals.h>

namespace ia::fixpoint
{
  using SourceLocation = clang::SourceLocation;
  using MatchFinder = clang::ast_matchers::MatchFinder;
  using MatchResult = clang::ast_matchers::MatchFinder::MatchResult;
  using DeclarationMatcher = clang::ast_matchers::DeclarationMatcher;
  using MatchCallback = clang::ast_matchers::MatchFinder::MatchCallback;

  using Stmt = clang::Stmt;
  using Expr = clang::Expr;
  using Decl = clang::Decl;
  using VarDecl = clang::VarDecl;
  using FuncDecl = clang::FunctionDecl;
  using DeclRefExpr = clang::DeclRefExpr;
  using CXXRecordDecl = clang::CXXRecordDecl;
  using CFGImplicitDtor = clang::CFGImplicitDtor;
  using CXXCtorInitializer = clang::CXXCtorInitializer;

  template<typename FromT, typename  ToT>
  ToT* llvm_cast(FromT& v)
  {
    return llvm::dyn_cast_or_null(v);
  }

  template<typename FromT, typename  ToT>
  const ToT* llvm_cast(const FromT& v)
  {
    return llvm::dyn_cast_or_null(v);
  }
} // namespace ia::fixpoint