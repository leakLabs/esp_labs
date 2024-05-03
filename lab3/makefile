uartmk:
	avr-gcc -Wall -g -Os -mmcu=atmega8 -o uart.bin uart.c
	avr-objcopy -j .text -j .data -O ihex uart.bin uart.hex

adcmk:
	avr-gcc -Wall -g -Os -mmcu=atmega8 -o adc.bin adc.c
	avr-objcopy -j .text -j .data -O ihex adc.bin adc.hex

all: uartmk adcmk