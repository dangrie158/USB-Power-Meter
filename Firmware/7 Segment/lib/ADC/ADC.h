#ifndef __TM1637DISPLAY__
#define __TM1637DISPLAY__

#include <stdbool.h>
#include <inttypes.h>

void AdcInit();

uint16_t AdcGetSample();
uint16_t AdcGetMultiSample(uint8_t numSamples);

#endif //__TM1637DISPLAY__