#ifndef TWI_H
#define TWI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void twi_init(void);
void twi_start(void);
void twi_stop(void);
uint8_t twi_write(uint8_t data);
void twi_writeReg(uint8_t slave_addr, uint8_t reg, uint8_t data);
uint8_t twi_readAck(void);
uint8_t twi_readNack(void);
void twi_readRegs(uint8_t slave_addr, uint8_t reg, uint8_t *buf, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif // TWI_H
