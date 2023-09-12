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
    dev->hum_meas = data;
    hal_status |= HAL_I2C_Mem_Write(dev->i2c_handle, BME280_ADDRESS, BME280_CTRL_HUM_ADDRESS, I2C_MEMADD_SIZE_8BIT, &data, sizeof(data), HAL_MAX_DELAY);

    data = (osrs_t << 5) | (osrs_p << 2) | mode;
    dev->ctrl_meas = data;
    hal_status |= HAL_I2C_Mem_Write(dev->i2c_handle, BME280_ADDRESS, BME280_CTRL_MEAS_ADDRESS, I2C_MEMADD_SIZE_8BIT, &data, sizeof(data), HAL_MAX_DELAY);
    
    data = (time << 5) | (irr << 2);
    dev->config = data;
    hal_status |= HAL_I2C_Mem_Write(dev->i2c_handle, BME280_ADDRESS, BME280_CONFIG_ADDRESS, I2C_MEMADD_SIZE_8BIT, &data, sizeof(data), HAL_MAX_DELAY);

    if (hal_status) return E_I2C;
    return E_SUCCESS;
}

static BME280_error trim_read(BME280* dev)
{
    uint8_t hal_status, buffer[33];

    hal_status = HAL_I2C_Mem_Read(dev->i2c_handle, BME280_ADDRESS, BME280_REGISTER_DIG_1, I2C_MEMADD_SIZE_8BIT, buffer, 24, HAL_MAX_DELAY);
    hal_status |= HAL_I2C_Mem_Read(dev->i2c_handle, BME280_ADDRESS, BME280_REGISTER_DIG_2, I2C_MEMADD_SIZE_8BIT, buffer+24, 9, HAL_MAX_DELAY);

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
    dev->dig_H6 = buffer[32];

    return E_SUCCESS;
}

static BME280_error read_temperature(BME280* dev)
{
    uint8_t mode, hal_status;
    hal_status = HAL_I2C_Mem_Read(dev->i2c_handle, BME280_ADDRESS, BME280_CTRL_MEAS_ADDRESS, I2C_MEMADD_SIZE_8BIT, &mode, sizeof(mode), HAL_MAX_DELAY);
    if (hal_status) return E_I2C;

    // Check if temperature sampling is turned off
    if (dev->ctrl_meas & (0b111 << 5) == 0b000) {
        dev->temperature = 0.0;
        return E_SUCCESS;
    }

    float temp;
    int32_t adc_T, var1, var2, t_fine;
    uint8_t buffer[3], shift;

    hal_status = HAL_I2C_Mem_Read(dev->i2c_handle, BME280_ADDRESS, BME280_TEMPDATA_ADDRESS, I2C_MEMADD_SIZE_8BIT, buffer, sizeof(buffer), HAL_MAX_DELAY);
    adc_T = buffer[0] << 18 | buffer[1] << 8 | buffer[2];

    // Determine size of temperature readout depending on oversampling if IRR filter is off
    if (dev->config & (0b111 << 2) == 0b00) {
        shift = 24 - (16 + (dev->ctrl_meas & (0b11)));
    } else {
        shift = 4;
    } 

    adc_T >>= shift;
    if (hal_status) return E_I2C;

    var1 = ((((adc_T >> 3) - ((int32_t)dev->dig_T1 << 1))) * ((int32_t)dev->dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dev->dig_T1)) * ((adc_T >> 4) - ((int32_t)dev->dig_T1))) >> 12) * ((int32_t)dev->dig_T3)) >> 14;
    t_fine = var1 + var2;
    dev->t_fine = t_fine;

    temp = (float)((t_fine * 5 + 128) / 256) / 100;

    #ifdef CELSIUS
    dev->temperature = temp;
    #elseif FAHRENHEIT
    dev->temperature = (temp * 9 / 5) + 32;
    #elseif KELVIN
    dev->temperature = temp - 273.15;
    #endif

    return E_SUCCESS;
}

static BME280_error read_humidity(BME280* dev)
{
    // If humidity or temperature sampling is off, skip measuring humidity
    if ((dev->hum_meas == 0b000) || (dev->ctrl_meas & (0b111 << 5) == 0b000)) {
        dev->humidity = 0.0;
        return E_SUCCESS;
    }

    uint32_t adc_H, var1, var2, humidity;
    uint8_t buffer[2], hal_status;

    hal_status = HAL_I2C_Mem_Read(dev->i2c_handle, BME280_ADDRESS, BME280_HUMDATA_ADDRESS, I2C_MEMADD_SIZE_8BIT, buffer, sizeof(buffer), HAL_MAX_DELAY);
    if (hal_status) return E_I2C;
    adc_H = buffer[0] << 8 | buffer[1];

    humidity = dev->t_fine - 76800;
    humidity = (((((adc_H << 14) - (((int32_t)dev->dig_H4) << 2) -(((int32_t)dev->dig_H5) * humidity)) + ((int32_t)16384)) >> 15) * 
                ((((((humidity * ((int32_t)dev->dig_H6)) >> 10) * (((humidity * ((int32_t)dev->dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + 
                ((int32_t)2097152)) * ((int32_t)dev->dig_H2) + 8192) >> 14);
    humidity = (humidity - (((((humidity >> 15) * (humidity >> 15)) >> 7) * ((int32_t)dev->dig_H1)) >> 4));
    humidity = (humidity < 0 ? 0 : humidity);
    humidity = (humidity > 419430400 ? 419430400 : humidity);
    dev->humidity = (float)humidity / 1024;

    return hal_status;
}

static BME280_error read_pressure(BME280* dev)
{
    if (dev->ctrl_meas & (0b11 << 2) == 0b00) {
      dev->pressure = 0.0;
      return E_SUCCESS;
    }

    int64_t var1, var2, pressure;
    int32_t adc_P;
    uint8_t buffer[3], hal_status, shift;

    hal_status = HAL_I2C_Mem_Read(dev->i2c_handle, BME280_ADDRESS, BME280_PRESSDATA_ADDRESS, I2C_MEMADD_SIZE_8BIT, buffer, sizeof(buffer), HAL_MAX_DELAY);
    if (hal_status) return E_I2C;

    // Determine size of pressure readout depending on oversampling if IRR filter is off
    if (dev->config & (0b111 << 2) == 0b00) {
        shift = 24 - (16 + (dev->ctrl_meas & (0b11)));
    } else {
        shift = 4;
    } 

    adc_P = buffer[0] << 18 | buffer[1] << 8 | buffer[2];
    adc_P >>= shift;


    var1 = ((int64_t)dev->t_fine) - 128000;
    var2 = var1 * var1 * ((int64_t)dev->dig_P6); 
    var2 = var2 + ((var1 * (int64_t)dev->dig_P5) << 17);
    var2 = var2 + (((int64_t)dev->dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dev->dig_P3) >> 8) + ((var1 * (int64_t)dev->dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dev->dig_P1) >> 33;

    if (var1 == 0) {
        dev->pressure = 0.0;
        return E_OTHER;
    }

    pressure = 1048576 - adc_P;
    pressure = (((pressure << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dev->dig_P9) * (pressure >> 13) * (pressure >> 13)) >> 25;
    var2 = (((int64_t)dev->dig_P8) * pressure) >> 19;
    pressure = ((pressure + var1 + var2) >> 8) + (((int64_t)dev->dig_P7) << 4);

    dev->pressure = (float)pressure;
    return E_SUCCESS;
}

BME280_error BME280_measure(BME280* dev)
{
  read_temperature(dev); 
  read_humidity(dev);
  read_pressure(dev);
}

