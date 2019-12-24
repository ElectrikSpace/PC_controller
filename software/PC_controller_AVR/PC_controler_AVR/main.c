/*
 * PC_controler_AVR.c
 *
 * Created: 14/11/2019 21:28:41
 * Author : ElectrikSpace
 */ 
#define F_CPU (1200000UL) // 1.2 MHz

#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h> //Needed to enable/disable watch dog timer

#define min_batterie_level 773
#define total_bytes_message 20

//const uint8_t key[total_bytes_message] = {0x61, 0x33, 0x35, 0x64, 0x66, 0x30, 0x32, 0x66, 0x66, 0x63, 0x34, 0x62, 0x39, 0x66, 0x61, 0x64, 0x64, 0x31, 0x62, 0x33, 0x65, 0x63, 0x39, 0x37, 0x37, 0x66, 0x63, 0x33, 0x62, 0x33};
const uint8_t key[total_bytes_message] = {0x61, 0x33, 0x35, 0x64, 0x66, 0x30, 0x32, 0x66, 0x66, 0x63, 0x34, 0x62, 0x39, 0x66, 0x61, 0x64, 0x64, 0x31, 0x62, 0x33};

int timeout = 0;
uint8_t message[total_bytes_message];
uint8_t action;

ISR(TIM0_OVF_vect)
{
	timeout++;
}

ISR(INT0_vect)
{
	sleep_disable();
}

void init() {
	PORTB = 0b0000100; // outputs set to LOW and input set to h-Z
	DDRB = 0b00011100; // set inputs and outputs
	ADMUX = 0x00; // ADC multiplexer timer 
	ADCSRA = 0x87; // initialize ADC status register
	DIDR0 = 0x01;
	_delay_ms(1); // wait until complete write
}

void go_to_sleep() {
	ADCSRA &= ~(1<<ADEN);
	MCUCR &= ~(_BV(ISC01) | _BV(ISC00));      //INT0 on low level
	GIMSK |= _BV(INT0);                       //enable INT0
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sei();
	sleep_cpu();
	cli();
	ADCSRA |= (1<<ADEN);
}

void read_battery_level() {
	ADCSRA |= (1 << ADSC); // start conversion
	while(ADCSRA & (1 << ADSC)); // wait until complete
	return;
}

void HC12_write(uint8_t data) { // send a bytes other HC12
	PORTB = 0b00000000;
	for (int i = 0; i <= 7; i++) {
		_delay_us(175);
		if ( (data & (1<<i)) == 0x00 ){
			PORTB = 0b00000000;
		}
		else {
			PORTB = 0b00000100;
		}
	}
	_delay_us(175);
	PORTB = 0b00000100;
	_delay_us(175);
	return;
}

void HC12_read(){
	int remain_bytes = 0;
	timeout = 0;
	uint8_t value = 0b00000000;
	action = 0x00;
	while((timeout <= 5) & (remain_bytes >= 0)){
		_delay_us(1);
		if( !(PINB & (1<<1)) ) { // start bit
			_delay_us(280); // one and half a period
			//PORTB = 0b00000000;
			for(int index = 0; index <= 7; index++) { // values
				if(PINB & (1<<1)){
					value = value | (1<<index);
				}
				else if( !(PINB & (1<<1) ) ) {
					value = value & ~(1<<index);
				}
				_delay_us(170); // next bit 4800 HZ
				//PORTB = 0b00000100;
				
			}
			if(PINB & (1<<1)){ // stop bit
				if(remain_bytes != 0) {
					message[total_bytes_message - remain_bytes] = value;
					value = 0x00;
					remain_bytes--;
				}
				else {
					if (action == 0x00) {
						action = value;
						value = 0x00;
						if (action == 0x62){
							timeout = 600; // this is a battery level request
						}
						else {
							remain_bytes = total_bytes_message;
						}
					}
					else {
						value = 0x00;
						timeout = 600;
					}
				}
			}
			else{
				// error
			}
		}
	}
}

int main(void)
{
    init();
    while (1) 
    {
		go_to_sleep();
		MCUCR = 0b00000000;      //INT0 on low level
		GIMSK = 0b00000000;
		_delay_ms(250); // wait until before new complete message
		timeout = 0;
		TCCR0B |= _BV(CS02)|_BV(CS00); // set prescaler to 1024 (CLK=1200000Hz/1024/256=4Hz, 0.25s)
		TIMSK0 |= _BV(TOIE0); // enable Timer Overflow interrupt
		sei(); // enable global interrupts
		HC12_read();
		//HC12_write(action);
		/*for (int i = 0; i < 30; i++) {
			HC12_write(message[i]);
			//HC12_write(key[i]);
		}*/			
		if (action == 0x70) {
			uint8_t checked = 0x01;
			for (int k = 0; k < total_bytes_message; k++) {
				if (message[k] != key[k]) {
					checked = 0x00;
				}
			}
			if (checked == 0x01) {
				PORTB = 0b00001100; // PB3 ON = power switch and HC12 off
				_delay_ms(500);
				PORTB = 0b00000100; // all off
				_delay_ms(100);
				HC12_write(0x6f);
			}
			else {
				_delay_ms(100);
				HC12_write(0x6b);
			}
		} 	
		else if (action == 0x72) {
			uint8_t checked = 0x01;
			for (int k = 0; k < total_bytes_message; k++) {
				if (message[k] != key[k]) {
					checked = 0x00;
				}
			}
			if (checked == 0x01) {
				PORTB = 0b00010100; // PB4 ON = reset switch and HC12 off
				_delay_ms(500);
				PORTB = 0b00000100; // all off
				_delay_ms(100);
				HC12_write(0x6f);
			}
			else {
				_delay_ms(100);
				HC12_write(0x6b);
			}
		}	
		else if (action == 0x62) {
			read_battery_level();
			_delay_ms(100);
			HC12_write(0x62);
			HC12_write(ADCH);
			HC12_write(ADCL);
		}	
		else {
			_delay_ms(100);
			HC12_write(0x61);
		}
		// reset all values	
		for(int i = 0; i < total_bytes_message; i++) { // reset of message buffer
			message[i] = 0x00;
		}
		action = 0x00;
		_delay_ms(250);
    }
}

