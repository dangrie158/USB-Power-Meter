#include <avr/io.h>

#include "adc.h"

void AdcInit(){

	//DDRB are inputs
	DDRB &= ~(1 << PB3);
	DDRB &= ~(1 << PB4);

	//Select internal Voltage Reference (1.1V)
	ADMUX &= ~(1 << REFS2);
	ADMUX |= (1 << REFS1);
	ADMUX &= ~(1 << REFS0);

	//Enable the ADC
	ADCSRA |= (1 << ADEN);

	//disable the input buffers for ADC2 and ADC3
	DIDR0 |= (1 << ADC2D);
	DIDR0 |= (1 << ADC3D);

	//Get a dummy sample from the ADC and theow it away
	//just to warm up the ADC and make sure its ready
	AdcGetSample();
}

void AdcSetDifferentialInputMode(bool gainEnabled){
	//Positive input ADC2, negative input ADC3
	ADMUX &= ~(1 << MUX3);
	ADMUX |= (1 << MUX2);
	ADMUX |= (1 << MUX1);

	if(gainEnabled){
		//Gain x20 enabled
		ADMUX |= (1 << MUX0);
	}else{
		//Gain x1
		ADMUX &= ~(1 << MUX0);
	}

	//On the board, negative and positive inputs are flipped
	//this is a board design failure, but we can reverse the polarity
	//by setting the IPR bit
	ADCSRB |= (1 << IPR);
}

void AdcSetSingleEndedMode(uint8_t inputPin){
	//save the old mux value
	uint8_t oldMux = ADMUX;
	//clear the old channel select
	oldMux &= ~(0xF);
	//set the bifs for the input selection
	oldMux |= inputPin & 0xF;
	//write back the new value
	ADMUX = oldMux;
}

uint16_t AdcGetSample(){
	//Start a conversion
	ADCSRA |= (1 << ADSC);

	//wait until the conversion is finished
	while(ADCSRA & (1 << ADSC)){};

	return ADC;
}

uint16_t AdcGetMultiSample(uint8_t numSamples){
	uint32_t sum = 0;

	for (uint8_t i = 0; i < numSamples; ++i ) {
    	sum += AdcGetSample();
  	}

	return (uint16_t)( sum / numSamples );
}
