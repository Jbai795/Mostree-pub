#############################################
#               Build Mostree               #
#############################################
find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/emp-base.cmake)

find_package(emp-tool REQUIRED)
include_directories(${EMP-TOOL_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)

# ---- dpf library ----
add_library(dpf_lib dpf.cpp)
target_include_directories(dpf_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# dpf.cpp 用到了 emp-tool + crypto
target_link_libraries(dpf_lib PUBLIC ${EMP-TOOL_LIBRARIES} OpenSSL::Crypto)

# ---- dpf main ----
add_executable(dpf_main dpf_main.cpp)

# dpf_main 需要用到 dpf_lib 里实现的符号（gen_dpf_key_pair/serialization/...）
target_link_libraries(dpf_main PRIVATE dpf_lib ${EMP-TOOL_LIBRARIES} OpenSSL::Crypto)

set(DEPS "${CMAKE_SOURCE_DIR}/deps/build")
message(STATUS "DEPS: ${DEPS}")

# 建议不要用 CMAKE_C_FLAGS 覆盖 CXX flags，这里改成更安全的写法：
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
