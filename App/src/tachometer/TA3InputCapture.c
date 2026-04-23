// TA3InputCapture.c
// Runs on MSP432
// Use Timer A3 in capture mode to request interrupts on rising
// edges of P10.4 (TA3CCP0) and P8.2 (TA3CCP2) and call user
// functions.
// Use Timer A3 in capture mode to request interrupts on rising
// edges of P10.4 (TA3CCP0) and P10.5 (TA3CCP1) and call user
// functions.
// Daniel Valvano
// July 11, 2019

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

// external signal connected to P10.5 (TA3CCP1) (trigger on rising edge)
// external signal connected to P10.4 (TA3CCP0) (trigger on rising edge)

#include "TA3InputCapture.h"

// void ta3dummy(void);       // dummy function
void (*CaptureTask0)(void);// user function
void (*CaptureTask1)(void);// user function


//------------TimerA3Capture_Init01------------
// Initialize Timer A3 in edge time mode to request interrupts on
// the rising edges of P10.4 (TA3CCP0) and P10.5 (TA3CCP1).  The
// interrupt service routines acknowledge the interrupt and call
// a user function.
// Input: task0 is a pointer to a user function called when P10.4 (TA3CCP0) edge occurs
//              parameter is 16-bit up-counting timer value when P10.4 (TA3CCP0) edge occurred (units of 0.083 usec)
//        task1 is a pointer to a user function called when P10.5 (TA3CCP1) edge occurs
//              parameter is 16-bit up-counting timer value when P10.5 (TA3CCP1) edge occurred (units of 0.083 usec)
// Output: none
// Assumes: low-speed subsystem master clock is 12 MHz
void TimerA3Capture_Init01(void(*task0)(void), void(*task1)(void)){
    //assigning interrupt task to user defined task
    CaptureTask0 = task0;
    CaptureTask1 = task1;
    //Initialize Left (P5.2) & Right (P5.0) Encoder B
    P5->SEL0 &= ~0x05;
    P5->SEL1 &= ~0x05;
    P5->DIR &= ~0x05;
    // Initialize P10.4 as TA3CCP0 & P10.5 as TA3CPP1 and make them input
    P10->SEL0 |= 0x30;
    P10->SEL1 &= ~0x30;
    P10->DIR &= ~0x30;

    //********* Initializing Timer A3 */
    TIMER_A3->CTL &= ~0x0030; //halt Timer A3
    // bits15-10=XXXXXX, reserved
    // bits9-8=10,       clock source to SMCLK
    // bits7-6=00,       input clock divider /1
    // bits5-4=00,       stop mode
    // bit3=X,           reserved
    // bit2=0,           set this bit to clear
    // bit1=0,           interrupt disable
    // bit0=0,           clear interrupt pending
    TIMER_A3->CTL = 0x0200;
    TIMER_A3->EX0 &= ~0x0007;       // configure for input clock divider /1

    //*****TA3CPP0 */ P10.4, Right Wheel
    // bits15-14=01,     capture on rising edge
    // bits13-12=00,     capture/compare input on CCI0A
    // bit11=1,          synchronous capture source
    // bit10=X,          synchronized capture/compare input
    // bit9=X,           reserved
    // bit8=1,           capture mode
    // bits7-5=XXX,      output mode
    // bit4=1,           enable capture/compare interrupt
    // bit3=X,           read capture/compare input from here
    // bit2=X,           output this value in output mode 0
    // bit1=X,           capture overflow status
    // bit0=0,           clear capture/compare interrupt pending
    TIMER_A3->CCTL[0] = 0x4910;
    NVIC->IP[14] = 0x2<<5; // priority 2
    // NVIC->ISER[0] |= (1 << 14);
    NVIC->ISER[0] |= 0x00004000;

    //*****TA3CPP1 */ P10.5, Left Wheel
    TIMER_A3->CCTL[1] = 0x4910;
    NVIC->IP[15] = 0x2<<5; // priority 2
    // NVIC->ISER[0] |= (1 << 15);
    NVIC->ISER[0] |= 0x00008000;

    //********* reset and start Timer A3 in continuous up mode ******
    TIMER_A3->CTL |= 0x0024;        
}

void TA3_0_IRQHandler(void){ //Right wheel
    TIMER_A3->CCTL[0] &= ~0x0001; //acknowledge CCIFG bit
    (*CaptureTask0)();
}

void TA3_N_IRQHandler(void){ //Left wheel
    if (TIMER_A3->CCTL[1] & 0x0001){
        TIMER_A3->CCTL[1] &= ~0x0001; //acknowledge CCIFG bit
        (*CaptureTask1)();
    }
}
