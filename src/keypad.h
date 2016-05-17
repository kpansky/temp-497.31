/*
 * keypad.h
 *
 *  Authors: Gennadiy Dubilirer & Michal Starosta
 *  Class: RTOS
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdio.h>

/* Includes. */
#include "basic_io.h"

/* Library includes. */
#include "LPC17xx.h"

#define KEYPAD_TASK 1
#define UART_TASK 2

/* Define the structure type that will be passed on the queue. */
typedef struct
{
  char ucValue;
  unsigned char ucSource;
} xData;

typedef struct
{
  xQueueHandle xIoInputQueue;
  xQueueHandle xDACQueue;
} xQueueType;

/*
 * GPIOGetValue - Task - process keypad key press/release upon
 * external interrupt.  Send key value pressed, or 0 for release.
 *
 */
void gpioInterfaceTask(void *pvParameters);

#endif /* KEYPAD_H_ */
