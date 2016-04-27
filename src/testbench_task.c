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
//#define __CORDIC__
#ifndef __CORDIC__
			if (((ii / (p/2)) % 2) == 0) {
				s->samp[ii] += amp;
			}
			else {
				s->samp[ii] -= amp;
			}
#else
			float fx = (float)amp;
			float fy = 0.0f;
			float fz = need to calculat phasor rate
			cordic (fx,fy,fz,15);
			s->samp[ii] = (DTMFSampleType)fx;
#endif
		}
	}
}

void print_results(struct DTMFResult_t *r) {

}




float pi = 3.14159265359f;

float angles[] = {
    0.78539816339745f,	0.46364760900081f,	0.24497866312686f,	0.12435499454676f,
    0.06241880999596f,	0.03123983343027f,	0.01562372862048f,	0.00781234106010f,
    0.00390623013197f,	0.00195312251648f,	0.00097656218956f,	0.00048828121119f,
    0.00024414062015f,	0.00012207031189f,	0.00006103515617f,	0.00003051757812f,
    0.00001525878906f,	0.00000762939453f,	0.00000381469727f,	0.00000190734863f,
    0.00000095367432f,	0.00000047683716f,	0.00000023841858f,	0.00000011920929f,
    0.00000005960464f,	0.00000002980232f,	0.00000001490116f,	0.00000000745058f,
	};
int angles_n = sizeof(angles)/sizeof(angles[0]);

float Kvalues[] = {
    0.70710678118655f,	0.63245553203368f,	0.61357199107790f,	0.60883391251775f,
    0.60764825625617f,	0.60735177014130f,	0.60727764409353f,	0.60725911229889f,
    0.60725447933256f,	0.60725332108988f,	0.60725303152913f,	0.60725295913894f,
    0.60725294104140f,	0.60725293651701f,	0.60725293538591f,	0.60725293510314f,
    0.60725293503245f,	0.60725293501477f,	0.60725293501035f,	0.60725293500925f,
    0.60725293500897f,	0.60725293500890f,	0.60725293500889f,	0.60725293500888f,
	};
int Kvalues_n = sizeof(Kvalues)/sizeof(Kvalues[0]);

void cordic(float *x, float *y, float *z, int n) {

	float angle;
	float sigma;
	float flip = 1.0f;

	if (n > Kvalues_n) {
		n = Kvalues_n;
	}
	if (n > angles_n) {
		n = angles_n;
	}

	if ((*z < (-pi/2)) || (*z > (pi/2))) {
		if (z < 0) {
			*z += pi;
		} else {
			*z -= pi;
		}
		flip = -1.0f;
	}

	float Kn = Kvalues[n];
	float poweroftwo = 1.0f;

	int j;
	for (j = 0; j<n; j++) {

		if (z < 0) {
			sigma = -1.0f;
		} else {
			sigma = 1.0f;
		}

		angle = angles[j];

		float factor = sigma * poweroftwo;
		float x_new = *x - factor * *y;
		float y_new = *y + factor * *x;
		*x = x_new;
		*y = y_new;
		*z = *z - sigma * angle;
		poweroftwo /= 2.0f;
	}

	*x *= flip;
	*y *= flip;
}
