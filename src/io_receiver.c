/*
 * io_receiver.c
 *
 *  Created on: May 17, 2016
 *      Author: Tom Kadziulis
 */

/*-----------------------------------------------------------------------------
 * vIoRxTask
 *
 * Description:
 *   This task runs in an infinite loop checking the xIoQueue for a message.
 *   If a message contains a character, the task will validate the following
 *   characters.
 *   (1) If character is in the range of 'a'-'d' or 'A'-'D', send the embedded
 *       10 digits number (speed dial) to DAC via function xSend_Dac_Char()
 *       followed by '\0' (release key indicator).
 *   (2) If character is in the range of '0'-'9' send the single character
 *       to DAC via function xSend_Dac_Char() followed by '\0' (release key
 *       indicator).
 *   (3) If character is '+', send the embedded 10 digits number from UART to
 *       DAC via function xSend_Dac_Char(), followed by '\0' (release key
 *       indicator).
 *
 *   Otherwise return immediately if the queue is already empty.
 *
 * Inputs:
 *   xIoQueue - Message Queue for Character Input Requests
 *
 * Outputs:
 *   xQueueToneInput - Message Queue for New Tone Requests to DAC
 *
 */
#include "io_receiver.h"

// Forward declaration
portBASE_TYPE xSend_Dac_Char( const char *pcChar );

//=============================================================================
// vIoRxTask() - IO Receive Task
//=============================================================================
void vIoRxTask( void *pvParameters )
{
  /* Declare the structure that will hold the values received from the queue. */
  cIoMsg cIoMsgBuf;
  static char xDacTxChar;
  portBASE_TYPE xStatus;
  const portTickType xTicksToWait = 100 / portTICK_RATE_MS;
  TickType_t xLastWakeTime;
  const TickType_t xPeriod_200ms = (200 / portTICK_RATE_MS);
  const TickType_t xPeriod_10ms  = (10 / portTICK_RATE_MS);

  char speed_dial_number[SPEED_DAIL_NUMBER_COUNT]={"2246250000"};

  //set the UART to receive every ten characters
  uart_receive_block(NUMBER_COUNT, &xIoQueue);

  /* This task is also defined within an infinite loop. */
  for( ;; )
  {
	xStatus = xQueueReceive( xIoQueue, &cIoMsgBuf, 0 );

	if( xStatus == pdPASS )
	{
      // Check for speed dial characters
	  if( (cIoMsgBuf >= 'a' && cIoMsgBuf <= 'd') ||
          (cIoMsgBuf >= 'A' && cIoMsgBuf <= 'D') )
	  {
	    for(int i=0; i<SPEED_DAIL_NUMBER_COUNT; i++)
	    {
	      // Send one character
	      xDacTxChar = speed_dial_number[i];
	      xSend_Dac_Char(&xDacTxChar);
		  vTaskDelayUntil( &xLastWakeTime, xPeriod_200ms);

		  // Send null - keypad released.
		  // For autodial, we need to manually insert pauses
		  xDacTxChar = '\0';     // button released
	      xSend_Dac_Char(&xDacTxChar);
		  vTaskDelayUntil( &xLastWakeTime, xPeriod_10ms);
		}
	  }

	  else if(cIoMsgBuf >= '0' && cIoMsgBuf <= '9')
	  {
	    xDacTxChar = cIoMsgBuf;
	    xSend_Dac_Char(&xDacTxChar);   // Send one character

	    //This code block, if enabled, will put a fixed time on button presses
	    //if this is enabled, comment out the null char check below
	    /*vTaskDelayUntil( &xLastWakeTime, xPeriod_200ms);
	    xDacTxChar = '\0';             // null character, button released
	    xSend_Dac_Char(&xDacTxChar);
	    vTaskDelayUntil( &xLastWakeTime, xPeriod_10ms);*/
	  }

	  //button released
	  else if(cIoMsgBuf == '\0')
	  {
		  xDacTxChar = cIoMsgBuf;
		  xSend_Dac_Char(&xDacTxChar);
	  }

	  // Message from UART with 10 characters in message buffer.
	  // When we receive this character, we know that NUMBER_COUNT
	  else if(cIoMsgBuf == '+')
	  {
        for(int i=0; i<NUMBER_COUNT; i++)
		{
          xQueueReceive( xIoQueue, &cIoMsgBuf, 0 );

          // Send one character
          xDacTxChar = cIoMsgBuf;
          xSend_Dac_Char(&xDacTxChar);
          vTaskDelayUntil( &xLastWakeTime, xPeriod_200ms);

          // Send null - keypad released.
	      xDacTxChar = '\0';     // button released
          xSend_Dac_Char(&xDacTxChar);
          vTaskDelayUntil( &xLastWakeTime, xPeriod_10ms);
        }
      }
	}
	else
	{
	  /* We did not receive anything from the queue - even after blocking
	   * to wait for data to arrive.  */
	}
  }
}

//=============================================================================
// xSend_Dac_Char() - Send character to DAC
//=============================================================================
portBASE_TYPE xSend_Dac_Char( const char *pcChar )
{
  portBASE_TYPE xStatus;
  const portTickType xTicksToWait = 100 / portTICK_RATE_MS;

  //-- Send data to DAC
#if 1
  xStatus = xQueueSendToBack( xQueueToneInput, pcChar, xTicksToWait );
  if( xStatus != pdPASS )
  {
    vPrintString( "Could not send to the xQueueToneInput.\n" );
  }
#endif

  vPrintString( "xSend_Dac_Char " );
  vPrintChar(pcChar);

  return xStatus;
}
