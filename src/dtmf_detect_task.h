#ifndef DTMF_DETECT_TASK_H
#define DTMF_DETECT_TASK_H

/* Parameters passed to the task */
struct DTMFDetectTaskParam_t {
	QueueHandle_t sampQ;
	QueueHandle_t resultQ;
};

void vDTMFDetectTask( void *pvParameters );

#endif
