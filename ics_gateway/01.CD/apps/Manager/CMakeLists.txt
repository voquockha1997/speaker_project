cmake_minimum_required (VERSION 2.8.10)

include_directories(
    ../3rd
    ../Common
    ../../3rd.party.raspi/include
)

set(
    IT5-Manager_SOURCES
    
    ../Common/ComUtil.cpp
    ../Common/ComQueue.cpp
    ../Common/ComState.cpp
    ../Common/ComConfig.cpp
    ../Common/ComSysNetwork.cpp
    ../Common/ComSysInfo.cpp
    
    ManagerConfig.cpp
    ManagerBridgeHandler.cpp
    ManagerWatchDog.cpp
    Manager.cpp
    ManagerDownlinkHandler.cpp
    ManagerConsumer.cpp
    main.cpp
)

add_definitions( -DAPP_INDEX=0 )

add_executable(
    it5-manager
    ${IT5-Manager_SOURCES}
)

target_link_libraries(
    it5-manager
    -lextracore
    -lPocoNetSSL
    -lPocoNet
    -lPocoUtil
    -lPocoXML
    -lPocoCrypto
    -lPocoJSON
    -lPocoFoundation
    -lpthread
    -levent
    -lhiredis
    -lrt
    -lwiringPi 
    -lm 
    -lcrypt 
)

INSTALL(TARGETS it5-manager
    RUNTIME DESTINATION ${OUTPUT_PATH}/bin
)
