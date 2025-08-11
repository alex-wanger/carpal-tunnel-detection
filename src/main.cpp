#include "mpu6050_dual.h"
#include "usart.h"
#include "twi.h"
#include <util/delay.h>

static void print_hex8(uint8_t v) {
    const char hex[] = "0123456789ABCDEF";
    usart_transmit(hex[v >> 4]);
    usart_transmit(hex[v & 0x0F]);
}

int main(void) {
    usart_init(115200);
    twi_init();

    // I²C scan once
    usart_print("Scanning I2C bus...\r\n");
    for (uint8_t addr = 1; addr < 127; addr++) {
        twi_start();
        uint8_t ack = twi_write(addr << 1);
        twi_stop();
        if ((ack & 0xF8) == 0x18) {
            usart_print(" Found @0x");
            print_hex8(addr);
            usart_print("\r\n");
        }
        _delay_ms(20);
    }
    usart_print("Scan done\r\n\r\n");

    // init sensors
    mpu6050_dual_init();
    usart_print("Sensors initialized\r\n\r\n");

    float gx1,gy1,gz1,gx2,gy2,gz2;
    while (1) {
        usart_print("Reading...\r\n");
        mpu6050_dual_read(&gx1,&gy1,&gz1, &gx2,&gy2,&gz2);

        usart_print("MPU1 G:");
        usart_print_float(gx1,2); usart_print(",");
        usart_print_float(gy1,2); usart_print(",");
        usart_print_float(gz1,2);

        usart_print(" | MPU2 G:");
        usart_print_float(gx2,2); usart_print(",");
        usart_print_float(gy2,2); usart_print(",");
        usart_print_float(gz2,2);
        usart_print("\r\n\r\n");

        _delay_ms(1000);
    }
    return 0;
}
