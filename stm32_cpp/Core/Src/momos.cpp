//#include <main.h>
//#include <usbd_cdc_if.h>
//#include <MMA_7455.h>
//#include <momos.h>
//
//#define ADC_BUFF_NUM 16
//
///*
// * Variable
// * */
//int convComplete=0;
//
//float temp=0;
//
//uint16_t adcbuf[2];
//uint16_t adcin=0;
//
//char buffer[100];
//
///*==========================================
// * Custom variable
// *========================================== */
//
//typedef struct adcvalues{
//	uint16_t raw[ADC_BUFF_NUM];
//	float intTemp;
//}adcval_t;
//
//
//adcval_t adcint;
//
//MMA_7455 vib(spi_protocol, 9);
//
///*==========================================
// * function prototypes
// *========================================== */
//
//void setup(){
//	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adcint.raw, ADC_BUFF_NUM);
//	HAL_TIM_Base_Start(&htim3);
//
//}
//
//void loop(){
//	printf("while: %f.2\n", 2.34);
//	HAL_GPIO_TogglePin(LP3_GPIO_Port, LP3_Pin);
//	HAL_Delay(150);
//}
//
//
//
///*==========================================
// * function implementation
// *========================================== */
//
//
