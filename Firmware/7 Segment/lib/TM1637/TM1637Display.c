#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/io.h>


#include "TM1637Display.h"

#define TM1637_I2C_COMM1    0x40
#define TM1637_I2C_COMM2    0xC0
#define TM1637_I2C_COMM3    0x80

//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D

uint8_t m_brightness;

const uint8_t digitToSegment[] = {
 // XGFEDCBA
  0b00111111,    // 0
  0b00000110,    // 1
  0b01011011,    // 2
  0b01001111,    // 3
  0b01100110,    // 4
  0b01101101,    // 5
  0b01111101,    // 6
  0b00000111,    // 7
  0b01111111,    // 8
  0b01101111,    // 9
  0b01110111,    // A
  0b01111100,    // b
  0b00111001,    // C
  0b01011110,    // d
  0b01111001,    // E
  0b01110001     // F
  };

void TM1637DisplayBitDelay()
{
	_delay_us(50);
}
   
void TM1637DisplayStart()
{
  DDRB |= 1 << PB0;
  TM1637DisplayBitDelay();
}
   
void TM1637DisplayStop()
{
	DDRB |= 1 << PB0;
	TM1637DisplayBitDelay();
	DDRB &= ~(1 << PB2);
	TM1637DisplayBitDelay();
	DDRB &= ~(1 << PB0);
	TM1637DisplayBitDelay();
}
  
bool TM1637DisplayWriteByte(uint8_t b)
{
  uint8_t data = b;

  // 8 Data Bits
  for(uint8_t i = 0; i < 8; i++) {
    // CLK low
    DDRB |= 1 << PB2;
    TM1637DisplayBitDelay();
    
	// Set data bit
    if (data & 0x01)
      DDRB &= ~(1 << PB0);
    else
      DDRB |= 1 << PB0;
    
    TM1637DisplayBitDelay();
	
	// CLK high
    DDRB &= ~(1 << PB2);
    TM1637DisplayBitDelay();
    data = data >> 1;
  }
  
  // Wait for acknowledge
  // CLK to zero
  DDRB |= 1 << PB2;
  DDRB &= ~(1 << PB0);
  TM1637DisplayBitDelay();
  
  // CLK to high
  DDRB &= ~(1 << PB2);
  TM1637DisplayBitDelay();
  uint8_t ack = PINB & PB0;
  if (ack == 0)
  DDRB |= 1 << PB0;
	
	
  TM1637DisplayBitDelay();
  DDRB |= 1 << PB2;
  TM1637DisplayBitDelay();
  
  return ack;
}

uint8_t TM1637DisplayEncodeDigit(uint8_t digit)
{
	return digitToSegment[digit & 0x0f];
}

void TM1637DisplayInit()
{

	// Set the pin direction and default value.
	// Both pins are set as inputs, allowing the pull-up resistors to pull them up
	DDRB &= ~(1 << PB0);
	DDRB &= ~(1 << PB2);

	PORTB &= ~(1 << PB0);
	PORTB &= ~(1 << PB2);
}

void TM1637DisplaySetBrightness(uint8_t brightness)
{
	m_brightness = brightness;
}

void TM1637DisplaySetSegments(const uint8_t segments[])
{
    // Write COMM1
	TM1637DisplayStart();
	TM1637DisplayWriteByte(TM1637_I2C_COMM1);
	TM1637DisplayStop();
	
	// Write COMM2 + first digit address
	TM1637DisplayStart();
	TM1637DisplayWriteByte(TM1637_I2C_COMM2 + (0 & 0x03));
	
	// Write the data bytes
	for (uint8_t k=0; k < 4; k++) 
	  TM1637DisplayWriteByte(segments[k]);
	  
	TM1637DisplayStop();

	// Write COMM3 + brightness
	TM1637DisplayStart();
	TM1637DisplayWriteByte(TM1637_I2C_COMM3 + (m_brightness & 0x0f));
	TM1637DisplayStop();
}
 
void TM1637DisplayShowNumberDec(int num, bool leading_zero)
{
	uint8_t digits[4];
	const static int divisors[] = { 1, 10, 100, 1000 };
	bool leading = true;
	
	for(int8_t k = 0; k < 4; k++) {
	    int divisor = divisors[4 - 1 - k];
		int d = num / divisor;
		
		if (d == 0) {
		  if (leading_zero || !leading || (k == 3))
		    digits[k] = TM1637DisplayEncodeDigit(d);
	      else
		    digits[k] = 0;
		}
		else {
			digits[k] = TM1637DisplayEncodeDigit(d);
			num -= d * divisor;
			leading = false;
		}
	}
	
	TM1637DisplaySetSegments(digits);
}

