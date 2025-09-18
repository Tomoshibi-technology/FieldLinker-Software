#ifndef BNO055_H
#define BNO055_H

// bno055 registers
#define BNO055_I2C_ADDR_7BIT   0x28  // ADR=GND の場合
#define BNO055_CHIP_ID         0x00
#define BNO055_CHIP_ID_VALUE   0xA0

#define BNO055_PAGE_ID         0x07
#define BNO055_OPR_MODE        0x3D
#define BNO055_PWR_MODE        0x3E
#define BNO055_SYS_TRIGGER     0x3F
#define BNO055_CALIB_STAT      0x35

#define BNO055_EULER_H_LSB     0x1A  // Heading LSB
#define BNO055_QUATERNION_W_LSB 0x20 // Quaternion start (8 bytes)

#include "main.h"
#include "i2c.h"

typedef struct {
  float w, x, y, z;
} bno_quat_t;

typedef struct {
  float heading, roll, pitch; // degrees
} bno_euler_t;

HAL_StatusTypeDef BNO055_Init(I2C_HandleTypeDef *hi2c, uint8_t dev_addr7);
HAL_StatusTypeDef BNO055_ReadQuaternion(I2C_HandleTypeDef *hi2c, uint8_t dev_addr7, bno_quat_t *q);
HAL_StatusTypeDef BNO055_ReadEuler(I2C_HandleTypeDef *hi2c, uint8_t dev_addr7, bno_euler_t *e);
HAL_StatusTypeDef BNO055_ReadCalib(I2C_HandleTypeDef *hi2c, uint8_t dev_addr7, uint8_t *calib);
HAL_StatusTypeDef BNO055_ReadChipID(I2C_HandleTypeDef *hi2c, uint8_t dev_addr7, uint8_t *id);

#endif
