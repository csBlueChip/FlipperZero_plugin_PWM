#include  <stdint.h>

#include  <furi.h>               // Core API
#include  <furi_hal_gpio.h>      // GPIO

#include  "bc_pwm.h"
#include  "bc_logging.h"

//----------------------------------------------------------------------------- ----------------------------------------
// Harware definitions
//
const pwmHw_t  pwmHw[PWM_ID_CNT] = {
	[PWM_ID_IR]      = PWM_HW_IR,
	[PWM_ID_SPEAKER] = PWM_HW_SPEAKER,
	[PWM_ID_PB3]     = PWM_HW_PB3
};

//+============================================================================ ========================================
bool  pwmInit (pwmActor_t* pwm,  pwmId_t id,  uint16_t prescale,  pwmMode_t mode)
{
	ENTER;
	furi_assert(pwm);

	memset(pwm, 0, sizeof(*pwm));

	if ((mode >= PWM_MODE_CNT) || (id >= PWM_ID_CNT)) {
		pwm->hw = NULL;

	} else {
		// Initialise the Actor
		pwm->id         = id;
		pwm->hw         = &pwmHw[id];
		pwm->dcPrescale = prescale;
		pwm->dcMode     = mode;

		INFO("%s : %s", __func__, pwm->hw->name);

		// Initialise the Timer (I think)
	    FURI_CRITICAL_ENTER();
	    LL_TIM_DeInit(pwm->hw->timer);
	    FURI_CRITICAL_EXIT();

		// Initialise the GPIO pin
	    furi_hal_gpio_init_ex(pwm->hw->pin, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, pwm->hw->altFn);
	}

	LEAVE;
	return !!pwm->hw;
}

//+============================================================================ ========================================
static inline
uint32_t  pwmCalcAutoreload (pwmActor_t* pwm)
{
	furi_assert(pwm);

	pwm->dcReload = ((SystemCoreClock / pwm->dcPrescale) / pwm->fHz) - 1;
	if      (pwm->dcReload < 2)           return (pwm->dcReload = 2) ;           // floor limit
	else if (pwm->dcReload > UINT16_MAX)  return (pwm->dcReload = UINT16_MAX) ;  // ceiling limit
	else                                  return  pwm->dcReload ;                // in range
}


//+============================================================================ ========================================
static inline
uint32_t  pwmCalcCompare (pwmActor_t* pwm)
{
	furi_assert(pwm);

	// 0.0 <= duty scale <= 1.0
	float  scale;
	if      (pwm->dcDuty < 0.0)  scale = 0.0 ;          // floor limit
	else if (pwm->dcDuty > 1.0)  scale = 1.0 ;          // ceiling limit
	else                         scale = pwm->dcDuty ;  // in range

	// Different functions use different scaling mechanisms
	switch (pwm->dcMode) {
		default:
		case PWM_MODE_DUTY:
			// Use the duty cycle asis
			break;

		case PWM_MODE_BRIGHTNESS:
			// LEDs are brightness scaled using triangular numbers
			//! Not written (yet) - just use the volume curve for now
			//break;

		case PWM_MODE_VOLUME:
			// Stolen from the FZ demo code
			// No idea what this curve is called, but it looks nice
			// https://www.desmos.com/calculator/4quoxcgdxa
			scale = (scale * scale * scale) /2;
			break;
	}

	pwm->dcComp = LL_TIM_GetAutoReload(pwm->hw->timer) * scale;
	if (pwm->dcComp == 0)  pwm->dcComp = 1 ;  // 0 -> 1

	return pwm->dcComp;
}

//+============================================================================ ========================================
bool  pwmStop (pwmActor_t* pwm)
{
	ENTER;
	furi_assert(pwm);

	LL_TIM_DisableAllOutputs(pwm->hw->timer);  // disconnect the timer
	LL_TIM_DisableCounter(pwm->hw->timer);     // ...and stop it

	pwm->run = false;

	LEAVE;
	return true;
}

//+============================================================================ ========================================
bool  pwmRun (pwmActor_t* pwm)
{
	ENTER;
	furi_assert(pwm);

	bool  rv = true;  // assume success

	if ((pwm->fHz <= 0) || (pwm->dcDuty <= 0)) {
		WARN("%s : requested null signal", __func__);
		rv = false;

	} else {
		LL_TIM_EnableAllOutputs(pwm->hw->timer);  // connect the timer
		LL_TIM_EnableCounter(pwm->hw->timer);     // ...and start it
	}

	pwm->run = true;

	LEAVE;
	return rv;
}

//+============================================================================ ========================================
bool  pwmSet (pwmActor_t* pwm,  float fHz,  float duty,  bool run)
{
	ENTER;
	furi_assert(pwm);

	bool  rv = true;  // assume success

	pwm->fHz     = fHz;
	pwm->dcDuty  = duty;

	pwmStop(pwm);

	// --> lib/STM32CubeWB/Drivers/STM32WBxx_HAL_Driver/Inc/stm32wbxx_ll_tim.h 
	LL_TIM_InitTypeDef  tim = (LL_TIM_InitTypeDef){
		.Prescaler  = pwm->dcPrescale - 1,
		.Autoreload = pwmCalcAutoreload(pwm)
	};
	LL_TIM_Init(pwm->hw->timer, &tim);

	LL_TIM_OC_InitTypeDef  oc = (LL_TIM_OC_InitTypeDef){
		.OCMode       = LL_TIM_OCMODE_PWM1,
		.OCState      = LL_TIM_OCSTATE_ENABLE,
		.CompareValue = pwmCalcCompare(pwm)
	};
	LL_TIM_OC_Init(pwm->hw->timer, pwm->hw->chan, &oc);

	if (run)  rv = pwmRun(pwm) ;

	LEAVE;
	return rv;
}
