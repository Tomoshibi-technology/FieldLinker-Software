#include <string.h>
#include "main.h"
#include "usart.h"
#include "telemetry.h"

static TelemetryData_t receivedData;
static uint32_t frameCount = 0;

void UserApp_Init(void) 
{
    // Initialize LEDs (all off)
    HAL_GPIO_WritePin(LED_nR0_GPIO_Port, LED_nR0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_LG0_GPIO_Port, LED_LG0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_Y0_GPIO_Port, LED_Y0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_B0_GPIO_Port, LED_B0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_B1_GPIO_Port, LED_B1_Pin, GPIO_PIN_SET);

    // Start UART4 reception for telemetry data
    HAL_StatusTypeDef status = Telemetry_StartUartRxIT(&huart4);
    if (status != HAL_OK) {
        // Error handling - turn on red LED
        HAL_GPIO_WritePin(LED_nR0_GPIO_Port, LED_nR0_Pin, GPIO_PIN_RESET);
    } else {
        // Success - turn on yellow LED
        HAL_GPIO_WritePin(LED_Y0_GPIO_Port, LED_Y0_Pin, GPIO_PIN_RESET);
    }
}


void UserApp_Run(void) 
{
    // Main loop indicator - toggle green LED
    HAL_GPIO_TogglePin(LED_LG0_GPIO_Port, LED_LG0_Pin);
    HAL_Delay(10);

    // Check if new telemetry frame is received
    TelemetryRx_t* telRx = Telemetry_GetRxStruct();
    
    // Check for timeout and reset if needed
    Telemetry_CheckTimeout(telRx);
    
    if (Telemetry_IsFrameReady(telRx)) {
        // Get the received data
        receivedData = Telemetry_GetData(telRx);
        frameCount++;
        
        // Indicate frame received - toggle blue LED
        HAL_GPIO_TogglePin(LED_B0_GPIO_Port, LED_B0_Pin);
        
        // Process received telemetry data here
        // Example: Use receivedData.gyroZ, receivedData.mouseX, etc.
    }
}

/**
 * @brief Get the latest received telemetry data
 * @return Latest telemetry data structure
 */
TelemetryData_t UserApp_GetLatestTelemetryData(void)
{
    return receivedData;
}

/**
 * @brief Get the total number of received frames
 * @return Frame count
 */
uint32_t UserApp_GetFrameCount(void)
{
    return frameCount;
}
