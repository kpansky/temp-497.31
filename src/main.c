#include <stdio.h>

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo includes. */
#include "basic_io.h"

/* Project includes. */
#include "testbench_task.h"
#include "dtmf_detect_task.h"
#include "dtmf_data.h"

/*-----------------------------------------------------------*/

static QueueHandle_t sampQ;
static QueueHandle_t resultQ;
static struct TestBenchTaskParam_t TestBenchTaskParam;
static struct DTMFDetectTaskParam_t DTMFDetectTaskParam;

int main( void )
{
	/* Init the semi-hosting. */
	printf( "\n" );

	sampQ = xQueueCreate( 1, sizeof(struct DTMFSamples_t) );
	resultQ = xQueueCreate( 1, sizeof(struct DTMFResult_t) );

	TestBenchTaskParam.sampQ = sampQ;
	TestBenchTaskParam.resultQ = resultQ;
	xTaskCreate(	vTestBenchTask,
					"tTB",
					500,
					(void *)&TestBenchTaskParam,
					2,
					NULL );

	DTMFDetectTaskParam.sampQ = sampQ;
	DTMFDetectTaskParam.resultQ = resultQ;
	xTaskCreate(	vDTMFDetectTask,
					"tDetect",
					500,
					(void *)&DTMFDetectTaskParam,
					2,
					NULL );

	/* Start the scheduler so our tasks start executing. */
	vTaskStartScheduler();

	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );
	return 0;
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* This function will only be called if an API call to create a task, queue
	or semaphore fails because there is too little heap RAM remaining. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	/* This function will only be called if a task overflows its stack.  Note
	that stack overflow checking does slow down the context switch
	implementation. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* This example does not use the idle hook to perform any processing. */
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This example does not use the tick hook to perform any processing. */
}


