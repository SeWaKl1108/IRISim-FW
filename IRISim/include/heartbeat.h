#include <Arduino.h>

byte toggle =0x0;

void SetupTimerRegisters()
{
    cli(); // stop interrupts

    // set timer4 interrupt at 1Hz
    TCCR4A = 0; // set entire TCCR1A register to 0
    TCCR4B = 0; // same for TCCR1B
    TCNT4 = 0;  // initialize counter value to 0
    // set compare match register for 1hz increments
    OCR4A = 25624 / 1; // = (16*10^6) / (1*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR4B |= (1 << WGM12);
    // Set CS12 and CS10 bits for 1024 prescaler
    TCCR4B |= (1 << CS12) | (1 << CS10);
    // enable timer compare interrupt
    TIMSK4 |= (1 << OCIE4A);

    sei(); // allow interrupts
}

ISR(TIMER4_COMPA_vect)
{
    // timer4 interrupt service routine
    digitalWrite(LED_BUILTIN, toggle);    
    toggle=!toggle;
}