#include <bme280/bme280.h>

#define BME280_DOUBLE_ENABLE
#include <BME280_SensorAPI/bme280.h>

#include <StomaSense/defs.h>

#include <hardware/i2c.h>

using namespace BME280;

// i2c callbacks ////////
// typedef BME280_INTF_RET_TYPE (*bme280_read_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
static BME280_INTF_RET_TYPE bme280_read_i2c(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    // int i2c_read_blocking_until (i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop, absolute_time_t until)
    const bool nonstop = false;               // if false, release bus
    const uint timeout_us = 10 * 1000 * 1000; // 10 s

    // TODO: check if when FreeRTOS this line must be critical
    int rslt = i2c_read_timeout_us((i2c_inst_t *)intf_ptr, reg_addr, reg_data, len, nonstop, timeout_us);

    if (rslt == PICO_ERROR_TIMEOUT)
        return -1;
    else if (rslt == PICO_ERROR_GENERIC)
        return -2;
    else if (rslt != len)
        return -3;
    return BME280_INTF_RET_SUCCESS;
}

// typedef BME280_INTF_RET_TYPE (*bme280_write_fptr_t)(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
static BME280_INTF_RET_TYPE bme280_write_i2c(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    // int i2c_write_blocking_until (i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop, absolute_time_t until)
    const bool nonstop = false;               // if false, release bus
    const uint timeout_us = 10 * 1000 * 1000; // 10 s

    // TODO: check if when FreeRTOS this line must be critical
    int rslt = i2c_write_timeout_us((i2c_inst_t *)intf_ptr, reg_addr, reg_data, len, nonstop, timeout_us);

    if (rslt == PICO_ERROR_TIMEOUT)
        return -1;
    else if (rslt == PICO_ERROR_GENERIC)
        return -2;
    else if (rslt != len)
        return -3;
    return BME280_INTF_RET_SUCCESS;
}

// typedef void (*bme280_delay_us_fptr_t)(uint32_t period, void *intf_ptr);
static void bme280_delay_us_i2c(uint32_t period_us, void *intf_ptr)
{
    sleep_us(period_us);
}

static bme280_dev bme = {
    .intf = BME280_I2C_INTF,
    .intf_ptr = i2c0,
    .read = bme280_read_i2c,
    .write = bme280_write_i2c,
    .delay_us = bme280_delay_us_i2c};
const bme280_settings settings = {
    .osr_p = BME280_NO_OVERSAMPLING,
    .osr_t = BME280_OVERSAMPLING_1X,
    .osr_h = BME280_OVERSAMPLING_1X,
    .filter = BME280_FILTER_COEFF_OFF,
    .standby_time = BME280_STANDBY_TIME_1000_MS};
static uint32_t max_delay_us = 1000000;
static StomaSense::time_ms_t last_read_time = 0;

bool sleep()
{
    int8_t res = bme280_set_sensor_mode(BME280_POWERMODE_SLEEP, &bme);
    if (res > 0)
    {
        STOMASENSE_WARN("bme280 sleep with warning %i\n", res);
    }
    else if (res < 0)
    {
        STOMASENSE_WARN("bme280 sleep error %i\n", res);
    }

    return res >= 0;
}

bool wake_up()
{
    int8_t res = bme280_set_sensor_mode(BME280_POWERMODE_FORCED, &bme);
    if (res > 0)
    {
        STOMASENSE_WARN("bme280 wake up with warning %i\n", res);
    }
    else if (res < 0)
    {
        STOMASENSE_WARN("bme280 wake up error %i\n", res);
    }

    return res >= 0;
}

bool reset()
{
    int8_t res = bme280_soft_reset(&bme);
    if (res > 0)
    {
        STOMASENSE_WARN("bme280 reset with warning %i\n", res);
    }
    else if (res < 0)
    {
        STOMASENSE_WARN("bme280 reset error %i\n", res);
    }

    return res >= 0;
}

static inline bool init()
{
    int8_t res = bme280_init(&bme);
    if (res > 0)
    {
        STOMASENSE_WARN("bme280 init with warning %i\n", res);
    }
    else if (res < 0)
    {
        STOMASENSE_WARN("bme280 init error %i\n", res);
    }

    return res >= 0;
}

static inline bool set_settings()
{
    // best settings for humidity sampling
    // https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
    // page 20

    int8_t res = bme280_set_sensor_settings(
        BME280_SEL_ALL_SETTINGS,
        &settings,
        &bme);

    if (res > 0)
    {
        STOMASENSE_WARN("bme280 set settings with warning %i\n", res);
    }
    else if (res < 0)
    {
        STOMASENSE_WARN("bme280 set settings error %i\n", res);
    }

    return res >= 0;
}

static inline bool set_meas_delay()
{
    int8_t res = bme280_cal_meas_delay(&max_delay_us, &settings);

    if (res > 0)
    {
        STOMASENSE_WARN("bme280 meas_delay with warning %i\n", res);
    }
    else if (res < 0)
    {
        STOMASENSE_WARN("bme280 meas_delay error %i\n", res);
    }

    return res >= 0;
}

bool begin()
{
    if (!init())
        return false;
    if (!set_settings())
        return false;
    if (!set_meas_delay())
        return false;
    return true;
}

bool can_read()
{
    return has_timer_elapsed(millis(), last_read_time, max_delay_us / 1000);
}

bool read(float *hum, float *temp, float *pres)
{
    if (!can_read())
    {
        STOMASENSE_WARN("can't read bme280. have to wait a bit");
        return false;
    }

    bme280_data data;
    int8_t res = bme280_get_sensor_data(BME280_ALL, &data, &bme);

    if (res > 0)
    {
        STOMASENSE_WARN("bme280 read with warning %i\n", res);
    }
    else if (res < 0)
    {
        STOMASENSE_WARN("bme280 read error %i\n", res);
    }

    if (res >= 0)
    {
        *hum = static_cast<float>(data.humidity);
        *temp = static_cast<float>(data.temperature);
        *pres = static_cast<float>(data.pressure);
    }

    return res >= 0;
}