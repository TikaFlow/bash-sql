# This file is used to build the function library.
set(LIB_FUNCTION function)
# get all source files
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRCS)
# dependencies
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(UUID REQUIRED uuid)
# create library
add_library(${LIB_FUNCTION} STATIC)
target_sources(${LIB_FUNCTION} PRIVATE ${SRCS})
target_include_directories(${LIB_FUNCTION} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${UUID_INCLUDE_DIRS})
target_link_libraries(${LIB_FUNCTION} util OpenSSL::SSL OpenSSL::Crypto ${UUID_LIBRARIES})