#define F_CPU 16000000
#define __AVR_ATmega8__

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define FORW PC4
#define REVR PC3
#define BAUD 9600

#define MYUBRR (F_CPU / 16 / BAUD - 1)

short isEnabled = 0, isReverse = 0;

void startFunc();
void stopFunc();
void reverseFunc();
void setup();

struct command
{
    const char *text;
    const int size;
    int pos;
    void (*func)(void);
} start = {"start", 5, 0, startFunc}, stop = {"stop", 4, 0, stopFunc}, reverse = {"cd", 2, 0, reverseFunc};

void update(struct command *com, char ch);

ISR(USART_RXC_vect)
{
    char received_data = UDR;
    update(&start, received_data);
    update(&stop, received_data);
    update(&reverse, received_data);
}

int main(void)
{
    setup();
    sei();

    while (1)
    {
    }
}

void setup()
{
    DDRC |= (1 << FORW) | (1 << REVR);
    PORTC &= ~((1 << FORW) | (1 << REVR));

    UBRRH = (unsigned char)(MYUBRR >> 8);
    UBRRL = (unsigned char)MYUBRR;

    UCSRB = (1 << RXEN) | (1 << RXCIE);
    UCSRC = (1 << URSEL) | (3 << UCSZ0);
}

void update(struct command *com, char ch)
{
    if (com->text[com->pos] == ch)
        com->pos++;
    else
        com->pos = 0;

    if (com->pos == com->size)
    {
        com->pos = 0;
        com->func();
    }
}

void startFunc()
{
    isEnabled = 1;
    if (!isReverse)
        PORTC |= (1 << FORW);
    else
        PORTC |= (1 << REVR);
}

void stopFunc()
{
    isEnabled = 0;
    PORTC &= ~(1 << FORW);
    PORTC &= ~(1 << REVR);
}

void reverseFunc()
{
    isReverse = !isReverse;
    if (isEnabled)
    {
        stopFunc();
        startFunc();
    }
}