/**
 * @file    led.c
 * @brief   LED 驱动模块实现
 * @author  YanZQ-gif
 * @date    2026-04-15
 * @version V1.0.0
 * 
 * 硬件连接：
 * - LED1: PB3
 * - LED2: PB4
 */

/* Includes ------------------------------------------------------------------*/
#include "led.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

/**
 * @brief  LED 初始化
 */
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 使能 GPIOB 时钟 */
    LED1_GPIO_CLK_ENABLE();
    LED2_GPIO_CLK_ENABLE();
    
    /* 配置 LED1 (PB3) */
    GPIO_InitStruct.Pin = LED1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStruct);
    
    /* 配置 LED2 (PB4) */
    GPIO_InitStruct.Pin = LED2_PIN;
    HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);
    
    /* 初始状态：关闭所有 LED */
    LED_AllOff();
}

/**
 * @brief  点亮指定 LED
 * @param  led: LED 编号 (LED1 或 LED2)
 */
void LED_On(LED_TypeDef led)
{
    switch (led) {
        case LED1:
            HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_SET);
            break;
        case LED2:
            HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
            break;
        default:
            break;
    }
}

/**
 * @brief  熄灭指定 LED
 * @param  led: LED 编号 (LED1 或 LED2)
 */
void LED_Off(LED_TypeDef led)
{
    switch (led) {
        case LED1:
            HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET);
            break;
        case LED2:
            HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

/**
 * @brief  翻转指定 LED 状态
 * @param  led: LED 编号 (LED1 或 LED2)
 */
void LED_Toggle(LED_TypeDef led)
{
    switch (led) {
        case LED1:
            HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
            break;
        case LED2:
            HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
            break;
        default:
            break;
    }
}

/**
 * @brief  设置 LED 状态
 * @param  led: LED 编号 (LED1 或 LED2)
 * @param  state: 状态 (LED_ON, LED_OFF, LED_TOGGLE)
 */
void LED_Set(LED_TypeDef led, LED_State_TypeDef state)
{
    switch (state) {
        case LED_ON:
            LED_On(led);
            break;
        case LED_OFF:
            LED_Off(led);
            break;
        case LED_TOGGLE:
            LED_Toggle(led);
            break;
        default:
            break;
    }
}

/**
 * @brief  点亮所有 LED
 */
void LED_AllOn(void)
{
    LED_On(LED1);
    LED_On(LED2);
}

/**
 * @brief  熄灭所有 LED
 */
void LED_AllOff(void)
{
    LED_Off(LED1);
    LED_Off(LED2);
}

/**
 * @brief  翻转所有 LED 状态
 */
void LED_AllToggle(void)
{
    LED_Toggle(LED1);
    LED_Toggle(LED2);
}

/**
 * @brief  LED 闪烁
 * @param  led: LED 编号 (LED1 或 LED2)
 * @param  times: 闪烁次数
 * @param  interval: 间隔时间（毫秒）
 */
void LED_Blink(LED_TypeDef led, uint32_t times, uint32_t interval)
{
    for (uint32_t i = 0; i < times; i++) {
        LED_Toggle(led);
        HAL_Delay(interval);
        LED_Toggle(led);
        HAL_Delay(interval);
    }
}
