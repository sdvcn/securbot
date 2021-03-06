/**
 * @file ADS1015.cpp
 * @author your Cedric Godin (cedric.godin@me.com)
 * @brief Driver for the ADS1015 ADC
 * @version 0.1
 * @date 2019-10-02
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "chip/ADS1015.hpp"

#define CONFIG_REG_ADR 1    // Config register P value
#define CONV_REG_ADR 0      // Conversion register P value

esp_err_t ADS1015::configure()
{
    // Set address pointer to configuration register
    address_pointer_register_map map;
    map.fields.P = CONFIG_REG_ADR;

    // Write to config register
    return writeRegister(map, _config.bytes);
}

esp_err_t ADS1015::startReading(uint8_t channel_number)
{
    // Set mux to proper channel
    _config.fields.MUX_channel = channel_number;
    _config.fields.OS = 1;

    // Write config
    return configure();
}

esp_err_t ADS1015::isReading(bool &value)
{
    // Set address pointer to config register
    address_pointer_register_map map;
    map.fields.P = CONFIG_REG_ADR;

    config_register_map config; // Temp config to store red value

    // read config
    esp_err_t ret = readRegister(map, config.bytes);

    // extract OS bit and NOT (OS low is currently reading)
    value = !(bool)config.fields.OS;
    
    return ret;
}

esp_err_t ADS1015::getValue(float &value)
{
    // Set address pointer to conversion register
    address_pointer_register_map map;
    map.fields.P = CONV_REG_ADR;

    conversion_register_map conversion; // Temp conversion to store raw

    // read conversion
    esp_err_t ret = readRegister(map, conversion.bytes);

    float FS = 4.096;  // scale is -4.096 to 4.096 volts
    
    // convert to volt with full scale range and raw value
    value =  FS * (float)conversion.fields.D / 2048.0;

    return ret;
}

esp_err_t ADS1015::getConfig(ADS1015::config_register_map &map)
{
    // Set address pointer to configuration register
    address_pointer_register_map adr;
    adr.fields.P = CONFIG_REG_ADR;

    return readRegister(adr, map.bytes);
}

esp_err_t ADS1015::writeRegister(ADS1015::address_pointer_register_map address, uint8_t value[2])
{
    uint8_t buffer[3];

    buffer[0] = address.bytes[0];   // Write register map
    buffer[1] = value[1];   // Send MSB first
    buffer[2] = value[0];   // Then LSB

    return _i2c->write(_i2c_address, buffer, 3);
}

esp_err_t ADS1015::readRegister(ADS1015::address_pointer_register_map address, uint8_t value[2])
{
    uint8_t buffer[2];

    esp_err_t ret;

    ret = _i2c->write(_i2c_address, address.bytes, 1);    // Request config register
    if (ret != ESP_OK)  // Writing address failed so no need to read
    {
        return ret;
    }
    ret = _i2c->read(_i2c_address, buffer, 2);    // Read register

    value[1] = buffer[0];   // MSB received first
    value[0] = buffer[1];   // Then LSB

    return ret;
}

ADS1015::ADS1015(i2c_port_t i2c_bus, uint8_t i2c_address)
{
    // i2c bus and address
    _i2c_address = i2c_address;
    _i2c = I2C::instance(i2c_bus);

    // default configuration
    _config.fields.OS = 0;          // sleep
    _config.fields.MUX_diff = 1;    // single ended
    _config.fields.MUX_channel = 0; // AIN0 - GND
    _config.fields.PGA = 0b001;     // +- 4.096V
    _config.fields.MODE = 1;        // single shot
    _config.fields.DR = 0b100;      // 1600 SPS
    _config.fields.COMP_MODE = 0;   // traditional comparator
    _config.fields.COMP_POL = 0;    // active low
    _config.fields.COMP_LAT = 0;    // nonlatching
    _config.fields.COMP_QUE = 0b11; // disable comparator ALERT/RDY high-Z
}