#ifndef __BME280__
#define __BME280__

/**
*   GENERAL ADDRESSES
*/
#define BME280_ADDRESS              0x77 << 1
#define BME280_ID_ADDRESS           0xD0
#define BME280_CTRL_HUM_ADDRESS     0xF2
#define BME280_CTRL_MEAS_ADDRESS    0xF4
#define BME280_SOFTRESET_ADDRESS    0xE0
#define BME280_CONFIG_ADDRESS       0xF5
#define BME280_RESET_ADDRESS        0xE0

/**
*   CALIBRATION REGISTER ADDRESSES
*/
#define BME280_REGISTER_DIG_T1      0x88
#define BME280_REGISTER_DIG_T2      0x8A
#define BME280_REGISTER_DIG_T3      0x8C

/**
*   POWER MODES
*/
typedef enum PowerMode {
    SLEEP_MODE  = 0x00;
    FORCED_MODE = 0x01;
    NORMAL_MODE = 0x03;
} PowerMode;

/**
*   HUMIDITY OVERSAMPLING SETTINGS
*/
typdef enum OsrsH {
    H_OFF   = 0b000;
    H_1x    = 0b001;
    H_2x    = 0b010;
    H_4x    = 0b011;
    H_8x    = 0b100;
    H_16x   = 0b101;
} OsrsH;

/**
*   TEMPERATURE OVERSAMPLING SETTINGS
*/
typedef enum OsrsT {
    T_OFF   = 0b000 << 5;
    T_1x    = 0b001 << 5;
    T_2x    = 0b010 << 5;
    T_4x    = 0b011 << 5;
    T_8x    = 0b100 << 5;
    T_16x   = 0b101 << 5;
} OsrsT;

/**
*   PRESSURE OVERSAMPLING SETTINGS
*/
typedef enum OsrsP {
    P_OFF   = 0b000 << 2;
    P_1x    = 0b001 << 2;
    P_2x    = 0b010 << 2;
    P_4x    = 0b011 << 2;
    P_8x    = 0b100 << 2;
    P_16x   = 0b101 << 2;
} OsrsP;

/**
*   TIME BETWEEN MEASUREMENTS (DURING NORMAL MODE)
*/
typedef enum StandByTime {
    MS_HALF = 0b000 << 5;
    MS_62   = 0b001 << 5;
    MS_125  = 0b010 << 5;
    MS_250  = 0b011 << 5;
    MS_500  = 0b100 << 5;
    MS_1000 = 0b101 << 5;
    MS_10   = 0b110 << 5;
    MS_20   = 0b111 << 5;
} StandByTime;

/**
*   IRR FILTER TIME CONSTANT SETTINGS
*/
typdef enum FilterIRR {
    IRR_OFF = 0b000 << 2;
    IRR_2   = 0b001 << 2;
    IRR_4   = 0b010 << 2;
    IRR_8   = 0b011 << 2;
    IRR_16  = 0b100 << 2;
}

typedef struct BME280
{
    I2C_HandleTypeDef* i2c_handle;
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
} BME280;

typedef enum BME280_error
{
    E_SUCCESS       = 0,
    E_INCORRECT_ID  = 1,
    E_I2C           = 2,
} BME280_error;

#endif 