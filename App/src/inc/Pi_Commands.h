#ifndef PI_COMMANDS_H_
#define PI_COMMANDS_H_

#include <stdint.h>

/**
 * Pi_Command_t defines the specific movement states sent from the Raspberry Pi.
 * Each state is explicitly mapped to a uint8_t value to ensure 
 * consistent interpretation of the UART buffer.
 */
typedef enum instruction_t{
    IDLE     = '0',
    FORWARD  = '1',
    BACKWARD = '2',
    LEFT     = '3',
    RIGHT    = '4'
} Instruction_t;
typedef struct command_t{
    Instruction_t inst;
} Command_t;

//global varialbe for current command
extern volatile Command_t CurrCmd;




#endif /* PI_COMMANDS_H_ */
