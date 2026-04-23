// UART1.c
// Runs on MSP432
// Use UCA2 to implement bidirectional data transfer to and from a
// CC2650 BLE module, uses interrupts for receive and busy-wait for transmit

// Daniel Valvano
// May 24, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to Robotics,
   Jonathan W. Valvano, ISBN: 9781074544300, copyright (c) 2019
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2019, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/


// UCA2RXD (VCP receive) connected to P3.2
// UCA2TXD (VCP transmit) connected to P3.3
// J1.3  from Bluetooth (DIO3_TXD) to LaunchPad (UART RxD){MSP432 P3.2}
// J1.4  from LaunchPad to Bluetooth (DIO2_RXD) (UART TxD){MSP432 P3.3}

#include "UARTpi.h"

EUSCI_A_Type* uartPort;

static uint8_t state = 0;
//a correct sequence would be 0xAA Inst_T LeftDuty_MSB LeftDuty_LSB RightDuty_MSB RightDuty_LSB Checksum
//Big-Endian
static uint8_t buffer[7]; //buffer for storing uart_message from Pi
//uart_buffer[0]: start byte (0xAA if msg is correct)
//uart_buffer[1]: Instruction type
//uart_buffer[2:5]: LeftMSB, LeftLSB, RightMSB, RightLSB Duty Cycle
//uart_buffer[6]:  the XOR checksum of uart_buffer[1:5]


                    
//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 12 MHz SMCLK clock),
// 8 bit word length, no parity bits, one stop bit
// Input: none
// Output: none
void UART_Init(EUSCI_A_Type* uartSource){
  // RxFifo_Init(); 
  uartPort = uartSource;
  uartPort->CTLW0 = 0x0001;         // hold the USCI module in reset mode
  // bit15=0,      no parity bits
  // bit14=x,      not used when parity is disabled
  // bit13=0,      LSB first
  // bit12=0,      8-bit data length
  // bit11=0,      1 stop bit
  // bits10-8=000, asynchronous UART mode
  // bits7-6=11,   clock source to SMCLK
  // bit5=0,       reject erroneous characters and do not set flag
  // bit4=0,       do not set flag for break characters
  // bit3=0,       not dormant
  // bit2=0,       transmit data, not address (not used here)
  // bit1=0,       do not transmit break (not used here)
  // bit0=1,       hold logic in reset state while configuring
  uartPort->CTLW0 = 0x00C1;
                              // set the baud rate
                              // N = clock/baud rate = 12,000,000/115,200 = 104.1667
  uartPort->BRW = 104;        // UCBR = baud rate = int(N) = 104

  uartPort->MCTLW &= ~0xFFF1;   // clear first and second modulation stage bit fields


  if (uartPort == EUSCI_A0) {
      // EUSCI_A0 uses P1.2 (RX) and P1.3 (TX)
      P1->SEL0 |= 0x0C;
      P1->SEL1 &= ~0x0C;
      NVIC->IP[16] = 0x3<<5; // priority 3
      NVIC->ISER[0] |= 0x00010000; // enable interrupt 16 in NVIC
  } 
  else if (uartPort == EUSCI_A2) {
      // EUSCI_A2 uses P3.2 (RX) and P3.3 (TX)
      P3->SEL0 |= 0x0C;
      P3->SEL1 &= ~0x0C;
      NVIC->IP[18] = 0x3<<5; // priority 3
      NVIC->ISER[0] |= 0x00040000; // enable interrupt 18 in NVIC
  }
  
  uartPort->CTLW0 &= ~0x0001; // enable the USCI module                
  uartPort->IE |= 0x0001; // enable interrupts on receive full (only RXIE)
}

//******** Interrupt routines, to change Command whenever receive Command from Pi
//No FIFO, on receive, just change current instruction
void EUSCIA2_IRQHandler(void){
  if(EUSCI_A2->IFG&0x01){     // RX data register full
    parse_Pi_cmd((uint8_t)EUSCI_A2->RXBUF);
  } 
}

void EUSCIA0_IRQHandler(void){
  if(EUSCI_A0->IFG&0x01){    // RX data register full
    parse_Pi_cmd((uint8_t)EUSCI_A0->RXBUF);
  } 
}

void UART_OutChar(uint8_t data){
  while((uartPort->IFG&0x02) == 0);
  uartPort->TXBUF = data;
}

void parse_Pi_cmd(uint8_t uart_data){
  if (state == 0) {
    if (uart_data == PACKET_START_BYTE) { //only parse data if started with start_byte 
      state = 1;
    }
  } else {
    // Store uart_data into buffer
    buffer[state] = uart_data;
    state++;

    if (state == 7) { // finished length of command transmission
      uint8_t checksum = buffer[1] ^ buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5];
      
      if (checksum == buffer[6]) {
        CurrCmd.instructionType = (Instruction_t) buffer[1];
        CurrCmd.leftDuty  = ((uint16_t) buffer[2] << 8) | (uint16_t) buffer[3];
        CurrCmd.rightDuty = ((uint16_t) buffer[4] << 8) | (uint16_t) buffer[5];
        CurrCmd.isNew = 1;
      }
      state = 0; // Reset for next packet
    }
  }
}
