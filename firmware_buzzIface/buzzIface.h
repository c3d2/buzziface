/*
 * ============================================================================
 *
 *       Filename:  buzzIface.h
 *
 * ============================================================================
 */


typedef uint8_t u8;

//---BUZZER CHANNELS
#define CHAN_A 0
#define CHAN_B 1
#define CHAN_C 2
#define CHAN_D 3
#define CHAN_E 4
#define CHAN_NUM 5

//---BUZZER-HARDWARE CANNEL CONFIGURATION
//
struct channel {
typeof(&PORTC) led1port ;
typeof(PIN0)  led1pin ;
typeof(&PORTC) led2port ;
typeof(PIN0)  led2pin ;
typeof(&PINB)  keyport;
typeof(PIN7)  keypin;
} ; 



#define port2ddr(prt) ( (uint8_t *)( (int)&DDRB + (int)prt - (int)&(PORTB) ))
#define pin2port(pin) ((u8 *)((int)&PORTB + (int)pin - (int)&PINB ))
#define pin2ddr(pin) ( (u8*)((int)&DDRB + (int)pin - (int)&PINB ))


typedef struct {
	uint8_t channel;
	uint8_t led;
} ledid_t ;



//----------------
#define LED_OFF 0
#define LED_ON  255

//LED IDs: LED Base + CHAN num
#define LED1_BASE 16
#define LED2_BASE 32



/* ERROR DEFINITIONS: */
#define ERROR_GENERALI  1 //general error
#define ERROR_IKEY	2 //invalid keycode
#define ERROR_TIMEOUT	3 //timeout
#define ERROR_INIT	4 //init failed
#define ERROR_LED	5 //invalid LED
#define ERROR_COMAND	6 // invalid command
#define ERROR_COMCHAN	7 // invalid command channel argument
#define ERROR_COMLED	8 // invalid commane led argument





