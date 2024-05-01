include config84.mk

extract_port = $(shell echo $1 | awk '{print substr($$1, 1, 1)}')
extract_pin = $(shell echo $1 | awk '{print substr($$1, 2)}')

LED1_PORT = $(call extract_port,$(LED1))
LED1_PIN = $(call extract_pin,$(LED1))
LED2_PORT = $(call extract_port,$(LED2))
LED2_PIN = $(call extract_pin,$(LED2))
LEDOUT_PORT = $(call extract_port,$(LEDOUT))
LEDOUT_PIN = $(call extract_pin,$(LEDOUT))
B1_PORT = $(call extract_port,$(Bttn1))
B1_PIN = $(call extract_pin,$(Bttn1))
B2_PORT =$(call extract_port,$(Bttn2))
B2_PIN = $(call extract_pin,$(Bttn2))
SWITCH_PORT = $(call extract_port,$(SWITCH))
SWITCH_PIN = $(call extract_pin,$(SWITCH))

ifeq ($(LED1_PORT), B)
	DDR_L1 = DDRB
	PORT_L1 = PORTB
	PIN_L1 =PINB
else
	DDR_L1 =DDRA
	PORT_L1 =PORTA
	PIN_L1 =PINA
endif

ifeq ($(LED2_PORT), B)
	DDR_L2 = DDRB
	PORT_L2 =PORTB
	PIN_L2 =PINB
else
	DDR_L2 =DDRA
	PORT_L2 =PORTA
	PIN_L2 =PINA
endif

ifeq ($(B1_PORT), B)
	DDR_B1 = DDRB
	PORT_B1 = PORTB
	PIN_B1 = PINB
	ifeq ($(MCU), attiny24)
		PCIE_B1 = PCIE1
		PCMSK_B1 = PCMSK1
	else ifeq ($(MCU), attiny44)
		PCIE_B1 = PCIE1
		PCMSK_B1 = PCMSK1
	else ifeq ($(MCU), attiny84)
		PCIE_B1 = PCIE1
		PCMSK_B1 = PCMSK1
	else
		PCIE_B1 = PCIE
		PCMSK_B1 = PCMSK
	endif
else
	DDR_B1 = DDRA
	PORT_B1 = PORTA
	PIN_B1 = PINA
	ifeq ($(MCU), attiny24)
		PCIE_B1 = PCIE0
		PCMSK_B1 = PCMSK0
	else ifeq ($(MCU), attiny44)
		PCIE_B1 = PCIE0
		PCMSK_B1 = PCMSK0
	else ifeq ($(MCU), attiny84)
		PCIE_B1 = PCIE0
		PCMSK_B1 = PCMSK0
	else 
		PCIE_B1 = PCIE
		PCMSK_B1 = PCMSK
	endif
endif


ifeq ($(B2_PORT), B)
 	DDR_B2 = DDRB
 	PORT_B2 = PORTB
 	PIN_B2 = PINB
	ifeq ($(MCU), attiny24)
		PCIE_B2 = PCIE1
		PCMSK_B2 = PCMSK1
	else ifeq ($(MCU), attiny44)
		PCIE_B2 = PCIE1
		PCMSK_B2 = PCMSK1
	else ifeq ($(MCU), attiny84)
		PCIE_B2 = PCIE1
		PCMSK_B2 = PCMSK1
	else
		PCIE_B2 = PCIE
		PCMSK_B2 = PCMSK
	endif
else
	DDR_B2 = DDRA
	PORT_B2 = PORTA
	PIN_B2 = PINA
	ifeq ($(MCU), attiny24)
		PCIE_B2 = PCIE0
		PCMSK_B2 = PCMSK0
	else ifeq ($(MCU), attiny44)
		PCIE_B2 = PCIE0
		PCMSK_B2 = PCMSK0
	else ifeq ($(MCU), attiny84)
		PCIE_B2 = PCIE0
		PCMSK_B2 = PCMSK0
	else 
		PCIE_B2 = PCIE
		PCMSK_B2 = PCMSK
	endif
endif


ifeq ($(LEDOUT_PORT), B)
	DDR_LEDOUT = DDRB
	PORT_LEDOUT =PORTB
	PIN_LEDOUT =PINB
else
	DDR_LEDOUT =DDRA
	PORT_LEDOUT =PORTA
	PIN_LEDOUT =PINA
endif

ifeq ($(SWITCH_PORT), B)
	DDR_SWITCH = DDRB
	PORT_SWITCH =PORTB
	PIN_SWITCH =PINB
else
	DDR_SWITCH =DDRA
	PORT_SWITCH =PORTA
	PIN_SWITCH =PINA
endif

all:
	avr-gcc -Wall -g -Os -mmcu=${MCU} -DMCU=${MCU} -DF_CPU=$(F_CPU) \
	-DBUTTON1_PIN=$(BUTTON1_PIN) -DBUTTON2_PIN=$(BUTTON2_PIN) \
	-DSWITCH_PIN=$(SWITCH_PIN) -DLED1_PIN=$(LED1_PIN) \
	-DLED2_PIN=$(LED2_PIN) -DLED3_PIN=$(LED3_PIN) \
	-DLED1_PORT=${LED1_PORT} -DLED1_PIN=${LED1_PIN} \
	-DLED2_PORT=${LED2_PORT} -DLED2_PIN=${LED2_PIN} \
	-DLEDOUT_PORT=${LEDOUT_PORT} -DLEDOUT_PIN=${LEDOUT_PIN} \
	-DB1_PORT=${B1_PORT} -DB1_PIN=${B1_PIN} \
	-DB2_PORT=${B2_PORT} -DB2_PIN=${B2_PIN} \
	-DSWITCH_PORT=${SWITCH_PORT} -DSWITCH_PIN=${SWITCH_PIN} \
	-DDDR_L1=${DDR_L1} -DPORT_L1=${PORT_L1} -DPIN_L1=${PIN_L1} \
	-DDDR_L2=${DDR_L2} -DPORT_L2=${PORT_L2} -DPIN_L2=${PIN_L2} \
	-DDDR_LEDOUT=${DDR_LEDOUT} -DPORT_LEDOUT=${PORT_LEDOUT} -DPIN_LEDOUT=${PIN_LEDOUT} \
	-DDDR_B1=${DDR_B1} -DPORT_B1=${PORT_B1} -DPIN_B1=${PIN_B1} \
	-DDDR_B2=${DDR_B2} -DPORT_B2=${PORT_B2} -DPIN_B2=${PIN_B2} \
	-DDDR_SWITCH=${DDR_SWITCH} -DPORT_SWITCH=${PORT_SWITCH} -DPIN_SWITCH=${PIN_SWITCH} \
	-DPCIE_B1=${PCIE_B1} -DPCIE_B2=${PCIE_B2} \
	-DPCMSK_B1=${PCMSK_B1} -DPCMSK_B2=${PCMSK_B2} \
	-o main.bin main.c
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex