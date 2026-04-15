/**
 * @file    flash.c
 * @brief   STM32F767 内部 Flash 操作模块实现
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

/* Includes ------------------------------------------------------------------*/
#include "flash.h"
#include "printf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

/* ============================================================================
 * Flash 基础操作实现
 * ============================================================================ */

/**
 * @brief  擦除 Flash 扇区
 * @param  sector: 起始扇区号
 * @param  nbSectors: 扇区数量
 * @retval FLASH_OK: 成功，FLASH_ERROR: 失败
 */
FLASH_Status_TypeDef FLASH_EraseSector(uint32_t sector, uint32_t nbSectors)
{
    FLASH_EraseInitTypeDef erase_cfg;
    uint32_t sector_error;
    HAL_StatusTypeDef hal_status;
    
    /* 配置擦除参数 */
    erase_cfg.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_cfg.Sector = sector;
    erase_cfg.NbSectors = nbSectors;
    erase_cfg.VoltageRange = FLASH_VOLTAGE_RANGE_3;  /* 2.7V-3.6V */
    
    /* 解锁 Flash */
    HAL_FLASH_Unlock();
    
    /* 清除错误标志 */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                          FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR);
    
    /* 执行擦除 */
    hal_status = HAL_FLASHEx_Erase(&erase_cfg, &sector_error);
    
    /* 锁定 Flash */
    HAL_FLASH_Lock();
    
    if (hal_status == HAL_OK) {
        return FLASH_OK;
    } else {
        printf("[FLASH] 擦除失败，错误扇区：%lu\r\n", sector_error);
        return FLASH_ERROR;
    }
}

/**
 * @brief  写入一个字节
 * @param  addr: Flash 地址
 * @param  data: 数据
 * @retval FLASH_OK: 成功，FLASH_ERROR: 失败
 */
FLASH_Status_TypeDef FLASH_WriteByte(uint32_t addr, uint8_t data)
{
    HAL_StatusTypeDef hal_status;
    
    /* 解锁 Flash */
    HAL_FLASH_Unlock();
    
    /* 写入数据 */
    hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, data);
    
    /* 锁定 Flash */
    HAL_FLASH_Lock();
    
    if (hal_status == HAL_OK) {
        return FLASH_OK;
    } else {
        printf("[FLASH] 写入字节失败 @ 0x%08X\r\n", addr);
        return FLASH_ERROR;
    }
}

/**
 * @brief  写入半字（16 位）
 * @param  addr: Flash 地址
 * @param  data: 数据
 * @retval FLASH_OK: 成功，FLASH_ERROR: 失败
 */
FLASH_Status_TypeDef FLASH_WriteHalfWord(uint32_t addr, uint16_t data)
{
    HAL_StatusTypeDef hal_status;
    
    /* 解锁 Flash */
    HAL_FLASH_Unlock();
    
    /* 写入数据 */
    hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, data);
    
    /* 锁定 Flash */
    HAL_FLASH_Lock();
    
    if (hal_status == HAL_OK) {
        return FLASH_OK;
    } else {
        printf("[FLASH] 写入半字失败 @ 0x%08X\r\n", addr);
        return FLASH_ERROR;
    }
}

/**
 * @brief  写入字（32 位）
 * @param  addr: Flash 地址
 * @param  data: 数据
 * @retval FLASH_OK: 成功，FLASH_ERROR: 失败
 */
FLASH_Status_TypeDef FLASH_WriteWord(uint32_t addr, uint32_t data)
{
    HAL_StatusTypeDef hal_status;
    
    /* 解锁 Flash */
    HAL_FLASH_Unlock();
    
    /* 写入数据 */
    hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data);
    
    /* 锁定 Flash */
    HAL_FLASH_Lock();
    
    if (hal_status == HAL_OK) {
        return FLASH_OK;
    } else {
        printf("[FLASH] 写入字失败 @ 0x%08X\r\n", addr);
        return FLASH_ERROR;
    }
}

/**
 * @brief  写入缓冲区数据
 * @param  addr: Flash 地址
 * @param  buffer: 数据缓冲区
 * @param  size: 数据大小
 * @retval FLASH_OK: 成功，FLASH_ERROR: 失败
 */
FLASH_Status_TypeDef FLASH_WriteBuffer(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    FLASH_Status_TypeDef status;
    
    /* 按字对齐写入 */
    for (uint32_t i = 0; i < size; i += 4) {
        uint32_t data = buffer[i] | (buffer[i + 1] << 8) | 
                       (buffer[i + 2] << 16) | (buffer[i + 3] << 24);
        
        status = FLASH_WriteWord(addr + i, data);
        if (status != FLASH_OK) {
            return status;
        }
    }
    
    return FLASH_OK;
}

/**
 * @brief  读取一个字节
 * @param  addr: Flash 地址
 * @retval 读取的数据
 */
uint8_t FLASH_ReadByte(uint32_t addr)
{
    return *(volatile uint8_t *)addr;
}

/**
 * @brief  读取半字（16 位）
 * @param  addr: Flash 地址
 * @retval 读取的数据
 */
uint16_t FLASH_ReadHalfWord(uint32_t addr)
{
    return *(volatile uint16_t *)addr;
}

/**
 * @brief  读取字（32 位）
 * @param  addr: Flash 地址
 * @retval 读取的数据
 */
