/* Mouse example with debug channel, for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_mouse.html
 * Copyright (c) 2009 PJRC.COM, LLC
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "usb_tablet_debug.h"
#include "print.h"
#include "uart.h"

#define LED_CONFIG	(DDRD |= (1<<6))
#define LED_ON		(PORTD &= ~(1<<6))
#define LED_OFF		(PORTD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

uint16_t scantime = 0;
void t3setup(void);

int main(void)
{
	uint16_t y_current = 0;
	uint8_t xl, xh, yl, yh, pressure;
	uint8_t buttons = 0, prev_char = 0;
	uint8_t datass[9];
	

	// set for 8 MHz clock 
	CPU_PRESCALE(1);

	
	
	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;
	t3setup(); //set up timer 1 for the scan time thingy

	DDRD |= (1<<DDD6);

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);
	uart_init(57600);
	
	// while(uart_available() < 8);
	// while(uart_getchar()); //burn through the buffer until we find a value of 0
	
	while (1) {

		// for (char i = 0; i < 9; i++)
		// {
			// datass[i] = uart_getchar();
			// i++;
		// }
		
		// phex(datass[0]);
		// phex(datass[1]);
		// phex(datass[2]);
		// phex(datass[3]);
		// phex(datass[4]);
		// phex(datass[5]);
		// phex(datass[6]);
		// phex(datass[7]);
		// pchar('\n');
	
	

		//wait for data
		PORTD |= (1<<PORTD6);
		
		while(uart_available() < 8); 
		do
		{
			prev_char = uart_getchar();  //find the zeros and get to the char after em
			phex(prev_char);
		}while ((prev_char == 0)); //also wait here after the pen leaves 
		
		PORTD &= !(1<<PORTD6);
		//picking apart the button data byte
		buttons = ((prev_char & 0x40) >> 3);		// tap
		buttons = ((prev_char & 0x20) >> 5);		// in range
		buttons = buttons + (prev_char & 0x02); 	//barrel switch
		buttons = buttons + ((prev_char & 1) << 2);	//tip switch
		
		print(" B ");
		phex(buttons);
		print(" x ");
		xl = (uart_getchar() << 1);	//lower byte of x data ; the lower bytes were 0-127 this made gaps in the position data when rolling over
		xh = uart_getchar();	//upper byte of x data
		yl = (uart_getchar() << 1);	//lower byte of y data
		yh = uart_getchar();	//upper byte of y data
		phex(xh);
		phex(xl);
		print(" y ");
	
		
		//inverting Y axis
		y_current = yh;						//combine y data into a 16bit number 
		y_current = (y_current << 8) | yl;
		y_current = 0x3FF0 - y_current;		//invert its direction by subtracting it from the max value
		
		yl = y_current & 0x00FF;			//put it back in yh and yl to send over usb
		yh = (y_current >> 8) & 0x00FF;
		
		phex(yh);
		phex(yl);
		
		pressure = uart_getchar();	//pressure data
		
		print(" p ");
		phex(pressure);
		

		if ((xh | xl) > 0)
		{
			usb_tablet_update(xl, xh, yl, yh, buttons, pressure, scantime);
		}
		while(uart_available() >0)
		{
			phex(uart_getchar());
		}
		pchar('\n');
		
	}
}


//set up timer 3 to reset on compare match with its compare register A and generate an interrupt for incrementing the scan time
void t3setup()
{
	cli();
	TIMSK3 = (1 << OCIE3A); 				//enable output compare 3A interrupt
	TCCR3A = 0;								//all pins normal I/O
	TCCR3B = (1 << WGM32) + (1 << CS30) + (1 << ICES3); //output compare mode, reset on compare match
	OCR3A = 800;							//this is a 16 bit operation, it needs to be done atomically
	sei();
}

//increment the scan time variable every time there's a match on timer 3 compare register A
ISR(TIMER3_COMPA_vect) 
{
	cli();
	scantime++;
	sei();	
}