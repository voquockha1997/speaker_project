include(CMakeFindDependencyMacro)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
find_dependency(PahoMqttC REQUIRED)
list(REMOVE_AT CMAKE_MODULE_PATH -1)

include("${CMAKE_CURRENT_LIST_DIR}/PahoMqttCppTargets.cmake")
