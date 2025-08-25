#include "esc.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_pwm.h"
#include "fsl_clock.h"
#include "esc.h"
#include "servo.h"
#include "fsl_pwm.h"
#include "hbridge.h"

static double minDutyCycle = 5;
static double medDutyCycle = 7.5;
static double maxDutyCycle = 10;
static int currentSpeed;


void SetESCSpeed(double Speed) {
	PRINTF("%lf",Speed);
	double dutyCycle = 0;

    if (Speed <= -100) {
        dutyCycle = minDutyCycle;
    } else if (Speed >= 100) {
        dutyCycle = maxDutyCycle;
    } else {
        dutyCycle = medDutyCycle + (Speed * (maxDutyCycle - minDutyCycle)) / 200;
    }
    dutyCycle = ((65535U * dutyCycle) + 50U) / 100U;
    PWM_UpdatePwmDutycycleHighAccuracy(PWM1, kPWM_Module_0, kPWM_PwmB, kPWM_SignedCenterAligned, dutyCycle);
    // Trigger an update of PWM registers
	 PWM_SetPwmLdok(PWM1, kPWM_Control_Module_0, true);

}



