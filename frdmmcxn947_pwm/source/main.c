#include "fsl_debug_console.h"
#include "servo.h"
#include "esc.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_pwm.h"
#include "fsl_clock.h"
#include "hbridge.h"
#include "pixy.h"
#include <math.h>
#include <stdio.h>
#include <MCXN947_cm33_core0.h>

/// ----------------- DEFINIRI -------------------

#define MOTOR1_FORWARD_GPIO  GPIO5
#define MOTOR1_FORWARD_PIN   8U
#define MOTOR1_BACKWARD_GPIO GPIO5
#define MOTOR1_BACKWARD_PIN  9U
#define MOTOR2_FORWARD_GPIO  GPIO1
#define MOTOR2_FORWARD_PIN   13U
#define MOTOR2_BACKWARD_GPIO GPIO2
#define MOTOR2_BACKWARD_PIN  0U

#define PIXY2_I2C_ADDRESS (0x54U)

#define PWM1_SUBMODULE_MOTOR1       kPWM_Module_1
#define PWM1_CHANNEL_MOTOR1         kPWM_PwmA
#define PWM1_SUBMODULE_MOTOR2       kPWM_Module_1
#define PWM1_CHANNEL_MOTOR2         kPWM_PwmB

#define PWM_FREQ_HZ          1000U
#define SRC_CLOCK_HZ         CLOCK_GetFreq(kCLOCK_CoreSysClk)

#define PIXY_FRAME_CENTER_X 160  // imaginea e 320px latime, deci centru e 160

#define KP_SLOPE    66.0f  // Gain pt panta (tu poți regla)
#define KP_POSITION 0.0f

#define SERVO_CENTER_ANGLE  0.0
#define SERVO_MIN_ANGLE   -80.0
#define SERVO_MAX_ANGLE    80.0

#define BASE_SPEED 100

/// --------- Limiteaza valori ---------
static inline double clamp_steer(double val, double min, double max){
    if(val < min) return min;
    if(val > max) return max;
    return val;
}

uint32_t LPI2C2_GetFreq(void)
{
    return CLOCK_GetLPFlexCommClkFreq(2u);
}


volatile bool g_MasterCompletionFlag = false;

static void lpi2c_master_callback(uint32_t event)
{
    switch (event)
    {
        case ARM_I2C_EVENT_TRANSFER_DONE:
            g_MasterCompletionFlag = true;
            break;

        case ARM_I2C_EVENT_ARBITRATION_LOST:
            g_MasterCompletionFlag = true;
            break;

        default:
            break;
    }
}



/// --------- MAIN ---------
int main(void)
{
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
	CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

	CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

	BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_Gpio1);
    CLOCK_EnableClock(kCLOCK_Gpio2);

    gpio_pin_config_t motorPinConfig = {
    		kGPIO_DigitalOutput, 0
    };




    // Init Hbridge
    Hbridge hbridge;
    HbridgeInit(&hbridge,
                PWM1,
                PWM1_SUBMODULE_MOTOR1, PWM1_CHANNEL_MOTOR1,
                PWM1_SUBMODULE_MOTOR2, PWM1_CHANNEL_MOTOR2,
                MOTOR1_FORWARD_GPIO, MOTOR1_FORWARD_PIN,
                MOTOR1_BACKWARD_GPIO, MOTOR1_BACKWARD_PIN,
                MOTOR2_FORWARD_GPIO, MOTOR2_FORWARD_PIN,
                MOTOR2_BACKWARD_GPIO, MOTOR2_BACKWARD_PIN,
                PWM_FREQ_HZ, SRC_CLOCK_HZ,
                kPWM_Prescale_Divide_64,
                kPWM_Prescale_Divide_64);


    Pixy2Init(PIXY2_I2C_ADDRESS, 0);
    PixySetLed(0, 100, 0);


    InitNewPWM(PWM1, kPWM_Module_0, kPWM_PwmA, kPWM_Prescale_Divide_64, 50, SRC_CLOCK_HZ);//Servo

    Steer(SERVO_CENTER_ANGLE, 0);
    HbridgeSetSpeed(&hbridge, 0, 0);


    while(1)
    {
        DetectedVectors vectors_data = PixyGetVectors();

        if (vectors_data.NumberOfVectors > 0)
        {
            float weighted_slope_sum = 0.0f;
            float weight_total = 0.0f;

            for(int i = 0; i < vectors_data.NumberOfVectors; i++)
            {
                DetectedVector* v = &vectors_data.Vectors[i];
                int dx = v->x1 - v->x0;
                int dy = v->y1 - v->y0;

                // Calcul norma (lungimea vectorului)
                float norma = sqrtf((float)(dx*dx + dy*dy));

                if(abs(dy) > 0 && norma > 35.0f) // evit impartire la zero + ignor noise
                {
                    float slope = (float)dx / (float)dy;

                    // Pun un filtru: daca panta e prea mica, ignor (linie aproape verticala = vreau sa merg drept)
                    if(fabs(slope) > 0.2f)
                    {
                        weighted_slope_sum += slope * norma;  // ponderat cu norma
                        weight_total += norma;
                    }
                }
            }

            float avg_slope_error = 0.0f;

            if(weight_total > 0.0f)
            {
                avg_slope_error = weighted_slope_sum / weight_total;
            }

            double steering_adjustment = KP_SLOPE * avg_slope_error;
            double steering_angle = SERVO_CENTER_ANGLE + steering_adjustment;
            steering_angle = clamp_steer(steering_angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);

            Steer(-steering_angle, 0);  // atentie la semn, in functie de cum e montat servo
            HbridgeSetSpeed(&hbridge, BASE_SPEED, BASE_SPEED);
        }
    }


    return 0;
}