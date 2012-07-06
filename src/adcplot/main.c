// hdctrlr
// reads back emf of a hard drive platter
// spits out some info
//
// http://jwcxz.com/projects/hdctrlr
// J. Colosimo -- http://jwcxz.com

#include "main.h"

#include "uart.h"

int main(void) {
    uint8_t i = 0;

    // initialize UART
    uart_init();

    // setup ADC
    ADMUX = _BV(ADLAR) | _BV(REFS0);
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

    while (1) {
        uart_tx(':');
        for ( i=0 ; i<3 ; i++ ) {
            ADMUX = _BV(ADLAR) | _BV(REFS0) | i;
            ADCSRA |= _BV(ADSC);
            while ( !(ADCSRA & _BV(ADIF)) );
            ADCSRA |= _BV(ADIF);

            uart_tx(ADCH);
        }
        _delay_ms(50);
    }

	return 0;
}
