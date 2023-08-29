#############################################
#               Build Mostree           #
#############################################

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

find_package(emp-tool REQUIRED)
include_directories(${EMP-TOOL_INCLUDE_DIRS})


add_executable(dpf_batch_gen "dpf_batch_gen.cpp")
target_link_libraries(dpf_batch_gen ${EMP-TOOL_LIBRARIES})

add_executable(ohv_batch_gen "ohv_batch_gen.cpp")
target_link_libraries(ohv_batch_gen ${EMP-TOOL_LIBRARIES})