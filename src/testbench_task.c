#include <string.h>
#include <math.h>

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo includes. */
#include "basic_io.h"

#include "testbench_task.h"
#include "dtmf_data.h"

static struct DTMFSamples_t samps;
static struct DTMFResult_t result;

struct TB_Tone_t {
	uint16_t toneA;
	uint16_t toneB;
};

static struct TB_Tone_t tones[] =
	{
		{	DTMF_NO_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L0_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L1_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L2_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L3_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_NO_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_H0_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_H1_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_H2_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_H3_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_NO_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L0_FREQ,	DTMF_H0_FREQ	},
		{	DTMF_L0_FREQ,	DTMF_H1_FREQ	},
		{	DTMF_L0_FREQ,	DTMF_H2_FREQ	},
		{	DTMF_L0_FREQ,	DTMF_H3_FREQ	},
		{	DTMF_NO_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L1_FREQ,	DTMF_H0_FREQ	},
		{	DTMF_L1_FREQ,	DTMF_H1_FREQ	},
		{	DTMF_L1_FREQ,	DTMF_H2_FREQ	},
		{	DTMF_L1_FREQ,	DTMF_H3_FREQ	},
		{	DTMF_NO_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L2_FREQ,	DTMF_H0_FREQ	},
		{	DTMF_L2_FREQ,	DTMF_H1_FREQ	},
		{	DTMF_L2_FREQ,	DTMF_H2_FREQ	},
		{	DTMF_L2_FREQ,	DTMF_H3_FREQ	},
		{	DTMF_NO_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L3_FREQ,	DTMF_H0_FREQ	},
		{	DTMF_L3_FREQ,	DTMF_H1_FREQ	},
		{	DTMF_L3_FREQ,	DTMF_H2_FREQ	},
		{	DTMF_L3_FREQ,	DTMF_H3_FREQ	},
		{	DTMF_NO_FREQ,	DTMF_NO_FREQ	},
	};
static int num_tones = sizeof(tones)/sizeof(tones[0]);
float pi = 3.14159265359f;

void generate_tone(float freq, struct DTMFSamples_t *s);
void print_results(struct DTMFResult_t *r);

void vTestBenchTask( void *pvParameters ) {
	int tone_index = 0;
	struct TestBenchTaskParam_t* params = (struct TestBenchTaskParam_t *)pvParameters;

	vPrintString( "Testbench started\n" );

	for( ;; )
	{
		memset(&samps, 0, sizeof(samps));
		generate_tone(tones[tone_index].toneA, &samps);
		generate_tone(tones[tone_index].toneB, &samps);

		/* Move to the next tone */
		tone_index++;
		if (tone_index == num_tones) tone_index = 0;

		/* Pass the synthetic data to the detector */
		//vPrintString( "Testbench sent data\n" );
		xQueueSendToBack( params->sampQ, &samps, portMAX_DELAY );

		xQueueReceive( params->resultQ, &result, portMAX_DELAY );
		//vPrintString( "Testbench received results\n" );

		print_results(&result);
	}
}

void generate_tone(float freq, struct DTMFSamples_t *s) {
	int ii;
	float amp = 4000.0f;
	float fz_tmp = 2.0f * pi * freq / (float)DTMFSampleRate;
	for (ii=0; ii<DTMFSampleSize; ii++) {
		s->samp[ii] += (amp * sin(fz_tmp * (float)ii));
	}
}

void print_results(struct DTMFResult_t *r) {

}
