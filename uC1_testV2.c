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
#include "lcd.h"	// Include LCD header file 
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
	TimerOverflow++;		// Increment Timer Overflow count 
}
void Radar(){
	PORTA |= (1 << Trigger_pin); // Give 10us trigger pulse on trig. pin to HC-SR04 
	_delay_us(10);
	PORTA &= (~(1 << Trigger_pin));
	
	TCNT1 = 0;			// Clear Timer counter 
	TCCR1B = 0x41;		// Setting for capture rising edge, No pre-scaler
	TIFR1 = 1<<ICF1;		// Clear ICP flag (Input Capture flag) 
	TIFR1 = 1<<TOV1;		// Clear Timer Overflow flag

	//Calculate width of Echo by Input Capture (ICP) on PortD PD6 
	
	if ((TIFR1 & (1 << ICF1)) == 0);	// Wait for rising edge 
	TCNT1 = 0;			// Clear Timer counter 
	TCCR1B = 0x01;		// Setting for capture falling edge, No pre-scaler 
	TIFR1 = 1<<ICF1;		// Clear ICP flag (Input Capture flag) 
	TIFR1 = 1<<TOV1;		// Clear Timer Overflow flag 
	TimerOverflow = 0;	// Clear Timer overflow count 

	if ((TIFR1 & (1 << ICF1)) == 0); // Wait for falling edge 
	count = ICR1 + (65535 * TimerOverflow);	// Take value of capture register 
	
	distance = (double)count / 760.47 ;
	//distance = ((double)count / 466.47);
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
	
	initUSART(1);	
	Radar();
	
	unsigned char temp = 0;
	unsigned char distflag = 0;
	
    while (1) {
		Radar();
		
		//itoa(distance,string,10);
		//dtostrf(distance, 5, 2, string); // Convert distance into string
		//LCD_DisplayString(1, "Distance");
		//LCD_ClearScreen();
		//LCD_DisplayString(1, string);
		
		if(distance < 20.00) {
			PORTB |= (1 << PB0); // PB0 goes high
			distflag = 1;
		}
		else {
			PORTB &= ~(1 << PB0); // PD0 goes low
			distflag = 0;
		}
		_delay_us(5000);
		
		if(USART_HasReceived(1) && !distflag) {
			temp = USART_Receive(1);
			USART_Flush(1);
			if(temp == 0) {
				//LCD_DisplayString(17, "center");
				PORTC = 0;
			} 
			else if(temp == 1) {
				//LCD_DisplayString(17, "front");
				PORTC = 0x55;
			}
			else if(temp == 2) {
				//LCD_DisplayString(17, "front-left");
				PORTC = 0x59;
			}
			else if(temp == 3) {
				//LCD_DisplayString(17, "front-right");
				PORTC = 0x56;
			}
			else if(temp == 4) {
				//LCD_DisplayString(17, "left");
				PORTC = 0x69;
			}
			else if(temp == 5) {
				//LCD_DisplayString(17, "right");
				PORTC = 0x96;
			}
			else if(temp == 6) {
				//LCD_DisplayString(17, "left-rear");
				PORTC = 0x9A;
			}
			else if(temp == 7) {
				//LCD_DisplayString(17, "right-rear");
				PORTC = 0x6A;
			}
			else if(temp == 8) {
				//LCD_DisplayString(17, "rear");
				PORTC = 0xAA;
			}
		}
		//delay_ms(300);
    }
}
