#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "Arduino.h"
#include "ECU.h"
#include "BSE.h"

// BSE fault check
extern bool IsBSE_Fault;
// BSE input check
extern bool IsInput_Brake;
// APPS high-position check
extern bool IsAPPS_Higher25;

#endif