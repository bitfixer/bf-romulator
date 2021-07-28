#include <avr/io.h>
#include <avr/delay.h>

int main(void)
{
    // set PB3 as output
    DDRB = 1 << PB3;

    // SET reset low
    PORTB = 0;
    _delay_ms(1200);
    PORTB = 1 << PB3;

    while(1)
    {
        
    }
}