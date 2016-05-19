#include <math.h>

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo includes. */
#include "basic_io.h"

#include "dtmf_detect_task.h"
#include "dtmf_data.h"
#include "fft/fft.h"

#include "uart.h"

static DTMFSampleType* s;
static struct DTMFResult_t r;
static complex cs[DTMFSampleSize];

#define spec_power(x) ((x.Re * x.Re) + (x.Im * x.Im))
void pick_peaks(complex *cs, float thresh, int16_t *toneA, int16_t *toneB);
int8_t decode_tones(int16_t toneA, int16_t toneB);

void vDTMFDetectTask( void *pvParameters ) {

	struct DTMFDetectTaskParam_t* params = (struct DTMFDetectTaskParam_t *)pvParameters;
	char output[50];

	vPrintString( "DTMF Detector started\n" );

	/* Prepare FFT */
	init_Wn();

	for( ;; )
	{
		/* Peek at the data rather than receive to prevent the buffer from
		 * being reused until we are done */
		BaseType_t status = xQueuePeek( params->sampQ, &s, portMAX_DELAY );
		if(status == pdPASS) {

#ifdef __DTMF_PERF__
			UBaseType_t stack_max = uxTaskGetStackHighWaterMark( 0 );
			TickType_t t0 = xTaskGetTickCount();
			TickType_t t1;
#endif

			/* Convert samples to complex floating point */
			int ii;
			for (ii=0; ii<DTMFSampleSize; ii++) {
				cs[ii].Re = (float)s[ii] / 16384.0f;
				cs[ii].Im = 0.0f;
			}

			/* Now that we have a separate copy, release the buffer */
			xQueueReceive( params->sampQ, &s, portMAX_DELAY );

			/* Do real work here */
			fft(cs, DTMFSampleSize); // Need to ensure second arg is log2(DTMFSampleSize)
			pick_peaks(cs, 5.0f, &r.toneA, &r.toneB);
			r.code = decode_tones(r.toneA,r.toneB);

#ifdef __DTMF_PERF__
			t1 = xTaskGetTickCount();
			printf("DTMF STACK %d TIME %d\n", stack_max, t1-t0);
#endif

			// Send, but allow dropping
			if(r.code != ' ')
			{
				sprintf(output,"Detected code %c\r\n",r.code);
				uart_send_noblock(output,strlen(output));
				printf("Detected code %c\n",r.code);
			}
		}
	}
}

/* Pick the DTMF tones from FFT resuts */
/* threshold is the minimum amount above noise floor required to declare a tone */
/* toneX ar the detected tones (Hz) */
void pick_peaks(complex *cs, float threshold, int16_t *toneA, int16_t *toneB) {

	*toneA = 0;
	*toneB = 0;
	float avg = 0.0f;
	int ii;

	/* Compute the average spectrum power */
	for (ii=0; ii<DTMFSampleSize; ii++) {
		cs[ii].Re = spec_power(cs[ii]);
		avg += cs[ii].Re / DTMFSampleSize;
	}

	/* Scale the average power by the threshold */
	threshold *= avg;

	/* check low bins for power */
	if (cs[DTMF_L0_BIN].Re > threshold) {
		*toneA = DTMF_L0_FREQ;
	} else if (cs[DTMF_L1_BIN].Re > threshold) {
		*toneA = DTMF_L1_FREQ;
	} else if (cs[DTMF_L2_BIN].Re > threshold) {
		*toneA = DTMF_L2_FREQ;
	} else if (cs[DTMF_L3_BIN].Re > threshold) {
		*toneA = DTMF_L3_FREQ;
	}

	/* check high bins for power */
	if (cs[DTMF_H0_BIN].Re > threshold) {
		*toneB = DTMF_H0_FREQ;
	} else if (cs[DTMF_H1_BIN].Re > threshold) {
		*toneB = DTMF_H1_FREQ;
	} else if (cs[DTMF_H2_BIN].Re > threshold) {
		*toneB = DTMF_H2_FREQ;
	} else if (cs[DTMF_H3_BIN].Re > threshold) {
		*toneB = DTMF_H3_FREQ;
	}
}

/* Provided to frequencies (Hz) reurn an ASCII representation of the DTMF */
/* Illegal combinations return space */
int8_t decode_tones(int16_t toneA, int16_t toneB) {
	if ((toneA == DTMF_L0_FREQ) && (toneB == DTMF_H0_FREQ)) {
		return '1';
	}
	if ((toneA == DTMF_L0_FREQ) && (toneB == DTMF_H1_FREQ)) {
		return '2';
	}
	if ((toneA == DTMF_L0_FREQ) && (toneB == DTMF_H2_FREQ)) {
		return '3';
	}
	if ((toneA == DTMF_L0_FREQ) && (toneB == DTMF_H3_FREQ)) {
		return 'A';
	}
	if ((toneA == DTMF_L1_FREQ) && (toneB == DTMF_H0_FREQ)) {
		return '4';
	}
	if ((toneA == DTMF_L1_FREQ) && (toneB == DTMF_H1_FREQ)) {
		return '5';
	}
	if ((toneA == DTMF_L1_FREQ) && (toneB == DTMF_H2_FREQ)) {
		return '6';
	}
	if ((toneA == DTMF_L1_FREQ) && (toneB == DTMF_H3_FREQ)) {
		return 'B';
	}
	if ((toneA == DTMF_L2_FREQ) && (toneB == DTMF_H0_FREQ)) {
		return '7';
	}
	if ((toneA == DTMF_L2_FREQ) && (toneB == DTMF_H1_FREQ)) {
		return '8';
	}
	if ((toneA == DTMF_L2_FREQ) && (toneB == DTMF_H2_FREQ)) {
		return '9';
	}
	if ((toneA == DTMF_L2_FREQ) && (toneB == DTMF_H3_FREQ)) {
		return 'C';
	}
	if ((toneA == DTMF_L3_FREQ) && (toneB == DTMF_H0_FREQ)) {
		return '*';
	}
	if ((toneA == DTMF_L3_FREQ) && (toneB == DTMF_H1_FREQ)) {
		return '0';
	}
	if ((toneA == DTMF_L3_FREQ) && (toneB == DTMF_H2_FREQ)) {
		return '#';
	}
	if ((toneA == DTMF_L3_FREQ) && (toneB == DTMF_H3_FREQ)) {
		return 'D';
	}

	return ' ';
}
