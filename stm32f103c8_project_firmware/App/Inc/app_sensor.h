#ifndef __APP_SENSOR_H
#define __APP_SENSOR_H

#include <stdio.h>

#include "adc.h"
#include "main.h"

#define TEMP_ERROR -999

extern int last_temp;

void Sensor_Init(void);
int Sensor_ReadTemperature(void);

// Hàm mới cho Edge AI
int Sensor_DetectAnomaly(int new_temp);

#endif /* __APP_SENSOR_H */