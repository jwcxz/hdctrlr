// hdctrlr
// reads back emf of a hard drive platter
// spits out some info
//
// http://jwcxz.com/projects/hdctrlr
// J. Colosimo -- http://jwcxz.com

#include "main.h"

#include "uart.h"
#include "sampleproc.h"

int main(void) {
    enum action axn;

    // initialize UART
    uart_init();

    // setup ADC
    adc_init();

    while (1) {
        axn = process_sample();

        if ( axn == cw ) {
            uart_tx('>');
        } else if ( axn == ccw ) {
            uart_tx('<');
        }
    }

	return 0;
}
