/*************************************************************************************
# Released under MIT License

Copyright (c) 2020 SF Yip (yipxxx@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************/

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
#include "btldr_config.h"
#include "crypt.h"
#include "ihex_parser.h"
#include "crc.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#if (BTLDR_ACT_NoAppExist > 0u)
bool is_appcode_exist(void)
{
  uint32_t *mem = (uint32_t*)APP_ADDR;
  
  if ((mem[0] == 0x00000000 || mem[0] == 0xFFFFFFFF) && \
      (mem[1] == 0x00000000 || mem[1] == 0xFFFFFFFF) && \
      (mem[2] == 0x00000000 || mem[2] == 0xFFFFFFFF) && \
      (mem[3] == 0x00000000 || mem[3] == 0xFFFFFFFF))
  {
    return false;
  }
  else
  {
    return true;
  }
}
#endif

#if (BTLDR_ACT_CksNotVld > 0u)
bool app_cks_valid(void)
{
	uint32_t app_crc32 = 0;

	/* calculate CRC32 checksum from start of main application until CRC32 location */
	app_crc32 = crc32_calculate((const uint8_t *)APP_ADDR, (CRC_ADDR-APP_ADDR));

	/* compare self calculated CRC32 with CRC32 stored in flash */
	return (app_crc32 == *((uint32_t*)(CRC_ADDR)));
}
#endif

#if (BTLDR_ACT_ButtonPress > 0u)
bool is_button_down(void)
{
    return (!LL_GPIO_IsInputPinSet(BTLDR_EN_GPIO_Port, BTLDR_EN_Pin) );
}
#endif

#if (BTLDR_ACT_BootkeyDet > 0u)

volatile __attribute__((section("._bootkey_section.btldr_act_req_key"))) uint32_t btldr_act_req_key;

bool bootkey_detected(void){
    return(btldr_act_req_key == BOOTKEY);
}
#endif

void control_led_states(void)
{
#if defined(LED_RED_Pin) || defined(LED_BLUE_Pin)
    uint32_t tick = HAL_GetTick();
    if ((tick % 100) >= 50)
    {
#if defined(LED_RED_Pin)
        LL_GPIO_SetOutputPin(LED_RED_GPIO_Port, LED_RED_Pin);
#endif
#if defined(LED_BLUE_Pin)
        LL_GPIO_SetOutputPin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
#endif
    }
    else
    {
#if defined(LED_RED_Pin)
        LL_GPIO_ResetOutputPin(LED_RED_GPIO_Port, LED_RED_Pin);
#endif
#if defined(LED_BLUE_Pin)
        LL_GPIO_ResetOutputPin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
#endif
    }
#endif
}

extern PCD_HandleTypeDef hpcd_USB_FS;

void SystemReset(void){
    LL_mDelay(500);

    HAL_PCD_Stop(&hpcd_USB_FS);

    /* Pull USB D+ PIN to GND so USB Host detects device disconnect */
    LL_GPIO_SetPinMode(GPIOA,LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_12, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinOutputType(GPIOA,LL_GPIO_PIN_12, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12);

    LL_mDelay(1000);

    NVIC_SystemReset();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  static uint32_t jump_addr;

  /* USER CODE END 1 */

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
  
  /* USER CODE BEGIN 2 */

 if(	/* Check for configured activation options */
    #if (BTLDR_ACT_NoAppExist > 0u)
      !is_appcode_exist()
    #endif
    #if (BTLDR_ACT_ButtonPress > 0u)
      || is_button_down()
    #endif
    #if (BTLDR_ACT_CksNotVld > 0u)
      || !app_cks_valid()
    #endif
    #if (BTLDR_ACT_BootkeyDet > 0u)
      || bootkey_detected()
    #endif
	 )
  {
#if(CONFIG_SUPPORT_CRYPT_MODE > 0u)
    crypt_init();
#endif
    MX_USB_DEVICE_Init();
    while(1)
    {
        control_led_states();
#if (CONFIG_SOFT_RESET_AFTER_IHEX_EOF > 0u)
      if(ihex_is_eof()) {
        #if (BTLDR_ACT_BootkeyDet > 0u)
         btldr_act_req_key = 0;
        #endif
         SystemReset();
      }
#endif
    }
  }
  else
  {
    // Jump to the app code
	jump_addr = *((__IO uint32_t*)(APP_ADDR+4u));
	HAL_DeInit();

	// Disable all interrupts
	NVIC->ICER[0] = 0xFFFFFFFF;
	NVIC->ICER[1] = 0xFFFFFFFF;
	NVIC->ICER[2] = 0xFFFFFFFF;

	NVIC->ICPR[0] = 0xFFFFFFFF;
	NVIC->ICPR[1] = 0xFFFFFFFF;
	NVIC->ICPR[2] = 0xFFFFFFFF;

	/* deactivate SysTick */
	SysTick->CTRL = 0;

	/* Change the main stack pointer. */
	SCB->VTOR = APP_ADDR;
	__set_MSP((*(__IO uint32_t*)APP_ADDR));

	((void (*) (void)) (jump_addr)) ();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(72000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
  LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_PLL_DIV_1_5);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);

  /**/
  GPIO_InitStruct.Pin = BTLDR_EN_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.OutputType = 0;
  LL_GPIO_Init(BTLDR_EN_GPIO_Port, &GPIO_InitStruct);

#ifdef LED_RED_Pin
  GPIO_InitStruct.Pin = LED_RED_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Pull = 0;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  LL_GPIO_Init(LED_RED_GPIO_Port, &GPIO_InitStruct);
  LL_GPIO_ResetOutputPin(LED_RED_GPIO_Port, LED_RED_Pin);
#endif

#ifdef LED_BLUE_Pin
  GPIO_InitStruct.Pin = LED_BLUE_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Pull = 0;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  LL_GPIO_Init(LED_BLUE_GPIO_Port, &GPIO_InitStruct);
  LL_GPIO_ResetOutputPin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
#endif
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
