#ifndef TACHOMETER_H_
#define TACHOMETER_H_

// Your declarations and macros go here

#include <stdint.h>
#include "msp.h"

void Tachometer_Init(void);

extern volatile uint8_t send_uart_flag;

#endif /* TACHOMETER_H_ */
