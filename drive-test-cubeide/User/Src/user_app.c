#include <string.h>
#include "main.h"
#include "bno055.h"
#include "paa5100je.h"
#include "usart.h"
#include "telemetry.h"

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
static volatile int16_t pos_x = 0;
static volatile int16_t pos_y = 0;

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

void UserApp_Run(void) { // main loop
  HAL_GPIO_TogglePin(LED_LG0_GPIO_Port, LED_LG0_Pin);
  HAL_Delay(4);
  test++;

  if(BNO055_ReadEuler(&hi2c4, dev_addr, &e) == HAL_OK){
  }

  if (PAA5100JE_ReadMotionBurst(&paa, &m) == HAL_OK) {
    // Motion bit (DR/MOT) が立っているか軽くチェック（bit7）
    if (m.motion & 0x80) {
      // printf("dx=%d dy=%d squal=%u rawSum=%u shutter=%u obs=0x%02X\r\n",
                // m.dx, m.dy, m.squal, m.raw_sum, m.shutter, m.observation);
      pos_x += m.dx;
      pos_y += m.dy;

      // 送信用データ作成（角度Zは0.01deg単位にスケール）
      int16_t gyroZ_i16  = (int16_t)(e.heading * 100.0f);
      int16_t mouseX_i16 = (int16_t)pos_x;  // 必要ならクリップ
      int16_t mouseY_i16 = (int16_t)pos_y;  // 必要ならクリップ
      int16_t dx_i16     = (int16_t)m.dx;   // 将来の一貫化やフィルタ用途に
      int16_t dy_i16     = (int16_t)m.dy;

      Telemetry_Send11Bytes(&huart1, gyroZ_i16, mouseX_i16, mouseY_i16, dx_i16, dy_i16);
    }
  }
}
