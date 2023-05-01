#include <stdint.h>
#include "BME280.h"
#include "main.h"

BME280_error BME280_init(BME280* dev, I2C_HandleTypeDef* i2c_handle)
{
    BME280_error error_status;
    dev->i2c_handle = i2c_handle;

    // Check chip ID of sensor, should be 0x60
    error_status = validate_id(dev);
    if (error_status) {
        return error_status;
    } else if (dev->id != 0x60) {
        return E_INCORRECT_ID;
    }

    // Reset sensor
    error_status = reset(dev);
    if (error_status) return error_status;

    error_status = config(dev, OSRS_1x, OSRS_16x, OSRS_4x, NORMAL_MODE, MS_250, IRR_2);
    if (error_status) return error_status;
}

static BME280_error validate_id(BME280* dev)
{
    uint8_t id, hal_status;
    hal_status = HAL_I2C_Mem_Read(dev->i2c_handle, BME280_ADDRESS, BME280_ID_ADDRESS, I2C_MEMADD_SIZE_8BIT, &id, sizeof(id), HAL_MAX_DELAY);
    dev->id = id;
    if (hal_status) return E_I2C;
    return E_SUCCESS;
}


static BME280_error reset(BME280* dev)
{
    uint8_t hal_status = HAL_I2C_Mem_Write(dev->i2c_handle, BME280_ADDRESS, BME280_RESET_ADDRESS, I2C_MEMADD_SIZE_8BIT, BME280_SOFT_RESET, sizeof(BME280_SOFT_RESET), HAL_MAX_DELAY);
    if (hal_status) return E_I2C;
    return E_SUCCESS;
}

static BME280_error config(BME280* dev, Osrs osrs_h, Osrs osrs_t, Osrs osrs_p, PowerMode mode, StandByTime time, FilterIRR irr)
{
    uint8_t hal_status = 0;
    uint8_t data = osrs_h;
    hal_status |= HAL_I2C_Mem_Write(dev->i2c_handle, BME280_ADDRESS, BME280_CTRL_HUM_ADDRESS, I2C_MEMADD_SIZE_8BIT, &data, sizeof(data), HAL_MAX_DELAY);

    data = (osrs_t << 5) | (osrs_p << 2) | mode;
    hal_status |= HAL_I2C_Mem_Write(dev->i2c_handle, BME280_ADDRESS, BME280_CTRL_MEAS_ADDRESS, I2C_MEMADD_SIZE_8BIT, &data, sizeof(data), HAL_MAX_DELAY);
    
    data = (time << 5) | (irr << 2);
    hal_status |= HAL_I2C_Mem_Write(dev->i2c_handle, BME280_ADDRESS, BME280_CONFIG_ADDRESS, I2C_MEMADD_SIZE_8BIT, &data, sizeof(data), HAL_MAX_DELAY);

    if (hal_status) return E_I2C;
    return E_SUCCESS;
}

static BME280_error trim_read(BME280* dev)
{
    uint8_t hal_status, buffer[32];

    hal_status = HAL_I2C_Mem_Read(dev->i2c_handle, BME280_ADDRESS, BME_REGISTER_DIG_1, I2C_MEMADD_SIZE_8BIT, buffer, 24, HAL_MAX_DELAY);
    hal_status |= HAL_I2C_Mem_Read(dev->i2c_handle, BME280_ADDRESS, BME_REGISTER_DIG_2, I2C_MEMADD_SIZE_8BIT, buffer+25, 7, HAL_MAX_DELAY);

    if (hal_status) return E_I2C;

    dev->dig_T1 = buffer[1] << 8 | buffer[0];
    dev->dig_T2 = buffer[3] << 8 | buffer[2];
    dev->dig_T3 = buffer[5] << 8 | buffer[4];

    dev->dig_P1 = buffer[7] << 8 | buffer[6];
    dev->dig_P2 = buffer[9] << 8 | buffer[8];
    dev->dig_P3 = buffer[11] << 8 | buffer[10];
    dev->dig_P4 = buffer[13] << 8 | buffer[12];
    dev->dig_P5 = buffer[15] << 8 | buffer[14];
    dev->dig_P6 = buffer[17] << 8 | buffer[16];
    dev->dig_P7 = buffer[19] << 8 | buffer[18];
    dev->dig_P8 = buffer[21] << 8 | buffer[20];
    dev->dig_P9 = buffer[23] << 8 | buffer[22];

    dev->dig_H1 = buffer[24];
    dev->dig_H2 = buffer[26] << 8 | buffer[25];
    dev->dig_H3 = buffer[27];
    dev->dig_H4 = buffer[29] << 8 | buffer[28];
    dev->dig_H5 = buffer[31] << 8 | buffer[30];

    return E_SUCCESS;
}