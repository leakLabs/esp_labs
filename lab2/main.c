#define F_CPU 12000000
#define __AVR_ATtiny84__ 
#define BUTTON1_PIN PA0
#define BUTTON2_PIN PA1
#define SWITCH_PIN  PA2
#define LED1_PIN PB0
#define LED2_PIN PB1
#define LED3_PIN PB2

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t button1State = 0;
volatile uint8_t button2State = 0;

void setup();

ISR(PCINT0_vect) {
    if(!(PINA & (1 << SWITCH_PIN))) {
        if (!(PINA & (1 << BUTTON1_PIN))) {
            button1State = ~button1State;
        }

        if (!(PINA & (1 << BUTTON2_PIN))) {
            button2State = ~button2State;
        }
    }
}

int main(void) {
    setup();
    while(1) {
        if (!(PINA & (1 << SWITCH_PIN))) {
            PORTB = ((1 << LED1_PIN) & button1State | ~(1 << LED1_PIN) & PORTB);
            PORTB = ((1 << LED2_PIN) & button2State | ~(1 << LED2_PIN) & PORTB);
            PORTB = ((1 << LED3_PIN) & (button1State ^ button2State) | ~(1 << LED3_PIN) & PORTB);
        } else {
            PORTB &= ~((1 << LED1_PIN) | (1 << LED2_PIN) | (1 << LED3_PIN));
        }
    }
}

void setup() {
    DDRB |= (1 << LED1_PIN) | (1 << LED2_PIN) | (1 << LED3_PIN);
    DDRA &= ~((1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << SWITCH_PIN));

    PORTA |= (1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << SWITCH_PIN);

    GIMSK |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT1) | (1 << PCINT0);
    sei();
}