#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "ps2.h"
#include "usart.h"
#include "decode.h"
#include "config.h"

extern volatile uint8_t rx_count;
extern volatile uint8_t rx_error;
volatile uint8_t PS2_buffer[PS2_BUFFER];
volatile uint8_t PS2_head;
uint8_t PS2_tail;
void wait40us(void);
void wait_for_ack(void);
uint8_t put_byte(uint8_t);

void PS2_init(void) {
	MCUCR = (1<<ISC01);
	GIMSK = (1<<INT0);
	PS2_head  = 0;
	PS2_tail  = 0;
	rx_count = 11;
}

void PS2_clear_buffer(void) {
	while (TCCR0B);
	cli();
	PS2_head  = PS2_tail;
	sei();
}

uint8_t PS2_is_byte_in_buffer(void) {
	return (!(PS2_head == PS2_tail));
}

uint8_t PS2_get_byte_from_buffer(void) {
	register uint8_t t;
	t = PS2_buffer[PS2_tail++];
	PS2_tail &= PS2_MASK;
	return(t);
}

uint8_t PS2_cmd(uint8_t cmd) {
	if (TCCR0B == 0) {
		if (!(put_byte(cmd))) return(FALSE);
		wait_for_ack();
		if (!(rx_error)) return(TRUE);
	}
	return(FALSE);
}

void wait_for_ack(void) {
	TCNT0 = 0;
	TCCR0B = PRESCALE64;
	while ((!(PS2_is_byte_in_buffer())) && (TCCR0B));
	TCCR0B = 0;
}

uint8_t put_byte(uint8_t data) {
	uint8_t j;
	uint8_t parity = 0;
	uint8_t result = FALSE;
	GIMSK &= ~(1<<INT0);
	PS2_PORT |= (1<<PS2_CLOCK | 1<<PS2_DATA);
	PS2_DDR  |= (1<<PS2_CLOCK | 1<<PS2_DATA);
	PS2_PORT &= ~(1<<PS2_CLOCK);
	wait40us();
	wait40us();
	wait40us();
	PS2_PORT &= ~(1<<PS2_CLOCK | 1<<PS2_DATA);
	PS2_DDR  &= ~(1<<PS2_CLOCK);
	_delay_us(10);
	TCNT0  = 0;
	TCCR0B = PRESCALE64;
	j = 11;
	do {
		while ((PS2_PIN & (1<<PS2_CLOCK)) && TCCR0B);
		if (TCCR0B == 0) break;
		if (j > 3) {
			if (data & 0x01) {
				PS2_PORT |=(1<<PS2_DATA);
				parity   ^= 0x01;
			}
			else {
				PS2_PORT &= ~(1<<PS2_DATA);
			}
			data /= 2;
		}
		else if (j == 3) {
			if (parity & 0x01) {
				PS2_PORT &= ~(1<<PS2_DATA);
			}
			else {
				PS2_PORT |=(1<<PS2_DATA);
			}
		}
		else if (j == 2) {
			PS2_PORT |= (1<<PS2_DATA);
		}
		else if (j == 1) {
			PS2_DDR &= ~(1<<PS2_CLOCK | 1<<PS2_DATA);
			_delay_us(10);
			if ( ( !(PS2_PIN & (1<<PS2_DATA)) ) && TCCR0B) result = TRUE;
			while ( ((!(PS2_PIN) & (1<<PS2_DATA | 1<<PS2_CLOCK)) == (1<<PS2_DATA | 1<<PS2_CLOCK)) && TCCR0B);
			if (TCCR0B == 0) result = FALSE;
			break;
		}
		while ((!(PS2_PIN & (1<<PS2_CLOCK))) && TCCR0B);
		if (TCCR0B == 0) break;
		j--;
	} while (j);
	PS2_DDR &= ~(1<<PS2_CLOCK | 1<<PS2_DATA);
	GIMSK |= (1<<INT0);
	rx_count = 11;
	TCCR0B = 0;
	TCNT0  = 0;
	return(result);
}

void wait40us(void) {
	_delay_us(40);
}

ISR(INT0_vect) {
	register uint8_t j;
	static uint8_t data = 0;
	static uint8_t parity = 0;
	j = rx_count;
	if (j == 11) {
		TCCR0B = PRESCALE64;
		if(PS2_PIN & (1<<PS2_DATA)) parity = 0x80; else parity = 0x00;
	}
	else if (j > 2) {
		data /= 2;
		if (PS2_PIN & (1<<PS2_DATA)) {
			data   |= 0x80;
			parity ^= 0x01;
		}
	}
	else if (j == 2) {
		if ( (parity & 0x01) == (PS2_PIN & (1<<PS2_DATA)) ) parity |= 0x40;
	}
	else if (j == 1) {
		if (!(PS2_PIN & (1<<PS2_DATA))) parity |= 0x20;
	}
	j--;
	if (!(j)) {
		if (parity & 0xF0) rx_error = parity;
		PS2_buffer[PS2_head] = data;
		PS2_head++;
		PS2_head &= PS2_MASK;
		j = 11;
		TCCR0B = 0;
		TCNT0  = 0;
		data   = 0;
	}
	rx_count = j;
}
