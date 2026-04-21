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
#include "src/inc/Bump.h"
#include "src/inc/Clock.h"
#include "src/inc/LaunchPad.h"
#include "src/inc/CortexM.h"
#include "src/UART/UARTpi.h"
#include "src/motor/Motor.h"
#include "src/tachometer/Tachometer.h"

volatile Command_t CurrCmd;
volatile uint8_t send_uart_flag;

// Driver test
void Pause(void){
  while(LaunchPad_Input()==0) {};  // wait for touch
  while(LaunchPad_Input() != 0) {};     // wait for release
}

//Interrupt lists:
//UART Interrupt to receive instruction from pi (Priority 3)
//Interrupt from Tachometer reader (Priority 2)
//Interrupt from Tachometer.c every sample tie 349.5ms (Priority 2)
//Note: PWM wave for Motor does not use interrupt


void main (void) {
  Clock_Init48MHz();
  LaunchPad_Init(); // built-in switches and LEDs
  Bump_Init();      // bump switches

  //Enable Motor
  Motor_Init();     

  //Enable UART to communicate with the PI
  UART_Init(EUSCI_A2);

  //Enable Tachometer
  Tachometer_Init();

  EnableInterrupts();


  // CurrCmd.inst = IDLE; //this will get changed through UART interrupt
  CurrCmd.instructionType = STOP;
  while(1){
    if (CurrCmd.isNew){ //only change Motor State when Command changes
      CurrCmd.isNew = 0;
      switch (CurrCmd.instructionType) {
        case FORWARD: 
          Motor_Forward(CurrCmd.leftDuty, CurrCmd.rightDuty);
          break;
        case BACKWARD:
          Motor_Backward(CurrCmd.leftDuty, CurrCmd.rightDuty);
          break;
        case LEFT:
          Motor_Left(CurrCmd.leftDuty, CurrCmd.rightDuty);
          break;
        case RIGHT:
          Motor_Right(CurrCmd.leftDuty, CurrCmd.rightDuty);
          break;
        case STOP:
        default:
          Motor_Stop();
          break;
      }
    }

    if(send_uart_flag){
      send_uart_flag = 0;
      UA
    }

  }

}
