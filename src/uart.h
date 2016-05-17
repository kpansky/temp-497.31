/*
 * uart.h
 *
 *  Created on: Apr 24, 2016
 *      Author: Chris
 */
#include <LPC17xx.h>
#include "FreeRTOS.h"
#include "portable.h"
#include "semphr.h"
#include "queue.h"
#include <string.h>
#include <stdio.h>

/***********************************************************
 * UART functionality
 *
 * Description:
 *   This code controls the UART2 module of the LPC1769.
 *   It sends and receives at a rate of 9600 baud.
 *
 * Usage:
 *   To use the UART, first call the uart_configure()
 *   function.  This enables the UART module and configures
 *   it.
 *
 *   To send, use the uart_send_block() or
 *   uart_send_noblock(), depending on whether it is
 *   acceptable for the calling task to pend on previous
 *   send operations completing.
 *
 *   To receive, use the uart_receive_block() or
 *   uart_receive_noblock() functions.  The receiver
 *   passes in a pointer to the queue that the data
 *   will be sent to.  The queue should accept
 *   one character inputs.
 *
 *   The user will need to create two tasks with the
 *   following entry points:
 *   uart_tx_handler()
 *   uart_rx_handler()
 *   These are interrupt handlers and should be
 *   prioritized accordingly.
 *
 **********************************************************/


#ifndef UART_H_
#define UART_H_

//maximum send and receive lengths
#define UART_SEND_LENGTH 50
#define UART_RECV_LENGTH 50

//set this to non-zero to enable printf debug statements
#define UART_DEBUG 0

//set parity type
typedef enum{
	UART_PARITY_ODD = 0,
	UART_PARITY_EVEN,
	UART_PARITY_FORCED_1,
	UART_PARITY_FORCED_0,
	UART_PARITY_NONE
} parity_type;

//set start/stop bits
typedef enum{
	UART_1_STOP = 0,
	UART_2_STOP
} stop_type;

//set number of bits
typedef enum{
	UART_5_BIT = 0,
	UART_6_BIT,
	UART_7_BIT,
	UART_8_BIT
} data_size_type;

//configure the UART - this must be called before the UART will work
void uart_configure(parity_type parity,
					stop_type stop,
					data_size_type data_size);

//UART public send functions
//
// Send data - attempts to take mutex, waits until that happens
void uart_send_block(char* buf, int length);
// Send data - attempts to take mutex, returns immediately if
//   take operation fails.  Returns 0 on success or -1 on
//   failure.
int uart_send_noblock(char* buf, int length);

//UART public receive functions
//
// Receive data - attempts to take mutex, waits until that happens
void uart_receive_block(int length, QueueHandle_t* queue);
// Receive data - attempts to take mutex, returns immediately if
//   take operation fails.  Returns 0 on success or -1 on
//   failure
int uart_receive_noblock(int length, QueueHandle_t* queue);

//private functions - unsafe to call directly
void _uart_send(char* buf, int length);
void _uart_receive(int length, QueueHandle_t* queue);

//function names for handler tasks
void uart_tx_handler();
void uart_rx_handler();

#endif /* UART_H_ */
