#include <string.h>
#include <math.h>
#include <stdio.h>

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo includes. */
#include "basic_io.h"

#include "testbench_task.h"
#include "dtmf_data.h"

static DTMFSampleType samps[DTMFSampleSize];
static struct DTMFResult_t result;

struct TB_Tone_t {
	uint16_t toneA;
	uint16_t toneB;
};

static struct TB_Tone_t tones[] =
	{
		/* Discrete tones */
		{	DTMF_NO_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L0_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L1_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L2_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_L3_FREQ,	DTMF_NO_FREQ	},
		{	DTMF_NO_FREQ,	DTMF_H0_FREQ	},
		{	DTMF_NO_FREQ,	DTMF_H1_FREQ	},
		{	DTMF_NO_FREQ,	DTMF_H2_FREQ	},
		{	DTMF_NO_FREQ,	DTMF_H3_FREQ	},
		{	DTMF_NO_FREQ,	DTMF_NO_FREQ	},

		/* Codes 0-9,A-B */
		{	DTMF_L0_FREQ,	DTMF_H0_FREQ	},
		{	DTMF_L0_FREQ,	DTMF_H1_FREQ	},
		{	DTMF_L0_FREQ,	DTMF_H2_FREQ	},
		{	DTMF_L1_FREQ,	DTMF_H0_FREQ	},
		{	DTMF_L1_FREQ,	DTMF_H1_FREQ	},
		{	DTMF_L1_FREQ,	DTMF_H2_FREQ	},
		{	DTMF_L2_FREQ,	DTMF_H0_FREQ	},
		{	DTMF_L2_FREQ,	DTMF_H1_FREQ	},
		{	DTMF_L2_FREQ,	DTMF_H2_FREQ	},
		{	DTMF_L3_FREQ,	DTMF_H0_FREQ	},
		{	DTMF_L3_FREQ,	DTMF_H1_FREQ	},
		{	DTMF_L3_FREQ,	DTMF_H2_FREQ	},
		{	DTMF_L0_FREQ,	DTMF_H3_FREQ	},
		{	DTMF_L1_FREQ,	DTMF_H3_FREQ	},
		{	DTMF_L2_FREQ,	DTMF_H3_FREQ	},
		{	DTMF_L3_FREQ,	DTMF_H3_FREQ	},

	};
static int num_tones = sizeof(tones)/sizeof(tones[0]);
float pi = 3.14159265359f;

void generate_tone(float amp, float freq, DTMFSampleType *s);
void print_results(struct DTMFResult_t *r);

void vTestBenchTask( void *pvParameters ) {
	int tone_index = 0;
	struct TestBenchTaskParam_t* params = (struct TestBenchTaskParam_t *)pvParameters;

	vPrintString( "Testbench started\n" );

	for( ;; )
	{
#ifdef _DTMF_STANDALONE
		//memset(samps, 0, sizeof(samps));
		int ii=0;
		for (ii=0;ii<DTMFSampleSize; ii++) {
			samps[ii] = 0;
		}
		generate_tone(10.0f, tones[tone_index].toneA, samps);
		generate_tone(10.0f, tones[tone_index].toneB, samps);


		/* Pass the synthetic data to the detector */
		void *test = &samps;
		xQueueSendToBack( params->sampQ, &test, portMAX_DELAY );
#endif //_DTMF_STANDALONE
		xQueueReceive( params->resultQ, &result, portMAX_DELAY );

		if ((tones[tone_index].toneA != result.toneA) || (tones[tone_index].toneB != result.toneB)) {
			print_results(&result);
		}


		/* Move to the next tone */
		tone_index++;
		if (tone_index == num_tones) tone_index = 0;
	}
}

void generate_tone(float amp, float freq, DTMFSampleType *s) {
	int ii;
	float fz_tmp = 2.0f * pi * freq / (float)DTMFSampleRate;
	for (ii=0; ii<DTMFSampleSize; ii++) {
		s[ii] += (amp * sin(fz_tmp * (float)ii));
	}
}

void print_results(struct DTMFResult_t *r) {
	if (r->code != ' ' || r->toneA > 0 || r->toneB > 0) {
		printf("Detected Lo(% 4d) Hi(% 4d) Code(%c)\n",r->toneA,r->toneB,r->code);
	} else {
		printf("NO DETECT\n");
	}
}
