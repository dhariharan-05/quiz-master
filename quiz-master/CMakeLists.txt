cmake_minimum_required(VERSION 3.5.0)
project(basics VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()

set(CURSES_NEED_NCURSES TRUE)

find_package(SQLite3 REQUIRED)
find_package(Curses REQUIRED)
find_library(MENU_LIBRARY menu)
find_package(cJSON REQUIRED)

add_executable(basics main.c quiz_utility.c welcome_screen.c score_screen.c)
include_directories(${SQLite3_INCLUDE_DIRS})
include_directories(${CURSES_INCLUDE_DIRS})
include_directories(${CJSON_INCLUDE_DIRS})

target_link_libraries(basics SQLite::SQLite3)
target_link_libraries(basics ${CURSES_LIBRARIES})
target_link_libraries(basics ${MENU_LIBRARY} ${CJSON_LIBRARIES})

message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message("SQLITE3_LIBRARY: ${CJSON_INCLUDE_DIRS}")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)