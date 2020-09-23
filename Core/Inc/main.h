/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum eMode {POST, IDLE, SETTING, RUNNING, WAITING, ALARM, FAILSAFE} Mode;

typedef enum eEvent {EV_NONE, EV_OUTPUT_ERR, EV_COMMU_ERR, EV_SENSOR_ERR} Event;

typedef struct {
	Mode output;
	Mode commu;
	Mode sensor;
        Mode tracking;
} evState_t;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern volatile Mode currentMode;
extern int errno;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define S1_Pin GPIO_PIN_2
#define S1_GPIO_Port GPIOE
#define S2_Pin GPIO_PIN_3
#define S2_GPIO_Port GPIOE
#define S3_Pin GPIO_PIN_4
#define S3_GPIO_Port GPIOE
#define S4_Pin GPIO_PIN_5
#define S4_GPIO_Port GPIOE
#define S5_Pin GPIO_PIN_6
#define S5_GPIO_Port GPIOE
#define MB1_INVR_Pin GPIO_PIN_1
#define MB1_INVR_GPIO_Port GPIOA
#define DBG_TX_Pin GPIO_PIN_2
#define DBG_TX_GPIO_Port GPIOA
#define DBG_RX_Pin GPIO_PIN_3
#define DBG_RX_GPIO_Port GPIOA
#define EXT_TIM_Pin GPIO_PIN_0
#define EXT_TIM_GPIO_Port GPIOB
#define EXT_ADC_Pin GPIO_PIN_1
#define EXT_ADC_GPIO_Port GPIOB
#define EXT_GND_Pin GPIO_PIN_2
#define EXT_GND_GPIO_Port GPIOB
#define MB1_INVD_Pin GPIO_PIN_7
#define MB1_INVD_GPIO_Port GPIOE
#define MB1_RE_Pin GPIO_PIN_10
#define MB1_RE_GPIO_Port GPIOE
#define EXT_CS_Pin GPIO_PIN_14
#define EXT_CS_GPIO_Port GPIOE
#define MB1_TX_Pin GPIO_PIN_10
#define MB1_TX_GPIO_Port GPIOB
#define MB1_RX_Pin GPIO_PIN_11
#define MB1_RX_GPIO_Port GPIOB
#define MB1_DE_Pin GPIO_PIN_12
#define MB1_DE_GPIO_Port GPIOB
#define EXT_SCK_Pin GPIO_PIN_13
#define EXT_SCK_GPIO_Port GPIOB
#define EXT_MISO_Pin GPIO_PIN_14
#define EXT_MISO_GPIO_Port GPIOB
#define EXT_MOSI_Pin GPIO_PIN_15
#define EXT_MOSI_GPIO_Port GPIOB
#define EXT_TX_Pin GPIO_PIN_6
#define EXT_TX_GPIO_Port GPIOC
#define EXT_RX_Pin GPIO_PIN_7
#define EXT_RX_GPIO_Port GPIOC
#define EXT_CAN_TX_Pin GPIO_PIN_1
#define EXT_CAN_TX_GPIO_Port GPIOD
#define LR_NRST_Pin GPIO_PIN_6
#define LR_NRST_GPIO_Port GPIOD
#define EXT_SDA_Pin GPIO_PIN_7
#define EXT_SDA_GPIO_Port GPIOB
#define EXT_SCL_Pin GPIO_PIN_8
#define EXT_SCL_GPIO_Port GPIOB
#define EXT_STATUS_Pin GPIO_PIN_9
#define EXT_STATUS_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
typedef struct user_data_t {

	uint16_t temperature_adc_value;
	uint16_t vref_adc_value;
    uint16_t period;
#ifdef  USE_ADC_SENSOR
	uint16_t ac_current_value;
	uint16_t ac_current_adc_value;
#else
	uint16_t io_value;
#endif
#ifdef USE_HDC1080
	uint16_t amb_temperature_value;
	uint16_t amb_humidity_value;
#elif defined ( USE_ULTRASONIC_SENSOR )
        void* us_sensor;
#elif defined ( USE_AQI_SENSOR )
        void* pm_sensor;
        void* co2_sensor;
        void* sl_sensor;
#endif

} user_data_t;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
