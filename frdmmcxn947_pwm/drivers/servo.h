

#ifndef SERVO_H
#define SERVO_H

#include "fsl_pwm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void initServo(int module, uint32_t srcClock_Hz);
void PWM_Setup(void);
void Steer(double angle, int channel);
void delay_ms(uint32_t ms);

#endif
