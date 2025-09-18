#ifndef PAA5100JE_H
#define PAA5100JE_H

#include "stm32h7xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include "spi.h"

// ===== Register map (datasheet Table 8) =====
#define PAA_REG_PRODUCT_ID           0x00  // expect 0x49
#define PAA_REG_REVISION_ID          0x01
#define PAA_REG_MOTION               0x02
#define PAA_REG_DELTA_X_L            0x03
#define PAA_REG_DELTA_X_H            0x04
#define PAA_REG_DELTA_Y_L            0x05
#define PAA_REG_DELTA_Y_H            0x06
#define PAA_REG_SQUAL                0x07
#define PAA_REG_RAWDATA_SUM          0x08
#define PAA_REG_MAX_RAWDATA          0x09
#define PAA_REG_MIN_RAWDATA          0x0A
#define PAA_REG_SHUTTER_LOWER        0x0B
#define PAA_REG_SHUTTER_UPPER        0x0C
#define PAA_REG_OBSERVATION          0x15
#define PAA_REG_MOTION_BURST         0x16
#define PAA_REG_POWER_UP_RESET       0x3A
#define PAA_REG_SHUTDOWN             0x3B
#define PAA_REG_RESOLUTION           0x4E
#define PAA_REG_RAWDATA_GRAB         0x58
#define PAA_REG_RAWDATA_GRAB_STATUS  0x59
#define PAA_REG_ORIENTATION          0x5B
#define PAA_REG_INVERSE_PRODUCT_ID   0x5F

#define PAA_PRODUCT_ID_VAL           0x49
#define PAA_INVERSE_PRODUCT_ID_VAL   0xB6

// ===== LED/BANK control registers and values =====
#define PAA_REG_BANK_SELECT         0x7F
#define PAA_REG_LED_CONTROL         0x6F
#define PAA_BANK_LED                0x14
#define PAA_BANK_0                  0x00
#define PAA_LED_ON                  0x1C
#define PAA_LED_OFF                 0x00

// ORIENTATION bits (per Pimoroni driver)
#define PAA_ORIENT_SWAP_XY  (1u << 7)
#define PAA_ORIENT_INVERT_Y (1u << 6)
#define PAA_ORIENT_INVERT_X (1u << 5)

typedef struct {
    SPI_HandleTypeDef *hspi;

    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;

    // optional hardware reset (active low), set port=NULL if unused
    GPIO_TypeDef *rst_port;
    uint16_t rst_pin;
} PAA5100JE_Handle;

typedef struct {
    uint8_t  motion;       // MOT/DR bits from 0x02
    uint8_t  observation;  // 0x15
    int16_t  dx;           // ΔX
    int16_t  dy;           // ΔY
    uint8_t  squal;        // 0x07
    uint8_t  raw_sum;      // 0x08
    uint8_t  raw_max;      // 0x09
    uint8_t  raw_min;      // 0x0A
    uint16_t shutter;      // (upper<<8)|lower
} PAA5100JE_Motion;

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef PAA5100JE_Init(PAA5100JE_Handle *dev);
HAL_StatusTypeDef PAA5100JE_ReadMotionBurst(PAA5100JE_Handle *dev, PAA5100JE_Motion *m);
HAL_StatusTypeDef PAA5100JE_ReadMotion(PAA5100JE_Handle *dev, int16_t *dx, int16_t *dy);
HAL_StatusTypeDef PAA5100JE_SetOrientation(PAA5100JE_Handle *dev, bool invert_x, bool invert_y, bool swap_xy);
HAL_StatusTypeDef PAA5100JE_WriteReg(PAA5100JE_Handle *dev, uint8_t reg, uint8_t val);
HAL_StatusTypeDef PAA5100JE_ReadReg(PAA5100JE_Handle *dev, uint8_t reg, uint8_t *val);
HAL_StatusTypeDef PAA5100JE_SetLed(PAA5100JE_Handle *dev, bool on);

#ifdef __cplusplus
}
#endif
#endif // PAA5100JE_H
