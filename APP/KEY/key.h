/**
 * @file    key.h
 * @brief   KEY 按键驱动模块头文件
 * @author  YanZQ-gif
 * @date    2026-04-15
 * @version V1.0.0
 * 
 * 硬件连接：
 * - KEY1: PA0 (高电平有效，按下进入 Bootloader)
 */

#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
    KEY1 = 0,
    KEY_MAX
} KEY_TypeDef;

typedef enum {
    KEY_RELEASED = 0,
    KEY_PRESSED = 1
} KEY_State_TypeDef;

/* Exported constants --------------------------------------------------------*/
/* KEY 引脚定义 */
#define KEY1_PIN                GPIO_PIN_0
#define KEY1_GPIO_PORT          GPIOA
#define KEY1_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()

/* 按键有效电平：高电平有效 */
#define KEY_ACTIVE_LEVEL        GPIO_PIN_SET

/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
void KEY_Init(void);
KEY_State_TypeDef KEY_Read(KEY_TypeDef key);
uint8_t KEY_IsPressed(KEY_TypeDef key);
uint8_t KEY_WaitPressed(KEY_TypeDef key, uint32_t timeout);
uint8_t KEY_WaitReleased(KEY_TypeDef key, uint32_t timeout);
void KEY_Scan(void);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_H */
