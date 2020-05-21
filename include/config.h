#ifndef CONFIG_H
#define CONFIG_H
//----------------------------------------------------------------------------
#include <avr/io.h>
//----------------------------------------------------------------------------
//#define BEEPER_PORT         PORTC
#define BEEPER_BIT          2

#define ENCODER_A_PORT      PORTB
#define ENCODER_A_PIN       PINB
#define ENCODER_A_BIT       3

#define ENCODER_B_PORT      PORTB
#define ENCODER_B_PIN       PINB
#define ENCODER_B_BIT       4

#define BUTTON_PORT         PORTB
#define BUTTON_PIN          PINB
#define BUTTON_BIT          5

#define ONE_WIRE_PORT       PORTB
#define ONE_WIRE_DDR        DDRB
#define ONE_WIRE_PIN        PINB
#define ONE_WIRE_BIT        1

#define DISPLAY_PORT        PORTD

#define DEBUG_PORT          PORTB
#define DEBUG_DDR           DDRB
#define DEBUG_BIT           4
#define DEBUG_BAUD_RATE     1152000
#define DEBUG_EXTENDED_DUMP
#define RAM_GUARD_ENABLED


#define MIN_TEMP            30
#define MAX_TEMP            120
#define TEMP_STEP           1
#define TEMP_DIFF           2
//----------------------------------------------------------------------------
#endif