uint32_t FLASH_ReadWord(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

/**
 * @brief  检查 Flash 区域是否为空
 * @param  addr: 起始地址
 * @param  size: 检查大小
 * @retval 1: 空（全 0xFF），0: 非空
 */
uint8_t FLASH_IsEmpty(uint32_t addr, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        if (FLASH_ReadByte(addr + i) != 0xFF) {
            return 0;  /* 非空 */
        }
    }
    return 1;  /* 空 */
}

/* ============================================================================
 * Bootloader 相关函数实现
 * ============================================================================ */

/**
 * @brief  进入 Bootloader 模式
 *         此函数由 YModem 模块调用
 */
void BOOT_EnterBootloaderMode(void)
{
    printf("[BOOT] 进入 Bootloader 模式\r\n");
    printf("[BOOT] UART: USART2 (PA2/PA3)\r\n");
    printf("[BOOT] 波特率：115200\r\n");
    printf("[BOOT] 等待固件升级...\r\n\r\n");
    
    /* Bootloader 主逻辑在 main.c 中实现 */
    /* 这里只负责打印信息 */
}

/**
 * @brief  检查是否进入 Bootloader 模式
 * @retval 0: 进入 APP, 1: 保持 Bootloader
 */
uint8_t BOOT_CheckEnterBootloader(void)
{
    /* 方法 1：检查复位标志（看门狗复位后进入 IAP） */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET) {
        __HAL_RCC_CLEAR_RESET_FLAGS();
        printf("[BOOT] 检测到看门狗复位\r\n");
        return 1;
    }
    
    /* 方法 2：检查按键（KEY1 按下） */
    KEY_Init();  /* 初始化按键 */
    HAL_Delay(50);  /* 等待稳定 */
    
    if (KEY_IsPressed(KEY1)) {
        printf("[BOOT] 检测到 KEY1 按下\r\n");
        return 1;
    }
    
    /* 方法 3：检查 RAM 魔术字（APP 程序可设置此标志请求升级） */
    if (*(volatile uint32_t *)0x20000000 == BOOT_FLAG_IAP) {
        *(volatile uint32_t *)0x20000000 = 0;  /* 清除标志 */
        printf("[BOOT] 检测到 RAM 魔术字\r\n");
        return 1;
    }
    
    /* 方法 4：检查参数区标志 */
    uint32_t boot_flag = BOOT_GetFlag();
    if (boot_flag == BOOT_FLAG_IAP) {
        printf("[BOOT] 检测到参数区 IAP 标志\r\n");
        return 1;
    }
    
    return 0;  /* 进入 APP */
}

/**
 * @brief  跳转到 APP 程序
 */
void BOOT_JumpToApp(void)
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
    __set_MSP(*(volatile uint32_t *)FLASH_APP_START_ADDR);
    
    /* 5. 获取 APP 复位向量 */
    void (*app_reset_handler)(void) = 
        (void (*)(void))(*(volatile uint32_t *)(FLASH_APP_START_ADDR + 4));
    
    /* 6. 设置向量表偏移（APP 需要） */
    SCB->VTOR = FLASH_APP_START_ADDR;
    
    /* 7. 跳转 */
    printf("[BOOT] 跳转地址：0x%08lX\r\n", (uint32_t)app_reset_handler);
    app_reset_handler();
    
    /* 永远不会到这里 */
    while (1);
}

/**
 * @brief  验证 APP 固件
 * @retval 0: 有效，1: 无效
 */
uint8_t BOOT_VerifyAppFirmware(void)
{
    /* 检查 APP 区起始位置是否有有效代码 */
    uint32_t app_stack = *(volatile uint32_t *)FLASH_APP_START_ADDR;
    uint32_t app_reset = *(volatile uint32_t *)(FLASH_APP_START_ADDR + 4);
    
    /* 简单的有效性检查 */
    if (app_stack == 0xFFFFFFFF || app_reset == 0xFFFFFFFF) {
        printf("[BOOT] APP 栈指针或复位向量为空\r\n");
        return 1;
    }
    
    /* 检查栈地址是否在 RAM 范围内（STM32F767 为 0x20000000-0x20080000） */
    if (app_stack < 0x20000000 || app_stack > 0x20080000) {
        printf("[BOOT] APP 栈指针地址无效：0x%08lX\r\n", app_stack);
        return 1;
    }
    
    /* 检查复位向量地址是否在 Flash 范围内 */
    if (app_reset < FLASH_APP_START_ADDR || app_reset > 0x0810FFFF) {
        printf("[BOOT] APP 复位向量地址无效：0x%08lX\r\n", app_reset);
        return 1;
    }
    
    printf("[BOOT] APP 验证通过\r\n");
    return 0;  /* 有效 */
}

/**
 * @brief  设置 Bootloader 标志
 * @param  flag: 标志值
 */
void BOOT_SetFlag(uint32_t flag)
{
    FLASH_WriteWord(FLASH_PARAM_ADDR, flag);
}

/**
 * @brief  获取 Bootloader 标志
 * @retval 标志值
 */
uint32_t BOOT_GetFlag(void)
{
    return FLASH_ReadWord(FLASH_PARAM_ADDR);
}

/**
 * @brief  清除 Bootloader 标志
 */
void BOOT_ClearFlag(void)
{
    BOOT_SetFlag(0xFFFFFFFF);
}
