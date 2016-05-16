#if !defined(___DAC__H__)
#define ___DAC__H__

/*
 * DAC.h
 * Header for using the DAC (and support features)
 *
 * Theory of operation:
 * User sets up the DAC/DMA engine using calls from API, and then spawns the DAC handler task
 * The DAC handler task will follow the following sequence
 * 		1) Receives message on queue of new data to play out DAC (waits forever)
 * 		2) Sets up DMA of the data, pends until interrupt indicating completion
 * 		3) Sends response
 * 		4) Goes back to step 1
 * Following this sequence, it is safe to send messages to the task while it is still sending
 * a DMA transfer, and it will not be processed until completion. Failure to read from the
 * "completion" queue at a sufficient rate will cause breaks in the DAC playback, as this task
 * will pend.
 *
 *
 * Resources used:
 * DMA interrupt handler   -- set to configMAX_SYSCALL_INTERRUPT_PRIORITY
 * DMA channel 0
 * DAC output pins on board
 *
 * Sw resources:
 * Needs a task, highest priority is best. Only active during the setup of a DMA to the DAC, and to respond on completion
 * 1-2 message queues. One for messages to the DAC, other (optional) for responses for completion
 * Message queue DAC task receives messages on is passed in as the argument on creation
 * Creates/uses a semaphore for the ISR that defers processing to the task (processing is basically releasing a sempahore).
 * Reasonable minimum stack size will work (minimal call depth, little direct stack usage)
 *
 * CPU usage:
 * Task is active from receipt of a message indicating data to send to DAC until the DMA engine is hooked up
 * Task reawakened after DMA completion interrupt fires. Sends a message indicating processing is complete, and then pends
 * on a message queue until more data has arrived.
 *
 *
 * Assumptions:
 * Clock at 100 MHz
 * User of the DAC task is in charge of buffer management. It is legal to queue a transfer while
 * one is in progress, existing transfer will run to completion
 */

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "stdlib.h"


#define TICKS_PER_SECOND_DAC 50000000UL

/*
Samples to the DAC, following the format of the DAC register.
Reserved fields must be set to zero, per spec
*/
typedef struct DAC_Sample{
	uint32_t RESERVED_SET_ZERO:6;
	uint32_t Value:10;
	uint32_t Bias:1;
	uint32_t RESERVED_SET_ZERO_2:15;
} DAC_Sample;

/* If desire to hardcode the sample count, uncomment next line*/
/**/
#define DAC_SAMPLE_PER_SECOND 16000
/**/

/* If desire to hardcode the DAC response, uncomment the next line and point
 * to the name of the desired queue (will not be initialized in DAC cod)*/
/**/
#define DAC_RESPONSE_QUEUE dacResponseHandle
/**/


/*
This is the structure that is sent from the user of the DAC to send list
of samples to send. Optionally, can be told a sample rate, playback time, and
dynamic allocated queue to send response to
*/
typedef struct DAC_Setup_Message{
#if !defined(DAC_SAMPLE_PER_SECOND)
	uint32_t samplesPerSecond;
#endif
	DAC_Sample* firstSample;
	uint32_t numberSamples;
#if defined(ALLOW_LOOPING_DAC_DMA)
	double playTimeSeconds;
#endif
#if !defined(DAC_RESPONSE_QUEUE)
	xQueueHandle completionResponseQueue;
#endif
}DAC_Setup_Message;


/*
This is the response message to the user of the DAC to inform them a buffer
has finished making it out the DAC
*/
typedef struct DAC_Complete_Message{
	DAC_Sample* firstSample;
	uint32_t numberSamples;
}DAC_Complete_Message;

/*
 * Required as part of setup to initialize DMA resources used
 */
void InitializeDMA();

/*
 * Required as part of setup to initialize the DAC
 */
void InitializeDAC();

/*
 * This is the task that handles pumping data to the DMA. Strongly recommended
 * to be highest priority to reduce gaps between playing segments
 * If (optionally) the "ALLOW_LOOPING_DAC_DMA" flag is set, the task will do dynamic
 * malloc/frees to handle sizing a linked list
 */
void DAC_Handler(void* queue);
#endif
