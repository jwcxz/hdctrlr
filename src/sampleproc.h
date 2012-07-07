#ifndef _SAMPLEPROC_H
#define _SAMPLEPROC_H

#include "main.h"

#define ADMUX_DEF ( _BV(ADLAR) | _BV(REFS0) )

enum state {
    none,
    yellow,
    green,
    blue
};

enum action {
    idle,
    cw,
    ccw
};

void adc_init(void);
enum action process_sample(void);

#endif
