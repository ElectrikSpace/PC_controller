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
#define total_bytes_message 30

const uint8_t key[total_bytes_message] = {0x61, 0x33, 0x35, 0x64, 0x66, 0x30, 0x32, 0x66, 0x66, 0x63, 0x34, 0x62, 0x39, 0x66, 0x61, 0x64, 0x64, 0x31, 0x62, 0x33, 0x65, 0x63, 0x39, 0x37, 0x37, 0x66, 0x63, 0x33, 0x62, 0x33};

int counter;
int timeout = 0;
uint8_t message[total_bytes_message] = {0};
uint8_t action;

void init() {
	PORTB = 0x00; // outputs set to LOW and input set to h-Z
	DDRB = 0b00011010; // set inputs and outputs
	ADMUX = 0x00; // ADC multiplexer timer 
	ADCSRA = 0x87; // initialize ADC status register
	DIDR0 = 0x01;
	_delay_ms(1); // wait until complete write
}

void go_to_sleep() {
	ADCSRA &= ~(1<<ADEN);
	WDTCR=(0<<WDTIF) | (0<<WDTIE) | (1<<WDP3) | (1<<WDCE) | (0<<WDE) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0);
	WDTCR=(1<<WDTIF) | (1<<WDTIE) | (1<<WDP3) | (0<<WDCE) | (0<<WDE) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0);
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sei();
	sleep_cpu();
	cli();
	ADCSRA |= (1<<ADEN);
}

ISR(WDT_vect)
{
	sleep_disable();
}

char check_battery() {
	ADCSRA |= (1 << ADSC); // start conversion
	while(ADCSRA & (1 << ADSC)); // wait until complete
	return('y');
	/*if(ADC >= min_batterie_level) {
		return('y');		
	}
	else{
		return('n');
	}*/
}

ISR(TIM0_OVF_vect)
{
	 timeout+=1;
}

void HC12_read(){
	timeout = 0;
	int remain_bytes = total_bytes_message;
	while((timeout<=2000) & (remain_bytes > 0)){
		if( !(PINB & (1<<2)) ) { // start bit
			uint8_t value = 0x00;
			_delay_us(140); // one and half a period
			for(int index = 0; index <= 7; index++){ // values
				if(PINB & (1<<2)){
					value = value | (1<<index);
				}
				_delay_us(90); // next bit 9600 HZ
			}
			if(PINB & (1<<2)){ // stop bit
				remain_bytes--;
				if(remain_bytes == 0) {
					message[total_bytes_message - remain_bytes] = value;
				}
				else {
					action = value;
				}
				value = 0x00;
				
				timeout = 2500;
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
		if(check_battery() == 'y') {
			TCCR0B |= _BV(CS02)|_BV(CS00); // set prescaler to 1024 (CLK=1200000Hz/1024/256=4Hz, 0.25s)
			TIMSK0 |= _BV(TOIE0); // enable Timer Overflow interrupt
			sei(); // enable global interrupts
			while(counter<=50000){
				PORTB = 0x02; //enable HC12
				_delay_ms(1);
				timeout = 0;
				while(timeout <= 1){
					if( !(PINB & (1<<2)) ) {
						_delay_ms(250); // wait until before new complete message
						HC12_read(); 
						/*if (message == key) {
							if (action == 0x01) {*/
								PORTB = 0b00001000; // PB3 ON = power switch and HC12 off
								_delay_ms(500);
								PORTB = 0x00; // all off
							/*}
							else if (action == 0x02) {
								PORTB = 0b00010000; // PB4 ON = reset switch and HC12 off
								_delay_ms(500);
								PORTB = 0x00; // all off
							}
							else {
								// bad action nothing to do
							}
						}
						else { 
							// bad key nothing to do
						}*/
						for(int i = 0; i < total_bytes_message; i++) { // reset of message buffer
							message[i] = 0x00;
						}
						
					}
					else{
						// nothing to do
					}
				}
				timeout = 0;
				PORTB = 0x00; //disable HC12
				counter++;
				//go_to_sleep();
				//go_to_sleep();
			}
			counter = 0;
		}
		else{
			for(counter=0; counter<=100; counter++){ // enter in sleep for about 100*8 seconds
				go_to_sleep();
			}
		}
    }
}

