#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "spi.h"
#include "utils.h"

typedef struct {
  uint8_t lock_is_cfg : 1; //True if the lock value is an spi_peripheral_cfg pointer
  uint8_t is_transfer : 1; //True if we're doing a transfer (i.e. write+read), false if just a write
  uint8_t control_ss  : 1; //True if the ISR should de-assert SS when transfer is done
} spi_mode_t;

/* SPI State */
lock_t  _spi_lock;
volatile spi_mode_t _spi_mode = {
  .lock_is_cfg = 0,
  .is_transfer = 0,
  .control_ss  = 0
};

//Used during transmission
static uint8_t *volatile _spi_buf; //Pointer to the byte currently being written out
static const uint8_t *volatile _spi_buf_end;

void setup_spi() {
  //Make PB2 (MOSI) and PB1 (SCLK) outputs
  set_bits(DDRB, 0b00000110,0b00000110);
  set_bits(PORTB,0b00000001,0b00000001); //Activate SS-bar's pullup resistor
  
  //Enable SPI
  SPCR = (1 << SPIE) | //Interrupts
         (1 << SPE)  | //Global enable
         (0 << DORD) | //MSB First
         (1 << MSTR) | //Master Mode
         (0 << CPOL) |
         (0 << CPHA) |
         (0 << SPR1) | //f/4
         (1 << SPR0);
  SPSR = 0b00000000;  //No 2x
}

/*void write_spi(const uint8_t *buf, short len) {
  
    if(len == 0 || _spi_busy) return;
    
    _spi_busy = 1;
    //Send the first byte, the rest will be handled in the ISR
    SPDR = buf[0];
    _spi_buf = buf + 1;
    _spi_buf_end = buf+len;
}*/

void sync_rw_spi(const spi_peripheral_cfg *cfg, uint8_t *restrict buf, short len) {
  while(!acquire_spi(cfg)); //Spin until the SPI is locked

  async_rw_spi(cfg, buf, len); //Guaranteed to succeed since we have the lock

  while(has_spi(cfg)); //Spin until the async operation is complete
}

uint8_t async_rw_spi(const spi_peripheral_cfg *cfg, uint8_t *restrict buf, short len) {
  if(!acquire_spi(cfg)) return 0; //SPI is busy

  if(len == 0) { //This is pretty dumb, but completeness…
    release_spi();
    return 1;
  }

  volatile uint8_t *ss_reg = (volatile uint8_t*)WORD_FIELD(cfg, port_register);
  uint8_t ss_mask          = (uint8_t)BYTE_FIELD(cfg, pin_mask);

  //Pull the SS-pin low, pre-transfer
  if(ss_reg != NULL) *ss_reg &= ~ss_mask;

  //Setup the spi_mode
  spi_mode_t tmp = {
    .is_transfer = 1,
    .lock_is_cfg = 1,
    .control_ss  = 1
  };
  _spi_mode = tmp;

  //Initiate transfer
  _spi_buf = buf;
  _spi_buf_end = buf+len;
  SPDR = buf[0];

  return 1;
}


/*
 * ISR to continue sending bytes out SPI
 *
 */
ISR(SPI_STC_vect) {
  uint8_t       *buf     = _spi_buf;
  const uint8_t *buf_end = _spi_buf_end;
  spi_mode_t     mode    = _spi_mode;
  
  if(mode.is_transfer) {
    *buf = SPDR; //During transfers, we replace the buf with the data we read in
  }
  buf++; //buf points to the byte just sent, so incr before testing
  if(buf < buf_end) {
      SPDR = *buf;
  } else {
    //Read the config out of the lock value and deassert the pin
    if(mode.lock_is_cfg && mode.control_ss) {
      const spi_peripheral_cfg *cfg = current_spi();
      volatile uint8_t *ss_reg = (volatile uint8_t*)WORD_FIELD(cfg, port_register);
      uint8_t ss_mask          = (uint8_t)BYTE_FIELD(cfg, pin_mask);
      if(ss_reg != NULL) *ss_reg |= ss_mask;
    }
    release_spi(); //Indicate that we're done with the SPI
  }
  _spi_buf = buf;
}

