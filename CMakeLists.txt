cmake_minimum_required(VERSION 3.20)

project(BayesNet
  VERSION 1.0.0
  DESCRIPTION "Bayesian Network and basic classifiers Library."
  HOMEPAGE_URL "https://github.com/rmontanana/bayesnet"
  LANGUAGES CXX
)

if (CODE_COVERAGE AND NOT ENABLE_TESTING)
  MESSAGE(FATAL_ERROR "Code coverage requires testing enabled")
endif (CODE_COVERAGE AND NOT ENABLE_TESTING)

find_package(Torch REQUIRED)

if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif ()

# Global CMake variables
# ----------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED            ON)
set(CMAKE_CXX_EXTENSIONS                  OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS          ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
# Options
# -------
option(ENABLE_CLANG_TIDY "Enable to add clang tidy."              OFF)
option(ENABLE_TESTING "Unit testing build"                        OFF)
option(CODE_COVERAGE "Collect coverage from test library"         OFF)

# CMakes modules
# --------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
include(AddGitSubmodule)

if (CODE_COVERAGE)
    enable_testing()
    include(CodeCoverage)
    MESSAGE("Code coverage enabled")
    set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0 -g")
    SET(GCC_COVERAGE_LINK_FLAGS " ${GCC_COVERAGE_LINK_FLAGS} -lgcov --coverage")
endif (CODE_COVERAGE)

if (ENABLE_CLANG_TIDY)
  include(StaticAnalyzers) # clang-tidy
endif (ENABLE_CLANG_TIDY)

# External libraries - dependencies of BayesNet
# ---------------------------------------------
# include(FetchContent)
add_git_submodule("lib/mdlp")
add_git_submodule("lib/json")

# Subdirectories
# --------------
add_subdirectory(config)
add_subdirectory(lib/Files)
add_subdirectory(src/BayesNet)

file(GLOB BayesNet_HEADERS CONFIGURE_DEPENDS ${BayesNet_SOURCE_DIR}/src/BayesNet/*.h ${BayesNet_SOURCE_DIR}/BayesNet/*.h)
file(GLOB BayesNet_SOURCES CONFIGURE_DEPENDS ${BayesNet_SOURCE_DIR}/src/BayesNet/*.cc ${BayesNet_SOURCE_DIR}/src/BayesNet/*.cpp)

# Testing
# -------

if (ENABLE_TESTING)
  MESSAGE("Testing enabled")
  add_git_submodule("lib/catch2")  
  include(CTest)
  add_subdirectory(tests)
endif (ENABLE_TESTING)
