#include "Tachometer.h"

volatile uint8_t send_uart_flag;
volatile int16_t LCount;
volatile int16_t RCount;


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

	//***** Initialize Input Capture From Tachometer *****/
	TimerA3Capture_Init01(update_rcount, update_lcount);
}

void update_lcount(void){
	if(P5->IN & 0x04) { //if Left B (P5.2) is high when A is rising edge, then wheel is CCW
		LCount += 1;
	} else {
		LCount -= 1;
	}
}
void update_rcount(void){
	if(P5->IN & 0x01) { //if Right B (P5.0) is high when A is rising edge, then wheel is CCW
		RCount += 1;
	} else {
		RCount -= 1;
	}
	// RCount += 1;
}

// void IncrementLCount

void send_tachometer() {
	int16_t LC;
	int16_t RC;
	//*** Critical code block to ensure sending data is not affected by interrupts */
	DisableInterrupts();
	LC = LCount;
	RC = RCount;
	LCount = 0;
	RCount = 0;
	EnableInterrupts();

	uint8_t lc_high = (uint8_t)((LC >> 8) & 0xFF);
	uint8_t lc_low  = (uint8_t)(LC & 0xFF);
    uint8_t rc_high = (uint8_t)((RC >> 8) & 0xFF);
    uint8_t rc_low  = (uint8_t)(RC & 0xFF);

    //cast to uint8_t to ensure correct 8-bit wrapping behavior
    uint8_t checksum = lc_high ^ lc_low ^ rc_high ^ rc_low;

    UART_OutChar(0xAA); //send the start byte
    UART_OutChar(lc_high);  // Send LCount High Byte
    UART_OutChar(lc_low);   // Send LCount Low Byte
    UART_OutChar(rc_high);  // Send RCount High Byte
    UART_OutChar(rc_low);   // Send RCount Low Byte
    UART_OutChar(checksum); //send the Checksum
}

void TA1_0_IRQHandler(void){ //IRQ for every sample time
    TIMER_A1->CCTL[0] &= ~0x0001; //acknowledge CCIFG bit

	// P4->OUT ^= 0x01; //debug for scope
	//Toggle flag to send Tachometer data in the main loop
	send_uart_flag = 1;
}
