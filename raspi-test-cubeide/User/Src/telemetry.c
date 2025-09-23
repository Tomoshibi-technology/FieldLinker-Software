#include "telemetry.h"
#include "usart.h"

// Global variables for UART reception
static uint8_t uart4_rx_byte;
static TelemetryRx_t telemetryRx;

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

/**
 * @brief Initialize telemetry reception structure
 * @param telRx Pointer to telemetry reception structure
 */
void Telemetry_InitRx(TelemetryRx_t *telRx)
{
    telRx->rxIndex = 0;
    telRx->state = TELEM_RX_WAIT_START;
    telRx->frameReady = 0;
    telRx->lastByteTime = HAL_GetTick();
    telRx->data.gyroZ = 0;
    telRx->data.mouseX = 0;
    telRx->data.mouseY = 0;
    telRx->data.dx = 0;
    telRx->data.dy = 0;
}

/**
 * @brief Process a received byte for telemetry frame reconstruction
 * @param telRx Pointer to telemetry reception structure
 * @param byte Received byte
 */
void Telemetry_ProcessRxByte(TelemetryRx_t *telRx, uint8_t byte)
{
    // Update timestamp for timeout detection
    telRx->lastByteTime = HAL_GetTick();
    
    switch (telRx->state) {
        case TELEM_RX_WAIT_START:
            if (byte == TELEM_START_BYTE) {
                telRx->rxBuffer[0] = byte;
                telRx->rxIndex = 1;
                telRx->state = TELEM_RX_COLLECTING_DATA;
            }
            // Note: All other bytes are ignored in this state
            break;

        case TELEM_RX_COLLECTING_DATA:
            // Check if we receive another start byte (resync case)
            if (byte == TELEM_START_BYTE && telRx->rxIndex > 1) {
                // Restart frame collection - possible resync
                telRx->rxBuffer[0] = byte;
                telRx->rxIndex = 1;
                // Stay in TELEM_RX_COLLECTING_DATA state
                break;
            }
            
            telRx->rxBuffer[telRx->rxIndex] = byte;
            telRx->rxIndex++;
            
            if (telRx->rxIndex >= TELEM_FRAME_SIZE) {
                // Complete frame received, parse data
                telRx->data.gyroZ = (int16_t)(telRx->rxBuffer[1] | (telRx->rxBuffer[2] << 8));
                telRx->data.mouseX = (int16_t)(telRx->rxBuffer[3] | (telRx->rxBuffer[4] << 8));
                telRx->data.mouseY = (int16_t)(telRx->rxBuffer[5] | (telRx->rxBuffer[6] << 8));
                telRx->data.dx = (int16_t)(telRx->rxBuffer[7] | (telRx->rxBuffer[8] << 8));
                telRx->data.dy = (int16_t)(telRx->rxBuffer[9] | (telRx->rxBuffer[10] << 8));
                
                telRx->frameReady = 1;
                telRx->state = TELEM_RX_WAIT_START;  // Go back to waiting for next start byte
                telRx->rxIndex = 0;
            }
            break;

        case TELEM_RX_COMPLETE:
            // This state is no longer used - simplified state machine
            telRx->state = TELEM_RX_WAIT_START;
            telRx->rxIndex = 0;
            
            // Process this byte as potential start of new frame
            if (byte == TELEM_START_BYTE) {
                telRx->rxBuffer[0] = byte;
                telRx->rxIndex = 1;
                telRx->state = TELEM_RX_COLLECTING_DATA;
            }
            break;
    }
}

/**
 * @brief Check if a complete telemetry frame is ready
 * @param telRx Pointer to telemetry reception structure
 * @return 1 if frame is ready, 0 otherwise
 */
uint8_t Telemetry_IsFrameReady(TelemetryRx_t *telRx)
{
    return telRx->frameReady;
}

/**
 * @brief Get the parsed telemetry data
 * @param telRx Pointer to telemetry reception structure
 * @return Parsed telemetry data structure
 */
TelemetryData_t Telemetry_GetData(TelemetryRx_t *telRx)
{
    telRx->frameReady = 0; // Mark as consumed
    return telRx->data;
}

/**
 * @brief Check for timeout and reset state if needed
 * @param telRx Pointer to telemetry reception structure
 */
void Telemetry_CheckTimeout(TelemetryRx_t *telRx)
{
    if (telRx->state == TELEM_RX_COLLECTING_DATA) {
        uint32_t currentTime = HAL_GetTick();
        if ((currentTime - telRx->lastByteTime) > TELEM_TIMEOUT_MS) {
            // Timeout occurred - reset to waiting state
            telRx->state = TELEM_RX_WAIT_START;
            telRx->rxIndex = 0;
            telRx->lastByteTime = currentTime;
        }
    }
}

/**
 * @brief Start UART reception in interrupt mode
 * @param huart Pointer to UART handle
 * @return HAL status
 */
HAL_StatusTypeDef Telemetry_StartUartRxIT(UART_HandleTypeDef *huart)
{
    Telemetry_InitRx(&telemetryRx);
    return HAL_UART_Receive_IT(huart, &uart4_rx_byte, 1);
}

/**
 * @brief UART receive complete callback
 * @param huart Pointer to UART handle
 */
void Telemetry_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4) {
        // Process the received byte
        Telemetry_ProcessRxByte(&telemetryRx, uart4_rx_byte);
        
        // Restart reception for next byte
        HAL_StatusTypeDef status = HAL_UART_Receive_IT(huart, &uart4_rx_byte, 1);
        if (status != HAL_OK) {
            // If restart fails, try to reinitialize
            Telemetry_StartUartRxIT(huart);
        }
    }
}

/**
 * @brief UART error callback
 * @param huart Pointer to UART handle
 */
void Telemetry_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4) {
        // Clear error flags and restart reception
        __HAL_UART_CLEAR_PEFLAG(huart);
        __HAL_UART_CLEAR_FEFLAG(huart);
        __HAL_UART_CLEAR_NEFLAG(huart);
        __HAL_UART_CLEAR_OREFLAG(huart);
        
        // Restart reception
        Telemetry_StartUartRxIT(huart);
    }
}

/**
 * @brief Get global telemetry reception structure
 * @return Pointer to global telemetry reception structure
 */
TelemetryRx_t* Telemetry_GetRxStruct(void)
{
    return &telemetryRx;
}
