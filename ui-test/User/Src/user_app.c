#include <string.h>

#include "stm32h7xx_hal.h" //hal library
#include "user_app.h"
#include "main.h"


void UserApp_Init(void) { // initialization
  // Hello World message
  // const char *msg = "Hello World!\r\n";
  // HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}


void UserApp_Run(void) { // main loop
  // HAL_GPIO_WritePin(LED_LG0_GPIO_Port, LED_LG0_Pin, GPIO_PIN_SET);
  HAL_GPIO_TogglePin(LED_LG0_GPIO_Port, LED_LG0_Pin);
  HAL_Delay(500);

  // HAL_GPIO_TogglePin(LED_LG0_GPIO_Port, LED_LG0_Pin);
  // HAL_Delay(500);
}