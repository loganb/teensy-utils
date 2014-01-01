Introduction
============

Teensy-utils is a collection of useful modules for writing AVR applications in C, and on the Teensy microcontroller in particular. All of this code has been developed against an ATmega32U4, patches are welcome to generalize the code for other Atmel chips. 

Using Modules
=============

To use a module in your code, simply checkout `teesny-utils` into a subdirectory of your project (e.g. `teensy_utils/`), and include the corresponding `.c` file for each module you use in the `SRC` variable of your make file. For example: 

    SRC = $(TARGET).c \
          teensy_utils/timer.c \
          teensy_utils/lock.c


Demo Code
=============

To build demo apps for each module, simply run the associated makefile:

    make -f timer.mk

See corresponding demo file for explanation of the demo program. 

Modules
============

Timer
---------

The timer module provides a real-time counter with 100µs resolution using Timer 3. Applications may use it from within an event loop to execute code on a periodic basis. Applications may poll to determine if their timer has fired and perform an associated action. For example, 

    int main(void)
    {
      timer_t timer1, timer2, timer3;

      sei();
      setup_timer(); //Real-time clock

      init_timer(&timer1, 5000);  //5000 'ticks' == 500ms
      init_timer(&timer2, 10000); 
      init_timer(&timer3, 8000);
  
      while(1) {
        if(timer_fired(&timer1)) {
          LED_TOGGLE; //Toggles every 500ms
        }
    
        if(timer_fired(&timer2)) {
          printf("Fizz"); //Is printed every 1 second
        }
        if(timer_fired(&timer3)) {
          printf("Buzz"); //Is printed every 800ms
        }
      }
    }

The timer has 16-bit precision and thus "rolls over" every 6.55 seconds. timer_fired() must be called every ~3.27 seconds for each timer or firings will be lost. 

Lock
-----------

This module allows different modules to share access to a single resource, e.g. the SPI or USB interface. Modules may then start a long running operation on the interface and release the lock once it is completed. 

API TBD

SPI
------------ 

SPI is a module for using the SPI interface in master mode to transmit data. It allows code to send a complete buffer and poll for the completion

    int main(void) {
      setup_spi(); //Initialize the SPI module

      write_spi("HELLO WORLD", 12); //Writes 12 bytes out the SPI interface, returns immediately
      
      while(spi_busy()); //Spin until the entire buffer has been written out
    }