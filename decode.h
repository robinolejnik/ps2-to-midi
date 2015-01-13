#ifndef _DECODE_H
	#define _DECODE_H

	extern uint8_t midi_in_state;

	void decode(uint8_t scancode);
	void note_on(uint8_t note);
	void note_off(uint8_t note);

#endif
