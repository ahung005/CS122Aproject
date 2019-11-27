/*
 * uC01_testV2.c
 *
 * Created: 11/17/2019 6:26:02 PM
 * Author : aahun
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "timer.h"
#include "usart_ATmega1284.h"

void ADC_init() {
	ADMUX = (1<<REFS1) | (1<<REFS0) ;
	ADCSRA|= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

uint16_t analog_read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF8) | (channel & 7);
	
	ADCSRA|=(1<<ADSC);
	while ( !(ADCSRA & (1<<ADIF)));
	ADCSRA|=(1<<ADIF);
	
	return (ADC);
}


int main(void) {
	DDRA = 0x00;	PORTA = 0xFF;
	DDRB = 0xFF;	PORTB = 0x00;
	DDRC = 0xFF;	PORTC = 0x00;
	
	int x;
	int y;
	char bufferx[20];
	char buffery[20];
	
	ADC_init();
	initUSART(1);
	
	TimerSet(100);
	TimerOn();
	
	unsigned char i = 0;
	
    /* Replace with your application code */
    while (1) {
		x = analog_read(0);
		y = analog_read(1);
		
		if((x < 800 && x > 300) && (y < 800 && y > 300)) {
			i = 0;
		}
		else if(x > 800 && (y < 800 && y > 300)) {
			i = 1;
		}
		else if(x > 800 && y < 300) {
			i = 2;
		}
		else if(x > 800 && y > 800) {
			i = 3;
		}
		else if((x < 800 && x > 300) && y < 100) {
			i = 4;
		}
		else if((x < 800 && x > 300) && y > 800) {
			i = 5;
		}
		else if(x < 300 && y < 300) {
			i = 6;
		}
		else if(x < 300 && y > 800) {
			i = 7;
		}
		else if(x < 300 && (y < 800 && y > 300)) {
			i = 8;
		}
		
		if(USART_IsSendReady(1)) {
			USART_Send(i, 1);
		}
		
		while(!TimerFlag);
		TimerFlag = 0;
    }
	return 0;
}
