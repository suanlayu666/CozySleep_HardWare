#ifndef __SOUND_H
#define __SOUND_H

#include "stm32f1xx_hal.h"

// 声明全局分贝变量，这样 main.c 包含头文件后就能直接读取它
extern float db_value;

// 函数声明
void Sound_Init(void);
void Sound_Update_dB(void);

#endif /* __SOUND_H */