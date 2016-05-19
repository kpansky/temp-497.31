/*
 * uart.c
 *
 *  Created on: Apr 24, 2016
 *      Author: Chris
 */
#include "uart.h"

//uart variables - global to the file
static char send_buffer[UART_SEND_LENGTH];
static char recv_buffer[UART_RECV_LENGTH];
static int send_buffer_index = 0;
static int recv_buffer_index = 0;
static int send_buffer_length = 0;
static int recv_buffer_length = 0;
static SemaphoreHandle_t rxHandleSem;
static SemaphoreHandle_t txHandleSem;
static SemaphoreHandle_t transmit_mutex;
static SemaphoreHandle_t receive_mutex;
static QueueHandle_t* recv_queue = NULL;
static char autodial_char = '+';

//current baud rate is 12195

void uart_configure()
{
	//disable interrupt while configuring
	NVIC_DisableIRQ(UART2_IRQn);

	//set up FIFO - enable fifo, clean fifos, no DMA, trigger on 1 char
    LPC_SC->PCONP |= (1 << 24);
    LPC_SC->PCLKSEL1 |= 0x10000;

	//disable IrDA
	LPC_UART2->ICR = 0;

	//set serial characteristics, enable brg register r/w
    LPC_UART2->LCR |= 0x83;

	//configure baud rate generator
	LPC_UART2->FDR |= 0x11;
	LPC_UART2->DLM |= 0x1;
	LPC_UART2->DLL |= 0x1;

	//disable brg register r/w (enable ier register r/w)
	LPC_UART2->LCR &= ~0x80;

    LPC_UART2->FCR = 0x07;

	//set pins to UART2
	//TX = P2.8
	//RX = P2.9
	LPC_PINCON->PINSEL4 |= 0xa0000;	//set txd2 and rxd2
	LPC_PINCON->PINMODE4 |= 0xa0000;

	//create semaphores for task handoff
	rxHandleSem = xSemaphoreCreateBinary();
	txHandleSem = xSemaphoreCreateBinary();
	receive_mutex = xSemaphoreCreateMutex();
	transmit_mutex = xSemaphoreCreateMutex();

	//enable rx and tx interrupts
	LPC_UART2->IER = 0x03;

	NVIC_SetPriority(UART2_IRQn,10);
	NVIC_EnableIRQ(UART2_IRQn);
	__enable_irq();
	NVIC_ClearPendingIRQ(UART2_IRQn);
}

//attempts to take mutex, waits until that happens
void uart_send_block(char* buf, int length)
{
	xSemaphoreTake(transmit_mutex,portMAX_DELAY);
	_uart_send(buf,length);
}

//attempts to take mutex, returns immediately if send fails
int uart_send_noblock(char* buf, int length)
{
	if( pdPASS == xSemaphoreTake(transmit_mutex,0))
	{
		_uart_send(buf,length);
		return 0;
	}
	return -1;
}

// Internal send - sets the correct variables and loads the
// first character into the buffer
void _uart_send(char* buf, int length)
{
	send_buffer_index = 0;
	if( length <= UART_SEND_LENGTH )
	{
		send_buffer_length = length;
	}
	else if(length <= 0)
	{
#if UART_DEBUG
		printf("Warning - length set to 0, not sending data\n");
#endif
		return;
	}
	else
	{
		send_buffer_length = UART_SEND_LENGTH;
#if UART_DEBUG
		printf("Warning - truncating uart_send data to %i bytes\n",UART_SEND_LENGTH);
#endif
	}
	memcpy(send_buffer,buf,sizeof(send_buffer));

	LPC_UART2->THR = send_buffer[send_buffer_index];
}

