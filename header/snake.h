
// Permission to copy is granted provided that this header remains intact. 
// This software is provided with no warranties.

////////////////////////////////////////////////////////////////////////////////

#ifndef SNAKE_H
#define SNAKE_H
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
//Struct for Snake Pixels that build up a snake body
typedef struct pixel {
	// Pixels should have members that include row and col
	unsigned char row;	//Pixel row (hex)
	unsigned char col;	//Pixel column (hex)
};
////////////////////////////////////////////////////////////////////////////////
struct pixel head = { 0x10, 0x08 };
struct pixel snake[64];				//max length is 64
unsigned char length = 1;
unsigned char direction = 0x01;		//facing right initially
////////////////////////////////////////////////////////////////////////////////
//Functionality - resets snake after loss/win
//Parameter: N/A
//Returns: N/A
void Snake_init() {
	head.row = 0x10;
	head.col = 0x08;
	struct pixel temp[64] = { head };
	memcpy(snake, temp, sizeof(temp));
	length = 1;
}

//Functionality - moves snake forward in direction
//Parameter: N/A
//Returns: display - row and col values to be displayed on matrix
struct pixel move() {
	unsigned char i = 0;
	struct pixel temp = snake[0];
	switch (direction) {
		case 0x08:
			snake[0].row >>= 1;
			break;
		case 0x04:
			snake[0].row <<= 1;
			break;
		case 0x02:
			snake[0].col <<= 1;
			break;
		case 0x01:
			snake[0].col >>= 1;
			break;
		default:
			break;
	}
	//for(i = 1; i < length; i++) {	
	//}
	head = snake[0];
	
	struct pixel display = { 0x00, 0x00 };
	for(i = 0; i < length; i++) {
		display.row |= snake[i].row;
		display.col |= snake[i].col;
	}
	return display;
}

//Functionality - changes direction using input from joystick
//Parameter: dir - input from joystick
//Returns: N/A
void updateDir(unsigned char dir) {
	if(dir != 0x00) {
		switch (direction) {
			case 0x08:
				if(dir != 0x04) { direction = dir; }
				break;
			case 0x04:
				if(dir != 0x08) { direction = dir; }
				break;
			case 0x02:
				if(dir != 0x01) { direction = dir; }
				break;
			case 0x01:
				if(dir != 0x02) { direction = dir; }
				break;
			default:
				break;
		}
	}
}

//Functionality - determine game loss
//Parameter: N/A
//Returns: 1 if game fail, 0 if not
unsigned char failCollision() {
	return head.col == 0x00 || head.row == 0x00;
}

//Functionality - checks if head has collided with a specific pixel (namely food)
//Parameter: N/A
//Returns: bool - 1 if true, 0 if false
unsigned char foodCollision(const unsigned char r, const unsigned char c) { 
	//return 1;
	return ((snake[0].row == r) && (snake[0].col == c)); 
	//return snake[0].col;
}

//Functionality - extends length of snake after consuming food
//Parameter: N/A
//Returns: 1 if game win, 0 otherwise
unsigned char grow() {
	if(length == 63) { return 1; } //game win
	
	struct pixel tail;
	if(length == 1) {
		switch (direction) {
			case 0x08:
				tail.row = head.row << 1;
				tail.col = head.col;
				break;
			case 0x04:
				tail.row = head.row >> 1;
				tail.col = head.col;
				break;
			case 0x02:
				tail.row = head.row;
				tail.col = head.col >> 1;
				break;
			case 0x01:
				tail.row = head.row;
				tail.col = head.col << 1;
				break;
			default:
				LCD_DisplayString(1, "ERROR");
				break;
		}
	}
	//for(unsigned char i; i < length;
	length = length + 1;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////

#endif //SCHEDULER_H
