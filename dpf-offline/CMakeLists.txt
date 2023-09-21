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


add_executable(offline_batch_gen "offline_batch_gen.cpp")

set(DEPS "${CMAKE_SOURCE_DIR}/deps/build")
message(STATUS "DEPS: " ${DEPS})
target_include_directories(offline_batch_gen PUBLIC ${DEPS}/include)
target_link_directories(offline_batch_gen PUBLIC ${DEPS}/lib)

set(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS}  -std=c++14") # MUST SET! Otherwise you will get -std=c++11 and cannot compile

target_link_libraries(offline_batch_gen ${EMP-TOOL_LIBRARIES} cryptoTools aby3)