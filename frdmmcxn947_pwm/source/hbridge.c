#include "hbridge.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_pwm.h"
#include "fsl_clock.h"

#define BOARD_PWM_BASEADDR        PWM1
#define PWM_SRC_CLK_FREQ          CLOCK_GetFreq(kCLOCK_BusClk)
#define DEMO_PWM_FAULT_LEVEL      true
#define HBRIDGE_PWM_FREQ          1000U
#define DEADTIME_NS               650

#include "fsl_pwm.h"
#include "fsl_debug_console.h"
#include "fsl_clock.h"
#include "board.h"

/* Fault level */
#define DEMO_PWM_FAULT_LEVEL true


static int Handbrake = 0U;
static int CarSpeed = 0;
static int SpeedDutyCycle1;
static int SpeedDutyCycle2;

void InitNewPWM(PWM_Type *base,
                pwm_module_control_t module,
                pwm_channels_t pwmChannel,
                pwm_clock_prescale_t divider,
                uint32_t pwmFreq_Hz,
                uint32_t srcClock_Hz)
{
    pwm_config_t pwmConfig;
    pwm_fault_param_t faultConfig;
    uint16_t deadTimeVal;
    pwm_signal_param_t pwmSignal[2];

    /* Board pin, clock, debug console init */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Enable PWM Submodule Clock */
    SYSCON->PWM1SUBCTL |= (SYSCON_PWM1SUBCTL_CLK0_EN_MASK | SYSCON_PWM1SUBCTL_CLK1_EN_MASK | SYSCON_PWM1SUBCTL_CLK2_EN_MASK);

    PRINTF("FlexPWM driver example for module %d\n", module);

    /* Get default PWM configuration */
    PWM_GetDefaultConfig(&pwmConfig);

    /* Set the divider (prescaler) */
    pwmConfig.prescale = divider;

    /* Set reload logic and complementary operation */
    pwmConfig.reloadLogic = kPWM_ReloadPwmFullCycle;
    pwmConfig.pairOperation = kPWM_Independent;
    pwmConfig.enableDebugMode = true;


    /* Initialize PWM for the specified submodule */
    if (PWM_Init(base, module, &pwmConfig) == kStatus_Fail) {
        PRINTF("PWM initialization failed for Module %d\n", module);
        return;
    }

    /* Configure fault protection */
    PWM_FaultDefaultConfig(&faultConfig);

#ifdef DEMO_PWM_FAULT_LEVEL
    faultConfig.faultLevel = DEMO_PWM_FAULT_LEVEL;
#endif

    /* Setup PWM fault protection */
    PWM_SetupFaults(base, kPWM_Fault_0, &faultConfig);
    PWM_SetupFaults(base, kPWM_Fault_1, &faultConfig);
    PWM_SetupFaults(base, kPWM_Fault_2, &faultConfig);
    PWM_SetupFaults(base, kPWM_Fault_3, &faultConfig);

    /* Set fault disable mapping for the specified module */
    PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_0, kPWM_PwmA, kPWM_faultchannel_0,
                             kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);
    PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmA, kPWM_faultchannel_0,
                             kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);
    PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_2, kPWM_PwmA, kPWM_faultchannel_0,
                             kPWM_FaultDisable_0 | kPWM_FaultDisable_1 | kPWM_FaultDisable_2 | kPWM_FaultDisable_3);
    /* Set dead-time for complementary PWM signals */
    deadTimeVal = ((uint64_t)srcClock_Hz * 650) / 1000000000U;

    /* Configure PWM signals */
    pwmSignal[0].pwmChannel = kPWM_PwmA;
    pwmSignal[0].level = kPWM_HighTrue;
    pwmSignal[0].dutyCyclePercent = 50;   /* 50% duty cycle */
    pwmSignal[0].deadtimeValue = deadTimeVal;
    pwmSignal[0].faultState = kPWM_PwmFaultState0;
    pwmSignal[0].pwmchannelenable = true;

    pwmSignal[1].pwmChannel = kPWM_PwmB;
    pwmSignal[1].level = kPWM_HighTrue;
    pwmSignal[1].dutyCyclePercent = 50;   /* 50% duty cycle */
    pwmSignal[1].deadtimeValue = deadTimeVal;
    pwmSignal[1].faultState = kPWM_PwmFaultState0;
    pwmSignal[1].pwmchannelenable = true;

    /* Setup PWM for the specified module */
    PWM_SetupPwm(base, module, pwmSignal, 2, kPWM_SignedCenterAligned, pwmFreq_Hz, srcClock_Hz);

    /* Set the load okay bit for all submodules to load registers from their buffer */
        PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_0 | kPWM_Control_Module_1 | kPWM_Control_Module_2, true);

        /* Start the PWM generation from Submodules 0, 1 and 2 */
        PWM_StartTimer(BOARD_PWM_BASEADDR, kPWM_Control_Module_0 | kPWM_Control_Module_1 | kPWM_Control_Module_2);



    PRINTF("PWM initialized on %s, Module %d, Channel %d\n", (base == PWM1) ? "PWM1" : "PWM0", module, pwmChannel);
}


