/**
 * @file    printf.c
 * @brief   Printf 重定向模块实现
 * @author  YanZQ-gif
 * @date    2026-04-15
 * @version V1.0.0
 * 
 * 功能：重定向 printf 到 USART2
 */

/* Includes ------------------------------------------------------------------*/
#include "printf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

/**
 * @brief  Printf 初始化
 *         如果使用 USART2，确保已在 CubeMX 中配置
 */
void Printf_Init(void)
{
    /* USART2 已在 CubeMX 中配置，无需额外初始化 */
}

/**
 * @brief  重定向 printf 到 USART2
 *         兼容 GCC 和其他编译器
 * 
 * @note   使用 huart2 进行打印
 */
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
