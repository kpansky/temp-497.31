#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "basic_io.h"
#include "keypad.h"

void uiInterfaceTask( void *pvParameters )
{
	/* Declare the structure that` will hold the values received from the queue. */
	xData xReceivedStructure;
	portTickType xLastWakeTime;
	portBASE_TYPE xStatus;
	portTickType xTicksToWait = portMAX_DELAY;

	char string[256];
	char currentKey = 0;

	xQueueType * lQueues = (xQueueType*) pvParameters;

	/* This task is also defined within an infinite loop. */
	for (;;)
	{

		xStatus = xQueueReceive(lQueues->xIoInputQueue, &xReceivedStructure, portMAX_DELAY);

		if (xStatus == pdPASS)
		{
			/* Data was successfully received from the queue, print out the
			 * received value and the source of the value.
			 *
			 * NOTE add code to ensure only once device can input characters
			 * at the time.  Ignore the other device
			 * */

			if (xReceivedStructure.ucSource == KEYPAD_TASK)
			{
				if (xReceivedStructure.ucValue != 0)
				{
					currentKey = xReceivedStructure.ucValue;
					sprintf(string, "KEYPAD Symbol Pressed %c \n", currentKey);
					vPrintString(string);

					xStatus = xQueueSendToBack( lQueues->xDACQueue, &xReceivedStructure.ucValue, xTicksToWait );
				}
				else
				{
					sprintf(string, "KEYPAD Symbol Released %c \n", currentKey);
					vPrintString(string);

					xStatus = xQueueSendToBack( lQueues->xDACQueue, &xReceivedStructure.ucValue, xTicksToWait );
				}
			}
			if (xReceivedStructure.ucSource == UART_TASK)
			{
				if (xReceivedStructure.ucValue != 0)
				{
					currentKey = xReceivedStructure.ucValue;
					sprintf(string, "UART Symbol Pressed %c \n", currentKey);
					vPrintString(string);

					xStatus = xQueueSendToBack( lQueues->xDACQueue, &xReceivedStructure.ucValue, xTicksToWait );
				}
				else
				{
					sprintf(string, "UART Symbol Released %c \n", currentKey);
					vPrintString(string);

					xStatus = xQueueSendToBack( lQueues->xDACQueue, &xReceivedStructure.ucValue, xTicksToWait );
				}
			}
		}
		else
		{
			/* We did not receive anything from the queue. This must be an
			error as this task should only run when the queue is full. */
			vPrintString("Could not receive from the queue.\n");
		}

		taskYIELD();

	}
}
