cmake_minimum_required(VERSION 3.10)

# Project name
project(MyProject)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/cil
    ${PROJECT_SOURCE_DIR}/head
    ${PROJECT_SOURCE_DIR}/redis
    ${PROJECT_SOURCE_DIR}/sen_rec
    ${PROJECT_SOURCE_DIR}/ser
    ${PROJECT_SOURCE_DIR}/sql
)

# Add the executable for cli
add_executable(cli
    ${PROJECT_SOURCE_DIR}/cil/cil.cpp
    ${PROJECT_SOURCE_DIR}/cil/menu.cpp
    ${PROJECT_SOURCE_DIR}/cil/User.cpp
    ${PROJECT_SOURCE_DIR}/cil/user_work.cpp
    ${PROJECT_SOURCE_DIR}/cil/prich.cpp
    ${PROJECT_SOURCE_DIR}/cil/file.cpp
)

# Link libraries for cli
target_link_libraries(cli
    hiredis
    ncurses
)

# Add the executable for server
add_executable(server
    ${PROJECT_SOURCE_DIR}/redis/redis.cpp
    ${PROJECT_SOURCE_DIR}/ser/ser.cpp
    ${PROJECT_SOURCE_DIR}/ser/User.cpp
    ${PROJECT_SOURCE_DIR}/ser/message.cpp
)

# Link libraries for server
target_link_libraries(server
    hiredis
    mysqlclient
)
