/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
//#include <stdio.h>
#include "MMA_7455.h"
#include "SparkFunBME280.h"
#include "Modbus.h"
#include "semphr.h"
#include "task.h"
#include "FreeRTOS.h"
#include "printf.h"
#include <string.h>
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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;

/* Definitions for analogT */
osThreadId_t analogTHandle;
const osThreadAttr_t analogT_attributes = {
  .name = "analogT",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for modbusT */
osThreadId_t modbusTHandle;
const osThreadAttr_t modbusT_attributes = {
  .name = "modbusT",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for spiT */
osThreadId_t spiTHandle;
const osThreadAttr_t spiT_attributes = {
  .name = "spiT",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for cdcT */
osThreadId_t cdcTHandle;
const osThreadAttr_t cdcT_attributes = {
  .name = "cdcT",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* USER CODE BEGIN PV */

#define ADC_BUFF_NUM 16

typedef struct adcvalues{
	uint16_t raw[ADC_BUFF_NUM];
	float intTemp;
}adcval_t;
uint16_t adcBuffer[5];

union Num{
	float fnum;
	uint16_t unum;
}num;

adcval_t adcint;
uint16_t modbusData[128];

MMA_7455 vib(spi_protocol, 9);
BME280 	 temp;
modbusHandler_t mb;
uint8_t curADC=0;

modbus_t telegram[2];
int convcomplete=0;

char CDCreceivedBuffer[250];
int receiveData=0;
int lenReceiveData=0;
int adcInteruptCount=0;

#define ADCBUFFERSIZE 250
#define SPRINTFBUFFERSIZE 2048
uint16_t 	ia[ADCBUFFERSIZE],
			ib[ADCBUFFERSIZE],
			ic[ADCBUFFERSIZE],
			vref[ADCBUFFERSIZE],
			vtemp[ADCBUFFERSIZE];

char sprintfBuffer[SPRINTFBUFFERSIZE];

extern USBD_HandleTypeDef hUsbDeviceFS;
//USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;

extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
extern uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM3_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
void analogTask(void *argument);
void modbusTask(void *argument);
void spiTask(void *argument);
void cdcTask(void *argument);

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
  MX_DMA_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  mb.uModbusType = MB_SLAVE;
  mb.port 		 = &huart3;
  mb.u8id 		 = 2;
  mb.u16timeOut  = 3;
  mb.EN_Port	 = TRx_en_GPIO_Port;
  mb.EN_Pin		 = TRx_en_Pin;
  mb.u16regs	 = modbusData;
  mb.u16regsize	 = sizeof(modbusData)/sizeof(modbusData[0]);
  mb.xTypeHW	 = USART_HW_DMA;

  // init modbus and start
  ModbusInit(&mb);
  ModbusStart(&mb);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcBuffer, 5);
  HAL_TIM_Base_Start(&htim3);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of analogT */
  analogTHandle = osThreadNew(analogTask, NULL, &analogT_attributes);

  /* creation of modbusT */
//  modbusTHandle = osThreadNew(modbusTask, NULL, &modbusT_attributes);

  /* creation of spiT */
//  spiTHandle = osThreadNew(spiTask, NULL, &spiT_attributes);

  /* creation of cdcT */
//  cdcTHandle = osThreadNew(cdcTask, NULL, &cdcT_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
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
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 5;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 200-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 57600; //115200; //==> 115200 tidak bisa bekerja dengan HMI Vijeo
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA1_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_MMA7455_GPIO_Port, CS_MMA7455_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, tog_Pin|TRx_en_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_BME280_GPIO_Port, CS_BME280_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CS_MMA7455_Pin */
  GPIO_InitStruct.Pin = CS_MMA7455_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_MMA7455_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : tog_Pin TRx_en_Pin */
  GPIO_InitStruct.Pin = tog_Pin|TRx_en_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : CS_BME280_Pin */
  GPIO_InitStruct.Pin = CS_BME280_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_BME280_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/**
 * hapus extern "C" jika format file ini .c
 * tambahkan extern "C" jika format file ini .cpp.
 **/

uint16_t float_to_uint16 (float float_number)
{
  union
  {
    float f_number;
    uint16_t uint16_arr[2];
  } union_for_conv;
  union_for_conv.f_number = float_number;
  uint16_t a = union_for_conv.uint16_arr[1];
  return a;
}

extern "C" int _write(int file, char *ptr, int len) {
    CDC_Transmit_FS((uint8_t*) ptr, len); return len;
}

void printBin(unsigned char value)
{
    for (int i = sizeof(char) * 7; i >= 0; i--)
        printf("%d", (value & (1 << i)) >> i );
    putc('\n', stdout);
}

//char buffer[600];
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	if(hadc->Instance == ADC1){
		if (adcInteruptCount <50) {
			ia[adcInteruptCount] = adcBuffer[0];
			ib[adcInteruptCount] = adcBuffer[1];
			ic[adcInteruptCount] = adcBuffer[2];
			vref[adcInteruptCount] = adcBuffer[3];
			vtemp[adcInteruptCount] = adcBuffer[4];

			adcInteruptCount +=1;
		}
		else convcomplete = 1;

	}
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_analogTask */
/**
  * @brief  Function implementing the analogT thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_analogTask */
void analogTask(void *argument)
{
  /* init code for USB_DEVICE */
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	MX_USART3_UART_Init();
	HAL_GPIO_WritePin(CS_MMA7455_GPIO_Port, CS_MMA7455_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_BME280_GPIO_Port, CS_BME280_Pin, GPIO_PIN_SET);

	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;

	vib.begin();
	vib.setSensitivity(2);
	vib.setMode(measure);
	vib.setAxisOffset(0, 0, 0);


	temp.begin();

//	uint8_t buf[2];
//	uint8_t rec=0x01;
//	uint8_t read=0x16;
//	uint8_t chipID;
	float bxg, byg, bzg, rtemp;
	int idx;
//	float ia,ib,ic,vtemp,vref;

  for(;;)
  {
	  osSemaphoreAcquire(mb.ModBusSphrHandle, portMAX_DELAY);
//	  xSemaphoreTake((QueueHandle_t) mb.ModBusSphrHandle, portMAX_DELAY);

	  rtemp = temp.readTempC();
//	  int xg = (int) bxg;
	  vib.readAxis8g(&bxg, &byg, &bzg);
//	  bxg = vib.readAxis8g('x');
//	  byg = vib.readAxis8g('y');
//	  bzg = vib.readAxis8g('z');
//	  chipID = temp.readRegister(BME280_CHIP_ID_REG);

//	  if(xg  != 2) vib.setSensitivity(2);
//	  xg = vib.getSensitivity();
//	  printf(" %d ", xg);
//	  zg = bzg *10000;
//	  printf(" \t%.5f\t|\t%.5f\t|\t%.5f\t|\n", bxg, byg, bzg);
//	  ia = curADC/4095.0;
//	  && hcdc->TxState != USBD_BUSY
	  if (receiveData) {
		  char *pt;
		  int idxCDCModbus=0;
		  pt = strtok (CDCreceivedBuffer,",");
		  while (pt != NULL) {
			  float a= atof(pt);

			  modbusData[idxCDCModbus] = float_to_uint16(a);//(uint16_t) a;
			  pt = strtok (NULL, ",");
			  idxCDCModbus++;
		  }
		  memset(CDCreceivedBuffer, NULL, sizeof(CDCreceivedBuffer));
		  receiveData=0;
	  }
	  if(convcomplete == 1){

//		  idx = printf("%d,%d,%d,%d,%d,%d,%.5f,%.5f,%.5f,%.3f\n", HAL_GetTick(), adcBuffer[0], adcBuffer[1], adcBuffer[2], adcBuffer[3], adcBuffer[4], bxg, byg, bzg, rtemp);

		  idx=0;
		  for(int i=0; i<10; i++){
			  idx += sprintf((char*)UserTxBufferFS+idx,
					  	  	  "%d,%d,%d,%f,%f,%f,%.3f\n",
							  ia[i], ib[i], ic[i], bxg, byg, bzg, rtemp);

		  }
		  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, idx);
		  USBD_CDC_TransmitPacket(&hUsbDeviceFS);

		  convcomplete = 0;
		  adcInteruptCount=0;
	  }


//
//	  HAL_GPIO_WritePin(LP3_GPIO_Port, LP3_Pin, mb.u16regs[0] & 0x1==1 ? GPIO_PIN_RESET : GPIO_PIN_SET);
//	  printf("%04X \n", modbusData[0]);

//	  xSemaphoreGive((QueueHandle_t) mb.ModBusSphrHandle); //Release the semaphore
	  osSemaphoreRelease(mb.ModBusSphrHandle);
	  osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_modbusTask */
/**
* @brief Function implementing the modbusT thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_modbusTask */
void modbusTask(void *argument)
{
  /* USER CODE BEGIN modbusTask */
  /* Infinite loop */
  for(;;)
  {
//	  printf("modbus task: %d\n", HAL_GetTick());
	  osDelay(6000);
  }
  /* USER CODE END modbusTask */
}

/* USER CODE BEGIN Header_spiTask */
/**
* @brief Function implementing the spiT thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_spiTask */
void spiTask(void *argument)
{
  /* USER CODE BEGIN spiTask */
  /* Infinite loop */
//	HAL_GPIO_WritePin(CS_MMA7455_GPIO_Port, CS_MMA7455_Pin, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(CS_BME280_GPIO_Port, CS_BME280_Pin, GPIO_PIN_SET);
//
//	vib.begin();
//	vib.setSensitivity(2);
//	vib.setMode(measure);
//	vib.setAxisOffset(0, 0, 0);
//
//	temp.begin();
//
//
//	uint8_t buf[2];
//	uint8_t rec=0x01;
//	uint8_t read=0x16;
//	float bxg, byg, bzg;
//	int xg, yg, zg;
  for(;;)
  {

//	  float bxg = temp.readTempC();
//	  int xg = (int) bxg;
////	  printf("temp *C: %f \n", temp.readTempC());
////	  printf("temp *C: %i\n", xg);
//
//	  bxg = vib.readAxis8g('x');
//	  xg = bxg *10000;
//
//	  byg = vib.readAxis8g('y');
//	  yg = byg *10000;
//
//	  bzg = vib.readAxis8g('z');
//	  zg = bzg *10000;
//	  printf(" \t%.5f\t|\t%.5f\t|\t%.5f\t|\n", bxg, byg, bzg);


	  osDelay(250);
  }
  /* USER CODE END spiTask */
}

/* USER CODE BEGIN Header_cdcTask */
/**
* @brief Function implementing the cdcT thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_cdcTask */
void cdcTask(void *argument)
{
  /* USER CODE BEGIN cdcTask */
  /* Infinite loop */

//	modbusData[0]=0xFAFA;
//	modbusData[1]=0xFBFB;
//	modbusData[2]=0xFCFC;
//	modbusData[3]=0xFDFD;
//	modbusData[4]=0xFEFE;
//	uint32_t u32NotificationValue;

//	telegram[0].u8id = 1; // slave address
//	telegram[0].u8fct = MB_FC_READ_REGISTERS; // function code (this one is registers read)
//	//telegram[0].u16RegAdd = 0x160; // start address in slave
//	telegram[0].u16RegAdd = 0x0; // start address in slave
//	telegram[0].u16CoilsNo = 1; // number of elements (coils or registers) to read
//	telegram[0].u16reg = modbusData; // pointer to a memory array


	// telegram 0: read registers
//	telegram[1].u8id = 1; // slave address
//	telegram[1].u8fct = MB_FC_WRITE_REGISTER; // function code (this one is registers write)
	//telegram[1].u16RegAdd = 0x160; // start address in slave
//	telegram[1].u16RegAdd = 0x1;
//	telegram[1].u16CoilsNo = 5; // number of elements (coils or registers) to read
//	telegram[1].u16reg = modbusData+1; // pointer to a memory array
  for(;;)
  {
//	  printf("cdcTask %d \n", HAL_GetTick());
//	  ModbusQuery(&mb, telegram[0]); // make a query
//	  u32NotificationValue = ulTaskNotifyTake(pdTRUE, 500); // block until query finishes or timeout
//	  if(u32NotificationValue)
//	  {
//		  printf("Query1 Fail\n");
//	  }
//	  osDelay(500);

//	  modbusData[0];
//	  ModbusQuery(&mb, telegram[1]); // make a query
//	  u32NotificationValue = ulTaskNotifyTake(pdTRUE, 500); // block until query finishes or timeout
//	  if(u32NotificationValue)
//	  {
//		  printf("Query2 Fail\n");
//	  }
	  osDelay(500);
//	  telegram[1].u16RegAdd+=0x1;
  }
  /* USER CODE END cdcTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
	  printf("initialization Error!!");
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
