# cmake version check
cmake_minimum_required(VERSION 3.15)

# cmake info
message(STATUS ">> CMake version: ${CMAKE_VERSION}")
message(STATUS ">> CMake generator: ${CMAKE_GENERATOR}")

# language settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(APP_DESCRIPTION "Using SQL-like languages to process lightweight data in bash.")

# project info
project(bash-sql LANGUAGES CXX VERSION 1.0.0 DESCRIPTION ${APP_DESCRIPTION}
        HOMEPAGE_URL "https://github.com/TikaFlow/bash-sql")

# header files
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# macros
add_definitions(-DAPP_NAME="bash-sql")
add_definitions(-DAUTHOR="Tika Flow")
add_definitions(-DVERSION="1.3.1")
add_definitions(-DAPP_DESC=\"${APP_DESCRIPTION}\")

# subdirectories
add_subdirectory(util)
add_subdirectory(function)
add_subdirectory(parser)
add_subdirectory(process)

set(EXE_NAME sql)
# get all source files
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCS)
# add executable target
add_executable(${EXE_NAME} ${SRCS})
target_link_libraries(${EXE_NAME} util parser process)

# install
install(TARGETS ${EXE_NAME} DESTINATION bin)