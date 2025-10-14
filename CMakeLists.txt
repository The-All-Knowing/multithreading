cmake_minimum_required(VERSION 3.16)
project(Tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

add_compile_options(-g)

if (ENABLE_TSAN)
    message(STATUS "ThreadSanitizer enabled")
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread -latomic)
elseif (ENABLE_ASAN)
    message(STATUS "AddressSanitizer enabled")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

# ------------------------------------------------------------------------------

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)
set(BUILD_GMOCK OFF)
set(INSTALL_GTEST ON)
FetchContent_MakeAvailable(googletest)

# ------------------------------------------------------------------------------

add_library(commonutils INTERFACE)
target_include_directories(commonutils INTERFACE utils)

# ------------------------------------------------------------------------------

function(add_gtest TGT)
  set(UNIT_TARGET ${TGT}_unit)
  set(TEST_TARGET ${TGT}_test)

  add_executable(${UNIT_TARGET} ${SRCS})
  target_include_directories(${UNIT_TARGET} PRIVATE ${GTEST_INCLUDE_DIRS})
  target_link_libraries(${UNIT_TARGET} PRIVATE ${GTEST_BOTH_LIBRARIES})
  target_link_libraries(${UNIT_TARGET} PRIVATE Threads::Threads)
  target_link_libraries(${UNIT_TARGET} PRIVATE commonutils)
  target_compile_features(${UNIT_TARGET} PRIVATE cxx_std_20)

  add_test(
    NAME ${TEST_TARGET}
    COMMAND ${UNIT_TARGET}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  set_tests_properties(${TEST_TARGET} PROPERTIES DEPENDS ${UNIT_TARGET})
endfunction()

function(add_demo TGT)
  add_executable(${TGT} ${TGT}.cc)
  target_include_directories(${TGT} PRIVATE ${GTEST_INCLUDE_DIRS})
  target_link_libraries(${TGT} PRIVATE Threads::Threads)
  target_link_libraries(${TGT} PRIVATE atomic)
  target_link_libraries(${TGT} PRIVATE commonutils)
  target_compile_features(${TGT} PRIVATE cxx_std_20)
endfunction()

enable_testing()

# ------------------------------------------------------------------------------
add_subdirectory(thread)