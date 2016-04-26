#ifndef TESTBENCH_TASK_H
#define TESTBENCH_TASK_H

#define TestBenchTaskStackSz 1000
#define TestBenchTaskName "tTB"

struct TestBenchTaskParam_t {
	QueueHandle_t sampQ;
	QueueHandle_t resultQ;

};

void vTestBenchTask( void *pvParameters );

#endif
