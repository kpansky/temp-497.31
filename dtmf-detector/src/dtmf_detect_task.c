
/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo includes. */
#include "basic_io.h"

#include "dtmf_detect_task.h"
#include "dtmf_data.h"

void vDTMFDetectTask( void *pvParameters ) {

	struct DTMFDetectTaskParam_t* params = (struct DTMFDetectTaskParam_t *)pvParameters;

	vPrintString( "DTMF Detector started\n" );

	for( ;; )
	{
		struct DTMFSamples_t s;
		xQueueReceive( params->sampQ, &s, portMAX_DELAY );

		/* Do real work here */
		struct DTMFResult_t r;

		xQueueSend( params->resultQ, &r, 0 );
	}
}
