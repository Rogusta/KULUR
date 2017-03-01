/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Kulur
  * The maker          : R1881
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f3xx_hal.h"
#include "cmsis_os.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "crc.h"
#include <math.h>
#include <stdbool.h>


#define MESSAGE_BUFFERSIZE 43
#define DATE_BUFFERSIZE 10
#define TIME_BUFFERSIZE 5
#define TEMP_BUFFERSIZE 4

uint8_t aShowTime[50] = {0};
RTC_HandleTypeDef hrtc;

void SystemClock_Config(void);
void Error_Handler(void);
void Display(int, bool);
void RTC_TimeConfig(int[], int[]);
void RTC_TimeShow(uint16_t*, uint16_t*);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle);
void Uart_function();
void Radio_data(uint32_t duty_array[]);

//void MX_FREERTOS_Init(void);

ITStatus UartReady = RESET; 

int main(void)
{

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_RTC_Init();
  //Uart_function(); //:::::::::::::::::::::::::::::::::::::
  MX_TIM1_Init();
  MX_CRC_Init();

  /* Call init function for freertos objects (in freertos.c) */
  //MX_FREERTOS_Init();

  /* Start scheduler */
  //osKernelStart();
  


  while (1)
  {
    
    Display(0, false);

  }

}

void Radio_data(uint32_t duty_array[])
{
  static uint32_t data_array[40];
  uint32_t      BufferLength = 40;
  
  for(int i =0; i<40; i++)
  {
    /* Översätter värden i array till ett lr noll. (550-700)*/
    if(duty_array[i] > 500 && duty_array[i] < 800)
    {
      data_array[i] = 1;
    }
    else
      data_array[i] = 0;
  }

  //printf("crc: %d\n", HAL_CRC_Calculate(&hcrc, data_array, 40));
  int temp_value = 0;

  //if(HAL_CRC_Calculate(&hcrc, data_array, 40) == 0)
  //{
  
    //int temp_value = 0;
    for(int i = 0; i < 9; i++)
    {
      temp_value += data_array[23-i] * (pow(2, i));
    }

    Display(temp_value, true);

  //}
  
  return;
  
}

//____________________________UART_function_____________________________________
void Uart_function()
{
  uint8_t Buffer[] = "Set the date yyyy-mm-dd and time hh:mm \r\n";
  uint8_t Date[] = "Date: ";
  uint8_t Time[] = " Time: ";
  int date[3];
  int time[1];
  int i = 0;
  
  /*Starts the transmission process*/
  if(HAL_UART_Transmit_IT(&huart3, (uint8_t *)Buffer, MESSAGE_BUFFERSIZE) != HAL_OK)//skriver ut välkomsttext i UART
  {
    Error_Handler();
  }
  /*Wait for the end of the transfer*/
  while(UartReady != SET)
  {
  }
  /*Reset transmission flag*/
  UartReady = RESET;

  if(HAL_UART_Transmit_IT(&huart3, (uint8_t *)Date, 6) != HAL_OK)//skriver ut text date i UART
  {
    Error_Handler();
  }
  while(UartReady != SET)
  {
  }
  UartReady = RESET;
  
  memset(Buffer, 0, sizeof(Buffer)); //nollställer Buffer[].
  //---------------------------Start UART date----------------------------------
      /*Starts the receiving process*/
    if(HAL_UART_Receive_IT(&huart3, (uint8_t *)Buffer, DATE_BUFFERSIZE) != HAL_OK)// tar in datum
    {
      Error_Handler();
    }
    /*Wait for the end of the receive*/
    
    while(UartReady != SET)
    {
    }
    /*Reset receive flag*/
    UartReady = RESET;
    
    /*Starts the transmission process*/
    if(HAL_UART_Transmit_IT(&huart3, (uint8_t *)Buffer, DATE_BUFFERSIZE) != HAL_OK)//skriver ut datum
    {
    Error_Handler();
    }
    /*Wait for the end of the transfer*/
    while(UartReady != SET)
    {
    }
    /*Reset transmission flag*/
    UartReady = RESET;
    
   const char s[2] = "-";
   char *token;
   

   token = strtok(Buffer, s);
   while( token != NULL ) 
   {
      date[i] = atoi(token);
      token = strtok(NULL, s);
      //printf("%d\n", date[i]);
      i++;
   }
   date[0] = date[0] % 100;

//---------------------------End UART date--------------------------------------

  if(HAL_UART_Transmit_IT(&huart3, (uint8_t *)Time, 7) != HAL_OK)//skriver ut text Time i UART:
  {
    Error_Handler();
  }
  while(UartReady != SET)
  {
  }
  UartReady = RESET;
//---------------------------Start UART time------------------------------------
    memset(Buffer, 0, sizeof(Buffer)); //nollställer Buffer[].
    
    if(HAL_UART_Receive_IT(&huart3, (uint8_t *)Buffer, TIME_BUFFERSIZE) != HAL_OK)// tar in tid
    {
      Error_Handler();
    }
    /*Wait for the end of the receive*/
    while(UartReady != SET)
    {
    }
    /*Reset receive flag*/
    UartReady = RESET;
    
    /*Starts the transmission process*/
    if(HAL_UART_Transmit_IT(&huart3, (uint8_t *)Buffer, TIME_BUFFERSIZE) != HAL_OK)//skriver ut tid
    {
    Error_Handler();
    }
    /*Wait for the end of the transfer*/
    while(UartReady != SET)
    {
    }
    /*Reset transmission flag*/
    UartReady = RESET;
    
    time[0] = (Buffer[0] - 48)*10 + (Buffer[1] - 48); //omvandlar Ascii till int värde
    time[1] = (Buffer[3] - 48)*10 + (Buffer[4] - 48);
    //printf("tid: %d : %d\n", time[0], time[1]);
    
    RTC_TimeConfig(date, time);
//---------------------------End UART time--------------------------------------
}
//___________________________UART Callbacks_____________________________________
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /*Set transmittion flag: transfer complete*/
  UartReady = SET;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /*Set receive flag: receive complete*/
  UartReady = SET;
}
//______________________Time configuration function_____________________________
void RTC_TimeConfig(int date[], int time[])
{
  //printf("%d\n", date[0]); // print year

  RTC_TimeTypeDef stimestructure;
  RTC_DateTypeDef sdatestructure;
  
  sdatestructure.Year = date[0];
  sdatestructure.Month = date[1];
  sdatestructure.Date = date[2];
  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
  
   /*##-2- Configure the Time #################################*/
  /* Set Time: 02:00:00 */
  stimestructure.Hours = time[0];
  stimestructure.Minutes = time[1];
  stimestructure.Seconds = 0x00;
  //stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
  //stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  //stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
  
  HAL_RTC_SetDate(&hrtc, &sdatestructure, RTC_FORMAT_BCD);
  HAL_RTC_SetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
}

