# This file is used to build the process library.
set(LIB_PROCESS process)
# get all source files
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRCS)
# create library
add_library(${LIB_PROCESS} STATIC)
target_sources(${LIB_PROCESS} PRIVATE ${SRCS})
target_include_directories(${LIB_PROCESS} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${LIB_PROCESS} parser)