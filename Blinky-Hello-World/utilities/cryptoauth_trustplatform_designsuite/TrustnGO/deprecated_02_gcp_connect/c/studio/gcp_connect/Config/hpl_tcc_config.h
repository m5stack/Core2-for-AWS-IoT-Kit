/* Auto-generated config file hpl_tcc_config.h */
#ifndef HPL_TCC_CONFIG_H
#define HPL_TCC_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

#include "peripheral_clk_config.h"

#ifndef CONF_TCC0_ENABLE
#define CONF_TCC0_ENABLE 1
#endif

// <h> Basic configuration
// <y> Prescaler
// <TCC_CTRLA_PRESCALER_DIV1_Val"> No division
// <TCC_CTRLA_PRESCALER_DIV2_Val"> Divide by 2
// <TCC_CTRLA_PRESCALER_DIV4_Val"> Divide by 4
// <TCC_CTRLA_PRESCALER_DIV8_Val"> Divide by 8
// <TCC_CTRLA_PRESCALER_DIV16_Val"> Divide by 16
// <TCC_CTRLA_PRESCALER_DIV64_Val"> Divide by 64
// <TCC_CTRLA_PRESCALER_DIV256_Val"> Divide by 256
// <TCC_CTRLA_PRESCALER_DIV1024_Val"> Divide by 1024
// <i> This defines the prescaler value
// <id> timer_prescaler
#ifndef CONF_TCC0_PRESCALER
#define CONF_TCC0_PRESCALER TCC_CTRLA_PRESCALER_DIV1_Val
#endif

//<o> Length of one timer tick in uS <0-16777216>
// <id> timer_tick
#ifndef CONF_TCC0_TIMER_TICK
#define CONF_TCC0_TIMER_TICK 1000
#endif

// </h>

// <e> Advanced configuration
// <id> timer_advanced_configuration
#ifndef CONF_TCC0_ADVANCED_CONFIGURATION_ENABLE
#define CONF_TCC0_ADVANCED_CONFIGURATION_ENABLE 0
#endif

// <y> Prescaler and Counter Synchronization Selection
// <TCC_CTRLA_PRESCSYNC_GCLK_Val"> Reload or reset counter on next GCLK
// <TCC_CTRLA_PRESCSYNC_PRESC_Val"> Reload or reset counter on next prescaler clock
// <TCC_CTRLA_PRESCSYNC_RESYNC_Val"> Reload or reset counter on next GCLK and reset prescaler counter
// <i> These bits select if on retrigger event, the Counter should be cleared or reloaded on the next GCLK_TCCx clock or on the next prescaled GCLK_TCCx clock.
// <id> tcc_arch_presync
#ifndef CONF_TCC0_PRESCSYNC
#define CONF_TCC0_PRESCSYNC TCC_CTRLA_PRESCSYNC_GCLK_Val
#endif

// <q> Run in standby
// <i> Indicates whether the module will continue running in standby sleep mode or not
// <id> tcc_arch_runstdby
#ifndef CONF_TCC0_RUNSTDBY
#define CONF_TCC0_RUNSTDBY 0
#endif

// <q> Run in debug mode
// <i> Indicates whether the module will run in debug mode or not
// <id> tcc_arch_dbgrun
#ifndef CONF_TCC0_DBGRUN
#define CONF_TCC0_DBGRUN 0
#endif

// </e>

// <e> Event control
// <id> timer_event_control
#ifndef CONF_TCC0_EVENT_CONTROL_ENABLE
#define CONF_TCC0_EVENT_CONTROL_ENABLE 0
#endif

// <q> Match or Capture Channel 0 Event Output
// <i> This bit indicates whether match/capture event on channel 0 is enabled and will be generated
// <id> tcc_arch_mceo0
#ifndef CONF_TCC0_MCEO0
#define CONF_TCC0_MCEO0 0
#endif

// <q> Match or Capture Channel 0 Event Input
// <i> This bit indicates whether match/capture 0 incoming event is enabled
// <id> tcc_arch_mcei0
#ifndef CONF_TCC0_MCEI0
#define CONF_TCC0_MCEI0 0
#endif
// <q> Match or Capture Channel 1 Event Output
// <i> This bit indicates whether match/capture event on channel 1 is enabled and will be generated
// <id> tcc_arch_mceo1
#ifndef CONF_TCC0_MCEO1
#define CONF_TCC0_MCEO1 0
#endif

// <q> Match or Capture Channel 1 Event Input
// <i> This bit indicates whether match/capture 1 incoming event is enabled
// <id> tcc_arch_mcei1
#ifndef CONF_TCC0_MCEI1
#define CONF_TCC0_MCEI1 0
#endif
// <q> Match or Capture Channel 2 Event Output
// <i> This bit indicates whether match/capture event on channel 2 is enabled and will be generated
// <id> tcc_arch_mceo2
#ifndef CONF_TCC0_MCEO2
#define CONF_TCC0_MCEO2 0
#endif

