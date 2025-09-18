#include <string.h>
#include "main.h"
#include "bno055.h"
#include "paa5100je.h"

int test =0;
const uint8_t dev_addr = BNO055_I2C_ADDR_7BIT; // 0x28 or 0x29
bno_euler_t e;

PAA5100JE_Handle paa = {
    .hspi = &hspi2,
    .cs_port = CS2_Mouse_GPIO_Port,
    .cs_pin  = CS2_Mouse_Pin,
    .rst_port = INT_Mouse_GPIO_Port, // 使わない場合は NULL にする
    .rst_pin  = INT_Mouse_Pin
};

// Accumulated position in sensor counts (LSB = 1 count)
static volatile int32_t pos_x = 0;
static volatile int32_t pos_y = 0;

void UserApp_Init(void) { // initialization
  HAL_GPIO_WritePin(LED_nR0_GPIO_Port, LED_nR0_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(LED_LG0_GPIO_Port, LED_LG0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_Y0_GPIO_Port, LED_Y0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_B0_GPIO_Port, LED_B0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_B1_GPIO_Port, LED_B1_Pin, GPIO_PIN_SET);

  while(BNO055_Init(&hi2c4, dev_addr) != HAL_OK){
    // error handling
    HAL_GPIO_TogglePin(LED_Y0_GPIO_Port, LED_Y0_Pin);
  }

  HAL_GPIO_WritePin(CS2_Mouse_GPIO_Port,  CS2_Mouse_Pin,  GPIO_PIN_SET);   // CS inactive
  HAL_GPIO_WritePin(INT_Mouse_GPIO_Port, INT_Mouse_Pin, GPIO_PIN_RESET);   // RST deassert
  HAL_GPIO_WritePin(LED_B0_GPIO_Port, LED_B0_Pin, GPIO_PIN_SET);   // LED_N inactive (消灯)
  if (PAA5100JE_Init(&paa) != HAL_OK) {
    // 失敗時はLED点滅など
    while (1) {
      HAL_GPIO_TogglePin(LED_B0_GPIO_Port, LED_B0_Pin);
      HAL_Delay(200);
    }
  }

  PAA5100JE_SetLed(&paa, true);   // 点灯

  // 起動時に座標をゼロクリア
  pos_x = 0;
  pos_y = 0;
}

uint8_t motion=0;
PAA5100JE_Motion m;

int16_t dx, dy;

void UserApp_Run(void) { // main loop
  HAL_GPIO_TogglePin(LED_LG0_GPIO_Port, LED_LG0_Pin);
  HAL_Delay(4);
  test++;

  if(BNO055_ReadEuler(&hi2c4, dev_addr, &e) == HAL_OK){
    // int n = snprintf(tx, sizeof(tx), "E H=%.2f R=%.2f P=%.2f\r\n", e.heading, e.roll, e.pitch);
    // HAL_UART_Transmit(&huart1, (uint8_t*)tx, n, HAL_MAX_DELAY);
  }

  if (PAA5100JE_ReadMotionBurst(&paa, &m) == HAL_OK) {
    // Motion bit (DR/MOT) が立っているか軽くチェック（bit7）
    if (m.motion & 0x80) {
      // printf("dx=%d dy=%d squal=%u rawSum=%u shutter=%u obs=0x%02X\r\n",
                // m.dx, m.dy, m.squal, m.raw_sum, m.shutter, m.observation);
      pos_x += m.dx;
      pos_y += m.dy;
    }
  }

  if(PAA5100JE_ReadMotion(&paa, &dx, &dy) == HAL_OK) {
    // printf("dx=%d dy=%d\r\n", dx, dy);
    // pos_x += dx;
    // pos_y += dy;
  }
}
