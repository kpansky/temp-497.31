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

/* The task functions. */
void vTask2( void *pvParameters );

/*-----------------------------------------------------------*/

int main( void )
{
	/* Init the semi-hosting. */
	printf( "\n" );

	QueueHandle_t sampQ;
	QueueHandle_t resultQ;
	sampQ = xQueueCreate( 2, sizeof(struct DTMFSamples_t) );
	resultQ = xQueueCreate( 2, sizeof(struct DTMFResult_t) );

	struct TestBenchTaskParam_t TestBenchTaskParam;
	TestBenchTaskParam.sampQ = sampQ;
	TestBenchTaskParam.resultQ = resultQ;
	xTaskCreate(	vTestBenchTask,
					TestBenchTaskName,
					TestBenchTaskStackSz,
					&TestBenchTaskParam,
					1,			/* This task will run at priority 1. */
					NULL );

	struct DTMFDetectTaskParam_t DTMFDetectTaskParam;
	DTMFDetectTaskParam.sampQ = sampQ;
	DTMFDetectTaskParam.resultQ = resultQ;
	xTaskCreate(	vDTMFDetectTask,
					DTMFDetectTaskName,
					DTMFDetectTaskStackSz,
					&DTMFDetectTaskParam,
					2,			/* This task will run at priority 2. */
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


