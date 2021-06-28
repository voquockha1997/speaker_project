cmake_minimum_required (VERSION 2.8.10)

include_directories(
    ../3rd
    ../Common
)

set(
    IT5-M2C-MQTT_SOURCES
    ../Common/ComUtil.cpp
    ../Common/ComConfig.cpp
    ../Common/ComQueue.cpp
    ../Common/ComState.cpp
    ../Common/ComSysNetwork.cpp
    
    M2CMQTT.cpp
    main.cpp
    M2CMQTTConfig.cpp
    M2CMQTTDownlink.cpp
    M2CMQTTDataSet.cpp
)

add_definitions( -DAPP_INDEX=2 )

add_executable(
    it5-m2c-mqtt

    ${IT5-M2C-MQTT_SOURCES}
)

target_link_libraries(
    it5-m2c-mqtt
    -lextracore
    -lPocoNetSSL
    -lPocoNet
    -lPocoUtil
    -lPocoXML
    -lPocoJSON
    -lPocoCrypto
    -lPocoFoundation
    -lpthread
    -levent
    -lhiredis
    -lssl
    -lcrypto
    -lrt
    -lpaho-mqttpp3
)

INSTALL(TARGETS it5-m2c-mqtt
    RUNTIME DESTINATION ${OUTPUT_PATH}/bin
)
