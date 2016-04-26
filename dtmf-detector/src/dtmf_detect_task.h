#ifndef DTMF_DETECT_TASK_H
#define DTMF_DETECT_TASK_H

#define DTMFDetectTaskStackSz 2000
#define DTMFDetectTaskName "tDTMFDetect"

struct DTMFDetectTaskParam_t {
	QueueHandle_t sampQ;
	QueueHandle_t resultQ;
};

void vDTMFDetectTask( void *pvParameters );

#endif