/* Function to initialize the H-bridge PWM */
void HbridgeInit(Hbridge *hbridge,
                 PWM_Type *pwmBase,
                 pwm_submodule_t Motor1_Submodule, pwm_channels_t Motor1_Channel,
                 pwm_submodule_t Motor2_Submodule, pwm_channels_t Motor2_Channel,
                 GPIO_Type *Motor1_Forward_Port, uint32_t Motor1_Forward_Pin,
                 GPIO_Type *Motor1_Backward_Port, uint32_t Motor1_Backward_Pin,
                 GPIO_Type *Motor2_Forward_Port, uint32_t Motor2_Forward_Pin,
                 GPIO_Type *Motor2_Backward_Port, uint32_t Motor2_Backward_Pin,
                 uint32_t pwmFreq_Hz, uint32_t srcClock_Hz,
                 pwm_clock_prescale_t Motor1_Prescaler, pwm_clock_prescale_t Motor2_Prescaler) {

    /* GPIO pin configuration structure */
    gpio_pin_config_t motorPinConfig = {
        kGPIO_DigitalOutput,  // Set the pin as output
        0                     // Default output logic level (0 for low)
    };

    /* Save the configuration for the H-Bridge */
    hbridge->pwmBase = pwmBase;
    hbridge->Motor1_Submodule = Motor1_Submodule;
    hbridge->Motor1_Channel = Motor1_Channel;
    hbridge->Motor2_Submodule = Motor2_Submodule;
    hbridge->Motor2_Channel = Motor2_Channel;
    hbridge->Motor1_Forward_Port = Motor1_Forward_Port;
    hbridge->Motor1_Forward_Pin = Motor1_Forward_Pin;
    hbridge->Motor1_Backward_Port = Motor1_Backward_Port;
    hbridge->Motor1_Backward_Pin = Motor1_Backward_Pin;
    hbridge->Motor2_Forward_Port = Motor2_Forward_Port;
    hbridge->Motor2_Forward_Pin = Motor2_Forward_Pin;
    hbridge->Motor2_Backward_Port = Motor2_Backward_Port;
    hbridge->Motor2_Backward_Pin = Motor2_Backward_Pin;
    hbridge->pwmFreq_Hz = pwmFreq_Hz;
    hbridge->srcClock_Hz = srcClock_Hz;
    hbridge->Motor1_Prescaler = Motor1_Prescaler;
    hbridge->Motor2_Prescaler = Motor2_Prescaler;

    InitNewPWM(pwmBase, Motor1_Submodule, Motor1_Channel, Motor1_Prescaler, pwmFreq_Hz, srcClock_Hz);
    InitNewPWM(pwmBase, Motor2_Submodule, Motor2_Channel, Motor2_Prescaler, pwmFreq_Hz, srcClock_Hz);

    // GPIO initialization for motor control could be uncommented once required
     GPIO_PinInit(Motor1_Forward_Port, Motor1_Forward_Pin, &motorPinConfig);
     GPIO_PinInit(Motor1_Backward_Port, Motor1_Backward_Pin, &motorPinConfig);
     GPIO_PinInit(Motor2_Forward_Port, Motor2_Forward_Pin, &motorPinConfig);
     GPIO_PinInit(Motor2_Backward_Port, Motor2_Backward_Pin, &motorPinConfig);
}

