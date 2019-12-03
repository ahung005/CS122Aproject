/*
 * uC1_car.c
 *
 * Created: 11/30/2019 2:43:07 PM
 * Author : aahun
 */ 
#define F_CPU 8000000UL
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/portpins.h>
#include <avr/pgmspace.h>
#include "usart_ATmega1284.h"

#define  Trigger_pin	PA0	//this is for the Sonar sensor
//ultrasonic sensor global variable
int TimerOverflow = 0; // variable overflow for the SonarSensor
long count;
//double distance = 0.00;
int distance = 0;
//Radar Functions
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
	_delay_us(15000);
}

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"

unsigned char distFlag;
unsigned char temp;

// Distance SM
enum DistanceStates {dist_init,dist01} distance_state;

void Distance_Init(){
	distance_state = dist_init;
}

void Distance_Tick(){
	//Actions
	switch(distance_state){
		case dist_init:
			distFlag = 0;
			break;
		case dist01:
			Radar();
			if(distance <= 35) {
				distFlag = 1;
				PORTB |= (1 << PB0); // PB0 goes high
			}
			else {
				distFlag = 0;
				PORTB &= ~(1 << PB0); // PD0 goes low
			}
			break;
		default:
			break;
	}
	//Transitions
	switch(distance_state){
		case dist_init:
			distance_state = dist01;
			break;
		case dist01:
			distance_state = dist01;
			break;
		default:
			distance_state = dist_init;
			break;
	}
}

void DistanceOut() {
	Distance_Init();
	for(;;) {
		Distance_Tick();
		vTaskDelay(20);
	}
}

void Distance(unsigned portBASE_TYPE Priority) {
	xTaskCreate(DistanceOut,
	(signed portCHAR *)"DistanceOut",
	configMINIMAL_STACK_SIZE,
	NULL,
	Priority,
	NULL );
}
// Distance SM

// Receive SM
enum ReceiveStates {re_init,re01} receive_state;

void Receive_Init(){
	receive_state = re_init;
}

void Receive_Tick(){
	//Actions
	switch(receive_state){
		case re_init:
			temp = 0;
			break;
		case re01:
			if(USART_HasReceived(1)) {
				temp = USART_Receive(1);
				USART_Flush(1);
			}
			break;
		default:
			break;
	}
	//Transitions
	switch(receive_state){
		case re_init:
			receive_state = re01;
			break;
		case re01:
			receive_state = re01;
			break;
		default:
			receive_state = re_init;
			break;
	}
}

void ReceiveOut() {
	Receive_Init();
	for(;;) {
		Receive_Tick();
		vTaskDelay(1);
	}
}

void Receive(unsigned portBASE_TYPE Priority) {
	xTaskCreate(ReceiveOut,
	(signed portCHAR *)"ReceiveOut",
	configMINIMAL_STACK_SIZE,
	NULL,
	Priority,
	NULL );
}
// Receive SM

// Drive SM
enum DriveStates {drive_init, stop, front, frontleft, frontright, left, right, rearleft, rearright, rear} drive_state;

void Drive_Init(){
	drive_state = drive_init;
}

void Drive_Tick(){
	//Actions
	switch(drive_state){
		case drive_init:
			PORTC = 0;
			break;
		case stop:
			PORTC = 0;
			break;
		case front:
			PORTC = 0x55;
			break;
		case frontleft:
			PORTC = 0x59;
			break;
		case frontright:
			PORTC = 0x56;
			break;
		case left:
			PORTC = 0x69;
			break;
		case right:
			PORTC = 0x96;
			break;
		case rearleft:
			PORTC = 0x9A;
			break;
		case rearright:
			PORTC = 0x6A;
			break;
		case rear:
			PORTC = 0xAA;
			break;
		default:
			break;
	}
	//Transitions
	switch(drive_state){
		case drive_init:
			drive_state = stop;
			break;
		case stop:
			if(temp == 0) {
				drive_state = stop;
			}
			else if(temp == 1 && !distFlag) {
				drive_state = front;
			}
			else if(temp == 2 && !distFlag) {
				drive_state = frontleft;
			}
			else if(temp == 3 && !distFlag) {
				drive_state = frontright;
			}
			else if(temp == 4 && !distFlag) {
				drive_state = left;
			}
			else if(temp == 5 && !distFlag) {
				drive_state = right;
			}
			else if(temp == 6) {
				drive_state = rearleft;
			}
			else if(temp == 7) {
				drive_state = rearright;
			}
			else if(temp == 8) {
				drive_state = rear;
			}
			else {
				drive_state = stop;
			}
			break;
		case front:
			if(temp == 1 && !distFlag) {
				drive_state = front;
			}
			else {
				drive_state = stop;
			}
			break;
		case frontleft:
			if(temp == 2 && !distFlag) {
				drive_state = frontleft;
			}
			else {
				drive_state = stop;
			}
			break;
		case frontright:
			if(temp == 3 && !distFlag) {
				drive_state = frontright;
			}
			else {
				drive_state = stop;
			}
			break;
		case left:
			if(temp == 4 && !distFlag) {
				drive_state = left;
			}
			else {
				drive_state = stop;
			}
			break;
		case right:
			if(temp == 5 && !distFlag) {
				drive_state = right;
			}
			else {
				drive_state = stop;
			}
			break;
		case rearleft:
			if(temp == 6) {
				drive_state = rearleft;
			}
			else {
				drive_state = stop;
			}
			break;
		case rearright:
			if(temp == 7) {
				drive_state = rearright;
			}
			else {
				drive_state = stop;
			}
			break;
		case rear:
			if(temp == 8) {
				drive_state = rear;
			}
			else {
				drive_state = stop;
			}
			break;
		default:
			drive_state = drive_init;
		break;
	}
}

void DriveOut() {
	Drive_Init();
	for(;;) {
		Drive_Tick();
		vTaskDelay(1);
	}
}

void Drive(unsigned portBASE_TYPE Priority) {
	xTaskCreate(DriveOut,
	(signed portCHAR *)"DriveOut",
	configMINIMAL_STACK_SIZE,
	NULL,
	Priority,
	NULL );
}
// Drive SM

int main(void) {
	DDRA = 0x0F; PORTA = 0xF0;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0x00; PORTD = 0xFF;
	
	TIMSK1 = (1 << TOIE1);
	TCCR1A = 0;
	
	sei();
	
	initUSART(1);
	//Radar();
	
	//Start Tasks
	Distance(1);
	Receive(1);
	Drive(1);
	//RunSchedular
	vTaskStartScheduler();

	return 0;
}
