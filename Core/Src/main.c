/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body - Bootloader with YModem
  * @author         : YanZQ-gif
  * @date           : 2026-04-15
  * @version        : V1.0.0 - Bootloader
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ymodem.h"
#include "led.h"
#include "printf.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Bootloader 配置 */
#define BOOT_PIN                GPIO_PIN_0
#define BOOT_GPIO_PORT          GPIOA
#define BOOT_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()

/* 参数区地址 */
#define BOOT_PARAM_ADDR         0x0800FC00
#define BOOT_FLAG_ADDR          BOOT_PARAM_ADDR

/* 魔术字 */
#define BOOT_FLAG_APP           0x5A5A5A5A
#define BOOT_FLAG_IAP           0x12345678

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* 全局变量 */
static uint8_t rx_byte;
static uint8_t tx_buffer[16];
static uint8_t tx_len;
static YMODEM_T ym_ret;
static uint8_t bIsBootloader = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* Bootloader 相关函数声明 */
static void EnterBootloaderMode(void);
static void JumpToApp(void);
static uint8_t CheckEnterBootloader(void);
static uint8_t VerifyAppFirmware(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  
  /* USER CODE BEGIN 2 */
  
  /* 初始化 Printf 模块 */
  Printf_Init();
  
  /* 初始化 LED 模块 */
  LED_Init();
  
  /* 上电指示 */
  LED_Blink(LED1, 3, 100);
  
  /* 检查是否进入 Bootloader 模式 */
  bIsBootloader = CheckEnterBootloader();
  
  if (bIsBootloader) {
      printf("\r\n");
      printf("========================================\r\n");
      printf("  STM32F767 Bootloader\r\n");
      printf("========================================\r\n");
      printf("[BOOT] 进入 Bootloader 模式\r\n");
      printf("[BOOT] UART: USART2 (PA2/PA3)\r\n");
      printf("[BOOT] 波特率：115200\r\n");
      printf("[BOOT] 等待固件升级...\r\n");
      printf("========================================\r\n\r\n");
      LED_Blink(LED1, 5, 200);  // 5 次慢闪
  } else {
      /* 检查 APP 是否有效 */
      if (!VerifyAppFirmware()) {
          printf("[BOOT] APP 验证通过，准备跳转...\r\n");
          LED_Blink(LED2, 2, 500);  // 2 次快闪
          HAL_Delay(500);
          JumpToApp();
      } else {
          printf("[BOOT] APP 无效，进入 Bootloader\r\n");
          bIsBootloader = 1;
          LED_Blink(LED1, 5, 200);
      }
  }
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  if (bIsBootloader) {
      EnterBootloaderMode();
  }
  
  while (1)
  {
      /* 正常情况下不会到这里 */
      LED_Blink(LED1, 1, 1000);  // 错误指示
  }
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
}

/* ============================================================================
 * Bootloader 模式实现
 * ============================================================================ */

/**
 * @brief  进入 Bootloader 模式，等待固件升级
 */
static void EnterBootloaderMode(void)
{
    /* 初始化 YModem */
    YMODEM_Init();
    
    /* 发送 'C' 启动信号（CRC 模式） */
    HAL_UART_Transmit(&huart2, (uint8_t*)"C", 1, 0xFFFF);
    
    printf("[IAP] 已发送启动信号，等待上位机...\r\n");
    
    /* 主循环 - 接收 YModem 数据 */
    while (1) {
        /* 等待 UART 数据 */
        if (HAL_UART_Receive(&huart2, &rx_byte, 1, 100) == HAL_OK) {
            /* 处理接收到的字节 */
            ym_ret = YMODEM_ReceiveByte(rx_byte, tx_buffer, &tx_len);
            
            switch (ym_ret) {
                case YMODEM_OK:
                    /* 继续接收 */
                    break;
                    
                case YMODEM_TX_PENDING:
                    /* 需要发送响应 */
                    if (tx_len > 0) {
                        HAL_UART_Transmit(&huart2, tx_buffer, tx_len, 1000);
                    }
                    break;
                    
                case YMODEM_COMPLETE:
                    /* 升级完成 */
                    printf("\r\n");
                    printf("========================================\r\n");
                    printf("  [IAP] ✓ 固件升级完成！\r\n");
                    printf("  [IAP] 系统将在 2 秒后重启...\r\n");
                    printf("========================================\r\n");
                    
                    /* 设置启动标志 */
                    HAL_FLASH_Unlock();
                    uint32_t flag = BOOT_FLAG_APP;
                    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, BOOT_FLAG_ADDR, flag);
                    HAL_FLASH_Lock();
                    
                    /* 延时后重启 */
                    HAL_Delay(2000);
                    NVIC_SystemReset();
                    break;
                    
                case YMODEM_ABORTED:
                    /* 升级被取消 */
                    printf("\r\n[IAP] ⚠ 升级被取消\r\n");
                    break;
                    
                case YMODEM_WRITE_ERR:
                    /* Flash 写入错误 */
                    printf("\r\n[IAP] ❌ Flash 写入错误！\r\n");
                    break;
                    
                case YMODEM_SIZE_ERR:
                    /* 文件过大 */
                    printf("\r\n[IAP] ❌ 固件文件过大！\r\n");
                    break;
                    
                default:
                    break;
            }
        }
    }
}

