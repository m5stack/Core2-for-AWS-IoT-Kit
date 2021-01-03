/* ULP assembly files are passed through C preprocessor first, so include directives
   and C macros may be used in these files
 */
#include "sdkconfig.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc_ulp.h"

    .set rel_adc_channel, CONFIG_ULP_REL_ADC_CHANNEL

#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
    .set abs_adc_channel, CONFIG_ULP_ABS_ADC_CHANNEL
#endif

	/* Configure the number of ADC samples to average on each measurement.
	   For convenience, make it a power of 2. */
	.set adc_oversampling_factor_log, 4
	.set adc_oversampling_factor, (1 << adc_oversampling_factor_log)

	/* Define variables, which go into .bss section (zero-initialized data) */
	.bss

    .global rel_adc_reading
rel_adc_reading:
    .long 0

	.global rel_adc_prev_reading
rel_adc_prev_reading:
	.long 0

    .global rel_thres
rel_thres:
    .long 0

#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
    .global abs_adc_reading
abs_adc_reading:
    .long 0

    .global abs_adc_prev_reading
abs_adc_prev_reading:
    .long 0

    .global abs_low_level
abs_low_level:
    .long 0

    .global abs_high_level
abs_high_level:
    .long 0
#endif

	/* Code goes into .text section */
	.text
	.global entry
entry:
	/* do measurements using ADC */
	/* r0 will be used as accumulator */
	move    r0, 0

#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
    /* r2 will be used for abs ADC */
    move    r2, 0
#endif

	/* initialize the loop counter */
	stage_rst
measure:
	/* measure and add value to accumulator */
	adc     r1, 0, rel_adc_channel + 1
	add     r0, r0, r1

#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
    adc     r3, 0, abs_adc_channel + 1
    add     r2, r2, r3
#endif

	/* increment loop counter and check exit condition */
	stage_inc   1
	jumps   measure, adc_oversampling_factor, lt

    /* Reset stage_rst */
    stage_rst

	/* divide accumulator by adc_oversampling_factor.
	   Since it is chosen as a power of two, use right shift */
	rsh     r0, r0, adc_oversampling_factor_log

    /* Save rel_adc_reading as rel_adc_prev_reading before updating */
    move    r3, rel_adc_reading
    move    r1, rel_adc_prev_reading
    ld      r3, r3, 0
    st      r3, r1, 0

	/* Averaged value of rel ADC is now in r0; store it into rel_adc_reading */
    move    r3, rel_adc_reading
	st      r0, r3, 0


#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
	/* divide accumulator by adc_oversampling_factor.
	   Since it is chosen as a power of two, use right shift */
    rsh     r2, r2, adc_oversampling_factor_log

    /* Save abs_adc_reading as abs_adc_prev_reading before updating */
    move    r3, abs_adc_reading
    move    r1, abs_adc_prev_reading
    ld      r3, r3, 0
    st      r3, r1, 0

    /* Averaged value of abs ADC is in r2; store it into abs_adc_reading */
    move    r3, abs_adc_reading
    st      r2, r3, 0
#endif

rel_adc:
    /* Wakeup SoC if |rel_adc_reading - rel_adc_prev_reading| > rel_thres */
    move    r3, rel_adc_prev_reading
    ld      r1, r3, 0
    move    r3, rel_adc_reading
    ld      r2, r3, 0
    sub     r0, r1, r2
    jump    neg_num, ov
    jump    check_wakeup

neg_num:
    sub     r0, r2, r1

check_wakeup:
    move    r1, rel_thres
    ld      r2, r1, 0
    sub     r0, r2, r0
    /* Wakeup SoC if the difference in reading is greater than rel_thres */
    jump    wake_up, ov

#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
    /* If difference withing tolerance then try abs adc */
    jump    abs_adc
#endif

	/* value within range, end the program */
	.global exit
exit:
	halt


#ifdef CONFIG_ULP_ABS_ADC_CHANNEL_ENABLE
abs_adc:
    move    r3, abs_adc_reading
    ld      r0, r3, 0

	/* Wakeup SoC if abs_adc_reading < abs_low_level */
	move    r3, abs_low_level
	ld      r3, r3, 0
	sub     r3, r0, r3
	jump    wake_up, ov

	/* Wakeup SoC if abs_adc_reading > abs_high_level (For full charged ?) */
	move    r3, abs_high_level
	ld      r3, r3, 0
	sub     r3, r3, r0
	jump    wake_up, ov

	/* value within range, end the program */
    halt
#endif


	.global wake_up
wake_up:
	wake
	halt
