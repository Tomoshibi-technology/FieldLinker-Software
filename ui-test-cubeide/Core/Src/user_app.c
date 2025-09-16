#include <string.h>
#include "main.h"

int test =0;

void UserApp_Init(void) { // initialization
  // Hello World message
  // const char *msg = "Hello World!\r\n";
  // HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

  HAL_GPIO_WritePin(LED_nR0_GPIO_Port, LED_nR0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_LG0_GPIO_Port, LED_LG0_Pin, GPIO_PIN_SET);
}


void UserApp_Run(void) { // main loop
  HAL_GPIO_TogglePin(LED_LG0_GPIO_Port, LED_LG0_Pin);
  HAL_Delay(500);
  test++;
  // HAL_GPIO_TogglePin(LED_LG0_GPIO_Port, LED_LG0_Pin);
  // HAL_Delay(500);
}