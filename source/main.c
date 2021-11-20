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
#include "timer.h"
#include "scheduler.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum Demo_States {shift};
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
        	break;
        default:
    		break;
    }
    PORTA = pattern;    // Pattern to display
    PORTB = row;        // Row(s) displaying pattern    
    return state;
}


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
    static task task1;
    task *tasks[] = { &task1 }; 
    const unsigned short numTasks = sizeof(tasks)/sizeof(*tasks);
    unsigned char period = 100;
    unsigned char i;
    
    // Task 1 (LED cycle)
    task1.state = shift; 
    task1.period = period;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Demo_Tick;
    
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
