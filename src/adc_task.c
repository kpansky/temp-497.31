#include <adc_task.h>

static xQueueHandle sampQ;
DTMFSampleType ADC_BUFFERS[NUM_ADC_BUFFERS][DTMFSampleSize];

int32_t adc_init(void)
{
	/* Enable Power in SC register */
	LPC_SC->PCONP |= (1 << 12);

	/* Enable ADC in AD0CR */
	LPC_ADC->ADCR = (1 << 21); // PDN bit

	/* PCLK_ADC selection */
	LPC_SC->PCLKSEL0 |= (1 << 24); //PCLK_ADC - CCLK/1 = 100MHz/1 = 100Mhz

	/* Config ADC clock divider */
	LPC_ADC->ADCR |= (10 << 8); //ADC clock = 100/100 = 10MHz

	/* Pin select */
	LPC_PINCON->PINSEL1 |= (0x01 << 14);	//configure pin P0.23 as as AD0.0

	/* Select ADC channel 0 in ADCR register */
	LPC_ADC->ADCR |= (1 << 0);		// SEL in ADCR

	/* start the conversion so it is ready when the next interrupt fires */
	LPC_ADC->ADCR |= (0x1 << 24); //start in ADCR

	return 1;
}

DTMFSampleType adc_read(void)
{
	DTMFSampleType adc_data;

	/* Wait for conversion complete
	 * !!! WARNING !!!! This will block */
	while ((LPC_ADC->ADGDR & 0x80000000) == 0x0); //Done bit auto clear

	/* Get the result and mask it off, format it to a int16_t for DTMF Task */
	adc_data = (LPC_ADC->ADGDR >> 1) & 0x7FFF;

	/* start the conversion so it is ready when the next interrupt fires */
	LPC_ADC->ADCR |= (0x1 << 24); //start in ADCR

	return adc_data;
}

void vAdcTask( void *pvParameters )
{
	uint32_t read_cnt = 0;
	uint32_t current_buffer = 0;
	uint32_t previous_buffer = 0;
	static DTMFSampleType* buffer = NULL;

	sampQ = (xQueueHandle)pvParameters;

	xAdcSemaphore = xSemaphoreCreateBinary();

	if (xAdcSemaphore != NULL && sampQ != NULL)
	{
		adc_init();
		timer_init();

		/* Enable timer interrupt */
		NVIC_SetPriority(TIMER3_IRQn, 6);
		NVIC_ClearPendingIRQ(TIMER3_IRQn);
		NVIC_EnableIRQ(TIMER3_IRQn);

		vPrintString( "ADC Reading Started\n" );
		/* As per most tasks, this task is implemented in an infinite loop. */
		for( ;; )
		{
			/* Get semaphore give from ISR */
			xSemaphoreTake(xAdcSemaphore, portMAX_DELAY);

			ADC_BUFFERS[current_buffer][read_cnt] = adc_read();

			/* Fill current buffer */
			if (read_cnt < NUM_ADC_SAMPLES-1) {
				read_cnt++;
			} else {
				/* swap buffers and wait for processing of semaphore */
				read_cnt = 0;
				previous_buffer = current_buffer;
				if(current_buffer < NUM_ADC_BUFFERS-1) {
					current_buffer++;
				} else {
					current_buffer = 0;
				}
				buffer = ADC_BUFFERS[previous_buffer];
				xQueueSendToBack(sampQ, &buffer, portMAX_DELAY);
			}
		}
	} else {
		vPrintString( "ADC Task Failed to Initialize!\n" );
	}
}

int32_t timer_init(void)
{
	/* Enable the clock to the timer */
	LPC_SC->PCONP |= (1 << PCTIM3);

	/* Reset Timer */
	LPC_TIM3->TCR = TC_RESET;

	/* Set Timer Mode */
	LPC_TIM3->CTCR = TC_TIMER_MODE;

	/* Set count to force interrupt */
	LPC_TIM3->MR0 = ADC_TIMER_COUNT / ADC_TIMER_PRESCALE;

	/* Use a prescale factor */
	LPC_TIM3->PR = ADC_TIMER_PRESCALE;

	/* Set Timer/Counter to Timer Mode */
	LPC_TIM3->MCR = (CT_MR_INTERRUPT | CT_MR_RESET) << MATCH_REG0;

	/* Set Capture Control Register to disable  */
	LPC_TIM3->CCR = 0;

	/* Set External Match Register to disable  */
	LPC_TIM3->EMR = 0;

	/* Enable Timer */
	LPC_TIM3->TCR = TC_ENABLE;

	return 0;
}

void TIMER3_IRQHandler(void)
{

	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* If the time to match is pending for this timer
	 * clear the pending interrupt and give the semaphore
	 * for the ADC to begin processing
	 */
	if (LPC_TIM3->IR &= (1 << CT_MAT0_INTERRUPT) != 0) {
		LPC_TIM3->IR &= ~(1 << CT_MAT0_INTERRUPT);
		xSemaphoreGiveFromISR(xAdcSemaphore, &xHigherPriorityTaskWoken);
	}
	NVIC_ClearPendingIRQ(TIMER3_IRQn);
}

