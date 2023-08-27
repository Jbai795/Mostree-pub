#############################################
#               Build Mostree           #
#############################################

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

find_package(emp-tool REQUIRED)
include_directories(${EMP-TOOL_INCLUDE_DIRS})
add_executable(fss_main "fss_main.cpp")
target_link_libraries(fss_main ${EMP-TOOL_LIBRARIES})