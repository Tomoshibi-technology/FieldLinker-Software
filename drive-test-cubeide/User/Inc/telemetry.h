#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Start byte for framing
#ifndef TELEM_START_BYTE
#define TELEM_START_BYTE 0xA5
#endif

// Packet layout (little-endian):
// [0]=start,
// [1..2]=gyroZ (L,H), [3..4]=mouseX (L,H), [5..6]=mouseY (L,H), [7..8]=dx (L,H), [9..10]=dy (L,H)
#define TELEM_FRAME_SIZE 11u

HAL_StatusTypeDef Telemetry_Send11Bytes(UART_HandleTypeDef *huart,
                                        int16_t gyroZ,
                                        int16_t mouseX,
                                        int16_t mouseY,
                                        int16_t dx,
                                        int16_t dy);

#ifdef __cplusplus
}
#endif

#endif // TELEMETRY_H
