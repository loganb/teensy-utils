#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>
#include <avr/interrupt.h>

typedef struct _timer {
  uint16_t interval; //# ticks (100µs) between firing 
  uint16_t last;
} timer_t;

extern volatile uint16_t timer_ticker;

static inline uint16_t read_ticker(void) {
  uint8_t intr_state = SREG;
  cli();
  uint16_t current_tick = timer_ticker;
  SREG = intr_state;
  return current_tick;
}

/******
 *
 * Returns true if there have been at least timer->interval ticks since the
 * last time tick() was called, false otherwise
 *
 * WARNING: Must be called at least once before timer wraps around (6.55 seconds)
 *
 */
static inline uint8_t timer_fired(timer_t *timer) {
  uint16_t new_time = read_ticker();
  
  if((uint16_t)(new_time - timer->last) > timer->interval) {
    timer->last += timer->interval;
    return 1;
  } else {
    return 0;
  }
}


/*****
 *
 * Initializes a timer_t object to start firing in the given interval
 */
static inline void init_timer(timer_t *timer, uint16_t interval) {
  timer->last = read_ticker();
  timer->interval = interval;
}

/******
 *
 * Returns the number of ticks since the last time the method was called.
 * since is updated to the current tick, so that an immediate subsequent 
 * call will return 0
 */
static inline uint16_t ticks_elapsed(uint16_t *since) {
  uint16_t cur = read_ticker(),
           ret = cur - *since;

  *since = cur;
  return ret;
}

/********
 *
 * Initializes Counter3 and OCR3A to run a timer with a 100µs tick
 *
 */
void setup_timer(void);

#endif