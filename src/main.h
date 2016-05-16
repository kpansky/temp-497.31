#ifndef MAIN_H
#define MAIN_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "basic_io.h"
#include <stdint.h>

#define DAC_SAMPLE_PER_SECOND   16000

/* Debug Tasks */
#define TONEGEN_INPUT_UNIT_TEST    //CONFIGURABLE - Build in Tone Input Unit Test Tasks
//#define TONEGEN_DMA_UNIT_TEST      //CONFIGURABLE - Build in Tone DMA Unit Test Tasks

/* Debug Prints */
//#define DEBUG_TONE_SCHED           //CONFIGURABLE - Turn on Tone Scheduler Debug Prints
//#define DEBUG_TONE_SAMPLE            //CONFIGURABLE - Turn on print of first Tone Sample

/* Queue Into ToneGenerator Task */
xQueueHandle xQueueToneInput;

/* Queues Between ToneGenerator and DACHandler Task */
xQueueHandle xQueueDMARequest;
xQueueHandle dacResponseHandle;

#endif

