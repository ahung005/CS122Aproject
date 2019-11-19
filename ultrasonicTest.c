/*
 * ultrasonicTest.c
 *
 * Created: 11/14/2019 6:43:57 PM
 * Author : aahun
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"	/* Include LCD header file */

#define  Trigger_pin	PA0	//this is for the Sonar sensor

//sonic sensor global variable
int TimerOverflow = 0; // variable overflow for the SonarSensor
long count;
//double distance = 0.00;
int distance = 0;

//lcd variable
char string[10];

//Sonar Functions
ISR(TIMER1_OVF_vect)
{
	TimerOverflow++;		/* Increment Timer Overflow count */
} 
void Sonar(){
	PORTA |= (1 << Trigger_pin);/* Give 10us trigger pulse on trig. pin to HC-SR04 */
	_delay_us(10);
	PORTA &= (~(1 << Trigger_pin));
	
	TCNT1 = 0;			/* Clear Timer counter */
	TCCR1B = 0x41;		/* Setting for capture rising edge, No pre-scaler*/
	TIFR1 = 1<<ICF1;		/* Clear ICP flag (Input Capture flag) */
	TIFR1 = 1<<TOV1;		/* Clear Timer Overflow flag */

	/*Calculate width of Echo by Input Capture (ICP) on PortD PD6 */
	
	if ((TIFR1 & (1 << ICF1)) == 0);	/* Wait for rising edge */
	TCNT1 = 0;			/* Clear Timer counter */
	TCCR1B = 0x01;		/* Setting for capture falling edge, No pre-scaler */
	TIFR1 = 1<<ICF1;		/* Clear ICP flag (Input Capture flag) */
	TIFR1 = 1<<TOV1;		/* Clear Timer Overflow flag */
	TimerOverflow = 0;	/* Clear Timer overflow count */

	if ((TIFR1 & (1 << ICF1)) == 0); /* Wait for falling edge */
	count = ICR1 + (65535 * TimerOverflow);	/* Take value of capture register */
	
	//distance = (double)count / 760.47 ;
	distance = (int)((double)count / 466.47);
}



int main(void)
{
	DDRA = 0x0F; PORTA = 0xF0;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0x00; PORTD = 0xFF;
	
	TIMSK1 = (1 << TOIE1);
	TCCR1A = 0;
	
	sei();
	
	//LCD initialization
	LCD_init();
	
	Sonar();
	LCD_ClearScreen();
	LCD_DisplayString(1, "Distance(cm)");
	
	
    while (1) 
    {
		Sonar();
		itoa(distance,string,10);
		//dtostrf(distance, 5, 2, string); // Convert distance into string
		//LCD_DisplayString(1, "Distance");
		LCD_DisplayString(17, string);
		delay_ms(300);
    }
}
