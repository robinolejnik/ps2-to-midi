#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "ps2.h"
#include "usart.h"
#include "note_table.h"
#include "decode.h"
#include "config.h"

volatile uint8_t last, note_old;

void decode(uint8_t scancode) {
	uint8_t key;
	key = pgm_read_byte(&note_table[scancode]);

	if(key > 0 && last == 0) {
		note_on(key);
		last = 0;
	}
	if(key > 0 && last == 0xf0) {
		note_off(key);
		last = 0;
	}

	if(last == 0xff) {
		key = pgm_read_byte(&note_table2[scancode]);

		if(key > 0) {
			note_off(key);
		}
		last = 0;
		return;
	}

	if(last == 0xee && scancode == 0x14) {
		last = 0xdd;
		return;
	}
	if(last == 0xdd && scancode == 0x77) {
		last = 0xcc;
		return;
	}
	if(last == 0xcc && scancode == 0xe1) {
		last = 0xbb;
		return;
	}
	if(last == 0xbb && scancode == 0xf0) {
		last = 0xaa;
		return;
	}
	if(last == 0xaa && scancode == 0x14) {
		last = 0x99;
		return;
	}
	if(last == 0x99 && scancode == 0xf0) {
		last = 0x88;
		return;
	}
	if(last == 0x88 && scancode == 0x77) {

		for(uint8_t i=0;i<128;i++) {
			note_off(i);
		}
		last = 0;
		return;
	}

	if(last == 0xe0) {
		switch(scancode) {
			case 0xf0:
			last = 0xff;
			return;
			default:
			key = pgm_read_byte(&note_table2[scancode]);
			if(key > 0) {
				note_on(key);
			}
			return;
		}
	}

	if(last == 0) {
		switch(scancode) {
			case 0xf0:
			last = 0xf0;
			break;
			case 0xe0:
			last = 0xe0;
			break;
			case 0xe1:
			last = 0xee;
			break;
		}
	}
}

void note_on(uint8_t note) {
	if(note != note_old) {
		while(midi_in_state > 0) {
		}
		put_c(0b10010000);
		put_c(note);
		put_c(0b01111111);
		note_old = note;
	}
}

void note_off(uint8_t note) {
	if(note == note_old) {
		note_old = 0;
	}
	while(midi_in_state > 0) {
	}
	put_c(0b10000000);
	put_c(note);
	put_c(0b01111111);
}
