#ifndef _USART_H
	#define _USART_H

	#define BAUD  	31250
	#define TEILER ((F_CPU /(BAUD * 16l)) -1)

	void usart_init(void);
	void put_c(uint8_t i);

#endif
