#include "telemetry.h"

HAL_StatusTypeDef Telemetry_Send11Bytes(UART_HandleTypeDef *huart,
                                        int16_t gyroZ,
                                        int16_t mouseX,
                                        int16_t mouseY,
                                        int16_t dx,
                                        int16_t dy)
{
    uint8_t buf[TELEM_FRAME_SIZE];
    buf[0] = (uint8_t)TELEM_START_BYTE;
    buf[1] = (uint8_t)(gyroZ & 0xFF);
    buf[2] = (uint8_t)((gyroZ >> 8) & 0xFF);
    buf[3] = (uint8_t)(mouseX & 0xFF);
    buf[4] = (uint8_t)((mouseX >> 8) & 0xFF);
    buf[5] = (uint8_t)(mouseY & 0xFF);
    buf[6] = (uint8_t)((mouseY >> 8) & 0xFF);
    buf[7] = (uint8_t)(dx & 0xFF);
    buf[8] = (uint8_t)((dx >> 8) & 0xFF);
    buf[9]  = (uint8_t)(dy & 0xFF);
    buf[10] = (uint8_t)((dy >> 8) & 0xFF);

    return HAL_UART_Transmit(huart, buf, (uint16_t)sizeof(buf), HAL_MAX_DELAY);
}
