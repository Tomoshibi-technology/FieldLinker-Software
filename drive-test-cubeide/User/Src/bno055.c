#include "bno055.h"
#include <string.h>

#define BNO_TIMEOUT_MS 200

static HAL_StatusTypeDef bno_write(I2C_HandleTypeDef *hi2c, uint8_t dev7, uint8_t reg, uint8_t *p, uint16_t len){
    return HAL_I2C_Mem_Write(hi2c, (uint16_t)(dev7<<1), reg, I2C_MEMADD_SIZE_8BIT, p, len, BNO_TIMEOUT_MS);
}
static HAL_StatusTypeDef bno_read(I2C_HandleTypeDef *hi2c, uint8_t dev7, uint8_t reg, uint8_t *p, uint16_t len){
    return HAL_I2C_Mem_Read(hi2c, (uint16_t)(dev7<<1), reg, I2C_MEMADD_SIZE_8BIT, p, len, BNO_TIMEOUT_MS);
}

HAL_StatusTypeDef BNO055_ReadChipID(I2C_HandleTypeDef *hi2c, uint8_t dev_addr7, uint8_t *id){
    return bno_read(hi2c, dev_addr7, BNO055_CHIP_ID, id, 1);
}

HAL_StatusTypeDef BNO055_Init(I2C_HandleTypeDef *hi2c, uint8_t dev_addr7){
    HAL_StatusTypeDef ret;
    uint8_t tmp;

    // 1) 簡易チェック: chip id
    uint8_t id;
    ret = BNO055_ReadChipID(hi2c, dev_addr7, &id);
    if(ret != HAL_OK) return ret;
    // note: after reset chip id might be 0xFF briefly; we'll continue and reset device below if mismatch
    if(id != BNO055_CHIP_ID_VALUE){
        // try a soft reset to recover
    }

    // 2) set CONFIG mode
    tmp = 0x00; // CONFIGMODE
    ret = bno_write(hi2c, dev_addr7, BNO055_OPR_MODE, &tmp, 1);
    if(ret != HAL_OK) return ret;
    HAL_Delay(10);

    // 3) soft reset
    tmp = 0x20; // RST_SYS
    ret = bno_write(hi2c, dev_addr7, BNO055_SYS_TRIGGER, &tmp, 1);
    if(ret != HAL_OK) return ret;
    HAL_Delay(650); // datasheet: >=650 ms

    // after reset, verify chip id again
    ret = BNO055_ReadChipID(hi2c, dev_addr7, &id);
    if(ret != HAL_OK) return ret;
    if(id != BNO055_CHIP_ID_VALUE){
        return HAL_ERROR;
    }

    // 4) power mode normal
    tmp = 0x00; // NORMAL
    ret = bno_write(hi2c, dev_addr7, BNO055_PWR_MODE, &tmp, 1);
    if(ret != HAL_OK) return ret;
    HAL_Delay(10);

    // 5) set operation mode to NDOF (fusion)
    tmp = 0x0C; // NDOF
    ret = bno_write(hi2c, dev_addr7, BNO055_OPR_MODE, &tmp, 1);
    if(ret != HAL_OK) return ret;
    HAL_Delay(20);

    return HAL_OK;
}

HAL_StatusTypeDef BNO055_ReadCalibration(I2C_HandleTypeDef *hi2c, uint8_t dev_addr7, uint8_t *calib){
    return bno_read(hi2c, dev_addr7, BNO055_CALIB_STAT, calib, 1);
}

HAL_StatusTypeDef BNO055_ReadCalib(I2C_HandleTypeDef *hi2c, uint8_t dev_addr7, uint8_t *calib){
    return BNO055_ReadCalibration(hi2c, dev_addr7, calib);
}

HAL_StatusTypeDef BNO055_ReadQuaternion(I2C_HandleTypeDef *hi2c, uint8_t dev_addr7, bno_quat_t *q){
    uint8_t buf[8];
    HAL_StatusTypeDef ret = bno_read(hi2c, dev_addr7, BNO055_QUATERNION_W_LSB, buf, 8);
    if(ret != HAL_OK) return ret;
    int16_t qw = (int16_t)((buf[1]<<8) | buf[0]);
    int16_t qx = (int16_t)((buf[3]<<8) | buf[2]);
    int16_t qy = (int16_t)((buf[5]<<8) | buf[4]);
    int16_t qz = (int16_t)((buf[7]<<8) | buf[6]);
    const float scale = 1.0f / 16384.0f; // datasheet
    q->w = (float)qw * scale;
    q->x = (float)qx * scale;
    q->y = (float)qy * scale;
    q->z = (float)qz * scale;
    return HAL_OK;
}

HAL_StatusTypeDef BNO055_ReadEuler(I2C_HandleTypeDef *hi2c, uint8_t dev_addr7, bno_euler_t *e){
    uint8_t buf[6];
    HAL_StatusTypeDef ret = bno_read(hi2c, dev_addr7, BNO055_EULER_H_LSB, buf, 6);
    if(ret != HAL_OK) return ret;
    int16_t heading = (int16_t)((buf[1]<<8) | buf[0]);
    int16_t roll    = (int16_t)((buf[3]<<8) | buf[2]);
    int16_t pitch   = (int16_t)((buf[5]<<8) | buf[4]);
    // scale: LSB = 1/16 degree
    const float scale = 1.0f / 16.0f;
    e->heading = (float)heading * scale;
    e->roll    = (float)roll * scale;
    e->pitch   = (float)pitch * scale;
    return HAL_OK;
}
