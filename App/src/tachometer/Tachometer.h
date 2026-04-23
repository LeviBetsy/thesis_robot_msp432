#ifndef TACHOMETER_H_
#define TACHOMETER_H_

// Your declarations and macros go here

#include <stdint.h>
#include "msp.h"
#include "../UART/UARTpi.h"
#include "../inc/CortexM.h"
#include "./TA3InputCapture.h"

void Tachometer_Init(void);
void send_tachometer();

extern volatile uint8_t send_uart_flag;
extern volatile int16_t LCount;
extern volatile int16_t RCount;

void update_lcount(void);
void update_rcount(void);

#endif /* TACHOMETER_H_ */
