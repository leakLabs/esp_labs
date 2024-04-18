// #define F_CPU 12000000
// #define __AVR_ATtiny84__ 

// #define LED1_PORT 0
// #define LED1_PIN 0
// #define LED2_PORT 0
// #define LED2_PIN 0
// #define LEDOUT_PORT 0
// #define LEDOUT_PIN 0
// #define B1_PORT 0
// #define B1_PIN 0
// #define B2_PORT 0
// #define B2_PIN 0
// #define SWITCH_PORT 0
// #define SWITCH_PIN 0
// #define DDR_L1 0
// #define PORT_L1 0
// #define PIN_L1 0
// #define DDR_L2 0
// #define PORT_L2 0
// #define PIN_L2 0
// #define DDR_LEDOUT 0
// #define PORT_LEDOUT 0
// #define PIN_LEDOUT 0
// #define DDR_B1 0
// #define PORT_B1 0
// #define PIN_B1 0
// #define DDR_B2 0
// #define PORT_B2 0
// #define PIN_B2 0
// #define DDR_SWITCH 0
// #define PORT_SWITCH 0
// #define PIN_SWITCH 0
// #define PCIE_B1 0
// #define PCIE_B2 0
// #define PCMSK_B1 0
// #define PCMSK_B2 0

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t button1State = 0;
volatile uint8_t button2State = 0;

void setup();

#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
ISR(PCINT1_vect, ISR_ALIASOF(PCINT0_vect));
#endif
ISR(PCINT0_vect)
{
    if(!(PIN_SWITCH & (1 << SWITCH_PIN))) {
        if (!(PIN_B1 & (1 << B1_PIN))) {
            button1State = ~button1State;
        }

        if (!(PIN_B2 & (1 << B2_PIN))) {
            button2State = ~button2State;
        }
    }
}

int main(void) {
    setup();
    while(1) {
        if (!(PIN_SWITCH & (1 << SWITCH_PIN))) {
            PORT_L1 = ((1 << LED1_PIN) & button1State | ~(1 << LED1_PIN) & PORT_L1);
            PORT_L2 = ((1 << LED2_PIN) & button2State | ~(1 << LED2_PIN) & PORT_L2);
            PORT_LEDOUT = ((1 << LEDOUT_PIN) & (button1State ^ button2State) | ~(1 << LEDOUT_PIN) & PORT_LEDOUT);
        } else {
            //PORTB &= ~((1 << LED1_PIN) | (1 << LED2_PIN) | (1 << LED3_PIN));
            PORT_L1 &= ~(1 << LED1_PIN);
            PORT_L2 &= ~(1 << LED2_PIN);
            PORT_LEDOUT &= ~(1 << LEDOUT_PIN);
        }
    }
}

void setup() {
    //DDRB |= (1 << LED1_PIN) | (1 << LED2_PIN) | (1 << LED3_PIN);
    DDR_L1 |= (1 << LED1_PIN);
    DDR_L2 |= (1 << LED2_PIN);
    DDR_LEDOUT |= (1 << LEDOUT_PIN);

    //DDRA &= ~((1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << SWITCH_PIN));
    DDR_B1 &= ~(1 << B1_PIN);
    DDR_B2 &= ~(1 << B2_PIN);
    DDR_SWITCH &= ~(1 << SWITCH_PIN);

    //PORTA |= (1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << SWITCH_PIN);
    PORT_B1 |= (1 << B1_PIN);
    PORT_B2 |= (1 << B2_PIN);
    PORT_SWITCH |= (1 << SWITCH_PIN);

    GIMSK |= (1 << PCIE_B1);
    GIMSK |= (1 << PCIE_B2);

    //PCMSK0 |= (1 << PCINT1) | (1 << PCINT0);
    PCMSK_B1 |= (1 << B1_PIN);
    PCMSK_B2 |= (1 << B2_PIN);
    sei();
}