/*
 * uC1_testV1.c
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
#include "usart_ATmega1284.h"
#include "timer.h"

#define  Trigger_pin	PA0	//this is for the Sonar sensor

//ultrasonic sensor global variable
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
void Radar(){
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
	
	initUSART(0);
	initUSART(1);
	//LCD initialization
	LCD_init();
	
	Radar();
	LCD_ClearScreen();
	
	
	//LCD_DisplayString(1, "Distance(cm)");
	unsigned char temp = 0;
	
    while (1) 
    {
		Radar();
		itoa(distance,string,10);
		//dtostrf(distance, 5, 2, string); // Convert distance into string
		//LCD_DisplayString(1, "Distance");
		LCD_ClearScreen();
		LCD_DisplayString(1, string);
		
		
		if(USART_HasReceived(1)) {
			temp = USART_Receive(1);
			USART_Flush(1);
			if(temp == 0x00) {
				LCD_DisplayString(17, "center");
			} 
			else if(temp == 0x01) {
				LCD_DisplayString(17, "front");
			}
			else if(temp == 0x02) {
				LCD_DisplayString(17, "front-left");
			}
			else if(temp == 0x03) {
				LCD_DisplayString(17, "front-right");
			}
			else if(temp == 0x04) {
				LCD_DisplayString(17, "left");
			}
			else if(temp == 0x05) {
				LCD_DisplayString(17, "right");
			}
			else if(temp == 0x10) {
				LCD_DisplayString(17, "left-rear");
			}
			else if(temp == 0x20) {
				LCD_DisplayString(17, "right-rear");
			}
			else if(temp == 0x30) {
				LCD_DisplayString(17, "rear");
			}
		}
		delay_ms(300);
    }
}