/**
 * @brief  检查是否进入 Bootloader 模式
 * @retval 0: 进入 APP, 1: 保持 Bootloader
 */
static uint8_t CheckEnterBootloader(void)
{
    /* 方法 1：检查复位标志（看门狗复位后进入 IAP） */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) {
        __HAL_RCC_CLEAR_RESET_FLAGS();
        printf("[BOOT] 检测到看门狗复位\r\n");
        return 1;
    }
    
    /* 方法 2：检查按键（BOOT 按键） */
    BOOT_GPIO_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = BOOT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BOOT_GPIO_PORT, &GPIO_InitStruct);
    
    HAL_Delay(50);  // 等待稳定
    
    if (HAL_GPIO_ReadPin(BOOT_GPIO_PORT, BOOT_PIN) == GPIO_PIN_RESET) {
        HAL_Delay(50);  // 消抖
        if (HAL_GPIO_ReadPin(BOOT_GPIO_PORT, BOOT_PIN) == GPIO_PIN_RESET) {
            printf("[BOOT] 检测到 BOOT 按键按下\r\n");
            return 1;
        }
    }
    
    /* 方法 3：检查 RAM 魔术字（APP 程序可设置此标志请求升级） */
    if (*(volatile uint32_t *)0x20000000 == BOOT_FLAG_IAP) {
        *(volatile uint32_t *)0x20000000 = 0;  // 清除标志
        printf("[BOOT] 检测到 RAM 魔术字\r\n");
        return 1;
    }
    
    /* 方法 4：检查参数区标志 */
    uint32_t boot_flag = *(volatile uint32_t *)BOOT_FLAG_ADDR;
    if (boot_flag == BOOT_FLAG_IAP) {
        printf("[BOOT] 检测到参数区 IAP 标志\r\n");
        return 1;
    }
    
    return 0;  // 进入 APP
}

/**
 * @brief  验证 APP 固件
 * @retval 0: 有效，1: 无效
 */
static uint8_t VerifyAppFirmware(void)
{
    /* 检查 APP 区起始位置是否有有效代码 */
    uint32_t app_stack = *(volatile uint32_t *)0x08010000;
    uint32_t app_reset = *(volatile uint32_t *)(0x08010000 + 4);
    
    /* 简单的有效性检查 */
    if (app_stack == 0xFFFFFFFF || app_reset == 0xFFFFFFFF) {
        printf("[BOOT] APP 栈指针或复位向量为空\r\n");
        return 1;
    }
    
    /* 检查栈地址是否在 RAM 范围内（STM32F767 为 0x20000000-0x20080000） */
    if (app_stack < 0x20000000 || app_stack > 0x20080000) {
        printf("[BOOT] APP 栈指针地址无效：0x%08X\r\n", app_stack);
        return 1;
    }
    
    /* 检查复位向量地址是否在 Flash 范围内 */
    if (app_reset < 0x08010000 || app_reset > 0x0810FFFF) {
        printf("[BOOT] APP 复位向量地址无效：0x%08X\r\n", app_reset);
        return 1;
    }
    
    printf("[BOOT] APP 验证通过\r\n");
    return 0;  // 有效
}

/**
 * @brief  跳转到 APP 程序
 */
static void JumpToApp(void)
{
    /* 1. 禁用所有中断 */
    __disable_irq();
    
    /* 2. 禁用 SysTick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    
    /* 3. 清除所有中断挂起 */
    for (uint8_t i = 0; i < 8; i++) {
        NVIC->ICPR[i] = 0xFFFFFFFF;
        NVIC->ICER[i] = 0xFFFFFFFF;
    }
    
    /* 4. 设置 APP 的栈指针 */
    __set_MSP(*(volatile uint32_t *)0x08010000);
    
    /* 5. 获取 APP 复位向量 */
    void (*app_reset_handler)(void) = 
        (void (*)(void))(*(volatile uint32_t *)(0x08010000 + 4));
    
    /* 6. 设置向量表偏移（APP 需要） */
    SCB->VTOR = 0x08010000;
    
    /* 7. 跳转 */
    printf("[BOOT] 跳转地址：0x%08X\r\n", (uint32_t)app_reset_handler);
    app_reset_handler();
    
    /* 永远不会到这里 */
    while (1);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
      /* 错误时快速闪烁 */
      LED_Blink(LED1, 1, 50);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
