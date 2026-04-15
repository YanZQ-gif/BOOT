/**
 * @file    flash.h
 * @brief   STM32F767 内部 Flash 操作模块
 * @author  YanZQ-gif
 * @date    2026-04-15
 * @version V1.0.0
 * 
 * 功能：
 * - Flash 扇区擦除
 * - Flash 字节/半字/字写入
 * - Flash 数据读取
 * - Bootloader 相关函数实现
 */

#ifndef __FLASH_H
#define __FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
    FLASH_OK = 0,
    FLASH_ERROR = 1,
    FLASH_BUSY = 2,
    FLASH_TIMEOUT = 3
} FLASH_Status_TypeDef;

/* Exported constants --------------------------------------------------------*/
/* Flash 地址定义 */
#define FLASH_APP_START_ADDR        0x08010000  /* APP 起始地址 */
#define FLASH_PARAM_ADDR            0x0800FC00  /* 参数区地址 */

/* 魔术字定义 */
#define BOOT_FLAG_APP               0x5A5A5A5A
#define BOOT_FLAG_IAP               0x12345678

/* Flash 操作超时 */
#define FLASH_TIMEOUT               1000

/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/* Flash 基础操作 */
FLASH_Status_TypeDef FLASH_EraseSector(uint32_t sector, uint32_t nbSectors);
FLASH_Status_TypeDef FLASH_WriteByte(uint32_t addr, uint8_t data);
FLASH_Status_TypeDef FLASH_WriteHalfWord(uint32_t addr, uint16_t data);
FLASH_Status_TypeDef FLASH_WriteWord(uint32_t addr, uint32_t data);
FLASH_Status_TypeDef FLASH_WriteBuffer(uint32_t addr, uint8_t *buffer, uint32_t size);
uint8_t FLASH_ReadByte(uint32_t addr);
uint16_t FLASH_ReadHalfWord(uint32_t addr);
uint32_t FLASH_ReadWord(uint32_t addr);
uint8_t FLASH_IsEmpty(uint32_t addr, uint32_t size);

/* Bootloader 相关函数 */
void BOOT_EnterBootloaderMode(void);
uint8_t BOOT_CheckEnterBootloader(void);
void BOOT_JumpToApp(void);
uint8_t BOOT_VerifyAppFirmware(void);
void BOOT_SetFlag(uint32_t flag);
uint32_t BOOT_GetFlag(void);
void BOOT_ClearFlag(void);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_H */
