#include "Tachometer.h"

void Tachometer_Init(void){
	// //P4.0 to debug with oscilloscope
	// P4->SEL0 &= ~0x01; // Set SEL0 bit 0 to 0 (GPIO mode)
	// P4->SEL1 &= ~0x01; // Set SEL1 bit 0 to 0 (GPIO mode)
	// P4->DIR  |= 0x01;  // Set DIR bit 0 to 1 (Output direction)
	// P4->OUT  &= ~0x01; // Set OUT bit 0 to 0 (Initialize pin to LOW)
	//**** Initialize Timer A1 *******/
	//TA1R starts at 0 and counts to TA1CCR0 and reset
	//Using TA0CCR0 to generate interrupt
	TIMER_A1->CTL &= ~0x0030; //halt Timer A1 (MC=00)
	TIMER_A1->CTL |= 0x02C0; //set TASSEL to select 12MHz SMCLK, set ID to 0b11 aka 3
	TIMER_A1->CCTL[0] = 0x0010; //set CCIE to enable interrupt, set CAP to 0 for compare modes
	TIMER_A1->CCR[0] = 0xFFFF; //period of sampling is 349.5ms
	TIMER_A1->EX0 |= 0x0007; //set TAIDEX to 0b111 aka 7 ==> prescaler is 64
	NVIC->IP[10] = 0x2<<5; // priority 2, found IRQ of TA1_0 to be 10 from page 228 of Valvano book
	NVIC->ISER[0] |= 0x00000400; //set 10th bit of ISER[0] see page 229
	TIMER_A1->CTL |= 0x0014; //set MC to be 01 up mode, set TACLR to reset clock
	//***** FINISHED Initialize Timer A1 ********/
}

void TA1_0_IRQHandler(void){
    TIMER_A1->CCTL[0] &= ~0x0001; //acknowledge CCIFG bit

	// P4->OUT ^= 0x01; //debug for scope
	//Toggle flag to send Tachometer data in the main loop
	send_uart_flag = 1;
}
