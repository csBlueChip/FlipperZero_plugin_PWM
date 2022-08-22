#ifndef BC_PWM_H
#define BC_PWM_H

#include  <stdint.h>

#include  <furi_hal_gpio.h>      // GPIO
#include  <furi_hal.h>
#include  <furi_hal_infrared.h>  // NOT included by furi_hal.h !?

//***************************************************************************** ****************************************
//	This is a pretty good tutorial on PWM:
//		https://www.allaboutcircuits.com/technical-articles/introduction-to-microcontroller-timers-pwm-timers/
//		https://web.archive.org/web/20220000000000*/https://www.allaboutcircuits.com/technical-articles/introduction-to-microcontroller-timers-pwm-timers/
//

//----------------------------------------------------------------------------- ----------------------------------------
// The PWM duty cycle can be shaped/scaled for different applications
// Eg. Setting the duty cycle to 50% does NOT set an   LED   to 50% brightness ...cos physics
//     Setting the duty cycle to 50% does NOT set a  Speaker to 50% volume     ...cos biology
//
// Eg. When the user says "0.37" they mean 37% of PERCEIVED volume, NOT 37% of a waveform on an oscilloscope screen!
//
typedef
	enum {
		PWM_MODE_DUTY,
		PWM_MODE_VOLUME,
		PWM_MODE_BRIGHTNESS,

		PWM_MODE_CNT // MUST be last
	}
pwmMode_t;

//----------------------------------------------------------------------------- ----------------------------------------
// These values are predefined by the hardware
// https://www.st.com/resource/en/datasheet/stm32wb55cc.pdf     ->  page 73, Table 18
// https://docs.flipperzero.one/development/hardware/schematic  ->  MCU STM32WB55
//
// These values are lifted straight out of the MCU datasheet:
//
//    PA4 :                                  14 = LPTIM2_OUT
//    PA6 :  1 = TIM1_BKIN   12 = TIM1_BKIN  14 = TIM16_CH1
//    PA7 :  1 = TIM1_CH1N                   14 = TIM17_CH1
//    PB2 :  1 = LPTIM1_OUT
//   +PB3 :  1 = TIM2_CH2
//   >PB8 :  1 = TIM1_CH2N                   14 = TIM16_CH1
//   *PB9 :  1 = TIM1_CH3N                   14 = TIM17_CH1 
//    PC0 :  1 = LPTIM1_IN1                  14 = LPTIM2_IN1
//    PC1 :  1 = LPTIM1_OUT
//    PC3 :  1 = LPTIM1_ETR                  14 = LPTIM2_EPR
//
//   + TIM2 is 32bit (all other timers are 16bit)
//   > Internal Speaker
//   * Internal IR LED Array
//
typedef
	struct pwmHw {
		const char*     name;   // Something friendly
		const GpioPin*  pin;    // --> firmware/targets/f7/furi_hal/furi_hal_gpio.h
		GpioAltFn       altFn;  // --> firmware/targets/f7/furi_hal/furi_hal_gpio.h
		TIM_TypeDef*    timer;  // --> lib/STM32CubeWB/Drivers/CMSIS/Device/ST/STM32WBxx/Include/stm32wb55xx.h
		uint32_t        chan;   // --> lib/STM32CubeWB/Drivers/STM32WBxx_HAL_Driver/Inc/stm32wbxx_ll_tim.h
		                        //     --> lib/STM32CubeWB/Drivers/CMSIS/Device/ST/STM32WBxx/Include/stm32wb55xx.h
	}
pwmHw_t;

//-----------------------------------------------------------------------------
// .pin    --> firmware/targets/f7/furi_hal/furi_hal_resources.h
// .altFn  --> firmware/targets/f7/furi_hal/furi_hal_gpio.h
// .timer  --> lib/STM32CubeWB/Drivers/CMSIS/Device/ST/STM32WBxx/Include/stm32wb55xx.h
// .chan   --> lib/STM32CubeWB/Drivers/STM32WBxx_HAL_Driver/Inc/stm32wbxx_ll_tim.h
//
typedef
	enum pwmId {
		PWM_ID_IR,       // PB9/14 = TIM17_CH1
		PWM_ID_SPEAKER,  // PB8/14 = TIM16_CH1
		PWM_ID_PB3,      // PB3/1  = TIM2_CH2

		PWM_ID_CNT  // MUST be last
	}
pwmId_t;

//-----------------------------------------------------------------------------
#define  PWM_HW_IR       (pwmHw_t){  \
	.name  = "PB9: IR",              \
	.pin   = &gpio_infrared_tx,      \
	.altFn = GpioAltFn14TIM17,       \
	.timer = TIM17,                  \
	.chan  = LL_TIM_CHANNEL_CH1,     \
}

#define  PWM_HW_SPEAKER  (pwmHw_t){  \
	.name  = "PB8: Spkr",            \
	.pin   = &gpio_speaker,          \
	.altFn = GpioAltFn14TIM16,       \
	.timer = TIM16,                  \
	.chan  = LL_TIM_CHANNEL_CH1,     \
}

#define  PWM_HW_PB3      (pwmHw_t){  \
	.name  = "PB3: GPIO",            \
	.pin   = &gpio_ext_pb3,          \
	.altFn = GpioAltFn1TIM2,         \
	.timer = TIM2,                   \
	.chan  = LL_TIM_CHANNEL_CH2,     \
}

//-----------------------------------------------------------------------------
// The actual array exists in bc_pwm.c
// ...uncomment this to make it "public"
//
extern  const pwmHw_t  pwmHw[PWM_ID_CNT];

//----------------------------------------------------------------------------- ----------------------------------------
typedef
	struct pwmActor {
		pwmId_t         id;          // PWM hardware ID
		const pwmHw_t*  hw;          // PWM (iummutable) hardware details

		// Set by the user
		float           fHz;         // Frequency in Hz
		float           dcDuty;      // Duty cycle (0.0 <= dcDuty <= 1.0)
		uint16_t        dcPrescale;  //! How do I choose a good value for the prescaler?
		pwmMode_t       dcMode;      // Duty cycle scaling algorithm

		// Used by the engine - exposed here
		uint32_t        dcComp;      // Duty cycle comparator
		uint32_t        dcReload;    // Autoreload value
		bool            run;         // true if PWM is running
	}
pwmActor_t;

//----------------------------------------------------------------------------- ----------------------------------------
bool  pwmInit (pwmActor_t* pwm,  pwmId_t id,  uint16_t prescale,  pwmMode_t mode) ;
bool  pwmSet  (pwmActor_t* pwm,  float fHz,  float duty,  bool run) ;
bool  pwmRun  (pwmActor_t* pwm) ;
bool  pwmStop (pwmActor_t* pwm) ;

#endif // BC_PWM_H
