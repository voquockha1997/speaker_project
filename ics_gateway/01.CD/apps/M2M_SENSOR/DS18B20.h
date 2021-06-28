#ifndef DS18B20_H_
#define DS18B20_H_
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ComUtil.h"
#define CELCIUS 0
#define FAHRENHEIT 1
#define BUS "/sys/bus/w1/devices/"
#define TEMPFILE "/w1_slave"

#define COMMON_LOG_GROUP LOG_G_M2MINTERFACE

class DS18B20 {
public:
	DS18B20(const char* address);
	virtual ~DS18B20();
	uint8_t getUnits();
	void setUnits(uint8_t);
	float getTemp();
	float CtoF(float);
private:
	uint8_t unit_;
	char* address_;
	char path[47]; // path should be 46 chars

};

#endif /* DS18B20_H_ */
