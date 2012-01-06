
#include "timer.h"

#include <avr/io.h>


volatile uint16_t timer_ticker;

void setup_timer(void) {
  uint8_t intr_state = SREG;
  
  //Setup
  //TCCR3A = 0; //WGM 1:0 == 0 (CTC Mode)
  TCCR3B = (0 << WGM33) |
           (1 << WGM32) | //CTC Mode, OCR3A is TOP
           (0 << CS32)  |
           (1 << CS31)  |
           (0 << CS30);    //Clk/8, 2M clks/sec

  intr_state = SREG;
  cli();
  OCR3A = 200; //16-bit Write

  TIMSK3 |= (1 << OCIE3A); //Enable OC3A interrupt
  SREG = intr_state;
}

ISR(TIMER3_COMPA_vect) {
  timer_ticker++;
}