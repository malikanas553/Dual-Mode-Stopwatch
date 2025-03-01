/*
 * main.c
 *
 *  Created on: Feb 12, 2025
 *      Author: LightStore
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define DISPLAY_DELAY 2

unsigned char count_up = 1;
unsigned char seconds = 0;
unsigned char minutes = 0;
unsigned char hours = 0;

unsigned char stopped = 0;
unsigned char resume = 1;

unsigned char hour_inc_flag = 0;
unsigned char hour_dec_flag = 0;
unsigned char min_inc_flag = 0;
unsigned char min_dec_flag = 0;
unsigned char sec_inc_flag = 0;
unsigned char sec_dec_flag = 0;
unsigned char toggle_mode_flag = 0;

void TIMER1_CTC_INIT(void)
{
	TCCR1A = (1<<FOC1A);
	ICR1 = 62499;
	TCCR1B = (1<<CS12) | (1<<WGM12) | (1<<WGM13);
	TIMSK |= (1<<OCIE1A);
	TCNT1 = 0;

}
void DISPLAY_INIT(void)
{
	DDRA |= 0x3F;
	DDRC |= 0x0F;
	PORTA &= 0xC0;

}

void RESET_INIT(void)
{
	MCUCR |= (1<<ISC01);
	GICR |= (1<<INT0);
	DDRD &= ~(1<<PD2);
	PORTD |= (1<<PD2);
}
void PAUSE_INIT(void)
{
	MCUCR |= (1<<ISC10) | (1<<ISC11);
	GICR |= (1<<INT1);
	DDRD &= ~(1<<PD3);
}
void RESUME_INIT(void)
{
	MCUCR &= ~(1<<ISC2);
	GICR |= (1<<INT2);
	DDRB &= ~(1<<PB2);
	PORTB |= (1<<PB2);
}
void NORMAL_BUTTONS_INIT(void)
{
	DDRB &= 0x04;
	PORTB |= 0xFB;
}
void ALARM_INIT(void)
{
	DDRD |= (1<<PD0);
	PORTD &= ~(1<<PD0);
}
void COUNT_LEDS_INIT()
{
	DDRD |= 0x30;
	PORTD |= (1<<PD4);
	PORTD &= ~(1<<PD5);
}
void DisplayTime(void)
{
	PORTA = (PORTA & 0xC0) | (1<<PA0);
	PORTC = (PORTC & 0xF0) | ((hours / 10) & 0x0F );

	_delay_ms(DISPLAY_DELAY);

	PORTA = (PORTA & 0xC0) | (1<<PA1);
	PORTC = (PORTC & 0xF0) | ((hours % 10) & 0x0F );

	_delay_ms(DISPLAY_DELAY);

	PORTA = (PORTA & 0xC0) | (1<<PA2);
	PORTC = (PORTC & 0xF0) | ((minutes / 10) & 0x0F );

	_delay_ms(DISPLAY_DELAY);

	PORTA = (PORTA & 0xC0) | (1<<PA3);
	PORTC = (PORTC & 0xF0) | ((minutes % 10) & 0x0F );

	_delay_ms(DISPLAY_DELAY);

	PORTA = (PORTA & 0xC0) | (1<<PA4);
	PORTC = (PORTC & 0xF0) | ((seconds / 10) & 0x0F );

	_delay_ms(DISPLAY_DELAY);

	PORTA = (PORTA & 0xC0) | (1<<PA5);
	PORTC = (PORTC & 0xF0) | ((seconds % 10) & 0x0F );

	_delay_ms(DISPLAY_DELAY);
}

int main(void)
{
	TIMER1_CTC_INIT();
	DISPLAY_INIT();
	RESET_INIT();
	PAUSE_INIT();
	RESUME_INIT();
	NORMAL_BUTTONS_INIT();
	COUNT_LEDS_INIT();
	ALARM_INIT();
	SREG = (1<<7);
	while(1)
	{

		DisplayTime();
		if(stopped)
		{
			if(!(PINB & (1<<PB0)))
			{
				if(hours != 0 && !hour_dec_flag)
				{
					hours--;
					hour_dec_flag = 1;
				}
			}
			else if(!(PINB & (1<<PB1)))
			{
				if(hours != 99 && !hour_inc_flag)
				{
					hours++;
					hour_inc_flag = 1;
				}

			}else
			{
				hour_dec_flag = 0;
				hour_inc_flag = 0;
			}

			if(!(PINB & (1<<PB3)))
			{

				if(minutes != 0 && !min_dec_flag)
				{
					minutes--;
					min_dec_flag = 1;
				}
			}
			else if(!(PINB & (1<<PB4)))
			{
				if(!min_inc_flag){
					minutes++;
					if(minutes == 60)
					{
						hours++;
						minutes = 0;
					}
					min_inc_flag = 1;
				}
			}else
			{
				min_dec_flag = 0;
				min_inc_flag = 0;
			}

			if(!(PINB & (1<<PB5)))
			{

				if(seconds != 0  && !sec_dec_flag)
				{
					seconds--;
					sec_dec_flag = 1;
				}
			}
			else if(!(PINB & (1<<PB6)))
			{
				if(!sec_inc_flag){
					seconds++;
					if(seconds == 60)
					{
						minutes++;
						seconds = 0;
					}
					sec_inc_flag = 1;
				}
			}else
			{
				sec_dec_flag = 0;
				sec_inc_flag = 0;
			}
			if(!(PINB & (1<<PB7)))
			{
				if(!toggle_mode_flag)
				{
					if(count_up)
					{
						count_up = 0;
						PORTD |= (1<<PD5);
						PORTD &= ~(1<<PD4);

					}
					else
					{
						count_up = 1;
						PORTD |= (1<<PD4);
						PORTD &= ~(1<<PD5);
					}
					toggle_mode_flag = 1;
				}
			}else
			{
				toggle_mode_flag = 0;
			}
		}

	}
	return 0;
}


ISR(TIMER1_COMPA_vect)
{
	if(resume){
		if(count_up){
			PORTD &= ~(1<<PD0);
			seconds++;
			if(seconds == 60)
			{
				seconds = 0;
				minutes++;
			}
			if(minutes == 60)
			{
				hours++;
				minutes = 0;
			}
		}else
		{

			if(!hours && !minutes && !seconds)
			{

				PORTD|= (1<<PD0);

			}else{
				PORTD &= ~(1<<PD0);
				seconds--;
				if(seconds == 0xFF)
				{
					seconds = 59;
					minutes--;
				}
				if(minutes == 0xFF)
				{
					hours--;
					minutes = 59;
				}
			}

		}
	}
}
ISR(INT0_vect)
{
	seconds = 0;
	minutes = 0;
	hours = 0;
	count_up = 1;
	PORTD |= (1<<PD4);
	PORTD &= ~(1<<PD5);
	PORTD &= ~(1<<PD0);
}

ISR(INT1_vect)
{
	if(resume){
		TCCR1B &= ~(1<<CS12);
		stopped = 1;
		resume = 0;
	}
}
ISR(INT2_vect)
{
	if(stopped)
	{
		TCCR1B |= (1<<CS12);
		stopped = 0;
		resume = 1;
	}
}
