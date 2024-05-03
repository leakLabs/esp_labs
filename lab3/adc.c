#define F_CPU 16000000
#define __AVR_ATmega8__

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define IN1 PC5
#define IN2 PC4

#define S0 PC3
#define S1 PC2
#define S2 PC1
#define S3 PC0

#define SEGD1 PB4
#define SEGD2 PB3
#define SEGD3 PB2
#define SEGD4 PB1

#define RPM 60.0
#define RPMTOV (RPM / 500.0)

short selSeg = 0;
const uint8_t segs[4] = {SEGD1, SEGD2, SEGD3, SEGD4};
volatile uint8_t segNumbers[4] = {9, 0, 7, 8};
volatile uint16_t adc_c4;
volatile uint16_t adc_c5;

void init();
void updateSEG();
void adcStart(uint8_t channel);
void processVoltage();

int main(void)
{
    init();
    sei();
    adcStart(4);
    while (1)
    {
        updateSEG();
        _delay_ms(1);
    }
}

void init()
{
    DDRB |= (1 << SEGD1) | (1 << SEGD2) | (1 << SEGD3) | (1 << SEGD4);
    PORTB &= ~((1 << SEGD1) | (1 << SEGD2) | (1 << SEGD3) | (1 << SEGD4));

    DDRC |= (1 << S0) | (1 << S1) | (1 << S2) | (1 << S3);
    PORTC &= ~((1 << S0) | (1 << S1) | (1 << S2) | (1 << S3));

    DDRC &= ~((1 << IN1) | (1 << IN2));

    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADCSRA |= (1 << ADEN) | (1 << ADIE);
    ADMUX &= ~((1 << REFS1) | (1 << REFS0));
}

void updateSEG()
{
    PORTB &= ~(1 << segs[selSeg]);
    PORTC &= ~(segNumbers[selSeg] << S3);
    // PORTC &= ~((1 << S0) | (1<<S2));
    selSeg = (selSeg + 1) % 4;
    PORTC |= (segNumbers[selSeg] << S3);
    // PORTC |= (1 << S0) | (1<<S2);
    PORTB |= (1 << segs[selSeg]);
}

void adcStart(uint8_t channel)
{
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
}

void processVoltage()
{
    uint16_t voltage;
    if (adc_c4 > adc_c5)
        voltage = adc_c4 - adc_c5;
    else
        voltage = adc_c5 - adc_c4;

    float tmp = (float)voltage / 204.8;
    voltage = tmp * 100.0 * RPMTOV;
    voltage = voltage;
    for (short i = 3; i >= 0; i--)
    {
        segNumbers[i] = voltage % 10;
        voltage /= 10;
    }
}

ISR(ADC_vect)
{
    static uint8_t channel = 0;
    uint16_t adc_value = ADC;

    if (channel == 0)
    {
        adc_c4 = adc_value;
        adcStart(5);
    }
    else
    {
        adc_c5 = adc_value;
        processVoltage();
        adcStart(4);
        // PORTB |= (1 << PB5);
    }

    channel ^= 0x01;
}