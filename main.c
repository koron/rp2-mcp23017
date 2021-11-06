#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include "onboard_led.h"

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
#   warning rp2-mcp23017 requires a board with I2C pins
#endif

static bool mcp23017_set_regaddr(uint8_t devid, uint8_t reg, bool nostop) {
    int ret;
    ret = i2c_write_blocking(i2c_default, 0x20 | devid, &reg, 1, nostop);
    if (ret != 1) {
        printf("MCP23017: failed to set register address: devid=%d reg=%02x: %d\n", devid, reg, ret);
        return false;
    }
    return true;
}

static bool mcp23017_read(uint8_t devid, uint8_t reg, uint8_t* buf, size_t n) {
    int ret;
    if (!mcp23017_set_regaddr(devid, reg, false)) {
        return false;
    }
    ret = i2c_read_blocking(i2c_default, 0x20 | devid, buf, n, false);
    if (ret != n) {
        printf("MCP23017: failed to read register value: devid=%d reg=%02x: want=%d got=%d\n", devid, reg, n, ret);
        return false;
    }
    return true;
}

static bool mcp23017_write(uint8_t devid, uint8_t reg, const uint8_t* buf, size_t n) {
    int ret;
    if (!mcp23017_set_regaddr(devid, reg, true)) {
        return false;
    }
    ret = i2c_write_blocking(i2c_default, 0x20 | devid, buf, n, false);
    if (ret != n) {
        printf("MCP23017: failed to write register value: devid=%d reg=%02x: want=%d got=%d\n", devid, reg, n, ret);
        return false;
    }
    return true;
}

static bool mcp23017_dump(uint8_t devid, uint8_t reg, size_t n) {
    uint8_t buf[21] = {0};
    if (!mcp23017_read(devid, reg, buf, n)) {
        return false;
    }
    printf("MCP23017: dump %02x %02x:", devid, reg);
    for (size_t i = 0; i < n; i++) {
        printf(" %02x", buf[i]);
    }
    printf("\n");
    return true;
}

static bool mcp23017_iodir_dump() {
    // read IODIR
    printf("*** read IODIR\n");
    if (!mcp23017_dump(0x00, 0x00, 2)) {
        return false;
    }
    if (!mcp23017_dump(0x01, 0x00, 2)) {
        return false;
    }
    return true;
}

static bool mcp23017_gpio_read(uint8_t devid, uint16_t *out) {
    uint8_t data[2] = {0};
    if (!mcp23017_read(devid, 0x12, data, 2)) {
        return false;
    }
    if (out != NULL) {
        *out = (uint16_t)data[0] << 8 | data[1];
    }
    return true;
}

int main() {
    setup_default_uart();
    printf("\nrp2-mcp23017: start\n");

    onboard_led_init();

    i2c_init(i2c_default, 1000 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));


    mcp23017_iodir_dump();

    printf("*** dump GPPU #0\n");
    mcp23017_dump(0, 0x0c, 2);
    mcp23017_dump(1, 0x0c, 2);

    // write GPPU
    printf("*** write GPPU\n");
    const uint8_t data[2] = { 0xff, 0xff };
    mcp23017_write(0, 0x0c, data, 2);
    mcp23017_write(1, 0x0c, data, 2);

    printf("*** dump GPPU #1\n");
    mcp23017_dump(0, 0x0c, 2);
    mcp23017_dump(1, 0x0c, 2);

    uint16_t gpio[2] = {0};

    while(true) {
        uint64_t now = time_us_64();
        onboard_led_task(now);
        for (uint8_t devid = 0; devid < 2; devid++) {
            uint16_t v;
            if (!mcp23017_gpio_read(devid, &v)) {
                printf("GPIO read failure: devid=%d\n", devid);
                return 1;
            }
            if (v != gpio[devid]) {
                gpio[devid] = v;
                printf("changed#%d: %04x (now=%llu)\n", devid, v, now);
            }
        }
    }
}
