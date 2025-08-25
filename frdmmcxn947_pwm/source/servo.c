#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_pwm.h"

#include "fsl_clock.h"

#define BOARD_PWM_BASEADDR        PWM1
#define PWM_SRC_CLK_FREQ          CLOCK_GetFreq(kCLOCK_BusClk)
#define DEMO_PWM_FAULT_LEVEL      true
#define APP_DEFAULT_PWM_FREQUENCY (50UL)
#define DEMO_PWM_CLOCK_DEVIDER kPWM_Prescale_Divide_128
#define SRC_CLOCK_HZ         CLOCK_GetFreq(kCLOCK_CoreSysClk)  // Use system clock


void delay_ms(uint32_t ms)
{
    SDK_DelayAtLeastUs(ms * 1000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
}

static void PWM_DRV_Init3PhPwm(void)
{
    uint16_t deadTimeVal;
    pwm_signal_param_t pwmSignal[1];
    uint32_t pwmSourceClockInHz;
    uint32_t pwmFrequencyInHz = APP_DEFAULT_PWM_FREQUENCY;

    pwmSourceClockInHz = PWM_SRC_CLK_FREQ;

    /* Set deadtime count, we set this to about 650ns */
    deadTimeVal = ((uint64_t)pwmSourceClockInHz * 650) / 1000000000;

    pwmSignal[0].pwmChannel       = kPWM_PwmA;
    pwmSignal[0].level            = kPWM_HighTrue;
    pwmSignal[0].dutyCyclePercent = 50; /* 1 percent dutycycle */
    pwmSignal[0].deadtimeValue    = deadTimeVal;
    pwmSignal[0].faultState       = kPWM_PwmFaultState0;
    pwmSignal[0].pwmchannelenable = true;


    /*********** PWMA_SM0 - phase A, configuration, setup 2 channel as an example ************/
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_0, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz);

    /*********** PWMA_SM1 - phase B configuration, setup PWM A channel only ************/
#ifdef DEMO_PWM_CLOCK_DEVIDER
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_1, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz / (1 << DEMO_PWM_CLOCK_DEVIDER));
#else
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_1, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz);
#endif

    /*********** PWMA_SM2 - phase C configuration, setup PWM A channel only ************/
#ifdef DEMO_PWM_CLOCK_DEVIDER
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_2, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz / (1 << DEMO_PWM_CLOCK_DEVIDER));
#else
    PWM_SetupPwm(BOARD_PWM_BASEADDR, kPWM_Module_2, pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz);
#endif
}

/*!
 * @brief Main function
 */
void PWM_Setup(void)
{
    /* Structure of initialize PWM */
    pwm_config_t pwmConfig;
    pwm_fault_param_t faultConfig;

    /* Board pin, clock, debug console init */
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Enable PWM1 SUB Clockn */
    SYSCON->PWM1SUBCTL |=
        (SYSCON_PWM1SUBCTL_CLK0_EN_MASK | SYSCON_PWM1SUBCTL_CLK2_EN_MASK);

    PRINTF("FlexPWM driver example\n");

    PWM_GetDefaultConfig(&pwmConfig);

#ifdef DEMO_PWM_CLOCK_DEVIDER
    pwmConfig.prescale = DEMO_PWM_CLOCK_DEVIDER;
#endif
    pwmConfig.reloadLogic = kPWM_ReloadPwmFullCycle;
    pwmConfig.pairOperation   = kPWM_Independent;
    pwmConfig.enableDebugMode = true;


    if (PWM_Init(BOARD_PWM_BASEADDR, kPWM_Module_0, &pwmConfig) == kStatus_Fail)
    {
        PRINTF("PWM initialization failed\n");
        return;
    }

    /* Initialize submodule 1, make it use same counter clock as submodule 0. */
    pwmConfig.clockSource           = kPWM_Submodule0Clock;
    pwmConfig.prescale              = kPWM_Prescale_Divide_1;
    pwmConfig.initializationControl = kPWM_Initialize_MasterSync;


    PWM_FaultDefaultConfig(&faultConfig);

#ifdef DEMO_PWM_FAULT_LEVEL
    faultConfig.faultLevel = DEMO_PWM_FAULT_LEVEL;
#endif

    /* Sets up the PWM fault protection */
    PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_0, &faultConfig);
   // PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_1, &faultConfig);
    PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_2, &faultConfig);
    PWM_SetupFaults(BOARD_PWM_BASEADDR, kPWM_Fault_3, &faultConfig);

    /* Set PWM fault disable mapping for submodule 0/1/2 */
    PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_0, kPWM_PwmA, kPWM_faultchannel_0,
                             kPWM_FaultDisable_0 );
   // PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmA, kPWM_faultchannel_0,
   //                          kPWM_FaultDisable_0 );
    PWM_SetupFaultDisableMap(BOARD_PWM_BASEADDR, kPWM_Module_2, kPWM_PwmA, kPWM_faultchannel_0,
                             kPWM_FaultDisable_0 );

    /* Call the init function with demo configuration */
    PWM_DRV_Init3PhPwm();

    /* Set the load okay bit for all submodules to load registers from their buffer */
    PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_0, true);

    /* Start the PWM generation from Submodules 0, 1 and 2 */
    PWM_StartTimer(BOARD_PWM_BASEADDR, kPWM_Control_Module_0 );


}


void Steer (double angle, int module){
	if(angle > 100){
		angle = 100;
	}
	if(angle < -100){
		angle=-100;
	}

	double dutyCycle = 5.0 + ((angle + 100.0) / 200.0) * 5.0;
	dutyCycle = ((65535U * dutyCycle) + 50U) / 100U;

	switch (module) {
	    case 0:
	    	PWM_UpdatePwmDutycycleHighAccuracy(BOARD_PWM_BASEADDR, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, dutyCycle);
	    	PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_0, true);
	    	break;

	    case 1:
	    	PWM_UpdatePwmDutycycleHighAccuracy(BOARD_PWM_BASEADDR, kPWM_Module_1, kPWM_PwmA, kPWM_SignedCenterAligned, dutyCycle);
	    	PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_1, true);
	    	break;

	    case 2:
	    	PWM_UpdatePwmDutycycleHighAccuracy(BOARD_PWM_BASEADDR, kPWM_Module_2, kPWM_PwmA, kPWM_SignedCenterAligned, dutyCycle);
	  	    PWM_SetPwmLdok(BOARD_PWM_BASEADDR, kPWM_Control_Module_2, true);
	    	break;

	    default:
	        PRINTF("Invalid channel: %d\n", module);
	        break;
	}
}


