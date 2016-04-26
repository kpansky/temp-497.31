/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo includes. */
#include "basic_io.h"

#include "testbench_task.h"
#include "dtmf_data.h"

static struct DTMFSamples_t s;

void vTestBenchTask( void *pvParameters ) {

	struct TestBenchTaskParam_t* params = (struct TestBenchTaskParam_t *)pvParameters;

	vPrintString( "Testbench started\n" );

	for( ;; )
	{
		/* Do real work here */
		int ii;
		for (ii=0; ii<DTMFSampleSize; ii++) {
			s.samp[ii] = 0;
		}

		/* Pass the synthetic data to the detector */
		vPrintString( "Testbench sent data\n" );
		xQueueSendToBack( params->sampQ, &s, portMAX_DELAY );


		struct DTMFResult_t r;
		xQueueReceive( params->resultQ, &r, portMAX_DELAY );
		vPrintString( "Testbench received results\n" );

		/* Do real work here */
	}
}
