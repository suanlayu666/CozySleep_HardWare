#include "MQ135.h"

extern ADC_HandleTypeDef hadc1;

void MQ135_Init(void)
{
    HAL_ADCEx_Calibration_Start(&hadc1);
}

uint16_t MQ135_ReadRaw(void)
{
    uint16_t value = 0;

    HAL_ADC_Start(&hadc1);

    if (HAL_ADC_PollForConversion(&hadc1, 50) == HAL_OK)
    {
        value = (uint16_t)HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Stop(&hadc1);

    return value;
}

float MQ135_ReadVoltage(void)
{
    uint16_t raw = MQ135_ReadRaw();

    return (float)raw * 3.3f / 4095.0f;
}