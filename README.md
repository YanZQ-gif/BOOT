# STM32F767 Bootloader - 模块化版本

## 📁 项目结构

```
BOOT/
├── Core/                    # CubeMX 生成的核心代码
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       └── main.c          # 主程序（包含 Bootloader 逻辑）
├── bootloader/              # Bootloader 相关文件
│   ├── ymodem.c            # YModem 协议实现
│   ├── ymodem.h
│   ├── ymodem_conf.h       # YModem 配置
│   ├── STM32F767XX_BOOTLOADER.ld
│   └── README.md
├── APP/                     # 应用模块（模块化结构）
│   ├── LED/                # LED 模块
│   │   ├── led.c
│   │   └── led.h
│   └── Printf/             # Printf 重定向模块
│       ├── printf.c
│       └── printf.h
├── CMakeLists.txt          # CMake 构建配置
└── README.md
```

---

## 🎯 模块化设计原则

### 1. 文件夹结构规范

```
APP/
├── [模块名]/
│   ├── [模块名].c
│   └── [模块名].h
```

**示例：**
- `APP/LED/led.c` + `led.h`
- `APP/Printf/printf.c` + `printf.h`
- `APP/KEY/key.c` + `key.h`（未来扩展）
- `APP/UART/uart.c` + `uart.h`（未来扩展）

### 2. 代码风格规范

**头文件保护：**
```c
#ifndef __MODULE_NAME_H
#define __MODULE_NAME_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
/* Exported types */
/* Exported constants */
/* Exported functions */

#ifdef __cplusplus
}
#endif

#endif
```

**Printf 重定向（标准方式）：**
```c
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
```

---

## 🔧 硬件配置

### 基于 CubeMX 配置

**UART:**
- USART2 (PA2/PA3) - 用于调试打印和 YModem 通信
- 波特率：115200

**LED:**
- LED1: PB3
- LED2: PB4

**BOOT 按键:**
- PA0 (低电平有效)

---

## 📋 模块说明

### LED 模块 (`APP/LED/`)

**功能：**
- LED 初始化
- 开关控制
- 闪烁控制
- 状态管理

**API：**
```c
void LED_Init(void);
void LED_On(LED_TypeDef led);
void LED_Off(LED_TypeDef led);
void LED_Toggle(LED_TypeDef led);
void LED_Set(LED_TypeDef led, LED_State_TypeDef state);
void LED_Blink(LED_TypeDef led, uint32_t times, uint32_t interval);
void LED_AllOn(void);
void LED_AllOff(void);
```

**使用示例：**
```c
#include "led.h"

LED_Init();
LED_On(LED1);
LED_Blink(LED2, 3, 500);
```

### Printf 模块 (`APP/Printf/`)

**功能：**
- 重定向 printf 到 USART2
- 兼容 GCC 和其他编译器

**API：**
```c
void Printf_Init(void);
// printf 函数可直接使用
```

**使用示例：**
```c
#include "printf.h"

Printf_Init();
printf("Hello, World!\r\n");
```

---

## 🚀 编译步骤

### 1. 使用 CMake

```cmd
cd D:\dwPrj-Clion\BOOT

REM 创建 build 目录
mkdir build
cd build

REM 配置
cmake .. -DCMAKE_BUILD_TYPE=Debug

REM 编译 Bootloader
cmake --build . --target bootloader.elf

REM 生成 BIN 文件
arm-none-eabi-objcopy -O binary bootloader.elf bootloader.bin
```

### 2. 使用 STM32CubeIDE

1. 导入项目
2. Project → Build Target → Create
   - Target Name: `bootloader`
   - Build Command: `cmake --build . --target bootloader.elf`
3. Project → Build Project

---

## 🧪 测试验证

### LED 测试

```c
#include "led.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    
    LED_Init();
    
    while (1) {
        LED_Blink(LED1, 3, 200);
        LED_Blink(LED2, 3, 200);
        HAL_Delay(1000);
    }
}
```

### Printf 测试

```c
#include "printf.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    
    Printf_Init();
    
    printf("STM32F767 Bootloader\r\n");
    printf("UART: USART2 (PA2/PA3)\r\n");
    printf("Baudrate: 115200\r\n");
    
    while (1) {
        printf("LED Test...\r\n");
        HAL_Delay(1000);
    }
}
```

---

## 📊 Flash 分区

```
┌────────────────────────────────────────────┐
│ 0x08000000 │ Bootloader (64KB)             │
│            │ Sector 0-1                    │
├────────────────────────────────────────────┤
│ 0x08010000 │ APP 应用区 (956KB)            │
│            │ Sector 2-11                   │
├────────────────────────────────────────────┤
│ 0x0800FC00 │ 参数区 (4KB)                  │
│            │ Bootloader 最后 4KB           │
└────────────────────────────────────────────┘
```

---

## 🔌 硬件连接

**USB 转串口模块：**
```
USB-TTL    →    STM32F767
TX         →    PA3 (USART2_RX)
RX         →    PA2 (USART2_TX)
GND        →    GND
3.3V       →    3.3V (可选)
```

**LED 连接：**
```
LED1 正极  →    PB3 (通过限流电阻)
LED1 负极  →    GND

LED2 正极  →    PB4 (通过限流电阻)
LED2 负极  →    GND
```

**BOOT 按键：**
```
按键一端   →    PA0
按键另一端 →    GND
```

---

## 📝 代码规范

### 1. 新增模块

**步骤：**
1. 在 `APP/` 下创建新文件夹
2. 创建 `.c` 和 `.h` 文件
3. 在 `CMakeLists.txt` 中添加源文件和头文件路径
4. 在 `main.c` 中包含头文件并调用初始化函数

**示例（新增 KEY 模块）：**
```
APP/KEY/
├── key.c
└── key.h
```

**CMakeLists.txt 修改：**
```cmake
add_executable(bootloader.elf
    ...
    APP/KEY/key.c
)

target_include_directories(bootloader.elf PRIVATE
    ...
    APP/KEY
)
```

**main.c 修改：**
```c
#include "key.h"

int main(void)
{
    ...
    KEY_Init();
    ...
}
```

### 2. Printf 使用

**始终使用 huart2：**
```c
// ✅ 正确
HAL_UART_Transmit(&huart2, ...);

// ❌ 错误
HAL_UART_Transmit(&huart1, ...);
```

**标准重定向格式：**
```c
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
```

### 3. 硬件抽象

**所有硬件相关代码必须在模块中实现：**
- ✅ `APP/LED/led.c` - LED 驱动
- ✅ `APP/KEY/key.c` - 按键驱动
- ❌ 不在 `main.c` 中直接操作 GPIO

---

## 🎯 后续扩展

**计划添加的模块：**
- [ ] `APP/KEY/` - 按键模块
- [ ] `APP/UART/` - UART 管理模块
- [ ] `APP/FLASH/` - Flash 操作模块
- [ ] `APP/I2C/` - I2C 设备模块
- [ ] `APP/SPI/` - SPI 设备模块

---

## 📚 参考资源

- [STM32F767 参考手册](https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [YModem 协议规范](http://pauillac.inria.fr/~doligez/zmodem/ymodem.txt)

---

**作者**: YanZQ-gif  
**日期**: 2026-04-15  
**版本**: V1.0.0  
**许可证**: MIT
