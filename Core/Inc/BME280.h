#ifndef __BME280__
#define __BME280__

#include <stdint.h>
#include "main.h"

/**
*   GENERAL ADDRESSES
*/
#define BME280_ADDRESS              0x77 << 1
#define BME280_ID_ADDRESS           0xD0
#define BME280_CTRL_HUM_ADDRESS     0xF2
#define BME280_CTRL_MEAS_ADDRESS    0xF4
#define BME280_CONFIG_ADDRESS       0xF5
#define BME280_RESET_ADDRESS        0xE0

/**
*   CALIBRATION REGISTER ADDRESSES
*/
#define BME280_REGISTER_DIG_1      0x88
#define BME280_REGISTER_DIG_2      0xE1

/**
*   COMMANDS
*/
#define BME280_SOFT_RESET    0xB6

/**
*   POWER MODES
*/
typedef enum PowerMode {
    SLEEP_MODE  = 0b00,
    FORCED_MODE = 0b01,
    NORMAL_MODE = 0b11,
} PowerMode;

/**
*   OVERSAMPLING SETTINGS
*/
typedef enum Osrs {
    OSRS_OFF   = 0b000,
    OSRS_1x    = 0b001,
    OSRS_2x    = 0b010,
    OSRS_4x    = 0b011,
    OSRS_8x    = 0b100,
    OSRS_16x   = 0b101,
} Osrs;

/**
*   TIME BETWEEN MEASUREMENTS (DURING NORMAL MODE)
*/
typedef enum StandByTime {
    MS_HALF = 0b000,
    MS_62   = 0b001,
    MS_125  = 0b010,
    MS_250  = 0b011,
    MS_500  = 0b100,
    MS_1000 = 0b101,
    MS_10   = 0b110,
    MS_20   = 0b111,
} StandByTime;

/**
*   IRR FILTER TIME CONSTANT SETTINGS
*/
typedef enum FilterIRR {
    IRR_OFF = 0b000,
    IRR_2   = 0b001,
    IRR_4   = 0b010,
    IRR_8   = 0b011,
    IRR_16  = 0b100,
} FilterIRR;

typedef struct BME280
{
    I2C_HandleTypeDef* i2c_handle;
    uint8_t id;

    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;

    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
} BME280;

typedef enum BME280_error
{
    E_SUCCESS       = 0,
    E_INCORRECT_ID  = 1,
    E_I2C           = 2,
} BME280_error;

/**
*   SENSOR API
*/
BME280_error BME280_init(BME280* dev, I2C_HandleTypeDef* i2c_handle);

/**
*   PRIVATE FUNCTIONS
*/
static BME280_error validate_id(BME280* dev);
static BME280_error reset(BME280* dev);
static BME280_error config(BME280* dev, Osrs osrs_h, Osrs osrs_t, Osrs osrs_p, PowerMode mode, StandByTime time, FilterIRR irr);
static BME280_error trim_read(BME280* dev);

#endif 