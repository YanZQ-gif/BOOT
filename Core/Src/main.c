/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body - Bootloader with YModem
  * @author         : YanZQ-gif
  * @date           : 2026-04-16
  * @version        : V1.1.0 - Bootloader with UPDATE command
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
#include "key.h"
#include "printf.h"
#include "flash.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* 升级命令 */
#define UPDATE_CMD        "UPDATE"
#define UPDATE_CMD_LEN    6
#define BOOT_WINDOW_TIME  1000    /* 1 秒窗口期 */

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
static uint8_t update_cmd_buffer[UPDATE_CMD_LEN];
static uint8_t update_cmd_index = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

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
  
  /* 初始化 KEY 模块 */
  KEY_Init();
  
  /* 上电指示 */
  LED_Blink(LED1, 3, 100);
  
  printf("\r\n");
  printf("========================================\r\n");
  printf("  STM32F767 Bootloader\r\n");
  printf("  Version: V1.1.0\r\n");
  printf("========================================\r\n");
  printf("[BOOT] 系统初始化完成\r\n");
  printf("[BOOT] 等待升级指令窗口期：%dms\r\n", BOOT_WINDOW_TIME);
  printf("[BOOT] 请输入 'UPDATE' 命令进入升级模式\r\n");
  printf("========================================\r\n\r\n");
  
  /* 等待升级指令窗口期 */
  uint32_t window_start = HAL_GetTick();
  uint8_t cmd_received = 0;
  
  while ((HAL_GetTick() - window_start) < BOOT_WINDOW_TIME) {
      if (HAL_UART_Receive(&huart2, &rx_byte, 1, 10) == HAL_OK) {
          /* 接收字符并匹配命令 */
          update_cmd_buffer[update_cmd_index++] = rx_byte;
          
          /* 检查是否匹配 UPDATE 命令 */
          if (update_cmd_index >= UPDATE_CMD_LEN) {
              if (memcmp(update_cmd_buffer, UPDATE_CMD, UPDATE_CMD_LEN) == 0) {
                  cmd_received = 1;
                  printf("[BOOT] 收到升级命令：UPDATE\r\n");
                  break;
              }
              
              /* 重置索引，继续匹配 */
              update_cmd_index = 0;
          }
      }
  }
  
  /* 检查是否收到升级命令 */
  if (cmd_received) {
      /* 进入 Bootloader 模式 */
      bIsBootloader = 1;
      printf("[BOOT] 收到升级命令：UPDATE\r\n");
      printf("[BOOT] 持续发送 'C' 等待上位机连接...\r\n\r\n");
      LED_Blink(LED1, 5, 200);
  } else {
      /* 窗口期结束，未收到升级命令 */
      printf("[BOOT] 窗口期结束，未收到升级命令\r\n");
      
      /* 检查 APP 是否有效 */
      if (!BOOT_VerifyAppFirmware()) {
          printf("[BOOT] APP 验证通过，准备跳转...\r\n");
          LED_Blink(LED2, 2, 500);
          HAL_Delay(500);
          BOOT_JumpToApp();
      } else {
          printf("[BOOT] APP 无效，进入升级模式\r\n");
          bIsBootloader = 1;
          LED_Blink(LED1, 5, 200);
      }
  }
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  if (bIsBootloader) {
      /* 进入 YModem 升级模式 */
      
      /* 初始化 YModem */
      YMODEM_Init();
      
      /* 持续发送 'C' 等待上位机响应 */
      uint32_t wait_start = HAL_GetTick();
      uint8_t pc_connected = 0;
      
      printf("[IAP] 持续发送 'C' 等待上位机连接...\r\n");
      
      /* 等待上位机响应，最多等待 10 秒 */
      while ((HAL_GetTick() - wait_start) < 10000) {
          /* 持续发送 'C' */
          HAL_UART_Transmit(&huart2, (uint8_t*)"C", 1, 100);
          HAL_Delay(100);  /* 每 100ms 发送一次 */
          
          /* 检查是否收到上位机响应 */
          if (HAL_UART_Receive(&huart2, &rx_byte, 1, 10) == HAL_OK) {
              /* 收到数据，说明上位机已连接 */
              pc_connected = 1;
              printf("[IAP] 上位机已连接，开始升级...\r\n\r\n");
              break;
          }
      }
      
      if (!pc_connected) {
          printf("[IAP] 等待超时，未检测到上位机\r\n");
          printf("[IAP] 系统将在 2 秒后重启...\r\n");
          HAL_Delay(2000);
          NVIC_SystemReset();
      }
      
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
                      BOOT_SetFlag(BOOT_FLAG_APP);
                      
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
  
  while (1)
  {
      /* 正常情况下不会到这里 */
      LED_Blink(LED1, 1, 1000);  // 错误指示
  }
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
}

/* ============================================================================
 * 系统配置函数
 * ============================================================================ */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
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
