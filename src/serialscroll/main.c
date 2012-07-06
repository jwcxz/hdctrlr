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
    uint8_t samples[3];
    enum state last, curr;
    enum action axn;

    // initialize UART
    uart_init();

    // setup ADC
    ADMUX = _BV(ADLAR) | _BV(REFS0);
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

    // set state information
    axn = idle;
    curr = none;
    last = none;

    while (1) {
        // read ADCs
        for ( i=0 ; i<3 ; i++ ) {
            ADMUX = _BV(ADLAR) | _BV(REFS0) | i;
            ADCSRA |= _BV(ADSC);
            while ( !(ADCSRA & _BV(ADIF)) );
            ADCSRA |= _BV(ADIF);

            // XXX: more resolution needed later?
            samples[i] = ADCH;
        }

        // determine current position based on max value of each (and ensure
        // that the value is above the threshold)
        if ( samples[0] > SAMPLE_THRESH && 
                samples[0] > samples[1] && 
                samples[0] > samples[2] ) {
            curr = yellow;
        } else if ( samples[1] > SAMPLE_THRESH && 
                samples[1] > samples[0] && 
                samples[1] > samples[2] ) {
            curr = blue;
        } else if ( samples[2] > SAMPLE_THRESH && 
                samples[2] > samples[0] && 
                samples[2] > samples[1] ) {
            curr = green;
        } else {
            curr = none;
        }
        

        switch (last) {
            case none:
                // no previous information known
                break;

            case yellow:
                switch (curr) {
                    case none:
                        axn = idle;
                        break;
                    case yellow:
                        axn = idle;
                        break;
                    case blue:
                        axn = cw;
                        break;
                    case green:
                        axn = ccw;
                        break;
                }
                break;

            case blue:
                switch (curr) {
                    case none:
                        axn = idle;
                        break;
                    case yellow:
                        axn = ccw;
                        break;
                    case blue:
                        axn = idle;
                        break;
                    case green:
                        axn = cw;
                        break;
                }
                break;

            case green:
                switch (curr) {
                    case none:
                        axn = idle;
                        break;
                    case yellow:
                        axn = cw;
                        break;
                    case blue:
                        axn = ccw;
                        break;
                    case green:
                        axn = idle;
                        break;
                }
                break;
        }

        if ( last == none || curr != none ) {
            last = curr;
        }

        if ( axn == cw ) {
            uart_tx('>');
        } else if ( axn == ccw ) {
            uart_tx('<');
        }
    }

	return 0;
}
