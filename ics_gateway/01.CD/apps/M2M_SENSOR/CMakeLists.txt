cmake_minimum_required (VERSION 2.8.10)

include_directories(
    ../3rd
    ../3rd/Exprtk
    ../Common
    ../M2M_SENSOR
    ../../3rd.party.raspi/include
)

add_definitions( -DAPP_INDEX=6 )

#####################################################################

set(
    IT5-M2M-Sensor_SOURCES

    ../Common/ComSysNetwork.cpp
    ../Common/ComConfig.cpp
    ../Common/ComUtil.cpp
    ../Common/ComQueue.cpp
    ../Common/ComState.cpp
    ../Common/ComSysInfo.cpp
    
    M2MSensorConfig.cpp
    main.cpp
    DS18B20.cpp
    SensorDevice.cpp
)

add_executable(
    it5-m2m-sensor

    ${IT5-M2M-Sensor_SOURCES}
)

target_link_libraries(
   it5-m2m-sensor
   -lextracore
   -lPocoNet
   -lPocoUtil
   -lPocoCrypto
   -lPocoXML
   -lPocoJSON
   -lPocoFoundation

   -lpthread
   -levent
   -lhiredis
   -lssl
   -lcrypto
   -lrt
)


INSTALL(TARGETS it5-m2m-sensor
    RUNTIME DESTINATION ${OUTPUT_PATH}/bin
)