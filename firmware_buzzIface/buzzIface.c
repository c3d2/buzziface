/*
 * ============================================================================
 *
 *       Filename:  buzzIface.c
 *
 *    Description:  this is the first test
 *
 *        Version:  0.0
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Johannes Steinmetz (js), john ät  tuxcode dot org
 *
 * ============================================================================
 */



#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
/*#include <avr/pgmspace.h>*/
#include <avr/sleep.h>
#include <util/delay.h>
#include "usart.h"


inline void hardware_init(void)
{
	// Configure LED Port C  as OUTPUT:
	DDRC = (1<<DDC3) | (1<<DDC2)  ;
	PORTC = 0 | (1<<DDC2);
	USART_Init();
}


int main(void)
{
	hardware_init();
	sei();
//------------------------------------
	
	uint8_t c ,i ;
	char buf[16];


	USART_puts("Test");
	USART_puts("\n\r");

	
	for(;;){  //ever

		i = uart_getc_nb(&c);
		if (i) {
			USART_putc(c);
			switch (c){
				case 'r' :
					PORTC = 0 | (1<<DDC3);
					break;
				case 'g' :
					PORTC = 0 | (1<<DDC2);
					break;
				default :
					PORTC = 0 | (1<<DDC3)|(1<<DDC2);
			}
		}

	}

	/* never gets here*/
	return 0;
}
