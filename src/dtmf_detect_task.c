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

static struct DTMFSamples_t s;
static struct DTMFResult_t r;
complex cs[DTMFSampleSize];

#define spec_power(x) (sqrt((x.Re * x.Re) + (x.Im * x.Im)))
void pick_peaks(complex *cs, float thresh, int16_t *toneA, int16_t *toneB);
int16_t decode_tones(int16_t toneA, int16_t toneB);

void vDTMFDetectTask( void *pvParameters ) {

	struct DTMFDetectTaskParam_t* params = (struct DTMFDetectTaskParam_t *)pvParameters;

	vPrintString( "DTMF Detector started\n" );
	init_Wn();

	for( ;; )
	{
		xQueueReceive( params->sampQ, &s, portMAX_DELAY );

		vPrintString( "DTMF Detector working\n" );

		int ii;
		for (ii=0; ii<DTMFSampleSize; ii++) {
			cs[ii].Re = (float)s.samp[ii] / 16384.0f; // Need to convert this to float
			cs[ii].Im = 0.0f;
		}

		/* Do real work here */
		fft(cs, DTMFSampleSize); // Need to ensure second arg is log2(DTMFSampleSize)
		pick_peaks(cs, 5.0f, &r.toneA, &r.toneB);
		r.code = decode_tones(r.toneA,r.toneB);

		xQueueSendToBack( params->resultQ, &r, portMAX_DELAY );
	}
}

void pick_peaks(complex *cs, float threshold, int16_t *toneA, int16_t *toneB) {

	*toneA = -1;
	*toneB = -1;

	/* check low bins for power */
	if (spec_power(cs[DTMF_L0_BIN]) >= threshold) {
		*toneA = DTMF_L0_FREQ;
		vPrintString( "Detected tone L0\n" );
	} else if (spec_power(cs[DTMF_L1_BIN]) >= threshold) {
		*toneA = DTMF_L1_FREQ;
		vPrintString( "Detected tone L1\n" );
	} else if (spec_power(cs[DTMF_L2_BIN]) >= threshold) {
		*toneA = DTMF_L2_FREQ;
		vPrintString( "Detected tone L2\n" );
	} else if (spec_power(cs[DTMF_L3_BIN]) >= threshold) {
		*toneA = DTMF_L3_FREQ;
		vPrintString( "Detected tone L3\n" );
	}

	/* check high bins for power */
	if (spec_power(cs[DTMF_H0_BIN]) >= threshold) {
		*toneB = DTMF_H0_FREQ;
		vPrintString( "Detected tone H0\n" );
	} else if (spec_power(cs[DTMF_H1_BIN]) >= threshold) {
		*toneB = DTMF_H1_FREQ;
		vPrintString( "Detected tone H1\n" );
	} else if (spec_power(cs[DTMF_H2_BIN]) >= threshold) {
		*toneB = DTMF_H2_FREQ;
		vPrintString( "Detected tone H2\n" );
	} else if (spec_power(cs[DTMF_H3_BIN]) >= threshold) {
		*toneB = DTMF_H3_FREQ;
		vPrintString( "Detected tone H3\n" );
	}
}

int16_t decode_tones(int16_t toneA, int16_t toneB) {
	return -1;
}
