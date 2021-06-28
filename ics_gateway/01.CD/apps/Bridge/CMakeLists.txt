cmake_minimum_required (VERSION 2.8.10)

include_directories(
    ../3rd
    ../Common
)

add_definitions( -DAPP_INDEX=1 )
 
set(
    IT5-Bridge_SOURCES

    ../Common/ComUtil.cpp
    ../Common/ComState.cpp
    ../Common/ComConfig.cpp
    ../Common/ComQueue.cpp
    ../Common/ComSysNetwork.cpp
    
    BridgeConfig.cpp
    Bridge.cpp
    main.cpp
)

add_executable(
    it5-bridge

    ${IT5-Bridge_SOURCES}
)

target_link_libraries(
    it5-bridge
    -lextracore
    -lPocoNet
    -lPocoUtil
    -lPocoXML
    -lPocoJSON
    -lPocoCrypto
    -lPocoFoundation
    -lpthread
    -levent
    -lhiredis
    -lrt
    -lm
)

INSTALL(TARGETS it5-bridge
    RUNTIME DESTINATION ${OUTPUT_PATH}/bin
)
