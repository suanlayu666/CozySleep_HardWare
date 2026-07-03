#ifndef __MQ135_H__
#define __MQ135_H__

#include "main.h"

void MQ135_Init(void);
uint16_t MQ135_ReadRaw(void);
float MQ135_ReadVoltage(void);

#endif