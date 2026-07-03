#include "sound.h"
#include <math.h> // 引入对数函数 log10

// 替换这里！改为外部引用 ADC2
extern ADC_HandleTypeDef hadc2; 

const uint32_t sampleWindow = 50; // 50ms 采样窗口[cite: 10]
float db_value = 0.0f;            // 最终的分贝值[cite: 10]

static double Sample_Sound_Peak(void)
{
    uint32_t startMillis = HAL_GetTick(); 
    uint16_t signalMax = 0;
    uint16_t signalMin = 4095; 
    uint16_t sample;

    while ((HAL_GetTick() - startMillis) < sampleWindow)
    {
        // 替换这里！改为操作 hadc2
        HAL_ADC_Start(&hadc2); 
        
        if (HAL_ADC_PollForConversion(&hadc2, 10) == HAL_OK)
        {
            sample = HAL_ADC_GetValue(&hadc2); 
            if (sample < 4096) 
            {
                if (sample > signalMax)      signalMax = sample;
                else if (sample < signalMin) signalMin = sample;
            }
        }
        HAL_ADC_Stop(&hadc2);
    }
    return (double)(signalMax - signalMin);
}

/**
 * @brief  初始化声音传感器
 */
void Sound_Init(void)
{
    db_value = 0.0f;
}

/**
 * @brief  采集声音并自动转换为分贝值，更新全局变量 db_value
 */
void Sound_Update_dB(void)
{
    // 1. 获取 50ms 内的 ADC 峰峰差值
    double soundSensed = Sample_Sound_Peak();

    // 2. 转换为峰峰电压值 (Vpp)
    double volts = (soundSensed * 3.3) / 4096.0;

    // 3. 使用对数公式换算为分贝 (dB)
    if (volts > 0.005) // 防止接近 0 时对数运算产生负无穷大
    {
        // 这里的 65.0 是经验校准常数。
        // 调试时，你可以在完全安静的寝室用手机分贝 APP 测一下环境音（比如 40dB），
        // 如果板子输出 45dB，就把 65.0 改成 60.0，以此类推。
        db_value = (float)(20.0 * log10(volts) + 65.0);
    }
    else
    {
        db_value = 30.0f; // 寝室极度安静时的基础环境底噪
    }
}