#ifndef HBRIDGE_H
#define HBRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fsl_pwm.h"
#include "fsl_gpio.h"

/* H-Bridge structure */
typedef struct {
    PWM_Type *pwmBase;                    // PWM base address  PWM0 or PWM1 ...
    pwm_submodule_t Motor1_Submodule;     // Submodule for Motor1 speed control
    pwm_channels_t Motor1_Channel;        // PWM channel for Motor1 speed control (A or B)
    pwm_submodule_t Motor2_Submodule;     // Submodule for Motor2 speed control
    pwm_channels_t Motor2_Channel;        // PWM channel for Motor2 speed control (A or B)
    GPIO_Type *Motor1_Forward_Port;       // GPIO port for Motor1 forward control
    uint32_t Motor1_Forward_Pin;          // GPIO pin number for Motor1 forward control
    GPIO_Type *Motor1_Backward_Port;      // GPIO port for Motor1 backward control
    uint32_t Motor1_Backward_Pin;         // GPIO pin number for Motor1 backward control
    GPIO_Type *Motor2_Forward_Port;       // GPIO port for Motor2 forward control
    uint32_t Motor2_Forward_Pin;          // GPIO pin number for Motor2 forward control
    GPIO_Type *Motor2_Backward_Port;      // GPIO port for Motor2 backward control
    uint32_t Motor2_Backward_Pin;         // GPIO pin number for Motor2 backward control
    uint32_t pwmFreq_Hz;                  // PWM frequency in Hz
    uint32_t srcClock_Hz;                 // PWM source clock frequency in Hz
    pwm_clock_prescale_t Motor1_Prescaler; // Prescaler for Motor1
    pwm_clock_prescale_t Motor2_Prescaler; // Prescaler for Motor2
} Hbridge;

/* Function prototypes */
void HbridgeInit(Hbridge *hbridge,
                 PWM_Type *pwmBase,
                 pwm_submodule_t Motor1_Submodule, pwm_channels_t Motor1_Channel,
                 pwm_submodule_t Motor2_Submodule, pwm_channels_t Motor2_Channel,
                 GPIO_Type *Motor1_Forward_Port, uint32_t Motor1_Forward_Pin,
                 GPIO_Type *Motor1_Backward_Port, uint32_t Motor1_Backward_Pin,
                 GPIO_Type *Motor2_Forward_Port, uint32_t Motor2_Forward_Pin,
                 GPIO_Type *Motor2_Backward_Port, uint32_t Motor2_Backward_Pin,
                 uint32_t pwmFreq_Hz, uint32_t srcClock_Hz,
                 pwm_clock_prescale_t Motor1_Prescaler, pwm_clock_prescale_t Motor2_Prescaler);
void InitNewPWM(PWM_Type *base,
                pwm_module_control_t module,
                pwm_channels_t pwmChannel,
                pwm_clock_prescale_t divider,
                uint32_t pwmFreq_Hz,
                uint32_t srcClock_Hz);

void HbridgeSetSpeed(Hbridge *hbridge, int motor1_speed, int motor2_speed);
int HbridgeGetSpeed(const Hbridge *hbridge, int motor_index);
void HbridgeSetBrake(Hbridge *hbridge, uint8_t brake_level);
uint8_t HbridgeGetBrake(const Hbridge *hbridge);

#ifdef __cplusplus
}
#endif

#endif /* HBRIDGE_H */
