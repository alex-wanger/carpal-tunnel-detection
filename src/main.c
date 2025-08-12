#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#include "led.h"
#include "timer.h"          // timer0_init() + millis()
#include "debounce.h"
#include "cal.h"
#include "mpu6050_dual.h"
#include "usart.h"
#include "twi.h"
#include "simpson.h"

// 动态数组存储
static simpson_store_t g_store;

// ---- 工具打印 ----
static void print_hex8(uint8_t v) {
    const char hex[] = "0123456789ABCDEF";
    usart_transmit(hex[v >> 4]);
    usart_transmit(hex[v & 0x0F]);
}

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

// ---- I2C 扫描（基于当前 twi_enqueue 驱动） ----
static bool twi_probe_addr(uint8_t addr7) {
    twi_message_t m = {
        .address = TWI_WRITE_ADDRESS(addr7),
        .buffer  = NULL,
        .size    = 0
    };
    twi_enqueue(&m, 1);
    while (!twi_isr.idle) { }
    return (twi_status() == SUCCESS);
}

static void scan_i2c_bus(void) {
    usart_print("Scanning I2C bus...\r\n");
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (twi_probe_addr(addr)) {
            usart_print(" Found 0x"); print_hex8(addr); usart_print("\r\n");
        }
        _delay_ms(20);
    }
    usart_print("Scan done\r\n");
}

// 如果你已有不同签名的读取函数，这里可改为你的函数名
extern void mpu6050_dual_read(float* gx1,float* gy1,float* gz1,
                              float* gx2,float* gy2,float* gz2);

int main(void)
{
    led_init();
    timer0_init();          // 1ms 计时
    debounce_init();        // PD2 上拉，按下=LOW（与你现有实现保持一致）
    usart_init(115200);
    twi_init(100000);       // 100kHz
    simpson_init(&g_store);

    scan_i2c_bus();

    // ***** MPU6050 初始化 *****
    mpu6050_dual_t mpus;
    mpu6050_dual_init(&mpus, MPU6050_ADDR_1, MPU6050_ADDR_2);
    mpu6050_dual_config(&mpus);
    usart_print("Sensors initialized\r\n\r\n");
    // ***** MPU6050 初始化 *****

    float gx1, gy1, gz1, gx2, gy2, gz2;
    float dt = 1.0f;

    while (1) {
        // 1) 按键一次触发清空数组（去抖）
        if (debounce_pressed_edge()) {
            simpson_clear(&g_store);
            usart_print("Button pressed -> cleared arrays\r\n");
        }

        // ****** MPU6050 数据读取 ******
        mpu6050_dual_read(&gx1, &gy1, &gz1, &gx2, &gy2, &gz2);

        simpson_append(&g_store, gx1, gy1, gz1);

        // **** MPU6050 数据打印 ****
        usart_print("MPU1 G: ");
        usart_print_float(gx1, 2); usart_print(", ");
        usart_print_float(gy1, 2); usart_print(", ");
        usart_print_float(gz1, 2); usart_print("\r\n");

        usart_print(" MPU2 G: ");
        usart_print_float(gx2, 2); usart_print(", ");
        usart_print_float(gy2, 2); usart_print(", ");
        usart_print_float(gz2, 2); usart_print("\r\n");
        // **** MPU6050 数据打印 ****

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