// <q> Match or Capture Channel 2 Event Input
// <i> This bit indicates whether match/capture 2 incoming event is enabled
// <id> tcc_arch_mcei2
#ifndef CONF_TCC0_MCEI2
#define CONF_TCC0_MCEI2 0
#endif
// <q> Match or Capture Channel 3 Event Output
// <i> This bit indicates whether match/capture event on channel 3 is enabled and will be generated
// <id> tcc_arch_mceo3
#ifndef CONF_TCC0_MCEO3
#define CONF_TCC0_MCEO3 0
#endif

// <q> Match or Capture Channel 3 Event Input
// <i> This bit indicates whether match/capture 3 incoming event is enabled
// <id> tcc_arch_mcei3
#ifndef CONF_TCC0_MCEI3
#define CONF_TCC0_MCEI3 0
#endif

// <q> Timer/Counter Event Input 0
// <i> This bit is used to enable input event 0 to the TCC
// <id> tcc_arch_tcei0
#ifndef CONF_TCC0_TCEI0
#define CONF_TCC0_TCEI0 0
#endif

// <q> Timer/Counter Event Input 0 Invert
// <i> This bit inverts the event 0 input
// <id> tcc_arch_tceinv0
#ifndef CONF_TCC0_TCINV0
#define CONF_TCC0_TCINV0 0
#endif
// <q> Timer/Counter Event Input 1
// <i> This bit is used to enable input event 1 to the TCC
// <id> tcc_arch_tcei1
#ifndef CONF_TCC0_TCEI1
#define CONF_TCC0_TCEI1 0
#endif

// <q> Timer/Counter Event Input 1 Invert
// <i> This bit inverts the event 1 input
// <id> tcc_arch_tceinv1
#ifndef CONF_TCC0_TCINV1
#define CONF_TCC0_TCINV1 0
#endif

// <q> Timer/Counter Event Output
// <i> This bit is used to enable the counter cycle event.
//<id> tcc_arch_cnteo
#ifndef CONF_TCC0_CNTEO
#define CONF_TCC0_CNTEO 0
#endif

// <q> Re-trigger Event Output
// <i> This bit is used to enable the counter re-trigger event.
//<id> tcc_arch_trgeo
#ifndef CONF_TCC0_TRGEO
#define CONF_TCC0_TRGEO 0
#endif

// <q> Overflow/Underflow Event Output
// <i> Generates event for counter overflows/underflows
//<id> tcc_arch_ovfeo
#ifndef CONF_TCC0_OVFEO
#define CONF_TCC0_OVFEO 0
#endif

// <o> Timer/Counter Interrupt and Event Output Selection
// <0=>An interrupt/event is generated at the beginning of each counter cycle
// <1=>An interrupt/event is generated at the end of each counter cycle
// <2=>An interrupt/event is generated between each counter cycle
// <3=>An interrupt/event is generated at beginning of the first counter cycle, and at the end of the last counter cycle
// <i> These bits define on which part of the counter cycle the counter event output is generated
// <id> arch_tcc_cntsel
#ifndef CONF_TCC0_CNTSEL
#define CONF_TCC0_CNTSEL 0
#endif

// <o> Timer/Counter Event Input 0 Action
// <0=>Event action disabled
// <1=>Start restart or re-trigger on event
// <2=>Count on event
// <3=>Start on event
// <4=>Increment on event
// <5=>Count on active state of asynchronous event
// <6=>Capture overflow times (Max value)
// <7=>Non-recoverable fault
// <i> These bits define the action the TCC performs on TCE0 event input 0
// <id> arch_tcc_evact0
#ifndef CONF_TCC0_EVACT0
#define CONF_TCC0_EVACT0 0
#endif

// <o> Timer/Counter Event Input 1 Action
// <0=>Event action disabled
// <1=>Start restart or re-trigger on event
// <2=>Count on event
// <3=>Start on event
// <4=>Increment on event
// <5=>Count on active state of asynchronous event
// <6=>Capture overflow times (Max value)
// <7=>Non-recoverable fault
// <i> These bits define the action the TCC performs on TCE0 event input 0
// <id> arch_tcc_evact1
#ifndef CONF_TCC0_EVACT1
#define CONF_TCC0_EVACT1 0
#endif
// </e>

#ifndef CONF_TCC0_ALOCK
#define CONF_TCC0_ALOCK 0
#endif

#ifndef CONF_TCC0_CPTEN0
#define CONF_TCC0_CPTEN0 0
#endif
#ifndef CONF_TCC0_CPTEN1
#define CONF_TCC0_CPTEN1 0
#endif
#ifndef CONF_TCC0_CPTEN2
#define CONF_TCC0_CPTEN2 0
#endif
#ifndef CONF_TCC0_CPTEN3
#define CONF_TCC0_CPTEN3 0
#endif

