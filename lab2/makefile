include config.mk

all:
	avr-gcc -Wall -g -Os -mmcu=attiny84 -DF_CPU=$(F_CPU) \
	-DBUTTON1_PIN=$(BUTTON1_PIN) -DBUTTON2_PIN=$(BUTTON2_PIN) \
	-DSWITCH_PIN=$(SWITCH_PIN) -DLED1_PIN=$(LED1_PIN) \
	-DLED2_PIN=$(LED2_PIN) -DLED3_PIN=$(LED3_PIN) -o main.bin main.c
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex