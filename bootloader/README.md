# STM32F767 Bootloader 移植指南

## 📁 项目信息

**原始工程**: https://github.com/YanZQ-gif/BOOT  
**移植日期**: 2026-04-15  
**移植作者**: MaXQ

---

## 📊 工程分析结果

### 你的工程配置：

| 项目 | 配置 |
|------|------|
| **MCU** | STM32F767IGTx |
| **Flash** | 1024KB |
| **RAM** | 512KB |
| **时钟** | 216MHz (HSE 25MHz + PLL) |
| **UART** | USART2 (PA2/PA3) |
| **工具链** | GCC + CMake |
| **CubeMX** | 6.16.1 |

### Bootloader 分区方案：

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

## 📦 已创建的文件

```
BOOT/
└── bootloader/
    ├── README.md                       # 本文件
    ├── STM32F767XX_BOOTLOADER.ld       # 链接脚本（修改版）
    ├── ymodem_conf.h                   # YModem 配置
    ├── main.c                          # Bootloader 主程序
    ├── ymodem.c                        # YModem 协议（需要复制）
    └── ymodem.h                        # YModem 头文件（需要复制）
```

---

## 🔧 集成步骤

### 步骤 1：复制 YModem 库文件

从 `stm32_iap_tool` 复制以下文件到 `bootloader` 目录：

```bash
# 复制 ymodem.c 和 ymodem.h
cp /home/admin/openclaw/workspace/stm32_iap_tool/ymodem.c bootloader/
cp /home/admin/openclaw/workspace/stm32_iap_tool/ymodem.h bootloader/
```

或者使用你上传的文件：
- `ymodem.c` → `bootloader/ymodem.c`
- `ymodem.h` → `bootloader/ymodem.h`

---

### 步骤 2：修改 CMakeLists.txt

在 `CMakeLists.txt` 中添加 Bootloader 配置：

```cmake
# 添加 Bootloader 可执行文件
add_executable(bootloader.elf
    bootloader/main.c
    bootloader/ymodem.c
    Core/Src/stm32f7xx_hal_msp.c
    Core/Src/system_stm32f7xx.c
    startup_stm32f767xx.s
)

# 设置 Bootloader 链接脚本
target_link_options(bootloader.elf PRIVATE
    -T${CMAKE_SOURCE_DIR}/bootloader/STM32F767XX_BOOTLOADER.ld
)

# 包含头文件
target_include_directories(bootloader.elf PRIVATE
    Core/Inc
    bootloader
    Drivers/STM32F7xx_HAL_Driver/Inc
)

# 编译定义
target_compile_definitions(bootloader.elf PRIVATE
    USE_HAL_DRIVER
    STM32F767xx
)
```

---

### 步骤 3：修改 main.h

在 `Core/Inc/main.h` 中添加 YModem 函数声明：

```c
/* USER CODE BEGIN Includes */
#include "ymodem.h"
/* USER CODE END Includes */
```

---

### 步骤 4：编译 Bootloader

**方法 A：使用 CMake**

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make bootloader.elf
```

**方法 B：使用 STM32CubeIDE**

1. 导入项目
2. 右键项目 → Build Target → Create
3. 输入 `bootloader`
4. 编译

**方法 C：使用 Make**

```bash
make -f Makefile.boot
```

---

### 步骤 5：烧录 Bootloader

**使用 ST-Link Utility：**

1. 连接 ST-Link
2. 选择文件：`bootloader.elf` 或 `bootloader.bin`
3. 起始地址：`0x08000000`
4. 点击"Program"

**使用 OpenOCD：**

```bash
openocd -f interface/stlink.cfg -f target/stm32f7x.cfg \
  -c "program bootloader.elf 0x08000000 verify reset exit"
