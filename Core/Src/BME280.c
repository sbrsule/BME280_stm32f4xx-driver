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
        return id_status;
    } else if (dev->id != 0x60) {
        return E_INCORRECT_ID;
    }

    // Reset sensor
    error_status = reset(dev);
    if (error_status) return error_status;

    error_status = config(dev);
    if (error_status) return error_status;
}

static BME280_error validate_id(BME280* dev)
{
    uint8_t id, hal_status;
    hal_status = HAL_I2C_Mem_Read(dev->i2c_handle, BME280_ADDRESS, BME280_ID_ADDRESS, I2C_MEMADD_SIZE_8BIT, &id, sizeof(id), HAL_MAX_DELAY);
    dev->id = id;
    if (hal_status != HAL_OK) {
        return E_I2C;
    }
    return E_SUCCESS
}

static BME280_error reset(BME280* dev)
{
    uint8_t hal_status = HAL_I2C_Mem_Write(dev->i2c_handle, BME280_ADDRESS, BME280_RESET_ADDRESS, I2C_MEMADD_SIZE_8BIT, BME280_SOFT_RESET, sizeof(BME280_SOFT_RESET), HAL_MAX_DELAY);
    if (hal_status != HAL_OK) return E_I2C;
    return E_SUCCESS;
}

static BME280_error config(BME280* dev, OsrsH osrs_h, OsrsT osrs_t, OsrsP osrs_p, PowerMode mode, StandByTime time, FilterIRR irr)
{
    uint8_t hal_status = 0;
    uint8_t data = OsrsH;
    hal_status |= HAL_I2C_Mem_Write(dev->i2c_handle, BME280_ADDRESS, BME280_CTRL_HUM_ADDRESS, I2C_MEMADD_SIZE_8BIT, &data, sizeof(data), HAL_MAX_DELAY);

    data = OsrsT | OsrsP | PowerMode;
    hal_status |= HAL_I2C_Mem_Write(dev->i2c_handle, BME280_ADDRESS, BME280_CTRL_MEAS_ADDRESS, I2C_MEMADD_SIZE_8BIT, &data, sizeof(data), HAL_MAX_DELAY);
    
    data = time | irr;
    hal_status |= HAL_I2C_Mem_Write(dev->i2c_handle, BME280_ADDRESS, BME280_CONFIG_ADDRESS, I2C_MEMADD_SIZE_8BIT, &data, sizeof(data), HAL_MAX_DELAY);

    if (hal_status) return E_I2C;
    return E_SUCCESS;
}