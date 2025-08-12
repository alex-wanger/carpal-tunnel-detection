#include <avr/io.h>
#include <util/delay.h>
#include "led.h"
#include "millis.h"
#include "debounce.h"
#include "cal.h"
#include "mpu6050_dual.h"
#include "usart.h"
#include "twi.h"
#include "simpson.h"

// 动态数组存储
static simpson_store_t g_store;

static void print_hex8(uint8_t v) {
    const char hex[] = "0123456789ABCDEF";
    usart_transmit(hex[v >> 4]);
    usart_transmit(hex[v & 0x0F]);
}

// --- 串口打印三个动态数组 ---
static void print_dynamic_arrays(void) {
    uint16_t n = simpson_size(&g_store);

    usart_print("\r\n=== Dynamic Arrays ===\r\n");

    usart_print("angle_x: ");
    for (uint16_t i = 0; i < n; i++) { usart_print_float(g_store.x[i], 2); usart_print(", "); }
    usart_print("\r\n");

    usart_print("angle_y: ");
    for (uint16_t i = 0; i < n; i++) { usart_print_float(g_store.y[i], 2); usart_print(", "); }
    usart_print("\r\n");

    usart_print("angle_z: ");
    for (uint16_t i = 0; i < n; i++) { usart_print_float(g_store.z[i], 2); usart_print(", "); }
    usart_print("\r\n======================\r\n");
}

int main(void) {
    led_init();
    millis_init();          // 1ms 计时
    debounce_init();        // PD2 上拉，按下=LOW
    usart_init(115200);
    twi_init();
    simpson_init(&g_store);

    usart_print("Scanning I2C bus...\r\n");
    for (uint8_t addr = 1; addr < 127; addr++) {
        twi_start();
        uint8_t ack = twi_write(addr << 1);
        twi_stop();
        if ((ack & 0xF8) == 0x18) { usart_print(" Found 0x"); print_hex8(addr); usart_print("\r\n"); }
        _delay_ms(20);
    }
    usart_print("Scan done\r\n");

    /***** MPU6050 初始化 *******/
    mpu6050_dual_init();
    usart_print("Sensors initialized\r\n");
    /***** MPU6050 初始化 ************/

    float gx1, gy1, gz1, gx2, gy2, gz2;
    float dt = 1.0f;

    while (1) {
        // 1) 按键一次触发清空数组（去抖）
        if (debounce_pressed_edge()) {
            simpson_clear(&g_store);
            usart_print("Button pressed -> cleared arrays\r\n");
        }

        /******* MPU6050 数据读取 *****/
        mpu6050_dual_read(&gx1, &gy1, &gz1, &gx2, &gy2, &gz2);
        /*** MPU6050 数据读取 ******/

        // 3) 示例：只把 MPU1 数据加入动态数组（需要两组就再加一套 store）
        simpson_append(&g_store, gx1, gy1, gz1);

        /**** MPU6050 数据打印 **/
        usart_print("MPU1 G: ");
        usart_print_float(gx1, 2); usart_print(", ");
        usart_print_float(gy1, 2); usart_print(", ");
        usart_print_float(gz1, 2); usart_print("\r\n");

        usart_print(" | MPU2 G: ");
        usart_print_float(gx2, 2); usart_print(", ");
        usart_print_float(gy2, 2); usart_print(", ");
        usart_print_float(gz2, 2); usart_print("\r\n");
        /***** MPU6050 数据打印 *******/

        print_dynamic_arrays();

        float result_x = simpson_angle_x(&g_store, dt);
        float result_y = simpson_angle_y(&g_store, dt);
        float result_z = simpson_angle_z(&g_store, dt);

        usart_print("Simpson result X/Y/Z: ");
        usart_print_float(result_x, 2); usart_print(", ");
        usart_print_float(result_y, 2); usart_print(", ");
        usart_print_float(result_z, 2); usart_print("\r\n\r\n");

        led_update(result_x, result_y, result_z);

        _delay_ms(1000);
    }
}
