<div align="center">
  <img src="logo.png" alt="Fixpoint Logo" width="130" style="border-radius: 1.15rem;"/>
  <br/>
  
  <img src="https://img.shields.io/badge/license-apache_v2-blue.svg" alt="License"/>
  <img src="https://img.shields.io/badge/standard-C%2B%2B20-yellow.svg" alt="C++ Standard"/>

  <p style="padding-top: 0.2rem;">
    <b>Powerful C++ Static Analysis, Simplified.</b>
  </p>
</div>

## **Description**

**Fixpoint** is a modular static analysis framework built on top of LLVM/Clang LibTooling. It abstracts away the complexity of raw Clang AST traversals and Control Flow Graph (CFG) iterations, providing a clean C++20 task-based architecture for building custom linters, safety checks, and data flow analysis tools.

Designed for the **Independent Architecture (IA)** ecosystem, it relies on the high-performance IACrux foundation library.

## **Features**

* **Task-Based Workloads**: Organize analysis logic into isolated, reusable Tasks managed by a central Workload executor.

* **Data Flow Analysis Engine**: A generic DataFlowSolver implementing a worklist algorithm for forward data flow analysis. simply define merge and transfer functions to track state across the CFG.

* **Declaration Policing**: The DeclPolice utility simplifies AST matching by automatically filtering system headers and non-main files, allowing you to focus strictly on compliance logic.

* **Control Flow Visitors**: Built-in hooks for visiting CFG blocks, conditional branches (if, switch), and loop decisions without manually traversing the graph.

* **Robust Tooling**: Wraps Clang's CommonOptionsParser and CompilationDatabase for seamless integration with compile_commands.json.

* **Cross-Platform**: Includes CMake Presets for Linux (x64/ARM64) and Windows (x64/ARM64).

## **Requirements**

* **C++ Compiler**: C++20 compliant (Clang 18+, or GCC 11+).  
* **LLVM/Clang**: Version **21.0** or higher.

* **CMake**: Version **3.28** or higher.

## **Integration**

Fixpoint is designed to be consumed as a CMake subdirectory. It depends on IACrux, which is included as a git submodule.

```bash
git clone https://github.com/I-A-S/Fixpoint.git  
cd Fixpoint  
git submodule update --init --recursive
```

## **Quick Start**

### **1. Creating a Simple Linter (DeclPolice)**

Inherit from DeclPolice to create a task that flags specific AST patterns. The base class handles source location filtering for you.

```cpp
#include <fixpoint/fixpoint.hpp>

using namespace ia::fixpoint;

class NoGlobalVarsTask : public DeclPolice {  
public:  
    // 1. Define the AST Matcher (using Clang AST Matchers)  
    auto get_matcher() const -> DeclarationMatcher override {  
        return ast::varDecl(ast::hasGlobalStorage()).bind("var");  
    }

    // 2. Implement the policing logic  
    auto police(Ref<MatchResult> result, const Decl *decl, Ref<SourceLocation> loc) -> void override {  
        const auto *var = llvm_cast<const VarDecl>(decl);  
        llvm::outs() << "[Violation] Global variable '" << var->getNameAsString()   
                     << "' found at " << utils::get_loc_str_path_and_line(loc) << "n";  
    }  
};
```

### **2. Implementing Data Flow Analysis (DataFlowSolver)**

Inherit from `DataFlowSolver<StateT>` to track abstract state through a function's control flow.

```cpp
struct AnalysisState {  
    int value = 0;  
    bool operator==(const AnalysisState& other) const = default;  
};

class ValueTracker : public DataFlowSolver<AnalysisState> {  
public:  
    auto get_matcher() const -> DeclarationMatcher override {  
        return ast::functionDecl(ast::isDefinition());  
    }

    auto get_initial_state() -> AnalysisState override {  
        return {0};  
    }

    // Merge states from predecessor blocks (e.g., after an if/else)  
    auto merge(Ref<AnalysisState> current, Ref<AnalysisState> incoming) -> AnalysisState override {  
        return { std::max(current.value, incoming.value) };  
    }

    // Update state based on a statement in the block  
    auto transfer(const Stmt *stmt, MutRef<AnalysisState> state) -> void override {  
        if (llvm_cast<const ReturnStmt>(stmt)) {  
            // Analyze return logic...  
        }  
        state.value++;  
    }  
};
```

### **3. Running the Tool**

Wire everything together in your main.cpp.

```cpp
#include <fixpoint/fixpoint.hpp>

int main(int argc, const char **argv) {  
    using namespace ia::fixpoint;

    // 1. Parse Options (compilation database is loaded automatically)  
    auto opts = Options::create("MyAnalyzer", argc, argv);  
    if (!opts) return 1;

    auto db = CompileDB::create(opts.value());  
    if (!db) return 1;

    auto tool = Tool::create(opts.value(), db.value());  
    if (!tool) return 1;

    // 2. Setup Workload  
    auto workload = make_box<Workload>();  
    workload->add_task<NoGlobalVarsTask>();  
    workload->add_task<ValueTracker>();

    // 3. Execute  
    if (auto res = tool.value()->run(workload); !res) {  
        llvm::errs() << "Analysis failed: " << res.error() << "n";  
        return 1;  
    }

    return 0;  
}
```

## **Building**

Fixpoint uses `CMakePresets.json` for build configuration.

**Linux (x64)**

```bash
cmake --preset fixpoint-x64-linux  
cmake --build --preset fixpoint-x64-linux  
ctest --preset fixpoint-x64-linux
```

**Windows (x64 Clang)**

```bash
cmake --preset fixpoint-x64-windows  
cmake --build --preset fixpoint-x64-windows  
ctest --preset fixpoint-x64-windows
```

## **License**

Copyright (C) 2026 IAS. Licensed under the [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0).
