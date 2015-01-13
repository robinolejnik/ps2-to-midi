#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "ps2.h"
#include "usart.h"
#include "decode.h"
#include "config.h"

volatile uint8_t rx_count;
volatile uint8_t rx_error = 0;

int main(void) {
	uint8_t i = 0, j;
	TIMSK |= (1<<TOIE0);
	JUMPER_DDR  &= ~(1<<JUMPER);
	JUMPER_PORT |= (1<<JUMPER);
	usart_init();
	PS2_init();
	sei();
	PS2_clear_buffer();
	i = 0;
	PS2_cmd(0xff);	//reset
	PS2_cmd(0xf6);	//load defaults
	PS2_cmd(0xf0);	//set scancode-set
	PS2_cmd(0x02);	//scancode-set 2
	PS2_cmd(0xf4);	//enable
	while(1) {
		if (PS2_is_byte_in_buffer()) {
			j = PS2_get_byte_from_buffer();
			decode(j);
		}
	}
}

ISR(TIMER0_OVF_vect) {
	TCCR0B 	 = 0;
	TCNT0  	 = 0;
	rx_count  = 11;
	rx_error  = 0x10;
}
