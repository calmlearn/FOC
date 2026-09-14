/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "AS5600.h"
#include "motorpwm.h"
#include "FOC.h"
#include "motorapp.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "config.h"
#include "Position_pid.h"
#include "Speed_pid.h"
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
static volatile uint8_t uart_send_request = 0;
static volatile uint8_t uart_tx_busy = 0;
static uint8_t uart_tx_buffer[12];

#define RX_SIZE 64U

static uint8_t rx_byte;                  // 每次收到的一个字节
static char rx_buffer[RX_SIZE];          // 新增接收数组
static volatile uint16_t rx_count = 0;
static volatile uint8_t rx_ready = 0;    // 完整命令已收到
static uint8_t rx_discard = 0;           // 丢弃超长或来不及处理的行

pid Speed_pid;
pid Position_pid;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void UART_CommandProcess(void)
{
    char command[RX_SIZE];
    char *end;
    float value;
    uint32_t irq_state;

    if (!rx_ready)
    {
        return;
    }

    // 把完整命令复制到局部数组，再释放接收数组
    irq_state = __get_PRIMASK();
    __disable_irq();

    memcpy(command, rx_buffer, sizeof(command));
    rx_count = 0;
    rx_ready = 0;

    __set_PRIMASK(irq_state);

    // 格式例如 P=0.01
    if (command[0] == '\0' || command[1] != '=')
    {
        return;
    }

    value = strtof(&command[2], &end);

    // 必须有数字，且不能有无效尾部、NaN或无穷大
    if (end == &command[2] || *end != '\0' || !isfinite(value))
    {
        return;
    }

    // 示例输入范围：目标速度±100rpm，增益0～10
    // 增益范围只用于检查输入，不代表推荐值
    if (command[0] == 'T')
    {
        if (value < -200.0f || value > 200.0f)
            return;
    }
    else if (command[0] == 'P' ||
             command[0] == 'I' ||
             command[0] == 'D')
    {
        if (value < 0.0f || value > 10.0f)
            return;
    }
    else
    {
        return;
    }

    // 避免TIM2在参数更新到一半时执行
    irq_state = __get_PRIMASK();
    __disable_irq();

    switch (command[0])
    {
        case 'P':
            Speed_pid.kp = value;
            break;

        case 'I':
            Speed_pid.ki = value;
            Speed_pid.errorint = 0.0f;
            break;

        case 'D':
            Speed_pid.kd = value;
            break;

        case 'T':
            Speed_pid.target = value;
            break;
    }

    __set_PRIMASK(irq_state);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  SpeedPid_Init(&Speed_pid);
  Speed_pid.kp = 0.1;
  Speed_pid.ki = 0.001;
  Speed_pid.kd = 0;
	
  Speed_pid.outmax =  0.5;
  Speed_pid.outmin = -0.5;
	
  if(Speed_pid.ki!=0)
  {
	  Speed_pid.intmax =  0.3/Speed_pid.ki;
	  Speed_pid.intmin = -0.3/Speed_pid.ki;
  }
  Speed_pid.target = 30.0f;
  
  PositionPid_Init(&Position_pid);
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
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
 
  MotorPWM_Init();
  MotorPWM_Enable();
  HAL_TIM_Base_Start_IT(&htim2);
  
  HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
  while (1)
  {
		
	  UART_CommandProcess();
	  
	  if((uart_send_request ==1)&&(uart_tx_busy==0))
	  {
			float angle = AS5600_GetAngle();
			float speed = AS5600_GetSpeed();

			uart_send_request = 0;

			memcpy(&uart_tx_buffer[0], &angle, sizeof(float));
			memcpy(&uart_tx_buffer[4], &speed, sizeof(float));

			uart_tx_buffer[8]  = 0x00;
			uart_tx_buffer[9]  = 0x00;
			uart_tx_buffer[10] = 0x80;
			uart_tx_buffer[11] = 0x7F;

			uart_tx_busy = 1;
		  
		  if(HAL_UART_Transmit_IT(&huart2,uart_tx_buffer,sizeof(uart_tx_buffer)) != HAL_OK)
		  {
			  uart_tx_busy = 0;
		  }
	  }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
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

  /** Initializes the CPU, AHB and APB buses clocks
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
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint16_t usart_cnt = 0;
    if (htim->Instance == TIM2)
    {
        usart_cnt++;
		
		if(AS5600_AngleIsValid())
		{
			float mechanical_angle = AS5600_GetAngle();
			float mechanical_rpm = AS5600_GetSpeed();
			
			SpeedPid_Update(&Speed_pid,mechanical_rpm);
			
			float electrical_angle = motorAngle(7.0f * mechanical_angle);
			
			SVPWM_FOC(0.0f, Speed_pid.out, electrical_angle);
		}
		
		HAL_StatusTypeDef I2C_statu =  AS5600_Read_RawAngle();
		
		if(usart_cnt>=10)
		{
			usart_cnt = 0;
			uart_send_request = 1;
		}
		//接下来是闭环控制阶段
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        uart_tx_busy = 0;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
    {
        char ch = (char)rx_byte;

    if (ch == '\n')
    {
        if (!rx_ready && !rx_discard && rx_count > 0U)
        {
            rx_buffer[rx_count] = '\0';
            rx_ready = 1;
        }

        if (!rx_ready)
        {
            rx_count = 0;
        }

        rx_discard = 0;
    }
    else if (ch != '\r')
    {
        if (rx_ready || rx_discard)
        {
            // 上一条未处理，或本行已超长：丢弃到换行
            rx_discard = 1;
        }
        else if (rx_count < RX_SIZE - 1U)
        {
            rx_buffer[rx_count++] = ch;
        }
        else
        {
            rx_count = 0;
            rx_discard = 1;
        }
    }

    // 继续接收下一个字节
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
	
}
/* USER CODE END 4 */

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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
