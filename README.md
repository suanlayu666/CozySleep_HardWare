# STM32 Dormitory Environment Monitor

基于 STM32F103C8T6 的宿舍环境监控下位机工程。项目使用 STM32CubeMX 生成 HAL + CMake 工程，在 VS Code / STM32Cube 插件环境下开发。下位机负责采集宿舍环境数据，并通过串口输出 JSON Lines 数据，方便后续上位机程序接收、展示和进行 AI 分析。

## 当前功能

- 通过 `USART1` 输出环境数据，默认波特率 `115200`
- 通过 `USART3` 同步输出同一份数据，可接蓝牙串口模块，默认波特率 `9600`
- 读取 MQ135 空气质量传感器模拟量
- 读取咪头声音传感器模拟量，并估算声音分贝值
- 读取 HC-SR501 人体红外传感器高低电平
- 读取 DHT11 温湿度传感器数据
- 主循环统一打包 JSON，便于上位机按行解析

## 硬件平台

| 类型 | 型号 / 模块 |
| --- | --- |
| MCU | STM32F103C8T6 |
| 温湿度 | DHT11 |
| 空气质量 | MQ135 |
| 人体检测 | HC-SR501 |
| 声音检测 | 咪头麦克风噪声传感器 |
| 通信 | USART1 串口、USART3 蓝牙串口 |

## 引脚分配

当前 CubeMX 配置如下：

| 模块 | STM32 引脚 | 外设 / 模式 | 说明 |
| --- | --- | --- | --- |
| MQ135 AO | `PA3` | `ADC1_IN3` | 读取空气质量模拟量 |
| 声音传感器 AO | `PA4` | `ADC2_IN4` | 读取声音传感器模拟量 |
| DHT11 DATA | `PA7` | GPIO Output/Input | 单总线温湿度读取 |
| HC-SR501 OUT | `PB5` | GPIO Input | 人体红外检测 |
| USART1 TX | `PA9` | USART1_TX | 调试串口输出 |
| USART1 RX | `PA10` | USART1_RX | 调试串口接收 |
| USART3 TX | `PB10` | USART3_TX | 蓝牙串口发送 |
| USART3 RX | `PB11` | USART3_RX | 蓝牙串口接收 |
| SWDIO | `PA13` | Serial Wire | 下载 / 调试 |
| SWCLK | `PA14` | Serial Wire | 下载 / 调试 |

注意：`PA13` 和 `PA14` 是 SWD 下载调试口，不要改成普通 GPIO。

## 接线说明

### MQ135

```text
MQ135 VCC -> 5V
MQ135 GND -> GND
MQ135 AO  -> PA3 / ADC1_IN3
```

STM32 ADC 输入最大为 `3.3V`。如果 MQ135 模块使用 `5V` 供电，`AO` 最高可能超过 `3.3V`，建议加分压保护：

```text
MQ135 AO --- 10kΩ --- PA3 --- 20kΩ --- GND
```

### 声音传感器

```text
Sound VCC -> 3.3V 或 5V，按模块要求选择
Sound GND -> GND
Sound AO  -> PA4 / ADC2_IN4
```

如果模块模拟输出可能超过 `3.3V`，同样需要分压后再接入 STM32 ADC。

### HC-SR501

```text
HC-SR501 VCC -> 5V
HC-SR501 GND -> GND
HC-SR501 OUT -> PB5
```

HC-SR501 检测到人体活动时输出高电平，主循环中会转换为 `motion:1`；未检测到时为 `motion:0`。

### DHT11

```text
DHT11 VCC  -> 3.3V
DHT11 GND  -> GND
DHT11 DATA -> PA7
```

DHT11 已接入主循环，正常情况下会输出温度 `temp` 和湿度 `humi` 字段；如果读取失败，会输出带 `error` 字段的 JSON。

## 串口输出格式

主循环大约每 2 秒发送一行 JSON。正常情况下示例：

```json
{"temp":26, "humi":60, "mq135_adc":2644, "mq135_mv":2130, "motion":0, "sound_db":42}
```

DHT11 读取失败时示例：

```json
{"error": "DHT11_Failed", "mq135_adc":2644, "motion":0, "sound_db":42}
```

串口数据以换行符 `\n` 结尾，上位机可以按行读取并解析 JSON。

## 工程结构

```text
.
├── Core
│   ├── Inc
│   │   ├── dht11.h
│   │   ├── MQ135.h
│   │   ├── sound.h
│   │   └── main.h
│   └── Src
│       ├── dht11.c
│       ├── MQ135.c
│       ├── sound.c
│       └── main.c
├── Drivers
├── cmake
├── CMakeLists.txt
├── CMakePresets.json
├── NextSteoDemo1.ioc
└── STM32F103XX_FLASH.ld
```

用户传感器源文件已经在根目录 `CMakeLists.txt` 中加入编译：

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Core/Src/dht11.c
    Core/Src/MQ135.c
    Core/Src/sound.c
)
```

## 开发环境

- STM32CubeMX `6.15.0`
- STM32Cube FW_F1 `V1.8.7`
- VS Code
- STM32 VS Code / Cube 插件
- CMake + Ninja
- GNU Arm Embedded Toolchain，工程当前使用 STM32Cube bundled toolchain

## 编译

在 VS Code 中选择 `Debug` preset 后直接 Build。

也可以在命令行中执行：

```powershell
cube-cmake -S . -B build/Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake
cube-cmake --build build/Debug --
```

编译产物位于：

```text
build/Debug/NextSteoDemo1.elf
```

## 烧录与调试

推荐使用 ST-LINK 通过 SWD 下载：

```text
ST-LINK SWDIO -> PA13
ST-LINK SWCLK -> PA14
ST-LINK GND   -> GND
ST-LINK 3.3V  -> 目标板 3.3V 参考电压
```

如果 VS Code 连接失败，可以先用 STM32CubeProgrammer 测试 ST-LINK 是否能正常连接目标芯片。

## 配套上位机

本项目为宿舍环境监控系统的 STM32 下位机部分，负责采集传感器数据并通过串口输出 JSON Lines。

配套上位机软件已经完成，详见：

[suanlayu666/CozySleep_SoftWare](https://github.com/suanlayu666/CozySleep_SoftWare)

上位机按行读取串口数据，然后解析 JSON，例如：

```json
{"mq135_adc":2644,"motion":0,"sound_db":42}
```

上位机负责数据接收、界面展示以及 AI 环境分析等功能。

## 当前状态与 TODO

- [x] CubeMX CMake 工程初始化
- [x] USART1 串口输出
- [x] USART3 蓝牙串口输出
- [x] MQ135 ADC 读取
- [x] 声音传感器 ADC 读取
- [x] HC-SR501 GPIO 读取
- [x] DHT11 稳定读取
- [ ] GY-302 / BH1750 光照传感器接入
- [x] 配套上位机软件，详见 [CozySleep_SoftWare](https://github.com/suanlayu666/CozySleep_SoftWare)
- [x] AI 环境分析与建议，上位机侧实现

## 备注

本仓库仍处于课程/原型开发阶段，部分传感器参数尚未标定。MQ135 和声音传感器当前主要输出原始值或估算值，实际 ppm、dB 等工程量需要结合模块型号、供电、电路和实验环境进一步校准。
