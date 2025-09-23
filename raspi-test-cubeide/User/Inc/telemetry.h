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

// Frame timeout in milliseconds
#ifndef TELEM_TIMEOUT_MS
#define TELEM_TIMEOUT_MS 100
#endif

// Packet layout (little-endian):
// [0]=start,
// [1..2]=gyroZ (L,H), [3..4]=mouseX (L,H), [5..6]=mouseY (L,H), [7..8]=dx (L,H), [9..10]=dy (L,H)
#define TELEM_FRAME_SIZE 11u

// Structure to hold parsed telemetry data
typedef struct {
    int16_t gyroZ;
    int16_t mouseX;
    int16_t mouseY;
    int16_t dx;
    int16_t dy;
} TelemetryData_t;

// Reception states for state machine
typedef enum {
    TELEM_RX_WAIT_START,
    TELEM_RX_COLLECTING_DATA,
    TELEM_RX_COMPLETE
} TelemetryRxState_t;

// Reception structure
typedef struct {
    uint8_t rxBuffer[TELEM_FRAME_SIZE];
    uint8_t rxIndex;
    TelemetryRxState_t state;
    TelemetryData_t data;
    uint8_t frameReady;
    uint32_t lastByteTime;  // For timeout detection
} TelemetryRx_t;

HAL_StatusTypeDef Telemetry_Send11Bytes(UART_HandleTypeDef *huart,
                                        int16_t gyroZ,
                                        int16_t mouseX,
                                        int16_t mouseY,
                                        int16_t dx,
                                        int16_t dy);

void Telemetry_InitRx(TelemetryRx_t *telRx);
void Telemetry_ProcessRxByte(TelemetryRx_t *telRx, uint8_t byte);
uint8_t Telemetry_IsFrameReady(TelemetryRx_t *telRx);
TelemetryData_t Telemetry_GetData(TelemetryRx_t *telRx);
void Telemetry_CheckTimeout(TelemetryRx_t *telRx);
HAL_StatusTypeDef Telemetry_StartUartRxIT(UART_HandleTypeDef *huart);
void Telemetry_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void Telemetry_UART_ErrorCallback(UART_HandleTypeDef *huart);
TelemetryRx_t* Telemetry_GetRxStruct(void);

#ifdef __cplusplus
}
#endif

#endif // TELEMETRY_H
