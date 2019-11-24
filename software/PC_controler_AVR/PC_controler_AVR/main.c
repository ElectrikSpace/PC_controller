/*
 * PC_controler_AVR.c
 *
 * Created: 14/11/2019 21:28:41
 * Author : sylvain
 */ 

#define F_CPU 8000000L

#define min_batterie_level 773

#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>

void init() {
	PORTB = 0x00; // outputs set to HIGH and input set to h-Z
	DDRB = 0x1A; // set inputs and outputs (00011010)
	ADMUX = 0x00; // ADC multiplexer timer 
	ADCSRA = 0x87; // initialize ADC status register
	DIDR0 = 0x01;
	_delay_ms(1); // wait until complete write
}

void go_to_sleep() {
	// prescale timer to 8s
	WDTCR |= (1<<WDP3 )|(0<<WDP2 )|(0<<WDP1)|(1<<WDP0); // 8s
	// Enable watchdog timer interrupts
	WDTCR |= (1<<WDTIE);
	sei(); // Enable global interrupts
	// Use the Power Down sleep mode
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

char check_battery() {
	ADCSRA |= (1 << ADSC); // start conversion
	while(ADCSRA & (1 << ADSC)); // wait until complete
	if(ADC >= min_batterie_level) {
		return('y');		
	}
	else{
		return('n');
	}
}

int main(void)
{
    init();
    while (1) 
    {
		if(check_battery() == 'y') {
			
		}
		else{
			
		}
    }
}

