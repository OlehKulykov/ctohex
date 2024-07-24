cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(ctohex LANGUAGES C CXX)

set(CTOHEX_APP ctohex)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

include(CheckLibraryExists)
include(CheckFunctionExists)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

if (MSVC)
  error("Not supported")
else()
  check_c_compiler_flag("-fPIC" WITH_FPIC_C)
  if (WITH_FPIC_C)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif()
  
  check_cxx_compiler_flag("-fPIC" WITH_FPIC_CXX)
  if(WITH_FPIC_CXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif()
  
  check_cxx_compiler_flag("-fno-rtti" WITH_FNO_RTTI_CXX)
  if(WITH_FNO_RTTI_CXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
  endif()
  
endif()

set(CTOHEX_HEADERS)

source_group("main" FILES
  ctohex.cpp
)

find_package(ZLIB)
if (ZLIB_FOUND)
  message("ZLIB found")
  message("ZLIB include dir: ${ZLIB_INCLUDE_DIRS}")
  message("ZLIB libraries: ${ZLIB_LIBRARIES}")
  include_directories(BEFORE SYSTEM ${ZLIB_INCLUDE_DIRS})
  add_definitions(-DHAVE_ZLIB=1)
  list(APPEND CTOHEX_HEADERS
    TPL1Fl.hpp
  )
  source_group("main" FILES
    TPL1Fl.hpp
  )
else()
  message("ZLIB not found")
endif()

find_package(zstd)
if (zstd_FOUND)
  message("zstd found")
  message("zstd include dir: ${zstd_INCLUDE_DIRS}")
  message("zstd libraries: ${zstd_LIBRARIES}")
  include_directories(BEFORE SYSTEM ${zstd_INCLUDE_DIRS})
  add_definitions(-DHAVE_ZSTD=1)
  list(APPEND CTOHEX_HEADERS
    TPL2Fl.hpp
  )
  source_group("main" FILES
    TPL2Fl.hpp
  )
else()
  message("zstd not found")
endif()

find_package(LibLZMA)
if (LIBLZMA_FOUND)
  message("LZMA found")
  message("LZMA include dir: ${LIBLZMA_INCLUDE_DIRS}")
  message("LZMA libraries: ${LIBLZMA_LIBRARIES}")
  include_directories(BEFORE SYSTEM ${LIBLZMA_INCLUDE_DIRS})
  add_definitions(-DHAVE_LZMA=1)
  list(APPEND CTOHEX_HEADERS
    TPL3Fl.hpp
  )
  source_group("main" FILES
    TPL3Fl.hpp
  )
else()
  message("LZMA not found")
endif()

set(CTOHEX_SOURCES
  ctohex.cpp
)

add_executable(${CTOHEX_APP} ${CTOHEX_HEADERS} ${CTOHEX_SOURCES})

if (ZLIB_FOUND)
  target_link_libraries(${CTOHEX_APP} ${ZLIB_LIBRARIES})
endif()

if (zstd_FOUND)
  target_link_libraries(${CTOHEX_APP} ${zstd_LIBRARIES})
endif()

if (LIBLZMA_FOUND)
  target_link_libraries(${CTOHEX_APP} ${LIBLZMA_LIBRARIES})
endif()