/* Function to set motor speeds */
/* Function to set motor speeds with center-aligned PWM */
void HbridgeSetSpeed(Hbridge *hbridge, int motor1_speed, int motor2_speed) {

	if (motor1_speed > 100) {
			motor1_speed = 100;
		} else if (motor1_speed < -100) {
			motor1_speed = -100;
		}

		/* Limit the motor2_speed between -100 and 100 */
		if (motor2_speed > 100) {
			motor2_speed = 100;
		} else if (motor2_speed < -100) {
			motor2_speed = -100;
		}

		/* Handle Motor 1 direction */
		if (motor1_speed >= 0) {
			SpeedDutyCycle1 = (uint32_t)motor1_speed;
			/* Set forward direction for Motor 1 */
			GPIO_PinWrite(hbridge->Motor1_Forward_Port, hbridge->Motor1_Forward_Pin, 1U);
			GPIO_PinWrite(hbridge->Motor1_Backward_Port, hbridge->Motor1_Backward_Pin, 0U);
		} else {
			SpeedDutyCycle1 = (uint32_t)(-motor1_speed);
			/* Set backward direction for Motor 1 */
			GPIO_PinWrite(hbridge->Motor1_Forward_Port, hbridge->Motor1_Forward_Pin, 0U);
			GPIO_PinWrite(hbridge->Motor1_Backward_Port, hbridge->Motor1_Backward_Pin, 1U);
		}

		/* Handle Motor 2 direction */
		if (motor2_speed >= 0) {
			SpeedDutyCycle2 = (uint32_t)motor2_speed;
			/* Set forward direction for Motor 2 */
			GPIO_PinWrite(hbridge->Motor2_Forward_Port, hbridge->Motor2_Forward_Pin, 1U);
			GPIO_PinWrite(hbridge->Motor2_Backward_Port, hbridge->Motor2_Backward_Pin, 0U);
		} else {
			SpeedDutyCycle2 = (uint32_t)(-motor2_speed);
			/* Set backward direction for Motor 2 */
			GPIO_PinWrite(hbridge->Motor2_Forward_Port, hbridge->Motor2_Forward_Pin, 0U);
			GPIO_PinWrite(hbridge->Motor2_Backward_Port, hbridge->Motor2_Backward_Pin, 1U);
		}


    PWM_UpdatePwmDutycycle(hbridge->pwmBase, hbridge->Motor1_Submodule, hbridge->Motor1_Channel, kPWM_SignedCenterAligned, SpeedDutyCycle1);
    PWM_UpdatePwmDutycycle(hbridge->pwmBase, hbridge->Motor2_Submodule, hbridge->Motor2_Channel, kPWM_SignedCenterAligned, SpeedDutyCycle2);

    /* Load the duty cycle changes */
   // PWM_SetPwmLdok(hbridge->pwmBase, kPWM_Control_Module_0, true);
	PWM_SetPwmLdok(hbridge->pwmBase, kPWM_Control_Module_0, true);
	PWM_SetPwmLdok(hbridge->pwmBase, kPWM_Control_Module_1, true);
	PWM_SetPwmLdok(hbridge->pwmBase, kPWM_Control_Module_2, true);

    CarSpeed = (motor1_speed + motor2_speed) / 2;

}


/* Function to get the current motor speed */
int HbridgeGetSpeed(const Hbridge *hbridge, int motor_index) {
	return CarSpeed;
}

/* Function to set the brake */
void HbridgeSetBrake(Hbridge *hbridge, uint8_t brake_level) {
	if(Handbrake != 0U){
			Handbrake = 1U;
			SpeedDutyCycle1 = 0;
			SpeedDutyCycle2 = 0;
	}
	else{
		Handbrake = 0U;
		SpeedDutyCycle1 = (uint32_t)(CarSpeed);
		SpeedDutyCycle2 = (uint32_t)(CarSpeed);
	}
	PWM_UpdatePwmDutycycle(hbridge->pwmBase, hbridge->Motor1_Submodule, hbridge->Motor1_Channel, kPWM_SignedCenterAligned, SpeedDutyCycle1);
	PWM_UpdatePwmDutycycle(hbridge->pwmBase, hbridge->Motor2_Submodule, hbridge->Motor2_Channel, kPWM_SignedCenterAligned, SpeedDutyCycle2);
	PWM_SetPwmLdok(hbridge->pwmBase, kPWM_Control_Module_0 | kPWM_Control_Module_1 | kPWM_Control_Module_2, true);
}

/* Function to get the brake status */
uint8_t HbridgeGetBrake(const Hbridge *hbridge) {
    return Handbrake;

    return 0;
}
