set(SRC_FILES
  main.cpp

  decl_police.cpp
  data_flow_solver.cpp
  control_flow_visitor.cpp
)

add_executable(Fixpoint_Test_Suite ${SRC_FILES})

target_link_libraries(Fixpoint_Test_Suite PRIVATE
  IATest
  Fixpoint
)

set_target_properties(Fixpoint_Test_Suite PROPERTIES USE_EXCEPTIONS ON)
