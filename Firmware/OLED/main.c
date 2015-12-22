#include <util/delay.h>

#include "lib/ADC/ADC.h"
#include "lib/ssd1306xled/ssd1306xled.h"
#include "lib/ssd1306xled/ssd1306xled8x16.h"
#include "logo.h"

// Factor to calculate count number to µA in gain 20 differential mode
#define COUNT_TO_MA_GAINED (11.f * 1000.f) / (20.f * 1024.f)

// Factor to calculate count number to µA in no gain mode
#define COUNT_TO_MA (11.f * 1000.f) / (1024.f)

//Factor to calculate the count number to mV in single ended mode
#define R1 10.0f
#define R2 2.2f
#define VOLTAGE_DIVIDER_RATIO (R2) / ((R1) + (R2))

#define COUNT_TO_MV (1100.0f * (1/(VOLTAGE_DIVIDER_RATIO))) / (1024.f)

#define GAIN_SWITCH_THRESHOLD_UPPER 1000
#define GAIN_SWITCH_THRESHOLD_LOWER 40

//if we measure a voltage below this mV we can assume 
//the switch was pressed and pulled the line low
//if we would have a Vin of 2000mV we couldn't
//work correctly anyway so we can choose this value this high
#define SWITCH_PRESS_MEASURE_THRESHOLD 2000

typedef enum { I_GAIN,
    I_NOGAIN } Mode;
Mode currentMode = I_GAIN;

float measureCurrent()
{
    // configure the ADC to the current Mode
    if (currentMode == I_GAIN) {
        AdcSetDifferentialInputMode(true);
    }
    else if (currentMode == I_NOGAIN) {
        AdcSetDifferentialInputMode(false);
    }

    uint16_t countNumber = AdcGetMultiSample(10);

    float microUnit;
    if (currentMode == I_GAIN) {
        microUnit = countNumber * COUNT_TO_MA_GAINED;
    }
    else if (currentMode == I_NOGAIN) {
        microUnit = countNumber * COUNT_TO_MA;
    }

    // handle the ranging of the gain and no-gain mode
    if (countNumber > GAIN_SWITCH_THRESHOLD_UPPER && currentMode == I_GAIN) {
        currentMode = I_NOGAIN;
    }
    else if (countNumber <= GAIN_SWITCH_THRESHOLD_LOWER && currentMode == I_NOGAIN) {
        currentMode = I_GAIN;
    }

    return microUnit;
}

uint32_t measureVoltage() { 
    //switch to single ended mode on the VIN channel
    AdcSetSingleEndedMode(1); 

    //take the measurement
    uint16_t countNumber = AdcGetMultiSample(10);

    float microUnit = countNumber * COUNT_TO_MV;

    return microUnit;
}

void cleanLine(uint8_t posY, uint8_t numchars){
    //clean the line
    uint8_t posX = 26 + numchars * 8;
    ssd1306_setpos(posX, posY);
    ssd1306_string_font6x8("        ");
    ssd1306_setpos(posX + 12, posY + 1);
    ssd1306_string_font6x8("        ");
}

int main()
{
    static buttonPressed = 0;
    AdcInit();

    _delay_ms(100);
    ssd1306_init();

    ssd1306_fill(0x00); // Clear screen
    ssd1306_draw_bmp(0, 0, 125, 8, logo);
    _delay_ms(1000);
    ssd1306_fill(0x00); // Clear screen

    while (1) {

        //take the measurements
        float currentVoltage = measureVoltage();
        float currentCurrent = measureCurrent();
        float currentPower = (currentVoltage / 1000) * (currentCurrent);

        //use the voltage measurement to determine if the switch was pressed
        if(currentVoltage < SWITCH_PRESS_MEASURE_THRESHOLD && !buttonPressed){
            ssd1306_flip();
            buttonPressed = 1;
            continue;
        }else if (currentVoltage > SWITCH_PRESS_MEASURE_THRESHOLD && buttonPressed){
            //reset the debounce state as soon as we measure 
            //a high enough value again to be sure the button 
            //was released
            buttonPressed = 0;
        }
        
        //convert the measurements to a string
        char voltageString[15];
        itoa((int)currentVoltage, voltageString, 10);

        char currentString[15];
        itoa((int)currentCurrent, currentString, 10);

        char powerString[15];
        itoa((int)currentPower, powerString, 10);

        //draw the voltage
        ssd1306_setpos(0, 1);
        ssd1306_string_font6x8("U =");
        ssd1306_char_f8x16(25, 0, voltageString);
        ssd1306_string_font6x8("mV");
        cleanLine(0, strlen(voltageString));

        ssd1306_setpos(0, 4);
        ssd1306_string_font6x8("I =");
        ssd1306_char_f8x16(25, 3, currentString);
        ssd1306_string_font6x8("mA");
        cleanLine(3, strlen(currentString));

        //draw the multiplier indicator at the top right
        if (currentMode == I_GAIN) {
            ssd1306_setpos(64, 4);
            ssd1306_string_font6x8(" (x20)");
        }
        

        ssd1306_setpos(0, 7);
        ssd1306_string_font6x8("P =");
        ssd1306_char_f8x16(25, 6, powerString);
        ssd1306_string_font6x8("mW");

        cleanLine(6, strlen(powerString));

        //_delay_ms(500);
    }
}