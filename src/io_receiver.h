/*
 * io_receiver.h
 *
 *  Created on: May 17, 2016
 *      Author: Tom
 */

#ifndef IO_RECEIVER_H_
#define IO_RECEIVER_H_

#include "main.h"
#include "uart.h"

#define NUMBER_COUNT 10
#define SPEED_DAIL_NUMBER_COUNT 10
#define IO_BUFFER_SIZE 16

typedef char cIoMsg;   // IO message buffer

// Tone Input Queue
xQueueHandle xQueueToneInput;

/* IO Receive Tasks */
void vIoRxTask( void *pvParameters );

#endif /* IO_RECEIVER_H_ */
