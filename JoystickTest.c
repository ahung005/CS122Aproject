/*
 * JoystickTest.c
 *
 * Created: 11/17/2019 6:26:02 PM
 * Author : aahun
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "lcd.h"

void ADC_init() {
	
	ADMUX = (1<<REFS1) | (1<<REFS0) ;
	ADCSRA|= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}
uint16_t analog_read(uint8_t channel)
{
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
	//ADC_Init();
	LCD_init();
	LCD_ClearScreen();
	
    /* Replace with your application code */
    while (1) {
		x = analog_read(0);
		//x = ADC_Read(0);
		LCD_ClearScreen();
		sprintf(bufferx, "X=%d   ", x);
		LCD_DisplayString(1, bufferx);
		
		y = analog_read(1);
		//y = ADC_Read(1);
		sprintf(buffery, "Y=%d   ", y);
		LCD_DisplayString(8, buffery);
		
		if((x < 800 && x > 300) && (y < 800 && y > 300)) {
			LCD_DisplayString(17, "center");	
		}
		else if(x > 800 && (y < 800 && y > 300)) {
			LCD_DisplayString(17, "front");
		}
		else if(x > 800 && y < 300) {
			LCD_DisplayString(17, "left-front");
		}
		else if(x > 800 && y > 800) {
			LCD_DisplayString(17, "right-front");
		}
		else if((x < 800 && x > 300) && y < 100) {
			LCD_DisplayString(17, "left");
		}
		else if((x < 800 && x > 300) && y > 800) {
			LCD_DisplayString(17, "right");
		}
		else if(x < 300 && y < 300) {
			LCD_DisplayString(17, "left-rear");
		}
		else if(x < 300 && y > 800) {
			LCD_DisplayString(17, "right-rear");
		}
		else if(x < 300 && (y < 800 && y > 300)) {
			LCD_DisplayString(17, "rear");
		}
		delay_ms(200);
    }
}

