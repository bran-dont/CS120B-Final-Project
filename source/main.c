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
#include "io.h"			// LCD
#include "timer.h"		// timer
#include "scheduler.h"	// task struct
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

//unsigned char col = 0x08;    // LED col
//unsigned char row = 0x10;    // LED row 
enum Snake_States { shift };
int Snake_Tick(int state) { // f7, 02
	// Local Variables
    static unsigned char col = 0x08;    // LED pattern - 0: LED off; 1: LED on
    static unsigned char row = 0x10;      // Row(s) displaying pattern. 
                            // 0: display pattern on row
                            // 1: do NOT display pattern on row
    unsigned short adc = ADC;
    unsigned short x, y;
    
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
			ADMUX = 0b00001;
			y = ADC;
			ADMUX = 0b00010;
			x = ADC;
			
			if(y < 263) {
				//col = 0xFF;
				row >>= 1;
				if(263 < x && x < 790 && row != 0x01) {
					row >>= 1;
				}
			}
			else if(790 < y) {
				if(263 < x && x < 790 && row != 0x80) {
					row <<= 1;
				}
			}
			else { // y in middle region
				if(x < 263 && col != 0x01) {
					col >>= 1;
				}
				else if(790 < x && col != 0x80) {
					col <<= 1;
				}
			}
			transmit_cdata(col);
			transmit_rdata(row);
			/*
			ADMUX = 0b00001;
			if(adc < 263) {
				if(col != 0x01) {
					//col >>= 1;
					y = 527 - adc;
				}
				//moved = 1;
			}
			else if(adc > 790) {
				//LCD_WriteData('0');
				if(col != 0x80){
					col <<= 1;
				}
				//moved = 1;
			}
			ADMUX = 0b00010;
			if(!moved) {
				if(adc < 263) {
					if(row != 0x01) {
						row >>= 1;
					}
				}
				else if(adc > 790) {
					if(row != 0x80) {
						row <<= 1;
					}
				}
			}
            */
            break;
        default:
		    break;
    }
    
    return state;

}

enum Output_States { output };
int Output_Tick(int state) {
	//transmit_cdata(col);
    //transmit_rdata(row);
    
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
    //LCD_init();
    //LCD_WriteData('9');
    //LCD_DisplayString(1, "Hello World");
    
    static task task1, task2;
    task *tasks[] = { &task1, &task2 }; 
    const unsigned short numTasks = sizeof(tasks)/sizeof(*tasks);
    unsigned short period = 1;
    unsigned char i;
    
    // Task 1 (Snake Spot)
    task1.state = shift; 
    task1.period = period * 100;
    task1.elapsedTime = task1.period;
    task1.TickFct = &Snake_Tick;
    
    // Task 2 (Display Output)
    task2.state = output;
    task2.period = period;
    task2.elapsedTime = task2.period;
    task2.TickFct = &Output_Tick;
    
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
