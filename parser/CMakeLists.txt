# This file is used to build the parser library.
set(LIB_PARSER parser)
# get all source files
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRCS)
# create library
add_library(${LIB_PARSER} STATIC)
target_sources(${LIB_PARSER} PRIVATE ${SRCS})
target_include_directories(${LIB_PARSER} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${LIB_PARSER} function)