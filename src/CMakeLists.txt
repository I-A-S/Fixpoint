set(SRC_FILES
    "cpp/fixpoint.cpp"
    "cpp/options.cpp"
    "cpp/utils.cpp"
    "cpp/compile_db.cpp"
    "cpp/control_flow_visitor.cpp"
)

add_library(Fixpoint STATIC ${SRC_FILES})

target_link_libraries(Fixpoint PUBLIC
    IACrux
)

if(MSVC)
    llvm_map_components_to_libnames(llvm_libs support core irreader)
    target_link_libraries(Fixpoint PUBLIC
        ${llvm_libs}
        clangInterpreter
        clangFrontend
        clangTooling
        clangAST
    )
else()
    target_link_libraries(Fixpoint PUBLIC
        clang-cpp
        LLVM
    )
endif()

target_include_directories(Fixpoint PRIVATE hpp)

target_include_directories(Fixpoint PUBLIC
    "${FIXPOINT_ROOT}/include"
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

target_link_directories(Fixpoint PUBLIC
    ${LLVM_LIBRARY_DIRS}
    ${CLANG_LIBRARY_DIRS}
)

target_compile_definitions(Fixpoint PUBLIC
    ${LLVM_DEFINITIONS}
)

target_precompile_headers(Fixpoint PUBLIC "${FIXPOINT_ROOT}/include/fixpoint/pch.hpp")