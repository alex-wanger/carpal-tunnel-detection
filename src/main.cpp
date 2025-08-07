#include "mpu6050_dual.h"
#include "usart.h"
#include <util/delay.h>

int main(void) {
    usart_init(115200);
    //usart_print("1: after usart_init\r\n");

    mpu6050_dual_init();
    //usart_print("2: after mpu6050_dual_init\r\n");

    float gx1, gy1, gz1, gx2, gy2, gz2;
    //usart_print("3: after variable decl\r\n");

    while (1) {
        //usart_print("4: before read\r\n");
        mpu6050_dual_read(&gx1, &gy1, &gz1, &gx2, &gy2, &gz2);
        //usart_print("5: after read\r\n");

        usart_print("MPU1 G:");
        usart_print_float(gx1,2); usart_print(",");
        usart_print_float(gy1,2); usart_print(",");
        usart_print_float(gz1,2);
/*
        usart_print(" | MPU2 G:");
        usart_print_float(gx2,2); usart_print(",");
        usart_print_float(gy2,2); usart_print(",");
        usart_print_float(gz2,2);*/

        usart_print("\r\n");
        _delay_ms(1000);
    }
    return 0;
}
