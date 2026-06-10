cmake_minimum_required(VERSION 3.16)
project(nrf52840_bacnet_field_node LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NRF_FIELD_NODE_BUILD_TESTS "Build host-side validation tests" ON)

add_library(field_node_core
  src/FieldNode.cpp
)

target_include_directories(field_node_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(field_node_core
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(${PROJECT_NAME}
  src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    field_node_core
)

target_compile_options(${PROJECT_NAME}
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

if(NRF_FIELD_NODE_BUILD_TESTS)
  enable_testing()

  add_executable(field_node_tests
    tests/FieldNodeTests.cpp
  )

  target_link_libraries(field_node_tests
    PRIVATE
      field_node_core
  )

  target_compile_options(field_node_tests
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
  )

  add_test(NAME field_node_core_tests COMMAND field_node_tests)
endif()
