// Lab12_Motorsmain.c
// Runs on MSP432
// Solution to Motors lab
// Daniel and Jonathan Valvano
// December 17, 2018

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



//**************RSLK1.1***************************
// Left motor direction connected to P5.4 (J3.29)
// Left motor PWM connected to P2.7/TA0CCP4 (J4.40)
// Left motor enable connected to P3.7 (J4.31)
// Right motor direction connected to P5.5 (J3.30)
// Right motor PWM connected to P2.6/TA0CCP3 (J4.39)
// Right motor enable connected to P3.6 (J2.11)

#include "msp.h"
#include "../inc/Bump.h"
#include "../inc/Clock.h"
#include "../inc/SysTick.h"
#include "../inc/LaunchPad.h"
#include "../inc/MotorSimple.h"
#include "../inc/UARTpi.h"
#include "../inc/Pi_Commands.h"

// Driver test
void Pause(void){
  while(LaunchPad_Input()==0) {};  // wait for touch
  while(LaunchPad_Input() != 0) {};     // wait for release
}


// int Program12_1(void){
  
//   while(1){
//     Pause();
//     Motor_ForwardSimple(5000,2000);  // your function
//     // Pause();
//     // Motor_BackwardSimple(5000,2000); // your function
//     // Pause();
//     // Motor_LeftSimple(5000,2000);     // your function
//     // Pause();
//     // Motor_RightSimple(5000,2000);    // your function
//   }
// }


void main (void) {
  Clock_Init48MHz();
  LaunchPad_Init(); // built-in switches and LEDs
  Bump_Init();      // bump switches
  Motor_InitSimple();     // your function
  SysTick_Init(); //DONT FORGET SYSTICK_INIT
  UART1_Init();

  Pi_Command_t instruction = IDLE;
  while(1){
    if (UART1_HasIn()){
      instruction = (Pi_Command_t) UART1_InChar(); 
    }
    //if uart input is larger than commands then stop motor later


    switch(instruction) {
      case FORWARD:
          Motor_ForwardSimple(5000,2000);
          instruction = IDLE;
          break;
      case BACKWARD:
          Motor_BackwardSimple(5000,2000);
          instruction = IDLE;
          break;
      case LEFT:
          Motor_LeftSimple(5000,2000);
          instruction = IDLE;
          break;
      case RIGHT:
          Motor_RightSimple(5000,2000);
          instruction = IDLE;
          break;
      case IDLE:
      default:
          Motor_StopSimple();
          break;
    }
  }

}
