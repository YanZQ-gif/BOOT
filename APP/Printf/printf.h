/**
 * @file    printf.h
 * @brief   Printf 重定向模块头文件
 * @author  YanZQ-gif
 * @date    2026-04-15
 * @version V1.0.0
 * 
 * 功能：重定向 printf 到 USART2
 */

#ifndef __PRINTF_H
#define __PRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
void Printf_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __PRINTF_H */