//__________________________Time show function__________________________________
void RTC_TimeShow(uint16_t *hours, uint16_t *minute)
{
  RTC_TimeTypeDef stimestructureget;
  RTC_DateTypeDef sdatestructureget;
  /* Get the RTC current Time */
  HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BCD);
  

  /* Display time Format : hh:mm:ss */
  //printf("%2d:%2d:%2d\n", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
  //printf("%2d:%2d:%2d\n", sdatestructureget.Year, sdatestructureget.Month, sdatestructureget.Date);
  *hours = stimestructureget.Hours;
  *minute = stimestructureget.Minutes;

}
//___________________________Display function___________________________________
void Display(int temp, bool temp_flag)
{
    
    int counter = 0;
    static int value_array[8];
    
   static int tempTioTal;
   static int tempEnTal;
   static int tempDecTal;
    
  /*if(temp> 512) //ska visa minustecken i display
    {
      int minus = 10; // case 10 för ett minus
      value_array[4] = minus;
    }
    else value_array[4] = 11;*/ // inget visas i display
    
    if(temp_flag)
    {
      tempTioTal = temp/100;
      tempEnTal = (temp%100)/10;
      tempDecTal = temp%10;
    }

    uint16_t hours = 0;
    uint16_t minute = 0;
    
    RTC_TimeShow(&hours, &minute);
    

    value_array[0] = hours/10;
    value_array[1] = hours%10;
    value_array[2] = minute/10;
    value_array[3] = minute%10;
    value_array[4] = 11; //Om temp är positivt, inget i display
    value_array[5] = tempTioTal;
    value_array[6] = tempEnTal;
    value_array[7] = tempDecTal;

    uint32_t current_second = HAL_GetTick();
    static uint32_t last_second;
    if ((current_second - last_second) > 500)
    {    
    last_second = current_second;        
    HAL_GPIO_TogglePin(GPIOC,Kolon_Pin);
    }
    
    
    int segmentSelect = 0;
while(segmentSelect < 8)
{
    for(int i= 0; i<9000; i++)
    {
    }
    switch(segmentSelect) // State machine val av 7-seg
    {
//----------------------------select clk display--------------------------------
      case 0:// clock display 1
        HAL_GPIO_WritePin(GPIOC, DIG1clk_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, DIG2clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4clk_Pin, GPIO_PIN_RESET);
       
        
        HAL_GPIO_WritePin(GPIOC, DIG1term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, DP_led_Pin, GPIO_PIN_SET); 
        
        break;
        
      case 1: // clock display 2
        HAL_GPIO_WritePin(GPIOC, DIG1clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2clk_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, DIG3clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4clk_Pin, GPIO_PIN_RESET);
       
       
        HAL_GPIO_WritePin(GPIOC, DIG1term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, DP_led_Pin, GPIO_PIN_SET);         
        break;
        
      case 2: // clock display 3
        HAL_GPIO_WritePin(GPIOC, DIG1clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3clk_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, DIG4clk_Pin, GPIO_PIN_RESET);
        
        HAL_GPIO_WritePin(GPIOC, DIG1term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, DP_led_Pin, GPIO_PIN_SET);         
        break;
        
      case 3: // clock display 4
        HAL_GPIO_WritePin(GPIOC, DIG1clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4clk_Pin, GPIO_PIN_SET);
       
        
        HAL_GPIO_WritePin(GPIOC, DIG1term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, DP_led_Pin, GPIO_PIN_SET);         
        break;
//-----------------------------select temp display---------------------------------
      case 4:// temp display 1
        HAL_GPIO_WritePin(GPIOC, DIG1term_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, DIG2term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, DP_led_Pin, GPIO_PIN_SET); 
        
        HAL_GPIO_WritePin(GPIOC, DIG1clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4clk_Pin, GPIO_PIN_RESET);
        break;
        
      case 5:// temp display 2
        HAL_GPIO_WritePin(GPIOC, DIG1term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2term_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, DIG3term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, DP_led_Pin, GPIO_PIN_SET);
        
        HAL_GPIO_WritePin(GPIOC, DIG1clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4clk_Pin, GPIO_PIN_RESET);        
        break;
        
      case 6:// temp display 3
        HAL_GPIO_WritePin(GPIOC, DIG1term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3term_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, DIG4term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, DP_led_Pin, GPIO_PIN_RESET);
        
        HAL_GPIO_WritePin(GPIOC, DIG1clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4clk_Pin, GPIO_PIN_RESET);
        break;
        
      case 7:// temp display 4
        HAL_GPIO_WritePin(GPIOC, DIG1term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3term_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4term_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, DP_led_Pin, GPIO_PIN_SET);
        
        HAL_GPIO_WritePin(GPIOC, DIG1clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG2clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG3clk_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, DIG4clk_Pin, GPIO_PIN_RESET);
        break;
        
    default:
        break;
    }//end switch
        
        
//--------------------------Number for displays---------------------------------
    switch(value_array[counter]) // State machine val av nummer
    {
      case 0: //number 0
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_SET);
        break;
        
      case 1: //number 1
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_SET);
        break;
        
      case 2://number 2
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_RESET);
        break;
        
      case 3://number 3
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_RESET);
        break;
        
      case 4://number 4
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_RESET);
        break;  
        
      case 5://number 5
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_RESET);
        break;  
        
      case 6://number 6
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_RESET);
        break;  
        
      case 7://number 7
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_SET);
        break;
        
      case 8://number 8
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_RESET);
        break;
        
      case 9://number 9
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_RESET);
        break;
      
      case 10://minus tecken i temp display
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_RESET);
      break;

      case 11://Om plus, inget i displayen
        HAL_GPIO_WritePin(GPIOD, A_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, B_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, C_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, D_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, E_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, F_led_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOD, G_led_Pin, GPIO_PIN_SET);
      break;
      default:
 
      break;
    } // end switch
    
    segmentSelect++;
    counter++;
    
  
}//end while

    return;

}
//______________________________________________________________________________
/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_RTC
                              |RCC_PERIPHCLK_TIM1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV32;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_HCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
