/**
 * @file    led.h
 * @brief   LED 驱动模块头文件
 * @author  YanZQ-gif
 * @date    2026-04-15
 * @version V1.0.0
 * 
 * 硬件连接：
 * - LED1: PB3
 * - LED2: PB4
 */

#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {
    LED1 = 0,
    LED2 = 1,
    LED_MAX
} LED_TypeDef;

typedef enum {
    LED_OFF = 0,
    LED_ON = 1,
    LED_TOGGLE = 2
} LED_State_TypeDef;

/* Exported constants --------------------------------------------------------*/
/* LED 引脚定义 */
#define LED1_PIN                GPIO_PIN_3
#define LED1_GPIO_PORT          GPIOB
#define LED1_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()

#define LED2_PIN                GPIO_PIN_4
#define LED2_GPIO_PORT          GPIOB
#define LED2_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()

/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
void LED_Init(void);
void LED_On(LED_TypeDef led);
void LED_Off(LED_TypeDef led);
void LED_Toggle(LED_TypeDef led);
void LED_Set(LED_TypeDef led, LED_State_TypeDef state);
void LED_Blink(LED_TypeDef led, uint32_t times, uint32_t interval);
void LED_AllOn(void);
void LED_AllOff(void);
void LED_AllToggle(void);

#ifdef __cplusplus
}
#endif

#endif /* __LED_H */
