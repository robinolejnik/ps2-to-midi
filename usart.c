#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "config.h"
#include "usart.h"
#include "ps2.h"


volatile uint8_t midi_in_state;
volatile uint8_t midi_in_data[3];
volatile uint8_t kbd_led;

void usart_init(void) {
	UBRRL = TEILER;
	UBRRH = 0;
	UCSRB |= (1<<RXCIE | 1<<TXEN | 1<<RXEN);
	UCSRC |= (0<<UMSEL) | (1<<UCSZ1) | (1<<UCSZ0);
}

void put_c(uint8_t Zeichen) {
	while(!(UCSRA & (1<<UDRE)));
	UDR = Zeichen;
}

ISR(USART_RX_vect) {
	if (!(JUMPER_PIN & (1<<JUMPER))) {

		uint8_t value = UDR;

		switch(midi_in_state) {
		case 0:
			if(value == 0xF8 || value == 0xFA || value == 0xFC) {
				put_c(value);
				break;
			}
			else {
				uint8_t a = value >> 4;
				if(a == 0x8 || a == 0x9 || a == 0xA || a == 0xB || a == 0xC || a == 0xD || a == 0xE) {
					midi_in_data[0] = value;
					midi_in_state = 1;
				}
			}
			break;
		case 1:
			midi_in_data[1] = value;
			midi_in_state = 2;
			break;
		case 2:
			midi_in_data[2] = value;
			midi_in_state = 0;
			put_c(midi_in_data[0]);
			put_c(midi_in_data[1]);
			put_c(midi_in_data[2]);
			break;
		}
	}
}
