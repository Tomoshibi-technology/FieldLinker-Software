#include "paa5100je.h"
#include "core_cm7.h"   // for __NOP()
#include <string.h>

// ===== Tunables =====
#define PAA_SPI_TIMEOUT_MS   50
// Datasheet: tSRAD >= 2us, tSWW ~10.5us (sufficiently over-satisfy with a few us)
#define PAA_DELAY_US_READ    3
#define PAA_DELAY_US_WRITE   15

// ===== Local helpers =====
static inline void cs_low(PAA5100JE_Handle *dev)  { HAL_GPIO_WritePin(dev->cs_port,   dev->cs_pin,   GPIO_PIN_RESET); }
static inline void cs_high(PAA5100JE_Handle *dev) { HAL_GPIO_WritePin(dev->cs_port,   dev->cs_pin,   GPIO_PIN_SET);   }

static inline void rst_assert(PAA5100JE_Handle *dev) {
    if (dev->rst_port) HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, GPIO_PIN_SET);
}
static inline void rst_deassert(PAA5100JE_Handle *dev) {
    if (dev->rst_port) HAL_GPIO_WritePin(dev->rst_port, dev->rst_pin, GPIO_PIN_RESET);
}

static void delay_us(uint32_t us) {
    // crude busy-wait; for production, use DWT cycle counter
    uint32_t cycles = (SystemCoreClock / 1000000U) * us / 5U;
    while (cycles--) { __NOP(); }
}

// ===== Low-level SPI R/W =====
HAL_StatusTypeDef PAA5100JE_WriteReg(PAA5100JE_Handle *dev, uint8_t reg, uint8_t val)
{
    HAL_StatusTypeDef st;
    uint8_t tx[2] = { (uint8_t)(reg | 0x80), val };

    cs_low(dev);
    st = HAL_SPI_Transmit(dev->hspi, tx, 2, PAA_SPI_TIMEOUT_MS);
    cs_high(dev);

    delay_us(PAA_DELAY_US_WRITE);
    return st;
}

HAL_StatusTypeDef PAA5100JE_ReadReg(PAA5100JE_Handle *dev, uint8_t reg, uint8_t *val)
{
    HAL_StatusTypeDef st;
    uint8_t tx = reg;
    uint8_t rx = 0;

    cs_low(dev);
    // send address
    st = HAL_SPI_Transmit(dev->hspi, &tx, 1, PAA_SPI_TIMEOUT_MS);
    if (st != HAL_OK) { cs_high(dev); return st; }

    delay_us(PAA_DELAY_US_READ);

    // read data byte
    st = HAL_SPI_Receive(dev->hspi, &rx, 1, PAA_SPI_TIMEOUT_MS);
    cs_high(dev);

    if (st == HAL_OK) *val = rx;
    return st;
}

// ===== API =====
HAL_StatusTypeDef PAA5100JE_SetOrientation(PAA5100JE_Handle *dev, bool invert_x, bool invert_y, bool swap_xy)
{
    uint8_t v = 0;
    if (swap_xy)  v |= PAA_ORIENT_SWAP_XY;
    if (invert_y) v |= PAA_ORIENT_INVERT_Y;
    if (invert_x) v |= PAA_ORIENT_INVERT_X;
    return PAA5100JE_WriteReg(dev, PAA_REG_ORIENTATION, v);
}

uint8_t pid=0, ipid=0;
HAL_StatusTypeDef PAA5100JE_Init(PAA5100JE_Handle *dev)
{
    HAL_StatusTypeDef st;
    uint8_t v;

    // optional hardware reset pulse
    if (dev->rst_port) {
        rst_assert(dev);
        HAL_Delay(2);
        rst_deassert(dev);
        HAL_Delay(5);
    } else {
        HAL_Delay(10);
    }

    // Power_Up_Reset: 0x5A を書き込み（PixArt系共通慣例）
    st = PAA5100JE_WriteReg(dev, PAA_REG_POWER_UP_RESET, 0x5A);
    if (st != HAL_OK) return st;

    // データシート: Reset後 motion 有効まで tMOT-RST ≈ 50ms
    HAL_Delay(50);

    // モーション周辺のレジスタを読んで既存カウントをフラッシュ（Pimoroni例に倣う）
    for (uint8_t i = 0; i < 5; ++i) {
        (void)PAA5100JE_ReadReg(dev, (uint8_t)(PAA_REG_MOTION + i), &v);
    }

    // IDチェック
    st = PAA5100JE_ReadReg(dev, PAA_REG_PRODUCT_ID, &pid); if (st != HAL_OK) return st;
    st = PAA5100JE_ReadReg(dev, PAA_REG_INVERSE_PRODUCT_ID, &ipid); if (st != HAL_OK) return st;
    if (pid != PAA_PRODUCT_ID_VAL || ipid != PAA_INVERSE_PRODUCT_ID_VAL) {
        return HAL_ERROR;
    }

    // 解像度(0x4E)はデフォルト0x14のままでOK（最小テスト）。必要なら変更。
    // ORIENTATION: 0°基準（swapXY+invertX+invertY）に合わせる（Pimoroni例の既定）
    st = PAA5100JE_SetOrientation(dev, true, true, true);
    if (st != HAL_OK) return st;

    // 必要に応じて、Pimoroniの「secret sauce」初期化（多数の隠しレジスタ設定）を移植可能。
    // 最小テストでは省略。安定性が必要なら該当実装を反映してください。:contentReference[oaicite:6]{index=6}

    return HAL_OK;
}

HAL_StatusTypeDef PAA5100JE_ReadMotionBurst(PAA5100JE_Handle *dev, PAA5100JE_Motion *m)
{
    // 連続12バイト: [02,15,03,04,05,06,07,08,09,0A,0C,0B]
    HAL_StatusTypeDef st;
    uint8_t rx[12] = {0};
    uint8_t addr = PAA_REG_MOTION_BURST;

    cs_low(dev);
    // 先にアドレス送出
    st = HAL_SPI_Transmit(dev->hspi, &addr, 1, PAA_SPI_TIMEOUT_MS);
    if (st != HAL_OK) { cs_high(dev); return st; }

    delay_us(PAA_DELAY_US_READ);

    // 12バイト受信
    st = HAL_SPI_Receive(dev->hspi, rx, sizeof(rx), PAA_SPI_TIMEOUT_MS);
    cs_high(dev);
    if (st != HAL_OK) return st;

    m->motion      = rx[0];
    m->observation = rx[1];
    m->dx          = (int16_t)((int16_t)((int8_t)rx[3]) << 8 | rx[2]); // little-endian
    m->dy          = (int16_t)((int16_t)((int8_t)rx[5]) << 8 | rx[4]);
    m->squal       = rx[6];
    m->raw_sum     = rx[7];
    m->raw_max     = rx[8];
    m->raw_min     = rx[9];
    m->shutter     = (uint16_t)(((uint16_t)rx[10] << 8) | rx[11]); // Upper, Lower

    return HAL_OK;
}

HAL_StatusTypeDef PAA5100JE_ReadMotion(PAA5100JE_Handle *dev, int16_t *dx, int16_t *dy)
{
    // 単発読み（MOTION→ΔX/ΔY）。最小限の確認用。
    HAL_StatusTypeDef st;
    uint8_t mot, xl, xh, yl, yh;

    st = PAA5100JE_ReadReg(dev, PAA_REG_MOTION, &mot); if (st != HAL_OK) return st;
    st = PAA5100JE_ReadReg(dev, PAA_REG_DELTA_X_L, &xl); if (st != HAL_OK) return st;
    st = PAA5100JE_ReadReg(dev, PAA_REG_DELTA_X_H, &xh); if (st != HAL_OK) return st;
    st = PAA5100JE_ReadReg(dev, PAA_REG_DELTA_Y_L, &yl); if (st != HAL_OK) return st;
    st = PAA5100JE_ReadReg(dev, PAA_REG_DELTA_Y_H, &yh); if (st != HAL_OK) return st;

    (void)mot; // 必要なら mot&0x80 で動作判定
    *dx = (int16_t)((int16_t)((int8_t)xh) << 8 | xl);
    *dy = (int16_t)((int16_t)((int8_t)yh) << 8 | yl);
    return HAL_OK;
}

HAL_StatusTypeDef PAA5100JE_SetLed(PAA5100JE_Handle *dev, bool on)
{
    HAL_StatusTypeDef st;
    // bank select to 0x14
    st = PAA5100JE_WriteReg(dev, PAA_REG_BANK_SELECT, PAA_BANK_LED); if (st != HAL_OK) return st;
    st = PAA5100JE_WriteReg(dev, PAA_REG_LED_CONTROL, on ? PAA_LED_ON : PAA_LED_OFF); if (st != HAL_OK) return st;
    // back to bank 0
    return PAA5100JE_WriteReg(dev, PAA_REG_BANK_SELECT, PAA_BANK_0);
}
