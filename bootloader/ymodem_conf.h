/**
 * @file   ymodem_conf.h
 * @brief  YMODEM 配置文件 - STM32F767 Bootloader 专用
 * @author MaXQ
 * @date   2026-04-15
 */

#ifndef YMODEM_CONF_H_
#define YMODEM_CONF_H_

/* ============================================================================
 * 1. 选择设备系列 - STM32F7
 * ============================================================================ */
#define DEVICE_FAMILY_STM32F7

/* ============================================================================
 * 2. Flash 配置 - 基于实际工程分区
 * ============================================================================ */

/**
 * @brief APP 区起始地址（Bootloader 占用前 64KB）
 * 
 * STM32F767 Flash 布局：
 * - Bootloader: 0x08000000 - 0x0800FFFF (64KB, Sector 0-1)
 * - APP:        0x08010000 - 0x0810FFFF (960KB, Sector 2-11)
 */
#define YMODEM_FLASH_START              0x08010000

/**
 * @brief APP 区最大大小
 * 
 * 总 Flash 1024KB - Bootloader 64KB = 960KB
 * 建议保留最后 4KB 用于参数存储，实际可用 956KB
 */
#define YMODEM_FLASH_SIZE               (956 * 1024)  // 956KB

/**
 * @brief APP 区起始扇区号
 * 
 * STM32F767 扇区分布：
 * - Sector 0:    32KB (0x08000000)
 * - Sector 1:    32KB (0x08008000)
 * - Sector 2:    32KB (0x08010000) ← APP 开始
 * - Sector 3:    32KB (0x08018000)
 * - Sector 4-11: 128KB each
 */
#define YMODEM_FLASH_FIRST_SECTOR_NUM   (2)

/**
 * @brief 用于 APP 的扇区数量
 * 
 * Sector 2-10 共 9 个扇区 = 32KB + 128KB*8 = 1056KB
 * 实际可用 956KB（受 YMODEM_FLASH_SIZE 限制）
 */
#define YMODEM_FLASH_NUM_OF_SECTORS     (9)

/* ============================================================================
 * 3. 可选配置
 * ============================================================================ */

/**
 * @brief 是否验证编程结果
 * 
 * 启用后会读取 Flash 与接收数据对比，增加可靠性但降低速度
 * 建议启用，确保升级可靠性
 */
#define YMODEM_VALIDATE_PROGRAMMING

/**
 * @brief UART 句柄定义
 * 
 * 根据实际工程使用 USART2 (PA2/PA3)
 * 需要在 main.h 中声明：extern UART_HandleTypeDef huart2;
 */
// #define YMODEM_UART_HANDLE    huart2

/* ============================================================================
 * 4. 中断和 RTOS 配置
 * ============================================================================ */

/**
 * @brief 如果使用 RTOS，取消注释并包含头文件
 * 
 * 当前工程未使用 RTOS，保持注释
 */
// #include "cmsis_os.h"
// #define YMODEM_USE_RTOS

/**
 * @brief 串口接收缓冲区大小
 */
#define YMODEM_RX_BUFFER_SIZE   (256)

/* ============================================================================
 * 5. 参数区定义（用于存储 Bootloader 标志）
 * ============================================================================ */

/**
 * @brief 参数区地址（Bootloader Flash 最后 4KB）
 */
#define BOOT_PARAM_ADDR         (0x08000000 + 64 * 1024 - 0x1000)  // 0x0800FC00

/**
 * @brief 启动标志地址
 */
#define BOOT_FLAG_ADDR          BOOT_PARAM_ADDR

/**
 * @brief APP CRC 地址
 */
#define APP_CRC_ADDR            (BOOT_PARAM_ADDR + 4)

/**
 * @brief APP 大小地址
 */
#define APP_SIZE_ADDR           (BOOT_PARAM_ADDR + 8)

/**
 * @brief 魔术字定义
 */
#define BOOT_FLAG_APP           0x5A5A5A5A    // 正常启动
#define BOOT_FLAG_IAP           0x12345678    // 进入 IAP

#endif /* YMODEM_CONF_H_ */
