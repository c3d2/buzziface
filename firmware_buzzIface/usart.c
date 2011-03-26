#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <stdarg.h>

/*#include "main.h"*/
#include "usart.h"


#define UART_RXBUFSIZE 64

volatile static uint8_t rxbuf[UART_RXBUFSIZE];
volatile static uint8_t *volatile rxhead, *volatile rxtail;



uint8_t uart_getc_nb(uint8_t *c)
{
	if (rxhead==rxtail) return 0;
	*c = *rxtail;
	if (++rxtail == (rxbuf + UART_RXBUFSIZE)) rxtail = rxbuf;
	return 1;
}


ISR (USART_RX_vect)
{
                                                                                                        	
		int diff;
		uint8_t c;
		c=UDR;
		diff = rxhead - rxtail;
		if (diff < 0) diff += UART_RXBUFSIZE;
		if (diff < UART_RXBUFSIZE -1) 
		{
			*rxhead = c;
			++rxhead;
			if (rxhead == (rxbuf + UART_RXBUFSIZE)) rxhead = rxbuf;
		};
		
		
		
		
}

void USART_Init (void)
{
	// set clock divider
	#undef BAUD
	#define BAUD USART_BAUD
	#include <util/setbaud.h>
	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
	
#if   USE_2X
	UCSRA |= (1 << U2X);	// enable double speed operation
#else
	UCSRA &= ~(1 << U2X);	// disable double speed operation
#endif
	
	// set 8N1
	UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
	UCSRB &= ~(1 << UCSZ2);

	// flush receive buffer
	while ( UCSRA & (1 << RXC) ) UDR;

	UCSRB |= (1 << RXEN) | (1 << TXEN);
	UCSRB |= (1 << RXCIE);

	rxhead = rxtail = rxbuf;

}




//*****************************************************************************
// 
void USART_putc (char c)
{
	loop_until_bit_is_set(UCSRA, UDRE);
	UDR = c;
}

//*****************************************************************************
// 
void USART_puts (char *s)
{
	while (*s)
	{
		USART_putc (*s);
		s++;
	}
}

//*****************************************************************************
// 
void USART_puts_p (const char *s)
{
	while (pgm_read_byte(s))
	{
		USART_putc (pgm_read_byte(s));
		s++;
	}
}