//The LPC1769 has one interrupt for transmit and receive
void UART2_IRQHandler()
{
	uint32_t interrupt_reg = LPC_UART2->IIR;

	//transmit interrupt (interrupt status = 0 and transmit interrupt)
	if( interrupt_reg & 0x2 )
	{
		xSemaphoreGiveFromISR(txHandleSem,NULL);

		//if not use this - from example
		portCLEAR_INTERRUPT_MASK_FROM_ISR( 0 );

		//context switch
		portEND_SWITCHING_ISR( uart_tx_handler );
	}
	//receive interrupt (interrupt status = 0 and receive interrupt
	else if( (interrupt_reg & 0xf) == 0x4 )
	{
		xSemaphoreGiveFromISR(rxHandleSem,NULL);

		//if not use this - from example
		portCLEAR_INTERRUPT_MASK_FROM_ISR( 0 );

		recv_buffer[recv_buffer_index] = LPC_UART2->RBR;

		//context switch
		portEND_SWITCHING_ISR( uart_rx_handler );
	}
}

//transmit handler, load next character into transmit buffer
void uart_tx_handler()
{
	while(1)
	{
		xSemaphoreTake(txHandleSem,portMAX_DELAY);
		send_buffer_index++;

		//not done sending
		if( send_buffer_index < send_buffer_length )
		{
			LPC_UART2->THR = send_buffer[send_buffer_index];
#if UART_DEBUG
			printf("Sending %i\n",send_buffer[send_buffer_index]);
#endif
		}
		else
		{
#if UART_DEBUG
			printf("Done sending\n");
#endif
			//done sending, unlock transmit resource
			xSemaphoreGive(transmit_mutex);
		}

	}
}

//Set up receive queue, or pend on the receive mutex
void uart_receive_block(int length, QueueHandle_t* queue)
{
	xSemaphoreTake(receive_mutex,portMAX_DELAY);
	_uart_receive(length,queue);
}

//Set up receive queue or return with a failure
int uart_receive_noblock(int length, QueueHandle_t* queue)
{
	if(pdPASS == xSemaphoreTake(receive_mutex,0))
	{
		_uart_receive(length,queue);
		return 0;
	}
	return -1;
}

//Internal receive - loads receive variables
void _uart_receive(int length, QueueHandle_t* queue)
{
	recv_buffer_index = 0;
	recv_queue = queue;
	memset(recv_buffer,0,sizeof(recv_buffer));
	if( length <= UART_RECV_LENGTH )
	{
		recv_buffer_length = length;
	}
	else if( length <= 0 )
	{
#if UART_DEBUG
		printf("Warning - length set to 0, not receiving data\n");
#endif
		return;
	}
	else
	{
		recv_buffer_length = UART_RECV_LENGTH;
#if UART_DEBUG
		printf("Warning - truncating received bytes to %i bytes\n",UART_RECV_LENGTH);
#endif
	}

#if UART_DEBUG
	printf("Receive buffer length set to %d, index is %d\n",recv_buffer_length,recv_buffer_index);
#endif
}

// Receive handler, loads next character into receive buffer,
// sends when complete
void uart_rx_handler()
{
	while(1)
	{
		if( pdFAIL == xSemaphoreTake(rxHandleSem,portMAX_DELAY) )
		{
#if UART_DEBUG
			printf("xSemaphoreTake failed!\n");
#endif
		}

		//recv_buffer[recv_buffer_index] = LPC_UART2->RBR;
		recv_buffer_index++;

		//if this was the last character we were expecting
		if( recv_buffer_index >= recv_buffer_length )
		{
			//don't send to the queue if the queue doesn't exist
			if( recv_queue != NULL )
			{
				//send the character to inform the IO layer that
				//a series of letters is incoming
#if UART_DEBUG
				printf("xQueueSend - Sending char %c\n",autodial_char);
#endif
				xQueueSend(*recv_queue,&autodial_char,0);
				for(int i = 0; i < recv_buffer_index; i++ )
				{
#if UART_DEBUG
					printf("xQueueSend - Sending char %c\n",recv_buffer[i]);
#endif
					//send to provided queue - verify that this would work
					xQueueSend(*recv_queue,&recv_buffer[i],0);
				}
			}
			else
			{
#if UART_DEBUG
				printf("Receive queue not set up, discarding characters %s\n",recv_buffer);
#endif
			}
			recv_buffer_index = 0;
		}
	}
}
