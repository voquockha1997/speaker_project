#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "PahoMqttCpp::paho-mqttpp3" for configuration ""
set_property(TARGET PahoMqttCpp::paho-mqttpp3 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(PahoMqttCpp::paho-mqttpp3 PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libpaho-mqttpp3.so.1.0.1"
  IMPORTED_SONAME_NOCONFIG "libpaho-mqttpp3.so.1"
  )

list(APPEND _IMPORT_CHECK_TARGETS PahoMqttCpp::paho-mqttpp3 )
list(APPEND _IMPORT_CHECK_FILES_FOR_PahoMqttCpp::paho-mqttpp3 "${_IMPORT_PREFIX}/lib/libpaho-mqttpp3.so.1.0.1" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
