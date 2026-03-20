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

// //**** FIFO */
// #define FIFOSIZE   256       // size of the FIFOs (must be power of 2)
// #define FIFOSUCCESS 1        // return value on success
// #define FIFOFAIL    0        // return value on failure
// uint32_t RxPutI;      // should be 0 to SIZE-1
// uint32_t RxGetI;      // should be 0 to SIZE-1 
// uint32_t RxFifoLost;  // should be 0 
// uint8_t RxFIFO[FIFOSIZE];
// void RxFifo_Init(void){
//   RxPutI = RxGetI = 0;                      // empty
//   RxFifoLost = 0; // occurs on overflow
// }
// int RxFifo_Put(uint8_t data){
//   if(((RxPutI+1)&(FIFOSIZE-1)) == RxGetI){
//     RxFifoLost++;
//     return FIFOFAIL; // fail if full  
//   }    
//   RxFIFO[RxPutI] = data;                    // save in FIFO
//   RxPutI = (RxPutI+1)&(FIFOSIZE-1);         // next place to put
//   return FIFOSUCCESS;
// }
// int RxFifo_Get(uint8_t *datapt){ 
//   if(RxPutI == RxGetI) return 0;            // fail if empty
//   *datapt = RxFIFO[RxGetI];                 // retrieve data
//   RxGetI = (RxGetI+1)&(FIFOSIZE-1);         // next place to get
//   return FIFOSUCCESS; 
// }
// //**** FIFO */

// //------------UART1_InStatus------------
// // Returns how much data available for reading
// // Input: none
// // Output: number of bytes in receive FIFO
// uint32_t UART_InStatus(void){  
//  return ((RxPutI - RxGetI)&(FIFOSIZE-1));  
// }



                    
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
      NVIC->IP[16] = 0x40; // priority 2
      NVIC->ISER[0] = 0x00010000; // enable interrupt 16 in NVIC
  } 
  else if (uartPort == EUSCI_A2) {
      // EUSCI_A2 uses P3.2 (RX) and P3.3 (TX)
      P3->SEL0 |= 0x0C;
      P3->SEL1 &= ~0x0C;
      NVIC->IP[18] = 0x2<<5; // priority 2
      NVIC->ISER[0] = 0x00040000; // enable interrupt 18 in NVIC
  }
  
  uartPort->CTLW0 &= ~0x0001; // enable the USCI module                
  uartPort->IE |= 0x0001; // enable interrupts on receive full (only RXIE)
}


//------------UART_InChar------------
// Wait for new serial port input, interrupt synchronization
// Input: none
// Output: an 8-bit byte received
// spin if RxFifo is empty

// uint8_t UART_InChar(void){
//   while((uartPort->IFG&0x01) == 0);
//   return((char)(uartPort->RXBUF));
// }

///------------UART_OutChar------------
// Output 8-bit to serial port, busy-wait
// Input: letter is an 8-bit data to be transferred
// Output: none
void UART_OutChar(uint8_t data){
  while((uartPort->IFG&0x02) == 0);
  uartPort->TXBUF = data;
}

void EUSCIA2_IRQHandler(void){
  if(EUSCI_A2->IFG&0x01){             // RX data register full
    // RxFifo_Put((uint8_t)EUSCI_A2->RXBUF);// clears UCRXIFG
    uint8_t data = (uint8_t)EUSCI_A2->RXBUF;
    CurrCmd.inst = (Instruction_t) data;
  } 
}

void EUSCIA0_IRQHandler(void){
  if(EUSCI_A0->IFG&0x01){             // RX data register full
    // RxFifo_Put((uint8_t)EUSCI_A2->RXBUF);// clears UCRXIFG
    uint8_t data = (uint8_t)EUSCI_A0->RXBUF;
    CurrCmd.inst = (Instruction_t) data;
  } 
}


//------------UART_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART_OutString(uint8_t *pt){
  while(*pt != '\0'){
    UART_OutChar(*pt);
    pt++;
  }
}

