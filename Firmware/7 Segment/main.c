#include <util/delay.h>

#include "lib/TM1637/TM1637Display.h"
#include "lib/ADC/ADC.h"

//Factor to calculate count number to µA in gain 20 differential mode
#define COUNT_TO_UA_GAINED (11.f * 1000.f)/(20.f * 1024.f)

//Factor to calculate count number to µA in no gain mode
#define COUNT_TO_UA (11.f * 1000.f)/(1024.f)

#define GAIN_SWITCH_THRESHOLD_UPPER 1000
#define GAIN_SWITCH_THRESHOLD_LOWER 1

typedef enum {I_GAIN, I_NOGAIN, U_IN, U_OUT} Mode;
Mode currentMode = I_GAIN;

int main(){
	TM1637DisplayInit();
	  
	AdcInit();

	AdcSetDifferentialInputMode(true);
	//AdcSetSingleEndedMode(3);

	TM1637DisplaySetBrightness(0x0f);

    while(1){


    	//configure the ADC to the current Mode
    	if(currentMode == I_GAIN){
    		AdcSetDifferentialInputMode(true);
    	}else if(currentMode == I_NOGAIN){
    		AdcSetDifferentialInputMode(false);
    	}


    	uint16_t countNumber = AdcGetMultiSample(100);

    	//handle the ranging of the gain and no-gain mode
    	if(countNumber > GAIN_SWITCH_THRESHOLD_UPPER && currentMode == I_GAIN){
    		currentMode = I_NOGAIN;
    		continue;
    	}else if(countNumber <= GAIN_SWITCH_THRESHOLD_LOWER && currentMode == I_NOGAIN){
    		currentMode = I_GAIN;
    		continue;
    	}

    	float microUnit;
    	if(currentMode == I_GAIN){
    		microUnit = countNumber * COUNT_TO_UA_GAINED;
    	}else if(currentMode == I_NOGAIN){
    		microUnit = countNumber * COUNT_TO_UA;
    	}

	  	TM1637DisplayShowNumberDec(microUnit, true);
	  	_delay_ms(500);
    }
}