/**
 * @file    key.c
 * @brief   KEY 按键驱动模块实现
 * @author  YanZQ-gif
 * @date    2026-04-15
 * @version V1.0.0
 * 
 * 硬件连接：
 * - KEY1: PA0 (高电平有效，按下进入 Bootloader)
 */

/* Includes ------------------------------------------------------------------*/
#include "key.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define KEY_DEBOUNCE_TIME       50      /* 消抖时间 50ms */
#define KEY_LONG_PRESS_TIME     2000    /* 长按时间 2000ms */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static KEY_State_TypeDef key_state[KEY_MAX] = {KEY_RELEASED};

/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

/**
 * @brief  KEY 初始化
 */
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 使能 GPIOA 时钟 */
    KEY1_GPIO_CLK_ENABLE();
    
    /* 配置 KEY1 (PA0) */
    GPIO_InitStruct.Pin = KEY1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;  /* 下拉，默认低电平 */
    HAL_GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStruct);
    
    /* 初始化按键状态 */
    for (uint8_t i = 0; i < KEY_MAX; i++) {
        key_state[i] = KEY_RELEASED;
    }
}

/**
 * @brief  读取按键状态（带消抖）
 * @param  key: 按键编号
 * @retval KEY_PRESSED: 按下，KEY_RELEASED: 释放
 */
KEY_State_TypeDef KEY_Read(KEY_TypeDef key)
{
    GPIO_PinState pin_state;
    
    switch (key) {
        case KEY1:
            pin_state = HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_PIN);
            break;
        default:
            return KEY_RELEASED;
    }
    
    /* 高电平有效 */
    if (pin_state == KEY_ACTIVE_LEVEL) {
        HAL_Delay(KEY_DEBOUNCE_TIME);  /* 消抖 */
        pin_state = HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_PIN);
        
        if (pin_state == KEY_ACTIVE_LEVEL) {
            key_state[key] = KEY_PRESSED;
            return KEY_PRESSED;
        }
    }
    
    key_state[key] = KEY_RELEASED;
    return KEY_RELEASED;
}

/**
 * @brief  检查按键是否按下
 * @param  key: 按键编号
 * @retval 1: 按下，0: 释放
 */
uint8_t KEY_IsPressed(KEY_TypeDef key)
{
    return (KEY_Read(key) == KEY_PRESSED) ? 1 : 0;
}

/**
 * @brief  等待按键按下
 * @param  key: 按键编号
 * @param  timeout: 超时时间（ms），0 表示无限等待
 * @retval 1: 按下，0: 超时
 */
uint8_t KEY_WaitPressed(KEY_TypeDef key, uint32_t timeout)
{
    uint32_t start = HAL_GetTick();
    
    while (1) {
        if (KEY_IsPressed(key)) {
            return 1;
        }
        
        if (timeout > 0) {
            if ((HAL_GetTick() - start) >= timeout) {
                return 0;  /* 超时 */
            }
        }
        
        HAL_Delay(10);
    }
}

/**
 * @brief  等待按键释放
 * @param  key: 按键编号
 * @param  timeout: 超时时间（ms），0 表示无限等待
 * @retval 1: 释放，0: 超时
 */
uint8_t KEY_WaitReleased(KEY_TypeDef key, uint32_t timeout)
{
    uint32_t start = HAL_GetTick();
    
    while (1) {
        if (!KEY_IsPressed(key)) {
            return 1;
        }
        
        if (timeout > 0) {
            if ((HAL_GetTick() - start) >= timeout) {
                return 0;  /* 超时 */
            }
        }
        
        HAL_Delay(10);
    }
}

/**
 * @brief  按键扫描（非阻塞）
 *         需要在主循环中周期性调用
 */
void KEY_Scan(void)
{
    static uint32_t key_timer[KEY_MAX] = {0};
    static uint8_t key_long_flag[KEY_MAX] = {0};
    
    for (uint8_t i = 0; i < KEY_MAX; i++) {
        if (KEY_IsPressed((KEY_TypeDef)i)) {
            if (key_state[i] == KEY_RELEASED) {
                /* 按键按下 */
                key_state[i] = KEY_PRESSED;
                key_timer[i] = HAL_GetTick();
                key_long_flag[i] = 0;
            }
            
            /* 检测长按 */
            if ((HAL_GetTick() - key_timer[i]) >= KEY_LONG_PRESS_TIME) {
                if (!key_long_flag[i]) {
                    key_long_flag[i] = 1;
                    /* 这里可以添加长按回调 */
                }
            }
        } else {
            key_state[i] = KEY_RELEASED;
            key_long_flag[i] = 0;
        }
    }
}
