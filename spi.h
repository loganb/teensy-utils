#ifndef __SPICONFIG_H_
#define __SPICONFIG_H_

#include <stdint.h>

#include "lock.h"

/*
 *
 */
extern lock_t _spi_lock;
extern uint8_t _spi_busy;

/*
 * Used to define a peripheral, notably its SS-pin. Create one of these in PROGMEM.
 *
 * Values:
 * port_register: port register containing the SS-bar pin for this peripheral
 * pin_mask: bit mask with a single bit set for the SS-bar pin
 *
 * If you want to manually control SS-bar, set port_register to NULL. 
 * 
 */
typedef struct _spi_peripheral_cfg {
	volatile uint8_t *port_register; //The PORTx register containing the SS-pin
	uint8_t           pin_mask; //Should be, e.g., 0b00001000 if pin 3 is the SS-pin

	//TODO: Speed/timing config too
} spi_peripheral_cfg; 

/*
 * Sets up the SPI interface as a master, full speed
 * Setup on the negedge
 */

void setup_spi(void);

/*
 * Returns a true-ish value if SPI is used by the module of the specified lock_val
 * Returns true if SPI is possessed by the given lock_val.
 *
 * A good convention for lock_val is to use a pointer to the peripheral's 
 * configuration struct in PROGMEM
 */
static inline uint8_t acquire_spi(const spi_peripheral_cfg *lock_val) {
  return acquire_lock(&_spi_lock, lock_val);
}

static inline uint8_t has_spi(const spi_peripheral_cfg *lock_val) {
  return has_lock(&_spi_lock, lock_val);
}

// Current holder of the SPI lock
static inline const spi_peripheral_cfg *current_spi(void) {
	return (const spi_peripheral_cfg *)lock_ptr(&_spi_lock);
}

static inline void release_spi(void) {
  release_lock(&_spi_lock);
}


//uint8_t write_spi_byte_nb(spi_buf_t *sb);

/*
 * Writes a buffer out the SPI interface asynchronously, returns immediately
 */
void write_spi(const uint8_t *buf, short len);

/*
 * Same as write_spi(), but additionally the buffer contents are overwritten 
 * with data read in from the SPI device. 
 */
void rw_spi(const uint8_t *buf, short len);

/*
 * Attempts to acquire the SPI lock for the given peripheral, lowers the 
 * configured SS pin, executes rw_spi(…), and releases the lock at the end.
 * A future call to has_spi(cfg) will return false once the transfer is 
 * complete. 
 *
 * Returns truthy if the transfer was initiated, falsy otherwise (and it 
 * should be retried later).
 */
uint8_t async_rw_spi(const spi_peripheral_cfg *cfg, uint8_t *restrict buf, short len);

/*
 * Spins until it can acquire the SPI lock for the given peripheral, lowers the 
 * configured SS pin, executes rw_spi(…), and releases the lock at the end.
 *
 * Returns once the transfer is complete. 
 */
void sync_rw_spi(const spi_peripheral_cfg *cfg, uint8_t *restrict buf, short len); 

/*
 * True if the SPI interface is busy sending a buf
 */
static inline uint8_t spi_busy(void) {
    return _spi_busy;
}

//void reset_latch(void);

//void set_latch(void);

#endif