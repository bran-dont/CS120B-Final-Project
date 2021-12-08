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
#include <stdlib.h> 	// used for rand #
#include <math.h> 		// pow
#include <stdio.h>		// printf (short to string)
#include "io.h"			// LCD
#include "timer.h"		// timer
#include "scheduler.h"	// task struct
#include "snake.h"		// snake data
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

/*
	for PORTB and PORTD
	PIN0 - SER
	PIN1 - Output enable (output Storage Reg)
	PIN2 - RCLK
	PIN3 - SRCLK
	PIN4 - SRCLR
*/

void ADC_init() { // 527 is middle for joystick
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are in Free Running Mode,
	//			a new conversion will trigger whenever the previous conversion completes.
}


void transmit_cdata(unsigned char data) {
	unsigned char inverse = ~data;
	PORTB |= 0x02; // OUTEN = 1, turns off storage register output until reg is ready
	
	int i;
    for (i = 0; i < 8 ; ++i) {
        PORTB |= 0x10; // SRCLR = 1, allows data to be set
        PORTB &= 0xF0; // SRCLK = 0, to prepare for sending data
        
        //PORTB = 0x10;
        
        PORTB |= ((inverse >> i) & 0x01); // SER = next data bit to be sent
        
        PORTB |= 0x08; // SRCLK = 1, rising edge shifts SER into shift reg  
    }
    PORTB |= 0x04; // RCLK = 1, rising edge copies data from shift reg to storage reg
    
    PORTB &= 0xC0; // PB5..0 = 0, clears all lines to prep for new data (PB7..6 used for LCD)
				   // PB1 = 0 also outputs reg to LED matrix (OUTEN = 0)
	//PORTB = 0x00;
}

void transmit_rdata(unsigned char data) {
	PORTD |= 0x02; // OUTEN = 1, turns off storage register output until reg is ready
	
	int i;
    for (i = 0; i < 8 ; ++i) {
        PORTD |= 0x10; // SRCLR = 1, allows data to be set
        PORTD &= 0xF0; // SRCLK = 0, to prepare for sending data
        
        //PORTD = 0x10;
        
        PORTD |= ((data >> i) & 0x01);  // SER = next data bit to be sent
        
        PORTD |= 0x08; // SRCLK = 1, rising edge shifts SER into shift reg
    }
    PORTD |= 0x04; // RCLK = 1, rising edge copies data from shift reg to storage reg
    
    PORTD &= 0xC0; // PB5..0 = 0, clears all lines to prep for new data (PB7..6 used for LCD)
				   // PB1 = 0 also outputs reg to LED matrix (OUTEN = 0)
				   
	//PORTD = 0x00;
}

unsigned char dir = 0x00;	// direction of snake head by input, b3..0 = up down left right
enum Input_States { shift };
int Input_Tick(int state) { // f7, 02
	// Local Variables
    unsigned short adc = ADC;
    static unsigned long x, y;
    static unsigned char axis = 1;
    //static unsigned char dir = 0x00; // b3..0 = up down left right
    
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
    		if(axis == 1) {
    			ADMUX = (ADMUX & 0xE0) | axis;
				y = adc;
				axis = 2;
			}
			else if(axis == 2) {
				ADMUX = (ADMUX & 0xE0) | axis;
				x = adc;
				axis = 1;
			}
			
			if(y > 1006 && axis == 1 && dir == 0x00) {
				// down
				//row = (row != 0x80) ? row << 1 : row;
				dir = 0x04;
			}
			else if(y < 32 && axis == 1 && dir == 0x00) {
				// up
				//row = (row != 0x01) ? row >> 1 : row;
				dir = 0x08;
			}
			else if(x > 1006 && axis == 2 && dir == 0x00) {
				// left
				//col = (col != 0x80) ? col << 1 : col;
				dir = 0x02;
			}
			else if(x < 32 && axis == 2 && dir == 0x00) {
				// right
				//col = (col != 0x01) ? col >> 1 : col;
				dir = 0x01;
			}
			else if(250 < y && y < 750 && 250 < x && x < 750) {
				dir = 0x00;
			}
			else {}
			
			if(dir != 0x00) { updateDir(dir); }
			
			//unsigned char* str;
			//sprintf(str, "%d", dir);
			//LCD_DisplayString(1, dir);
			//LCD_ClearScreen();
			//LCD_WriteData(dir + '0');
    		break;
        default:
		    break;
    }
    
    return state;
}

unsigned char col = 0x08;	// LED col
unsigned char row = 0x10;	// LED row 

unsigned char fcol = 0x20;
unsigned char frow = 0x10;

enum Move_States { MS_move };
int Move_Tick(int state) {
	
	struct pixel display = move();
	row = display.row;
	col = display.col;
	
	
	if(foodCollision(frow, fcol)) {
		LCD_WriteData('1');
		frow = (0x01 << (rand() % 8)); // random number between 0 and 7
		fcol = (0x01 << (rand() % 8));
	}
	
	
	unsigned char* str;
	sprintf(str, "%d", foodCollision(frow, fcol));
	//unsigned char* str2;
	//sprintf(str, "%d", frow);
	LCD_DisplayString(1, str);
	
	//LCD_ClearScreen();
	/*
	LCD_WriteData(frow + '0');
	LCD_WriteData(' ');
	LCD_WriteData(fcol + '0');
	*/
	
	return state;
}

enum Output_States { output };
int Output_Tick(int state) {
    transmit_cdata(col);
	transmit_rdata(row);
	
	transmit_cdata(0x00);
	transmit_rdata(0x00);
	
	transmit_cdata(fcol);
	transmit_rdata(frow);
	
	transmit_cdata(0x00);
	transmit_rdata(0x00);
    
	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF; // Joystick input 
	DDRB = 0xFF; PORTB = 0x00; // LED Matrix output (pattern) and LCD Control Lines
	DDRC = 0xFF; PORTC = 0x00; // LCD Data Lines
	DDRD = 0xFF; PORTD = 0x00; // LED Matrix output (row)
	
    /* Insert your solution below */
    ADC_init();
    LCD_init();
    LCD_ClearScreen();
    Snake_init();
    srand(time(0));
    
    static task task1, task2, task3;
    task *tasks[] = { &task1, &task2, &task3 }; 
    const unsigned short numTasks = sizeof(tasks)/sizeof(*tasks);
    unsigned short period = 1;
    unsigned char i;
    
    // Task 1 (ADC Inputs)
    task1.state = shift; 
    task1.period = period * 2;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Input_Tick;
    
    // Task 2 (Move Snake)
    task2.state = MS_move;
    task2.period = period * 1000;
    task2.elapsedTime = task2.period;
    task2.TickFct = &Move_Tick;
    
    // Task 2 (Display Output)
    task3.state = output;
    task3.period = period;
    task3.elapsedTime = task3.period;
    task3.TickFct = &Output_Tick;
    
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