#ifndef CONF_TCC0_LUPD
#define CONF_TCC0_LUPD 0
#endif

#ifndef CONF_TCC0_WAVEGEN
#define CONF_TCC0_WAVEGEN 0
#endif

#ifndef CONF_TCC0_CC0
#define CONF_TCC0_CC0 0
#endif
#ifndef CONF_TCC0_CC1
#define CONF_TCC0_CC1 0
#endif
#ifndef CONF_TCC0_CC2
#define CONF_TCC0_CC2 0
#endif
#ifndef CONF_TCC0_CC3
#define CONF_TCC0_CC3 0
#endif

// Calculating correct top value based on requested tick interval.
#define CONF_TCC0_PRESCALE (1 << CONF_TCC0_PRESCALER)

#if CONF_TCC0_PRESCALER > TCC_CTRLA_PRESCALER_DIV16_Val
#undef CONF_TCC0_PRESCALE
#define CONF_TCC0_PRESCALE 64
#endif

#if CONF_TCC0_PRESCALER > TCC_CTRLA_PRESCALER_DIV64_Val
#undef CONF_TCC0_PRESCALE
#define CONF_TCC0_PRESCALE 256
#endif

#if CONF_TCC0_PRESCALER > TCC_CTRLA_PRESCALER_DIV256_Val
#undef CONF_TCC0_PRESCALE
#define CONF_TCC0_PRESCALE 1024
#endif

#ifndef CONF_TCC0_PER
#define CONF_TCC0_PER                                                                                                  \
	(uint32_t)(((float)CONF_TCC0_TIMER_TICK / 1000000.f) / (1.f / (CONF_GCLK_TCC0_FREQUENCY / CONF_TCC0_PRESCALE)))
#endif

#define CONF_TCC0_CTRLA                                                                                                \
	TCC_CTRLA_PRESCALER(CONF_TCC0_PRESCALER) | (CONF_TCC0_RUNSTDBY << TCC_CTRLA_RUNSTDBY_Pos)                          \
	    | TCC_CTRLA_PRESCSYNC(CONF_TCC0_PRESCSYNC) | (CONF_TCC0_CPTEN0 << TCC_CTRLA_CPTEN0_Pos)                        \
	    | (CONF_TCC0_CPTEN1 << TCC_CTRLA_CPTEN1_Pos) | (CONF_TCC0_CPTEN2 << TCC_CTRLA_CPTEN2_Pos)                      \
	    | (CONF_TCC0_CPTEN3 << TCC_CTRLA_CPTEN3_Pos) | (CONF_TCC0_ALOCK << TCC_CTRLA_ALOCK_Pos)
#define CONF_TCC0_CTRLB (CONF_TCC0_LUPD << TCC_CTRLBSET_LUPD_Pos)
#define CONF_TCC0_DBGCTRL (CONF_TCC0_DBGRUN << TCC_DBGCTRL_DBGRUN_Pos)
#define CONF_TCC0_EVCTRL                                                                                               \
	TCC_EVCTRL_CNTSEL(CONF_TCC0_CNTSEL) | (CONF_TCC0_OVFEO << TCC_EVCTRL_OVFEO_Pos)                                    \
	    | (CONF_TCC0_TRGEO << TCC_EVCTRL_TRGEO_Pos) | (CONF_TCC0_CNTEO << TCC_EVCTRL_CNTEO_Pos)                        \
	    | (CONF_TCC0_MCEO0 << TCC_EVCTRL_MCEO0_Pos) | (CONF_TCC0_MCEI0 << TCC_EVCTRL_MCEI0_Pos)                        \
	    | (CONF_TCC0_MCEO1 << TCC_EVCTRL_MCEO1_Pos) | (CONF_TCC0_MCEI1 << TCC_EVCTRL_MCEI1_Pos)                        \
	    | (CONF_TCC0_MCEO2 << TCC_EVCTRL_MCEO2_Pos) | (CONF_TCC0_MCEI2 << TCC_EVCTRL_MCEI2_Pos)                        \
	    | (CONF_TCC0_MCEO3 << TCC_EVCTRL_MCEO3_Pos) | (CONF_TCC0_MCEI3 << TCC_EVCTRL_MCEI3_Pos)                        \
	    | (CONF_TCC0_TCEI0 << TCC_EVCTRL_TCEI0_Pos) | (CONF_TCC0_TCEI1 << TCC_EVCTRL_TCEI1_Pos)                        \
	    | (CONF_TCC0_TCINV0 << TCC_EVCTRL_TCINV0_Pos) | (CONF_TCC0_TCINV1 << TCC_EVCTRL_TCINV1_Pos)                    \
	    | TCC_EVCTRL_EVACT1(CONF_TCC0_EVACT1) | TCC_EVCTRL_EVACT0(CONF_TCC0_EVACT0)

// <<< end of configuration section >>>

#endif // HPL_TCC_CONFIG_H
