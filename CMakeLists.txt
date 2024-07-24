cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(ctohex LANGUAGES C CXX)

set(CTOHEX_APP ctohex)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

include(CheckLibraryExists)
include(CheckFunctionExists)
include(ExternalProject)

find_package(ZLIB REQUIRED)
message("ZLIB found: ${ZLIB_FOUND}")
message("ZLIB include dir: ${ZLIB_INCLUDE_DIRS}")
message("ZLIB libraries: ${ZLIB_LIBRARIES}")

find_package(zstd REQUIRED)
message("zstd found: ${zstd_FOUND}")
message("zstd include dir: ${zstd_INCLUDE_DIRS}")
message("zstd libraries: ${zstd_LIBRARIES}")

find_package(LibLZMA REQUIRED)
message("LibLZMA found: ${LIBLZMA_FOUND}")
message("LibLZMA include dir: ${LIBLZMA_INCLUDE_DIRS}")
message("LibLZMA libraries: ${LIBLZMA_LIBRARIES}")

include_directories(BEFORE SYSTEM ${ZLIB_INCLUDE_DIRS})
include_directories(BEFORE SYSTEM ${zstd_INCLUDE_DIRS})
include_directories(BEFORE SYSTEM ${LIBLZMA_INCLUDE_DIRS})

# ---- sources & headers ----

set(CTOHEX_HEADERS
  TPL1Fl.hpp
  TPL2Fl.hpp
  TPL3Fl.hpp
)

set(CTOHEX_SOURCES
  ctohex.cpp
)

# ---- groups ----

source_group("main"
  FILES
  ctohex.cpp
  TPL1Fl.hpp
  TPL2Fl.hpp
  TPL3Fl.hpp
 )

add_executable(${CTOHEX_APP} ${CTOHEX_HEADERS} ${CTOHEX_SOURCES})
target_link_libraries(${CTOHEX_APP} ${ZLIB_LIBRARIES})
target_link_libraries(${CTOHEX_APP} ${zstd_LIBRARIES})
target_link_libraries(${CTOHEX_APP} ${LIBLZMA_LIBRARIES})
