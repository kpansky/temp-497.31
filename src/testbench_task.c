#include "string.h"

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
		{	DTMF_NO_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_L0_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_L1_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_L2_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_L3_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_NO_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_H0_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_H1_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_H2_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_H3_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_NO_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_L0_PERIOD,	DTMF_H0_PERIOD},
		{	DTMF_L0_PERIOD,	DTMF_H1_PERIOD},
		{	DTMF_L0_PERIOD,	DTMF_H2_PERIOD},
		{	DTMF_L0_PERIOD,	DTMF_H3_PERIOD},
		{	DTMF_NO_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_L1_PERIOD,	DTMF_H0_PERIOD},
		{	DTMF_L1_PERIOD,	DTMF_H1_PERIOD},
		{	DTMF_L1_PERIOD,	DTMF_H2_PERIOD},
		{	DTMF_L1_PERIOD,	DTMF_H3_PERIOD},
		{	DTMF_NO_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_L2_PERIOD,	DTMF_H0_PERIOD},
		{	DTMF_L2_PERIOD,	DTMF_H1_PERIOD},
		{	DTMF_L2_PERIOD,	DTMF_H2_PERIOD},
		{	DTMF_L2_PERIOD,	DTMF_H3_PERIOD},
		{	DTMF_NO_PERIOD,	DTMF_NO_PERIOD},
		{	DTMF_L3_PERIOD,	DTMF_H0_PERIOD},
		{	DTMF_L3_PERIOD,	DTMF_H1_PERIOD},
		{	DTMF_L3_PERIOD,	DTMF_H2_PERIOD},
		{	DTMF_L3_PERIOD,	DTMF_H3_PERIOD},
		{	DTMF_NO_PERIOD,	DTMF_NO_PERIOD},
	};
static int num_tones = sizeof(tones)/sizeof(tones[0]);

void generate_tone(uint16_t period, struct DTMFSamples_t *s);
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
		vPrintString( "Testbench sent data\n" );
		xQueueSendToBack( params->sampQ, &samps, portMAX_DELAY );

		xQueueReceive( params->resultQ, &result, portMAX_DELAY );
		vPrintString( "Testbench received results\n" );

		print_results(&result);
	}
}

void generate_tone(uint16_t p, struct DTMFSamples_t *s) {
	static DTMFSampleType amp = 500;
	int ii;

	if (p != DTMF_NO_PERIOD) {
		for (ii=0; ii<DTMFSampleSize; ii++) {
			if (((ii / (p/2)) % 2) == 0) {
				s->samp[ii] += amp;
			}
			else {
				s->samp[ii] -= amp;
			}
		}
	}
}

void print_results(struct DTMFResult_t *r) {

}
