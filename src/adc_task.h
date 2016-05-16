#ifndef __ADC_H__
#define __ADC_H__

#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include "queue.h"
#include "semphr.h"
#include "basic_io.h"
#include "LPC17xx.h"
#include "dtmf_data.h"

/* Define the count to trigger the interrupt for performing an ADC measurement
 * with a 100MHz clock output from the PLL and a desire to run the ADC at 62.5 us
 * the count would be 62.5us / 10 ns = 6250
 */
/* #define ADC_TIMER_COUNT (6250  / 4) */
#define ADC_TIMER_COUNT (100000000 / 16000 / 4)
#define CT_MAT0_INTERRUPT (0)
#define ADC_TIMER_PRESCALE (10)
#define TC_TIMER_MODE 0b00
#define MRI 0
#define MRR 1
#define MRS 2
#define MATCH_REG0 (0)
#define MATCH_REG1 (3)
#define MATCH_REG2 (6)
#define MATCH_REG3 (9)
#define CT_MR_INTERRUPT (1 << MRI)
#define CT_MR_RESET (1 << MRR)
#define CT_MR_STOP (1 << MRS)
#define TC_RESET (1 << 1)
#define TC_ENABLE (1 << 0)
#define PCTIM3 (23)

#define NUM_ADC_BUFFERS 2
#define NUM_ADC_SAMPLES 256
#define ADC_DATA_TYPE uint16_t
#define ADC_QUEUE_LEN 1

/* Hanldes to RTOS types */
static xSemaphoreHandle xAdcSemaphore;

/* The task function. */
void vAdcTask( void *pvParameters );
int32_t adc_init(void);
int32_t timer_init(void);
DTMFSampleType adc_read(void);

#endif // __ADC_H__
