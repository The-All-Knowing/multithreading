cmake_minimum_required(VERSION 3.16)
project(multithreading LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
#option(ENABLE_TSAN "Enable ThreadSanitizer" ON)
set(ENABLE_TSAN OFF) # В win не работает под mingw 

# Отладочные флаги
add_compile_options(-g)

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

find_package(Threads REQUIRED)

if (NOT TARGET Threads::Threads AND CMAKE_THREAD_LIBS_INIT)
    add_library(Threads::Threads INTERFACE IMPORTED)
    set_property(TARGET Threads::Threads PROPERTY
        INTERFACE_LINK_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")
endif()

# ------------------------------------------------------------------------------

# Функция для добавления санитайзеров к target
function(enable_sanitizers TARGET)
    if(ENABLE_TSAN)
        message(STATUS "Enabling ThreadSanitizer for target ${TARGET}")
        target_compile_options(${TARGET} PRIVATE -fsanitize=thread)
        target_link_options(${TARGET} PRIVATE -fsanitize=thread -latomic)
    elseif(ENABLE_ASAN)
        message(STATUS "Enabling AddressSanitizer for target ${TARGET}")
        target_compile_options(${TARGET} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${TARGET} PRIVATE -fsanitize=address)
    endif()
endfunction()

# ------------------------------------------------------------------------------

function(add_gtest TGT SRCS)
    set(UNIT_TARGET ${TGT}_unit)
    set(TEST_TARGET ${TGT}_test)

    add_executable(${UNIT_TARGET} ${SRCS})
    target_link_libraries(${UNIT_TARGET}
        PRIVATE
            GTest::gtest
            GTest::gtest_main
            Threads::Threads
            commonutils
    )
    target_compile_features(${UNIT_TARGET} PRIVATE cxx_std_20)

    enable_sanitizers(${UNIT_TARGET})

    add_test(NAME ${TEST_TARGET} COMMAND ${UNIT_TARGET})
endfunction()

function(add_demo TGT SRCS)
    add_executable(${TGT} ${SRCS})
    target_include_directories(${TGT} PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(${TGT} PRIVATE Threads::Threads atomic commonutils)
    target_compile_features(${TGT} PRIVATE cxx_std_20)

    enable_sanitizers(${TGT})
endfunction()

enable_testing()

# ------------------------------------------------------------------------------

# Поддиректория с твоими тестами/демо
add_subdirectory(threads)
