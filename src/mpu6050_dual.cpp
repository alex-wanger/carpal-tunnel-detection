#include "mpu6050_dual.h"
#include "twi.h"
#include <util/delay.h>
#include <avr/io.h>
#include <util/twi.h>

// register addresses
#define REG_PWR_MGMT1    0x6B
#define REG_SMPLRT_DIV   0x19
#define REG_CONFIG       0x1A
#define REG_GYRO_CONFIG  0x1B
#define REG_GYRO_XOUT_H  0x43

static const float GYRO_SENS_250DPS = 131.0f;
#define I2C_TIMEOUT 1000

static bool writeRegAck(uint8_t addr, uint8_t reg, uint8_t data) {
    twi_start();
    if ((twi_write((addr<<1)|0) & 0xF8) != TW_MT_SLA_ACK) { twi_stop(); return false; }
    twi_write(reg);
    twi_write(data);
    twi_stop();
    return true;
}

static bool readRegsAck(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len) {
    uint16_t cnt;
    twi_start();
    if ((twi_write((addr<<1)|0) & 0xF8) != TW_MT_SLA_ACK) { twi_stop(); return false; }
    twi_write(reg);
    twi_start();
    if ((twi_write((addr<<1)|1) & 0xF8) != TW_MR_SLA_ACK) { twi_stop(); return false; }
    for (uint8_t i = 0; i < len; i++) {
        cnt = 0;
        if (i < len-1) TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
        else           TWCR = (1<<TWINT)|(1<<TWEN);
        while (!(TWCR & (1<<TWINT))) {
            if (++cnt > I2C_TIMEOUT) { twi_stop(); return false; }
        }
        buf[i] = TWDR;
    }
    twi_stop();
    return true;
}

void mpu6050_dual_init(void) {
    twi_init();
    _delay_ms(100);

    // wake
    writeRegAck(MPU6050_ADDR1, REG_PWR_MGMT1, 0x00);
    writeRegAck(MPU6050_ADDR2, REG_PWR_MGMT1, 0x00);
    _delay_ms(50);

    // ±250°/s
    writeRegAck(MPU6050_ADDR1, REG_GYRO_CONFIG, 0x00);
    writeRegAck(MPU6050_ADDR2, REG_GYRO_CONFIG, 0x00);
    _delay_ms(50);

    // sample rate 50Hz
    writeRegAck(MPU6050_ADDR1, REG_SMPLRT_DIV, 19);
    writeRegAck(MPU6050_ADDR2, REG_SMPLRT_DIV, 19);

    // DLPF = 5Hz
    writeRegAck(MPU6050_ADDR1, REG_CONFIG, 6);
    writeRegAck(MPU6050_ADDR2, REG_CONFIG, 6);
    _delay_ms(50);
}

void mpu6050_dual_read(float *gx1, float *gy1, float *gz1,
                       float *gx2, float *gy2, float *gz2) {
    uint8_t buf[6];
    int16_t raw[3];

    if (readRegsAck(MPU6050_ADDR1, REG_GYRO_XOUT_H, buf, 6)) {
        for (int i = 0; i < 3; i++)
            raw[i] = (int16_t)((buf[2*i]<<8)|(buf[2*i+1]));
        *gx1 = raw[0]/GYRO_SENS_250DPS;
        *gy1 = raw[1]/GYRO_SENS_250DPS;
        *gz1 = raw[2]/GYRO_SENS_250DPS;
    } else *gx1=*gy1=*gz1=0.0f;

    if (readRegsAck(MPU6050_ADDR2, REG_GYRO_XOUT_H, buf, 6)) {
        for (int i = 0; i < 3; i++)
            raw[i] = (int16_t)((buf[2*i]<<8)|(buf[2*i+1]));
        *gx2 = raw[0]/GYRO_SENS_250DPS;
        *gy2 = raw[1]/GYRO_SENS_250DPS;
        *gz2 = raw[2]/GYRO_SENS_250DPS;
    } else *gx2=*gy2=*gz2=0.0f;
}
