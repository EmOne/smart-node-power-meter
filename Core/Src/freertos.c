/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "ModBus.h"
#include "ModBusPort.h"
#include "WiMOD_LoRaWAN_API.h"
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
evState_t ev;
user_data_t sData_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
int iSlaveId;

bool bReadyToSend = false;
uint32_t iCounter = 0;
uint32_t Setup_BaudRate;
uint32_t Setup_Parity;
uint32_t Setup_Stopbit;
uint32_t Setup_Wordlength;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId modbusTaskHandle;
osThreadId loraTaskHandle;
osMessageQId msgQueueHandle;
osTimerId modbusTimerHandle;
osTimerId heartbeatTimerHandle;
osTimerId telemetryTimerHandle;
osMutexId processMutexHandle;
osSemaphoreId processBinarySemHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
int loraDataRx(uint8_t fport, uint8_t* data, size_t len)
{
    switch(fport) {
    case 99:
       //Restart system
      NVIC_SystemReset();
      break;

    case 3:
      if(len == 1) {
        //lora tx perior
        sData_t.period = loraAppStatus.period = *data;
        ev.commu = RUNNING;
      }
      break;
    default: break;
    };
    return 0;
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartModbusTask(void const * argument);
void StartLoRaTask(void const * argument);
void modbusTimerCallback(void const * argument);
void heartbeatTimerCallback(void const * argument);
void telemetryTimerCallback(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of processMutex */
  osMutexDef(processMutex);
  processMutexHandle = osMutexCreate(osMutex(processMutex));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of processBinarySem */
  osSemaphoreDef(processBinarySem);
  processBinarySemHandle = osSemaphoreCreate(osSemaphore(processBinarySem), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of modbusTimer */
  osTimerDef(modbusTimer, modbusTimerCallback);
  modbusTimerHandle = osTimerCreate(osTimer(modbusTimer), osTimerPeriodic, NULL);

  /* definition and creation of heartbeatTimer */
  osTimerDef(heartbeatTimer, heartbeatTimerCallback);
  heartbeatTimerHandle = osTimerCreate(osTimer(heartbeatTimer), osTimerPeriodic, NULL);

  /* definition and creation of telemetryTimer */
  osTimerDef(telemetryTimer, telemetryTimerCallback);
  telemetryTimerHandle = osTimerCreate(osTimer(telemetryTimer), osTimerPeriodic, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of msgQueue */
  osMessageQDef(msgQueue, 16, uint32_t);
  msgQueueHandle = osMessageCreate(osMessageQ(msgQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of modbusTask */
  osThreadDef(modbusTask, StartModbusTask, osPriorityNormal, 0, 1280);
  modbusTaskHandle = osThreadCreate(osThread(modbusTask), NULL);

  /* definition and creation of loraTask */
  osThreadDef(loraTask, StartLoRaTask, osPriorityNormal, 0, 2048);
  loraTaskHandle = osThreadCreate(osThread(loraTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  osTimerStart(heartbeatTimerHandle, 200);
  /* Infinite loop */
  for(;;)
  {
	  if(iCounter++ == (86400 / 6))
		  NVIC_SystemReset();
	  osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartModbusTask */
/**
* @brief Function implementing the modbusTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartModbusTask */
void StartModbusTask(void const * argument)
{
  /* USER CODE BEGIN StartModbusTask */
	int i, j;
	ev.sensor = POST;
  /* Infinite loop */
  for(;;)
  {
		switch (ev.sensor) {
		case POST:
			InitModbusMaster();
			//MX_USART3_UART_Init()	;
			osTimerStart(modbusTimerHandle, 1000);
			ev.sensor = IDLE;
			break;
		case IDLE:
		while(bReadyToSend == false)
// 			ev.commu = POST;
				osDelay(1000);
			ev.sensor = RUNNING;
			break;
		case SETTING:
			break;
		case RUNNING:
			for (j =0 ; j < NUMBER_MASTER_LOOKUP_SLAVE; ++j)
			{
				iSlaveId = j+1;


				if(iSlaveId==1)
				{
					Setup_BaudRate = 19200 ;
					Setup_Parity = UART_PARITY_NONE;
					Setup_Stopbit = UART_STOPBITS_2;
					Setup_Wordlength =UART_WORDLENGTH_8B;
					ModBusChange_UART_Config(Setup_BaudRate,Setup_Parity,Setup_Stopbit,Setup_Wordlength);
//					UART_Config(19200,UART_PARITY_NONE,UART_STOPBITS_2);
//					ModBusChange_UART_Config(Setup_BaudRate,Setup_Parity,Setup_Stopbit);
//					MX_USART3_UART_Init();
					for (i = 0; i < NUMBER_SCHNEIDER_LOOKUP_INPUTS; ++i) {
//						ModBusChange_UART_Config(Setup_BaudRate,Setup_Parity,Setup_Stopbit,Setup_Wordlength);
						while (ModBusMasterRead(iSlaveId, 3,
								SchneiderLookupTableInputs[i].LookupAddress,
								SchneiderLookupTableInputs[i].Size, 1000) != TRUE) {
							osDelay(10);
							MasterReadTimerValue += 10;
						}
						osDelay(1000);
					}

				}


			else
				if(iSlaveId==2)
				{

					Setup_BaudRate = 19200 ;
					Setup_Parity = UART_PARITY_NONE;
					Setup_Stopbit = UART_STOPBITS_2;
					Setup_Wordlength =UART_WORDLENGTH_8B;
					ModBusChange_UART_Config(Setup_BaudRate,Setup_Parity,Setup_Stopbit,Setup_Wordlength);
				//	UART_Config(1200,UART_PARITY_EVEN,UART_STOPBITS_1);
//					ModBusChange_UART_Config(1200,UART_PARITY_EVEN,UART_STOPBITS_1);
//					uint8_t a[] = {0x02, 0x03, 0, 0x66, 0, 0x1,0x6F, 0xBC};
//					ModBusMaster_UART_String(a,8);

					for (i = 0; i < NUMBER_SCHNEIDER_LOOKUP_INPUTS; ++i) {
//						ModBusChange_UART_Config(Setup_BaudRate,Setup_Parity,Setup_Stopbit,Setup_Wordlength);

						while (ModBusMasterRead(iSlaveId, 3,
								SchneiderLookupTableInputs[i].LookupAddress,
								SchneiderLookupTableInputs[i].Size, 1000) != TRUE) {
								osDelay(10);
								MasterReadTimerValue += 10;
						}
						osDelay(1000);
					}

				}

				else
					if(iSlaveId==3)
					{
								Setup_BaudRate = 1200 ;
								Setup_Parity = UART_PARITY_EVEN;
								Setup_Stopbit = UART_STOPBITS_1;
								Setup_Wordlength =UART_WORDLENGTH_9B;
								ModBusChange_UART_Config(Setup_BaudRate,Setup_Parity,Setup_Stopbit,Setup_Wordlength);
							//	UART_Config(1200,UART_PARITY_EVEN,UART_STOPBITS_1);
			//					ModBusChange_UART_Config(1200,UART_PARITY_EVEN,UART_STOPBITS_1);
			//					uint8_t a[] = {0x02, 0x03, 0, 0x66, 0, 0x1,0x6F, 0xBC};
			//					ModBusMaster_UART_String(a,8);

								for (i = 0; i < NUMBER_MITSU_LOOKUP_INPUTS; ++i) {
			//						ModBusChange_UART_Config(Setup_BaudRate,Setup_Parity,Setup_Stopbit,Setup_Wordlength);

									while (ModBusMasterRead(iSlaveId, 3,
											MitsuLookupTableInputs[i].LookupAddress,
											MitsuLookupTableInputs[i].Size, 1500) != TRUE) {
											osDelay(10);
											MasterReadTimerValue += 10;
									}
									osDelay(1000);
								}

					}

				ev.commu = RUNNING;
				while(ev.commu != IDLE);
				osDelay(5000);
			}
			ev.sensor = IDLE;
			break;
		case ALARM:
			break;
		case FAILSAFE:
			break;
		default:
			break;
		}
	osDelay(1);
  }
  /* USER CODE END StartModbusTask */
}


/* USER CODE BEGIN Header_StartLoRaTask */
/**
* @brief Function implementing the loraTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartLoRaTask */
void StartLoRaTask(void const * argument)
{
  /* USER CODE BEGIN StartLoRaTask */
//	osEvent _event;
	ev.commu = POST;
	//  uint8_t mac_cmd[] = {0x01, 0x02};
	uint8_t i, j, buf[48] = {};
//	struct user_data_t *st;
//  osTimerStart(telemetryTimerHandle, 15000);
	/* Infinite loop */


	  /* Infinite loop */
	for (;;) {
		switch (ev.commu) {
		case POST:
			memset(buf, 0x0, sizeof(buf));
			 MX_USART6_UART_Init();
			 WiMOD_LoRaWAN_Init(&huart6, loraDataRx);
			//WiMOD_LoRaWAN_Init();
			while (Ping() == 0)
				osDelay(1000);
//			//      Reset();
//			HAL_GPIO_WritePin(LR_NRST_GPIO_Port, LR_NRST_Pin, GPIO_PIN_SET);
//			osDelay(100);
//			HAL_GPIO_WritePin(LR_NRST_GPIO_Port, LR_NRST_Pin, GPIO_PIN_RESET);
//			osDelay(100);
//			HAL_GPIO_WritePin(LR_NRST_GPIO_Port, LR_NRST_Pin, GPIO_PIN_SET);
//			osDelay(1000);
//
//			while (Ping() == 0)
//				osDelay(1000);
//			//      FactoryReset();
//			//      osDelay(10000);
//			//        LORA_state = LORA_RUNNING;
//
			ev.commu = SETTING;
			//ev.commu=IDLE;

			break;
		case IDLE:
//			osDelay(1000);
//			//      osThreadSuspend(&commuTaskHandle);
//			      WiMOD_LoRaWAN_SetMAC_CMD(&mac_cmd, sizeof(mac_cmd));
			if (GetNwkStatus() != 1) {
					        //        LORA_state = LORA_IDLE;
					        ev.commu = FAILSAFE;
					        //      else
					        //        LORA_state = LORA_ERROR;

					      }
					      else   {
				       osDelay(5000);

					      if (loraAppStatus.maxPlayloadSize < loraAppStatus.playloadSize)
					         ev.commu = FAILSAFE;

					      }
		    bReadyToSend = true;

			//			SendUData(MasterTx_Buf[0] + 1, (uint8_t *) &sData_t, sizeof(sData_t));
			//			//TODO:Send Data
		    //ev.commu =RUNNING;
			break;
		case SETTING:
			SetOPMODE(3);
			osDelay(5000);
			GetOPMODE();
			osDelay(1000);
			if (loraAppStatus.opMode != 0) {
				SetLinkADR();
				osDelay(1000);
				SetDevEUI();
				osDelay(5000);
				SetOPMODE(0);
				osDelay(1000);
			}
			SetRadioStack();
			osDelay(1000);
			GetDeviceInfo();
			osDelay(5000);

			GetDevEUI();
			osDelay(5000);

			Deactivate();
			osDelay(500);

			ActivateABP();
			osDelay(5000);

			//      WiMOD_LoRaWAN_SetMAC_CMD(&mac_cmd, sizeof(mac_cmd));
			//
//			      osDelay(5000);
////			osTimerStart(telemetryTimerHandle, 5000 * loraAppStatus.period);
			ev.commu = IDLE;
			break;
		case RUNNING:
//			 if(GetDeviceStatus() != 0) {
//						        ev.commu = FAILSAFE;
//
//						        break;
//
//						      }
//
//			// 			BSP_LED_Toggle(LED6);
//			 else
//			 {
						bReadyToSend = false;
					if(iSlaveId==1)
					{
						for (i = 0; i < 9; ++i) {
							buf[0] = i;
							for (j = 0; j < SchneiderLookupTableInputs[i].Size; ++j) {
								memcpy(&buf[(j*2) + 1], &SchneiderLookupTableInputs[i].RegisterInput[j].ActValue, sizeof(int));
							}
							SendUData(iSlaveId + 1, (uint8_t *) &buf,
									(sizeof(int16_t) * SchneiderLookupTableInputs[i].Size) + 1);
							osDelay(30000);
						}
						ev.commu = IDLE;
					}
					else if(iSlaveId==2)
					{
						for (i = 0; i < 9; ++i) {
							buf[0] = i;
							for (j = 0; j < SchneiderLookupTableInputs[i].Size; ++j) {
								memcpy(&buf[(j*2) + 1], &SchneiderLookupTableInputs[i].RegisterInput[j].ActValue, sizeof(int));
								}
								SendUData(iSlaveId + 1, (uint8_t *) &buf,
										(sizeof(int16_t) * SchneiderLookupTableInputs[i].Size) + 1);
								osDelay(30000);
						}


						ev.commu = IDLE;

					}
					else if(iSlaveId==3)
					{
						for (i = 0; i < 8; ++i) {
							buf[0] = i;
							for (j = 0; j < MitsuLookupTableInputs[i].Size; ++j) {
								memcpy(&buf[(j*2) + 1], &MitsuLookupTableInputs[i].RegisterInput[j].ActValue, sizeof(int));
								}
								SendUData(iSlaveId + 1, (uint8_t *) &buf,
										(sizeof(int16_t) * MitsuLookupTableInputs[i].Size) + 1);
								osDelay(30000);
						}

						ev.commu = IDLE;
					}

//						SendUData(MasterTx_Buf[0] + 1, (uint8_t *) &sData_t, sizeof(sData_t));
//						SendUData(iSlaveId + 1, (uint8_t *) &buf,
//					              (sizeof(int16_t) * SchneiderLookupTableInputs[i].Size) + 1);
			//			//TODO:Send Data
//						ev.commu = ALARM;
//
//						//ev.commu = IDLE;
//
//			break;
//		case ALARM:
//			ev.commu = IDLE;
//		break;
//		case FAILSAFE:
			////			osMailFree(msgMailHandle, &sData_t);
			//			Reactivate();
			//
			//			osDelay(5000);
			//
			//			ev.commu = POST;
			//

//			break;
//		default:
//			ev.commu = POST;
//			break;
		}
		osDelay(10);
	}


  /* USER CODE END StartLoRaTask */
}

/* modbusTimerCallback function */
void modbusTimerCallback(void const * argument)
{
  /* USER CODE BEGIN modbusTimerCallback */
//  MasterReadTimerValue += 1000;

  /* USER CODE END modbusTimerCallback */
}

/* heartbeatTimerCallback function */
void heartbeatTimerCallback(void const * argument)
{
  /* USER CODE BEGIN heartbeatTimerCallback */

  /* USER CODE END heartbeatTimerCallback */
}

/* telemetryTimerCallback function */
void telemetryTimerCallback(void const * argument)
{
  /* USER CODE BEGIN telemetryTimerCallback */

  /* USER CODE END telemetryTimerCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
