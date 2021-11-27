/*	Author: Brandon Tran
 *  Partner(s) Name: 
 *	Lab Section: 22
 *	Assignment: Lab #FP  Exercise #
 *	Exercise Description: Final Project - Snake
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <stdlib.h> // used for rand #
#include <math.h> // pow
#include "timer.h"
#include "scheduler.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void transmit_pdata(unsigned char data) {
	PORTA |= 0x10;
	int i;
    for (i = 0; i < 8 ; ++i) {
        // Sets SRCLR to 1 allowing data to be set
        // Also clears SRCLK in preparation of sending data
        PORTA = 0x08;
        // set SER = next bit of data to be sent.
        PORTA |= ((data >> i) & 0x01);
        // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
        PORTA |= 0x02;  
    }
    // set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
    PORTA &= 0xEF;
    PORTA |= 0x04;
    // clears all lines in preparation of a new transmission
    PORTA = 0x00;
}

void transmit_rdata(unsigned char data) {
	PORTB |= 0x10;
	int i;
    for (i = 0; i < 5 ; ++i) {
        // Sets SRCLR to 1 allowing data to be set
        // Also clears SRCLK in preparation of sending data
        PORTB = 0x08;
        // set SER = next bit of data to be sent.
        PORTB |= ((data >> i) & 0x01);
        // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
        PORTB |= 0x02;  
    }
    // set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
    PORTB &= 0xEF;
    PORTB |= 0x04;
    // clears all lines in preparation of a new transmission
    PORTB = 0x00;
}

unsigned char pattern = 0x04; // snake spawn point
unsigned char row = 0xF7;
unsigned char length = 1;

enum Snake_States { Move };
int Snake_Tick(int state) {
	//static unsigned char pattern = 0x04; // snake spawn point
	//static unsigned char row = 0xF7;
	
	switch(state) {
		case Move:
			transmit_pdata(pattern);
			transmit_rdata(row);
		break;
		default:
		break;
	}
	return state;
}

enum Demo_States {shift};
int Demo_Tick(int state) {
	// Local Variables
	static unsigned char foodLoc;
		foodLoc = rand() % (40 - length);
	 
	static unsigned char fpattern = 0x40;
	if(fpattern == pattern) {
		pattern = foodLoc % 8; // snake spawn point
	}
	static unsigned char frow = 0xFD;
	if(frow == row) { frow = (char)(pow(2, foodLoc / 8)); }
	
	while(fpattern == pattern && frow == row) { 
		foodLoc = rand() % (40 - length);
		fpattern = foodLoc % 8;
		frow = (char)(pow(2, foodLoc / 8));
	}
	
	//static unsigned char newLoc = rand() % 
	
	transmit_pdata(fpattern);
	transmit_rdata(frow);
	// 0xFB = 1111 1011 & ~(0001 0000) = 1111 1011 & 1110 1111 = 1110 1011
	//16 - > 1 000 -> 0 1111
	return state;
}
/*
int Demo_Tick(int state) {

    // Local Variables
    static unsigned char pattern = 0x80;	// LED pattern - 0: LED off; 1: LED on
    static unsigned char row = 0xFE;		// Row(s) displaying pattern. 
    										// 0: display pattern on row
    										// 1: do NOT display pattern on row
    // Transitions
    switch (state) {
        case shift:    
			break;
        default:    
			state = shift;
        	break;
    }    
    // Actions
    switch (state) {
		case shift:    
			if (row == 0xEF && pattern == 0x01) { // Reset demo 
                pattern = 0x80;
                row = 0xFE;
            } else if (pattern == 0x01) { // Move LED to start of next row
                pattern = 0x80;
                row = (row << 1) | 0x01;
            } else { // Shift LED one spot to the right on current row
                pattern >>= 1;
            }
            transmit_pdata(pattern);
            transmit_rdata(row);
        	break;
        default:
        	transmit_pdata(pattern);
        	transmit_rdata(row);
    		break;
    }
    //transmit_rdata(row);
    //PORTA = pattern;    // Pattern to display
    //PORTB = row;        // Row(s) displaying pattern    
    return state;
}*/


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
    static task task1, task2;
    task *tasks[] = { &task1, &task2 }; 
    const unsigned short numTasks = sizeof(tasks)/sizeof(*tasks);
    unsigned short period = 1;
    unsigned char i;
    
    // Task 1 (Snake Spot)
    task1.state = shift; 
    task1.period = period;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Snake_Tick;
    
    // Task 1 (Food Spot)
    task2.state = shift; 
    task2.period = period;
    task2.elapsedTime = task2.period;
    task2.TickFct = &Demo_Tick;
    
    unsigned long GCD = tasks[0]->period;
    for(i = 1; i < numTasks; i++) {
    	GCD = findGCD(GCD, tasks[i]->period);
    }
    
    TimerSet(GCD);
    TimerOn();
    
    while (1) {
		for(i = 0; i < numTasks; i++) {
			if(tasks[i]->elapsedTime == tasks[i]->period) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += GCD;
		}
		while(!TimerFlag);
		TimerFlag = 0;
    }
    return 1;
}
