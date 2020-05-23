//-----------------------------------------------------------------------------
#include "lib/avr-misc/avr-misc.h"
#include "lib/avr-debug/debug.h"
#include "lib/DS1820/DS1820.h"
#include "disp7seg.h"
#include "config.h"
//-----------------------------------------------------------------------------
#ifdef BEEPER_PORT
    #define beep(ms)       { set_bit(BEEPER_PORT, BEEPER_BIT); mdelay(ms); clr_bit(BEEPER_PORT, BEEPER_BIT); }
#else
    #define beep(ms)
#endif
#define enable_timer()      TCCR0B = 0x04;
#define disable_timer()     TCCR0B = 0x00;
#define enable_encoder()    set_bit(PCMSK0, PCINT3);
#define disable_encoder()   clr_bit(PCMSK0, PCINT3);
#define button_pushed()     (!test_bit(BUTTON_PIN, BUTTON_BIT))
//-----------------------------------------------------------------------------
enum
{
    MODE_SLEEP,
    MODE_SET,
    MODE_STABILIZE,
    MODE_CONFIG,
    MODE_ERROR
};
//-----------------------------------------------------------------------------
enum controlChannel
{
    HEATER,
    BLOWER
};
//-----------------------------------------------------------------------------
BYTE            mode                = MODE_SLEEP;
WORD            mode_time           = 0;
WORD            button_pushed_time  = 0;
BYTE            target_temp;
BYTE            top_temp;
BYTE            bottom_temp;
bool            bottom_sensor_enabled;
BusDS1820       bus;
SensorDS1820*   top_sensor          = NULL;
SensorDS1820*   bottom_sensor       = NULL;
Display7Seg     display(3);
EEBYTE          saved_target_temp;
EEQWORD         saved_top_address;
EEQWORD         saved_bottom_address;
//-----------------------------------------------------------------------------
void turn_on(controlChannel channel)
{
    set_bit(PORTC, channel);
}
//-----------------------------------------------------------------------------
void turn_off(controlChannel channel)
{
    clr_bit(PORTC, channel);
}
//-----------------------------------------------------------------------------
void throwError(BYTE error_code)
{
    // turn off everything
    turn_off(HEATER);
    turn_off(BLOWER);

    // display error code
    char* buf = toHexString(error_code);
    display.print(buf);
    display.hideDot();
    free(buf);

    mode = MODE_ERROR;

    // disable encoder rotations
    disable_encoder();

    // clear pending interrupts flags
    PCIFR = 1;
}
//-----------------------------------------------------------------------------
__inline BYTE init_sensors()
{
    if(!bus.searchDevices())
    {
        return 0xE1;
    }

    QWORD tmp64;
    if(!top_sensor)
    {
        tmp64 = saved_top_address;
        top_sensor = new SensorDS1820(tmp64);
    }

    if(!top_sensor->available())
    {
        return 0xE4;
    }

    tmp64 = saved_bottom_address;
    if(tmp64)
    {
        if(!bottom_sensor)
        {
            bottom_sensor = new SensorDS1820(tmp64);
        }
        bottom_sensor_enabled = true;
        if(!bottom_sensor->available())
        {
            return 0xE5;
        }
    }
    else
    {
        bottom_sensor_enabled = false;
    }

    return 0;
}
//-----------------------------------------------------------------------------
void power_on()
{
    BYTE error = init_sensors();
    if(error)
    {
        throwError(error);
    }
    else
    {
        beep(150);
        target_temp = saved_target_temp;
        if(target_temp < MIN_TEMP || target_temp > MAX_TEMP)
        {
            target_temp = MIN_TEMP;
        }
        display.print(target_temp);
        enable_encoder();
        mode = MODE_SET;
        mode_time = 0;
    }
    enable_timer();
}
//-----------------------------------------------------------------------------
void power_off()
{
    disable_timer();
    DISPLAY_PORT = 0;
    display.hideDot();
    turn_off(HEATER);
    turn_off(BLOWER);
    if(saved_target_temp != target_temp)
    {
        saved_target_temp = target_temp;
    }
    beep(300);
    mode = MODE_SLEEP;
}
//-----------------------------------------------------------------------------
void configure_sensors()
{
    mode = MODE_CONFIG;
    display.print("TOP");
    enable_timer(); mdelay(500);
    display.showDot(); mdelay(500);
    display.hideDot(); mdelay(500);
    while(bus.searchDevices() != 1)
    {
        display.showDot(); mdelay(500);
        display.hideDot(); mdelay(500);
    }
    // save top sensor address
    saved_top_address = bus.getFirstDeviceAddress();
    beep(700);

    saved_bottom_address = 0;
    display.print("BOT"); mdelay(500);
    display.showDot(); mdelay(500);
    display.hideDot(); mdelay(500);
    while(bus.searchDevices() != 2)
    {
        display.showDot(); mdelay(500);
        display.hideDot(); mdelay(500);
    }

    // save bottom sensor address
    if(bus.getFirstDeviceAddress() != saved_top_address)
    {
        saved_bottom_address = bus.getFirstDeviceAddress();
    }
    else
    {
        saved_bottom_address = bus.getNextDeviceAddress();
    }
    display.showDot(); mdelay(500);
    display.hideDot(); mdelay(500);
    beep(700);

    power_on();
}
//-----------------------------------------------------------------------------
int main()
{
    DEBUG_INIT();

    // configure ouputs
    DDRD = 0xFF;
    DDRC = 0x3F;

    // configure buttons
    set_bit(ENCODER_A_PORT, ENCODER_A_BIT);
    set_bit(ENCODER_B_PORT, ENCODER_B_BIT);
    set_bit(BUTTON_PORT, BUTTON_BIT);
    set_bit(PCIFR, PCIF0);      // clear pending interrupt flag
    set_bit(PCMSK0, PCINT5);    // enable button

    // configure timer
    TIMSK0 = 0x01;

    enable_interrupts();

#ifdef DEBUG
    //configure_sensors();
    power_on();
    PCICR = 0x00;
#else
    if(button_pushed())
    {
        configure_sensors();
    }
    PCICR = 0x01;
#endif

    while(1)
    {
        switch(mode)
        {
        case MODE_SLEEP:
            turn_off(HEATER);
            turn_off(BLOWER);
            break;
        case MODE_ERROR:
            turn_off(HEATER);
            turn_off(BLOWER);
            beep(100); mdelay(100);
            beep(100); mdelay(100);
            beep(100); mdelay(100);
            while(mode == MODE_ERROR)
            {
                mdelay(100);
            };
            break;
        case MODE_SET:
            display.print(target_temp);
            bus.allSensors.measure();
            break;
        case MODE_STABILIZE:
            // get and display current temperature
            int tmp = top_sensor->readTempC();
            if((WORD)tmp == BAD_TEMP)
            {
                throwError(0xE6);
                break;
            }
            top_temp = tmp;
            display.print(top_temp);

            // stabilize the temperature
            if(top_temp < target_temp)
            {
                turn_on(HEATER);
                display.showDot();
            }
            else
            {
                turn_off(HEATER);
                display.hideDot();
            }

            if(bottom_sensor_enabled)
            {
                // stabilize the temperature difference
                tmp = bottom_sensor->readTempC();
                if((WORD)tmp == BAD_TEMP)
                {
                    throwError(0xE7);
                    break;
                }
                if(top_temp - tmp > TEMP_DIFF)
                {
                    turn_on(BLOWER);
                }
                else
                {
                    turn_off(BLOWER);
                }
            }

            bus.allSensors.measure();
            mdelay(900);
            break;
        }
        mdelay(100);
    }
    return 0;
}
//-----------------------------------------------------------------------------
ISR(TIMER0_OVF_vect)
{
    switch(mode)
    {
    case MODE_SLEEP:
        break;
    case MODE_SET:
        if(++mode_time > 500)
        {
            // save target temp to EEPROM
            if(saved_target_temp != target_temp)
            {
                saved_target_temp = target_temp;
            }

            // change mode
            mode = MODE_STABILIZE;
        }
    case MODE_ERROR:
    case MODE_STABILIZE:
#ifndef DEBUG
        // long button hold
        if(button_pushed())
        {
            if(++button_pushed_time > 300)
            {
                power_off();
                break;
            }
        }
#endif
    case MODE_CONFIG:
        display.refresh();
    }
}
//-----------------------------------------------------------------------------
ISR(PCINT0_vect)
{
    udelay(1500);           // anti-jitter delay
    set_bit(PCIFR, PCIF0);  // clear pending interrupt flag generated by jitter

    if(test_bit(ENCODER_A_PIN, ENCODER_A_BIT) == 0 && mode != MODE_SLEEP)
    {
        // encoder driver
        if(test_bit(ENCODER_B_PIN, ENCODER_B_BIT))
        {
            // LESS rotation
            if(target_temp > MIN_TEMP)
            {
                target_temp -= TEMP_STEP;
            }
        }
        else
        {
            // MORE rotation
            if(target_temp < MAX_TEMP)
            {
                target_temp += TEMP_STEP;
            }
        }
        display.print(target_temp);
        mode = MODE_SET;
        mode_time = 0;
    }

    if(button_pushed())
    {
        if(mode == MODE_SLEEP)
        {
            power_on();
        }
    }
    else
    {
        button_pushed_time = 0;
    }
}
//-----------------------------------------------------------------------------
