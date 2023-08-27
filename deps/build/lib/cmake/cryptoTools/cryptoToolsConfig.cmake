
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was Config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

####################################################################################

include("${CMAKE_CURRENT_LIST_DIR}/cryptoToolsTargets.cmake")


set(ENABLE_RELIC     OFF)
set(ENABLE_SODIUM    OFF)
set(ENABLE_CIRCUITS  ON)
set(ENABLE_NET_LOG   OFF)
set(ENABLE_WOLFSSL   OFF)
set(ENABLE_SSE       ON)
set(ENABLE_BOOST     ON)

include("${CMAKE_CURRENT_LIST_DIR}/cryptoToolsDepHelper.cmake")


set(cryptoTools_INCLUDE_DIRS "")
OC_getAllLinkedLibraries(oc::cryptoTools cryptoTools_LIBRARIES cryptoTools_INCLUDE_DIRS)
OC_getAllLinkedLibraries(oc::tests_cryptoTools tests_cryptoTools_LIBRARIES tests_cryptoTools_INCLUDE_DIRS)


# short names
set(cryptoTools_LIB ${cryptoTools_LIBRARIES})
set(cryptoTools_INC ${cryptoTools_INCLUDE_DIRS})
set(tests_cryptoTools_LIB ${tests_cryptoTools_LIBRARIES})
set(tests_cryptoTools_INC ${tests_cryptoTools_INCLUDE_DIRS})
