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

//uint8_t wait_keys_blocking(uint8_t keymask /* TODO timeout?,*/ );

uint8_t send_uart_key(uint8_t key , uint8_t is_key_up_ev);
static void PCI_init(void );
static void phasentimer_init(void);
static uint8_t do_command (uint8_t * commline);

/* GLOBALZ */

volatile uint8_t key_buf;





#define MASKI_B (0x03)
#define MASKI_D (0xd0)
#define SENDPLACESIZE (8)
#define DEBUG_SENDPLACE_ON

#undef KEYS_HAVE_EXTERN_PULLUP

volatile uint8_t changedmask;
volatile uint8_t hasunicorn;
volatile uint8_t pendingmask;
#define MAXUNICORN (6)
volatile uint8_t phasechanged[MAXUNICORN];
volatile uint8_t unicorn; //index to last free in phasechanged

volatile uint8_t sendline[SENDPLACESIZE] ;
volatile uint8_t sendplace ;

struct channel cz[5];


ISR(PCINT0_vect)
{
	uint8_t act ;
	act = ~PINB & (MASKI_B);
	if (act){  //one ore some of allowed ins are high
		switch (act){
			case (1 << 0): //Chan A key is B0 --> PCINT0 
			case (1 << 1): //Chan B key is B1 --> PCINT1
				PCMSK0 &= ~(act);
				changedmask |= act; 
				if (act & ~(hasunicorn)){
					phasechanged[unicorn++] = act;
					hasunicorn |= act ;
				}
				break;
			default :
				sendline[sendplace++] = 'G'; //invalid

		}
		if(unicorn >= MAXUNICORN) USART_puts("unicornl> MAX\n\r");
	} 
}

ISR(PCINT2_vect) {
	uint8_t act,c ;
	act = ~PIND & MASKI_D ;
	if (act){
		switch (act){
			case (1 << 6): //Chan C key is D6 --> PCINT22
				c=CHAN_C;
				PCMSK2 &= ~(act);
				break;
			case (1 << 7): //Chan D key is D7 --> PCINT23
				c=CHAN_D;
				PCMSK2 &= ~(act);
				break;
			case (1<<4): //Chan E key is D4 --> PCINT20
				c=CHAN_E;
				PCMSK2 &= ~(act);
				break;
			default :
				c= 0x0;
		}
				if (c )
				{
				changedmask |= (1<<c);
				if (c & ~(hasunicorn)) {
					phasechanged[unicorn++] = c;
					hasunicorn |= c;
				}
				if(unicorn >= MAXUNICORN) USART_puts("unicornl> MAX\n\r");
				}	
	}
}

ISR(TIMER0_COMPA_vect) {

	uint8_t j ,c;
	if(!changedmask) {//es gab keine flanken
		/* flanken wieder erlauben zu finden*/
		for (j=0; j <8 ; j++){
			c = phasechanged[j];
			if (c)
				send_uart_key(c , (*(cz[c-1].keyport) & (1 << cz[c-1].keypin) ) ) ;
			phasechanged[j] =0;
		}
		unicorn =0;
		hasunicorn =0;
	}
	//flanken wieder suchen:
	changedmask = 0;
	PCMSK0 |= (MASKI_B) ;
	PCMSK2 |= (MASKI_D) ;
}



static inline void hardware_init(void)
{
	uint8_t i ;
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

	sendplace = 0 ;
	USART_Init();
	PCI_init();
	phasentimer_init();
}



static inline void phasentimer_init()
{
//20MHz / 1024 = 19 KHz
//prescaler
//(1.0/19) * 18 = 0.9473684 -> phasenwechsel alle ~ millisekunde
	TCCR0A = 0 | (1 << WGM01);              // CTC Modus
	TCCR0B = 0 | (1<< CS02) | (1<< CS00);   //Prescale 1024
	OCR0A =  20; //0x09 ; 				//9 ist 18/2 s.o.
	TIMSK0 = 0 |  (1<<OCIE0A);		//IRQ CTM erlauben 
	return;
}

static inline void PCI_init()
{

	PCIFR = 0; //sanity
	PCMSK0 = MASKI_B;
	PCMSK2 = MASKI_D;
	PCICR = (1<<PCIE0) | (1<<PCIE2); //dont have keys at port c ... 
	return;
}



uint8_t set_led_state(ledid_t  led_id, uint8_t brightness)
{
	uint8_t ch=led_id.channel;
	switch (led_id.led){
		case (0) :
			if (brightness) {
				USART_puts("on(l0)\r\n");
				*(cz[ch].led2port) &= ~(cz[ch].led1pin) ;
			} else {
				USART_puts("off(l0)\r\n");
				*(cz[ch].led1port) |= cz[ch].led1pin ;
			}
			break ;
		case (1) :
			if (brightness) {
				USART_puts("on(l1)\r\n");
				*(cz[ch].led2port) &= ~(cz[ch].led2pin) ;
			} else {
				USART_puts("off(l1)\r\n");
				*(cz[ch].led2port) |= cz[ch].led2pin ;
			}
			break ;
		default:
			return ERROR_LED;
	}
	return 0;
}


uint8_t send_uart_key(uint8_t key ,uint8_t is_key_up_ev){
	char c ;
	char buf[5] = "K%\n\r";
	if ( is_key_up_ev ){
		//key down event get small k
		buf[0] = 'k';
	}
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

static inline uint8_t  do_setled_command(uint8_t * commline){
	ledid_t lid;
	switch (commline[0]){
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
			USART_puts("\r\nsetled ");
			USART_putc(commline[0]);
			lid.channel = commline[0] - 'A';
			break;

		default:
			return ERROR_COMCHAN ;
	}
	switch (commline[1]){
		case '1':
		case '2':
			USART_putc(commline[1]);
			lid.led = commline[1] - '1';
			break;

		default:
			return ERROR_COMLED ;
	}
	return set_led_state (lid, commline[2]) ;
}

	



static uint8_t do_command (uint8_t * commline){
	
	switch (commline[0]){
		case 'L':
			return do_setled_command(commline+1);
		default:
			return ERROR_COMAND ;
	}
	return 0; //useless
}



int main(void)
{
	uint8_t  commandbuf[8];
	uint8_t  commidx=0 , i=0 ;
	hardware_init();
	sei();
//------------------------------------
	
//	USART_puts("Test");
//	USART_puts("\n\r");

	
	for(;;){  //ever
		/* PART 1 :check if host tells us to do something:*/
		i = uart_getc_nb(&commandbuf[commidx]);
		if(i){
			if (commandbuf[commidx] == 'Q'){
				//befehlszeile zu ende :-)
				commandbuf[commidx] = 0 ;
				USART_puts(commandbuf); //DEBUG
				do_command(commandbuf);
				commidx = 0;
				commandbuf[0] = 0 ;
			}else{
				commidx ++ ;
				if(commidx >= 8 ){
					commidx = 0;
					commandbuf[0] = 0 ;
				}
			}
		}
	}

	/* never gets here*/
	return 0;
}

