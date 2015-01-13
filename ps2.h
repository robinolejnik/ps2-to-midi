#ifndef _MOUSE_H
	#define _MOUSE_H

	#define PRESCALE64	(1<<CS01 | 1<< CS00)	//Prescaler f�r Timer0 = 64

	#define PS2_BUFFER 32						// muss 2^x sein (4, 8, 16, 32)
	#define PS2_MASK	PS2_BUFFER - 1			// andernfalls die Logik der �berlaufkontrolle �ndern

	#define TRUE		1
	#define FALSE		0

	void PS2_init(void);
	void PS2_clear_buffer(void);
	uint8_t PS2_cmd(uint8_t cmd);
	uint8_t PS2_is_byte_in_buffer(void);
	uint8_t PS2_get_byte_from_buffer(void);

#endif
