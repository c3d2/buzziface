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

#include "buzzIface.h"

/* functions: */

uint8_t wait_keys_blocking(uint8_t keymask /* TODO timeout?,*/ );
uint8_t send_uart_key(uint8_t key);



/* GLOBALZ */

volatile uint8_t keys_armed;
volatile uint8_t key_input;
volatile uint8_t bufreg_b;
volatile uint8_t bufreg_d;
struct channel cz[5];




inline void hardware_init(void)
{
	uint8_t i ;
	keys_armed = 0;
	key_input = 0;
	cz[0].led1port = &PORTC ;
	cz[0].led1pin = PIN0;
	cz[0].led2port = &PORTC;
	cz[0].led2pin = PIN1;
	cz[0].keyport = &PINB;
	cz[0].keypin = PIN0;

	cz[1].led1port = &PORTC ;
	cz[1].led1pin = PIN2;
	cz[1].led2port = &PORTC;
	cz[1].led2pin = PIN3;
	cz[1].keyport = &PINB;
	cz[1].keypin = PIN1;

	cz[2].led1port = &PORTC ;
	cz[2].led1pin = PIN4;
	cz[2].led2port = &PORTC;
	cz[2].led2pin = PIN5;
	cz[2].keyport = &PIND;
	cz[2].keypin = PIN6;

	cz[3].led1port = &PORTD ;
	cz[3].led1pin = PIN5;
	cz[3].led2port = &PORTB;
	cz[3].led2pin = PIN2;
	cz[3].keyport = &PIND;
	cz[3].keypin = PIN7;

	cz[4].led1port = &PORTD ;
	cz[4].led1pin = PIN2;
	cz[4].led2port = &PORTD;
	cz[4].led2pin = PIN3;
	cz[4].keyport = &PIND;
	cz[4].keypin = PIN4;


/*{	PORTC,	PORTC,	PORTC,	PORTD,	PORTD };*/
/*{	PIN0,	PIN2,	PIN4,	PIN5,	PIN2  };*/
/*{	PORTC,	PORTC,	PORTC,	PORTB,	PORTD };*/
/*{	PIN1,	PIN3,	PIN5,	PIN2,	PIN3  };*/
/*{	PINB,	PINB,	PIND,	PIND,	PIND };*/
/*{	PIN0,	PIN1,	PIN6,	PIN7,	PIN4  };*/



	
	for(i=0; i< CHAN_NUM ; i++){
		/*leds get outputs */
		*(port2ddr(cz[i].led1port)) |= cz[i].led1pin ;
		*(port2ddr(cz[i].led2port)) |= cz[i].led2pin ;
		/* initially set to 1 (led off) */
		*(cz[i].led1port) |= cz[i].led1pin ;
		*(cz[i].led2port) |= cz[i].led2pin ;
		/* keys inputs */
		*(pin2ddr(cz[i].keyport)) &= ~(cz[i].keypin) ;
#ifndef KEYS_HAVE_EXTERN_PULLUP
		*(pin2port(cz[i].keyport)) |= cz[i].keypin;
#else
		*(pin2port(cz[i].keyport)) &= ~(cz[i].keypin);
#endif		
	} 

	USART_Init();

}


uint8_t set_led_state(ledid_t  led_id, uint8_t brightness)
{
	uint8_t ch;
	switch (led_id){
		case (LED1_BASE + CHAN_A) :
		case (LED1_BASE + CHAN_B) :
		case (LED1_BASE + CHAN_C) :
		case (LED1_BASE + CHAN_D) :
		case (LED1_BASE + CHAN_E) :
			ch = led_id - LED1_BASE;
			if (brightness) {
				*(cz[ch].led1port) |= cz[ch].led1pin ;
			} else {
				*(cz[ch].led2port) &= ~(cz[ch].led1pin) ;
			}
			break ;
		case (LED2_BASE + CHAN_A) :
		case (LED2_BASE + CHAN_B) :
		case (LED2_BASE + CHAN_C) :
		case (LED2_BASE + CHAN_D) :
		case (LED2_BASE + CHAN_E) :
			ch = led_id - LED2_BASE;
			if (brightness) {
				*(cz[ch].led2port) |= cz[ch].led2pin ;
			} else {
				*(cz[ch].led2port) &= ~(cz[ch].led2pin) ;
			}
			break ;
		default:
			return ERROR_LED;
	}
	return 0;
}


uint8_t send_uart_key(uint8_t key){
	char c ;
	char buf[5] = "K%\n\r";
	switch (key){
		case CHAN_A:
			c = 'A';
			break;
		case CHAN_B:
			c = 'B';
			break;	
		case CHAN_C:
			c = 'C';
			break;
		case CHAN_D:
			c = 'D';
			break;
		case CHAN_E:
			c = 'E';
			break;
		default:
			return ERROR_IKEY;
		}
	buf[1] = c;
	USART_puts(buf);
	return 0;
}



inline void arm_buttons(uint8_t buttonbits){
	
	keys_armed = 1 ;
	key_input = 0;

	return ;
	
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


uint8_t wait_keys_blocking(uint8_t keymask /* TODO timeout?,*/ );