```

**使用 J-Flash：**

1. 打开 J-Flash
2. 选择 STM32F767IGT
3. 打开文件：`bootloader.elf`
4. 下载到 `0x08000000`

---

## 🧪 测试流程

### 1. 进入 Bootloader 模式

**方法 A：按键进入**
1. 按住 PA0 按键（需要连接到 BOOT 按键）
2. 按复位键或上电
3. LED 闪烁 5 次表示进入成功

**方法 B：看门狗复位**
1. 触发看门狗复位
2. 自动进入 Bootloader

**方法 C：APP 请求**
```c
// 在 APP 中调用
*(volatile uint32_t *)0x20000000 = 0x12345678;
NVIC_SystemReset();
```

### 2. 使用上位机升级

**Windows：**
```cmd
cd stm32_iap_tool
python ymodem_upgrade.py
```

**配置：**
- 串口：选择你的 USB 转串口（如 COM3）
- 波特率：115200
- 文件：选择你的 APP 固件.bin
- 点击"开始升级"

**Linux：**
```bash
python3 ymodem_upgrade.py
```

### 3. 观察日志

**串口输出（使用串口助手）：**
```
[BOOT] 进入 Bootloader 模式
[BOOT] 波特率：115200
[BOOT] 等待固件升级...

[IAP] 已发送启动信号，等待上位机...
[IAP] ✓ 固件升级完成！
[IAP] 系统将在 2 秒后重启...
```

### 4. 验证升级

升级完成后：
- 设备自动重启
- LED 闪烁 2 次
- 跳转到 APP 运行
- 串口输出 APP 信息

---

## ⚠️ 重要注意事项

### 1. APP 项目修改

**在 APP 项目中必须修改：**

**链接脚本：**
```ld
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08010000, LENGTH = 956K
    RAM (xrw)  : ORIGIN = 0x20000000, LENGTH = 512K
}
```

**向量表偏移：**
```c
// 在 APP 的 main.c 开头
SCB->VTOR = 0x08010000;
```

### 2. 硬件连接

**USB 转串口模块：**
```
USB-TTL    →    STM32F767
TX         →    PA3 (USART2_RX)
RX         →    PA2 (USART2_TX)
GND        →    GND
3.3V       →    3.3V (可选)
```

**BOOT 按键（可选）：**
```
按键一端   →    PA0
按键另一端 →    GND
```

**LED（调试用，可选）：**
```
LED 正极   →    PB1 (通过限流电阻)
LED 负极   →    GND
```

### 3. 常见问题

**Q1: 升级失败，一直显示等待**

**原因：**
- 串口接线错误
- 波特率不匹配
- 未进入 Bootloader

**解决：**
1. 检查接线：TX→RX, RX→TX
2. 确认波特率 115200
3. 观察 LED 闪烁确认进入 Bootloader

**Q2: 升级完成但无法启动 APP**

**原因：**
- APP 向量表未偏移
- APP 编译地址错误

**解决：**
1. 检查 APP 链接脚本：`ORIGIN = 0x08010000`
2. 在 APP 的 main.c 添加：`SCB->VTOR = 0x08010000;`

**Q3: 文件大小超过限制**

**错误：** "固件文件过大"

**解决：**
- 确保 APP < 956KB
- 优化代码
- 启用编译器优化：`-O2` 或 `-Os`

---

## 📊 性能指标

| 指标 | 数值 |
|------|------|
| Bootloader 大小 | ~20KB |
| 可用 APP 空间 | 956KB |
| 升级速度 | ~10KB/s (115200 波特率) |
| CRC 校验 | CRC16-CCITT |
| 重传机制 | 支持，最多 10 次 |
| 进入方式 | 按键/看门狗/魔术字 |

---

## 🔐 安全建议

### 1. 启用 Flash 保护

```c
// 在升级完成后
HAL_FLASH_OB_Unlock();
FLASH_OB_RDPConfig(OB_RDP_LEVEL_1);  // 开启读保护
HAL_FLASH_OB_Lock();
```

### 2. 添加固件签名验证

在 `VerifyAppFirmware()` 中添加签名验证逻辑。

### 3. 添加版本回滚保护

在参数区记录最低版本号：
```c
#define MIN_VERSION_ADDR  (BOOT_PARAM_ADDR + 12)
```

---

## 📚 参考资源

- [STM32F767 参考手册](https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [YModem 协议规范](http://pauillac.inria.fr/~doligez/zmodem/ymodem.txt)
- [STM32F7 Flash 编程手册](https://www.st.com/resource/en/programming_manual/pm0253-stm32f7-series-flash-memory-programming-manual-stmicroelectronics.pdf)

---

## 📝 更新日志

| 版本 | 日期 | 更新内容 |
|------|------|----------|
| V1.0.0 | 2026-04-15 | 初始版本，基于 YanZQ-gif/BOOT 工程移植 |

---

**作者**: MaXQ  
**日期**: 2026-04-15  
**许可证**: MIT
