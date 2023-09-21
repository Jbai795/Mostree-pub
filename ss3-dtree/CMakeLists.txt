#############################################
#               Build Mostree           #
#############################################
set(TARGET mostree-main)

set(NTL_ROOT "${CMAKE_SOURCE_DIR}/deps/build")
message(STATUS "NTL_ROOT: ${NTL_ROOT}")
file(GLOB_RECURSE SRCS *.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(${TARGET} ${SRCS})
#add_backward(${TARGET})

target_include_directories(${TARGET} PUBLIC ${NTL_ROOT}/include)
target_link_directories(${TARGET}  PUBLIC ${NTL_ROOT}/lib)
#target_link_libraries(${TARGET} aby3 Backward::Backward ntl m pthread)
target_link_libraries(${TARGET} aby3 ${NTL_ROOT}/lib/ntl.a m pthread)
#target_compile_definitions(${TARGET} PUBLIC )


