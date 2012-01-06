#include <avr/io.h>
#include <avr/interrupt.h>

#include "spi.h"
#include "util.h"

/* SPI State */
const void *_spi_lock = 0;
uint8_t _spi_busy = 0;
//Used during transmission
static volatile const uint8_t *_spi_buf;
static volatile const uint8_t *_spi_buf_end;

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

void write_spi(const uint8_t *buf, short len) {
  
    if(len == 0 || _spi_busy) return;
    
    _spi_busy = 1;
    //Send the first byte, the rest will be handled in the ISR
    SPDR = buf[0];
    _spi_buf = buf + 1;
    _spi_buf_end = buf+len;
}

/*
 * ISR to continue sending bytes out SPI
 *
 */
ISR(SPI_STC_vect) {
    if(_spi_buf < _spi_buf_end) {
        SPDR = *_spi_buf++;
    } else {
        _spi_busy = 0;
    }
}

