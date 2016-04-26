/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo includes. */
#include "basic_io.h"

#include "testbench_task.h"
#include "dtmf_data.h"



void vTestBenchTask( void *pvParameters ) {

	struct TestBenchTaskParam_t* params = (struct TestBenchTaskParam_t *)pvParameters;

	vPrintString( "Testbench started\n" );

	for( ;; )
	{
		/* Do real work here */
		struct DTMFSamples_t s;
		int ii;
		for (ii=0; ii<DTMFSampleSize; ii++) {
			s.samp[ii] = 0;
		}

		/* Pass the synthetic data to the detector */
		vPrintString( "Testbench sent data" );
		xQueueSend( params->sampQ, &s, portMAX_DELAY );


		struct DTMFResult_t r;
		xQueueReceive( params->resultQ, &r, portMAX_DELAY );
		vPrintString( "Testbench received results" );

		/* Do real work here */
	}
}
