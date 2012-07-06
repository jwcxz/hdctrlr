// hdctrlr
// reads back emf of a hard drive platter
// spits out some info
//
// http://jwcxz.com/projects/hdctrlr
// J. Colosimo -- http://jwcxz.com

#include "main.h"

#include "usbdrv.h"
#include "oddebug.h"

#include "uart.h"

const PROGMEM char usbHidReportDescriptor[52] = { /* USB report descriptor, size must match usbconfig.h */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xA1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM
    0x29, 0x03,                    //     USAGE_MAXIMUM
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Const,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xC0,                          //   END_COLLECTION
    0xC0,                          // END COLLECTION
};

/* This is the same report descriptor as seen in a Logitech mouse. The data
 * described by this descriptor consists of 4 bytes:
 *      .  .  .  .  . B2 B1 B0 .... one byte with mouse button states
 *     X7 X6 X5 X4 X3 X2 X1 X0 .... 8 bit signed relative coordinate x
 *     Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 .... 8 bit signed relative coordinate y
 *     W7 W6 W5 W4 W3 W2 W1 W0 .... 8 bit signed relative coordinate wheel
 */
typedef struct{
    uchar   button_mask;
    char    dx;
    char    dy;
    char    d_wheel;
}report_t;

static report_t report_buf;
static uchar    idleRate;   /* repeat rate for keyboards, never used for mice */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

    /* The following requests are never used. But since they are required by
     * the specification, we implement them in this example.
     */
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        DBG1(0x50, &rq->bRequest, 1);   /* debug output: print our request */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
            usbMsgPtr = (void *)&report_buf;
            return sizeof(report_buf);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }
    }else{
        /* no vendor specific requests implemented */
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

int main(void) {
    uint8_t i = 0;
    uint8_t samples[3];
    enum state last, curr;
    enum action axn;

    wdt_enable(WDTO_1S);

    // initialize USB
    usbInit();
    usbDeviceDisconnect();

    // fake USB disconnect for > 250ms
    while (--i) {
        wdt_reset();
        _delay_ms(1);
    }

    usbDeviceConnect();
    sei();

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
            report_buf.button_mask = _BV(7);
            usbSetInterrupt((void *)&report_buf, sizeof(report_buf));
        } else if ( axn == ccw ) {
            uart_tx('<');
            report_buf.button_mask = _BV(6);
            usbSetInterrupt((void *)&report_buf, sizeof(report_buf));
        } else {
            report_buf.button_mask = 0;
        }
    }

	return 0;
}
