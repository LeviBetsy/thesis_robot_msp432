#ifndef PI_COMMANDS_H_
#define PI_COMMANDS_H_

#include <stdint.h>

/**
 * Pi_Command_t defines the specific movement states sent from the Raspberry Pi.
 * Each state is explicitly mapped to a uint8_t value to ensure 
 * consistent interpretation of the UART buffer.
 */
typedef enum {
    IDLE     = 0,
    FORWARD  = 1,
    BACKWARD = 2,
    LEFT     = 3,
    RIGHT    = 4
} Pi_Command_t;

#endif /* PI_COMMANDS_H_ */