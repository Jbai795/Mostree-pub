#############################################
#              Build aby3                 #
#############################################

file(GLOB_RECURSE SRCS *.cpp *.cc)


add_library(aby3 ${SRCS})

#######################################
# Eigen
if(NOT TARGET eigen)
    find_path(EIGEN_INCLUDE_DIR "Eigen/Eigen" REQUIRED)
    if(NOT EXISTS ${EIGEN_INCLUDE_DIR})
        message(FATAL_ERROR "failed to find eigen")
    endif()
    message(STATUS "EIGEN_INCLUDE_DIR=${EIGEN_INCLUDE_DIR}")
    add_library(eigen INTERFACE IMPORTED)
    target_include_directories(eigen INTERFACE 
                    $<BUILD_INTERFACE:${EIGEN_INCLUDE_DIR}>
                    $<INSTALL_INTERFACE:>)
endif()

#######################################
# json
if(NOT TARGET json)

    find_path(JSON_INCLUDE_DIR "nlohmann/json.hpp" REQUIRED)
    if(NOT EXISTS ${JSON_INCLUDE_DIR})
        message(FATAL_ERROR "failed to find json")
    endif()
    message(STATUS "JSON_INCLUDE_DIR=${JSON_INCLUDE_DIR}")
    add_library(json INTERFACE IMPORTED)
    target_include_directories(json INTERFACE 
                    $<BUILD_INTERFACE:${JSON_INCLUDE_DIR}>
                    $<INSTALL_INTERFACE:>)
endif()

#######################################
# function2
if(NOT TARGET function2)

    find_path(FUNCTION2_INCLUDE_DIR "function2/function2.hpp" REQUIRED)
    if(NOT EXISTS ${FUNCTION2_INCLUDE_DIR})
        message(FATAL_ERROR "failed to find function2")
    endif()
    message(STATUS "FUNCTION2_INCLUDE_DIR=${FUNCTION2_INCLUDE_DIR}")
    add_library(function2 INTERFACE IMPORTED)
    target_include_directories(function2 INTERFACE 
                    $<BUILD_INTERFACE:${FUNCTION2_INCLUDE_DIR}>
                    $<INSTALL_INTERFACE:>)
endif()


# NTL
if(NOT TARGET ntl)

    find_path(NTL_INCLUDE_DIR "NTL/version.h" REQUIRED)
    if(NOT EXISTS ${NTL_INCLUDE_DIR})
        message(FATAL_ERROR "failed to find ntl")
    endif()
    message(STATUS "NTL_INCLUDE_DIR=${NTL_INCLUDE_DIR}\n")
endif()
target_link_directories(aby3  PUBLIC ${NTL_INCLUDE_DIR}/../lib)

target_include_directories(aby3 PUBLIC ${NTL_INCLUDE_DIR})
target_include_directories(aby3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)

target_link_libraries(aby3 oc::libOTe eigen json function2 ntl m pthread)

