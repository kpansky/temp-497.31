#include "tonegen.h"
#include "sinelut.h"

/*-----------------------------------------------------------
 * vTaskToneGenerator
 *
 * Description:  This task runs in an infinite loop and fills a
 * buffer with samples for the tone currently being played.  Tone
 * type (i.e. 0, 1, #, etc.) and whether the tone should be on or off comes in through
 * the xQueueToneInput message queue.  The tasks fills the buffer
 * in steps, and checks for a new request before going on to the
 * next step.  This minimizes latency of filling an entire buffer
 * before receiving the next request.  The step size for filling the buffer is configurable
 * (TONE_BUFFER_SAMPLE_SET).  The buffer size is also configurable
 * (TONE_BUFFER_SIZE).  TONE_BUFFER_SIZE should be a multiple of
 * TONE_BUFFER_SAMPLE_SET or the buffer will be overrun.
 *
 * When a buffer is full, a DMA request is sent to the xQueueDMARequest
 * message queue and a new buffer is used while the previous buffer
 * contents are sent to the DAC via DMA.  Upon DMA completion, a message
 * is received from the xQueueDMAComplete queue and the buffer is made
 * available for use.
 *
 * Inputs:  xQueueToneInput   Message Queue for New Tone Requests
 *          xQueueDMAComplete Message Queue for DMA Complete
 *                             (free tone buffer)
 * Outputs: xQueueDMARequest  Message Queue for DMA Requests
 *
 */

/* Shared Memory Tone Buffer and Tone Buffer Management Constructs */
DAC_Sample sampleBuf[NUM_TONE_BUFFERS][TONE_BUFFER_SIZE];
uint8_t bufInUse[NUM_TONE_BUFFERS];
const uint32_t INVALID_BUFFER = 0xFF;
uint32_t t;

void vTaskToneGenerator( void *pvParameters )
{
  char dtfmReq;
  portBASE_TYPE xStatus;
  TONE_ON_OFF_TYPE tone_in_progress = TONE_OFF;
  uint8_t activeBuffer  = INVALID_BUFFER;
  uint8_t i = 0;
  t = 0;

  for(i = 0; i < NUM_TONE_BUFFERS; i++)
  {
	  vPrintStringAndNumber("sampleBuf[i] = ", (uint32_t) sampleBuf[i]);
	  bufInUse[i] = 0;
  }

  /* As per most tasks, this task is implemented in an infinite loop. */
  for( ;; )
  {
    /* Check for new input from the queue, don't wait if there is
     * not any new input
     */
    xStatus = xQueueReceive( xQueueToneInput, &dtfmReq, 0 );

    //If something was received from the queue, there is a new command
    //to process
    if( xStatus == pdPASS )
    {
      if(dtfmReq != 0) //Tone On request
      {
        #ifdef DEBUG_TONE_SCHED
          vPrintStringAndNumber("Received new ON request.  Tone =", dtfmReq.tone_type);
        #endif
        //Set tone in progress flag
        tone_in_progress = TONE_ON;

        //Fill buffer with some samples
        FillBuffer(dtfmReq, &activeBuffer);
        #ifdef DEBUG_TONE_SCHED
          vPrintString("   New request.  Samples Added.\n");
        #endif

        SendSamplesToDMA(&activeBuffer);
      }
      else // dtfmReq.on_off == TONE_OFF
      {
        #ifdef DEBUG_TONE_SCHED
          vPrintString("  Tone off\n");
        #endif

          //Clear tone in progress flag
        tone_in_progress = TONE_OFF;
      }
    }
    //Else nothing was received from the queue, continue previous
    //request
    else
    {
      if(tone_in_progress == TONE_ON)
      {
        FillBuffer(dtfmReq, &activeBuffer);
        #ifdef DEBUG_TONE_SCHED
          vPrintString("   No new request.  Samples Added.\n");
        #endif

        SendSamplesToDMA(&activeBuffer);
      }
      //else no active tone ... nothing to play
    }
  }
}

void FillBuffer(char btn, uint8_t *activeBuffer)
{
  circular_buffer cb;
  uint8_t newBufNeeded = 0;

  //Obtain valid buffer
  if(*activeBuffer == INVALID_BUFFER)
  {
    GetNextAvailableBuffer(activeBuffer);
    newBufNeeded = 1;
  }

  //Add samples to buffer
  if(*activeBuffer != INVALID_BUFFER)
  {
      cb_init(&cb, &sampleBuf[*activeBuffer][0], TONE_BUFFER_SIZE);
      dtmfGen(btn, &cb);
  }
  else
  {
    vPrintString("ERROR:  Buffer invalid\n");
  }
}


void SendSamplesToDMA(uint8_t *activeBuffer)
{
  portBASE_TYPE xStatus;
  DAC_Setup_Message dmaReq;

  if(*activeBuffer != INVALID_BUFFER)
  {
	  dmaReq.firstSample = &sampleBuf[*activeBuffer][0];
	  dmaReq.numberSamples = TONE_BUFFER_SIZE;
	  xStatus = xQueueSendToBack( xQueueDMARequest, &dmaReq, 0 );

	  if(xStatus != pdPASS)
	  {
		vPrintString("Failed to send DMA request");
	  }

	  //Clear offset and obtain next buffer
	  *activeBuffer = INVALID_BUFFER;
	  GetNextAvailableBuffer(activeBuffer);
  }
  else
  {
	  vPrintString("Unable to send DMA request - buffer invalid.\n");
  }
}

void GetNextAvailableBuffer(uint8_t *activeBuffer)
{
  uint8_t buf = 0;
  *activeBuffer = INVALID_BUFFER;
  uint8_t done = 0;
  portBASE_TYPE xStatus;
  DAC_Sample *bufPtrToRelease = NULL;

  while(done == 0)
  {
	  //Loop over all buffers
	  for(buf = 0; buf < NUM_TONE_BUFFERS; buf++)
	  {
		//Check for an available buffer
		if(bufInUse[buf] == 0)
		{
		  //available buffer found!
		  bufInUse[buf] = 1;
		  *activeBuffer = buf;
		  #ifdef DEBUG_TONE_SCHED
			vPrintStringAndNumber("  NewBuf ", buf);
		  #endif

		  //Set done flag, buffer is found
		  done = 1;
		  break;
		}
	  }

	  if(!done)
	  {
		  //At this point, no buffer is available
		  //Block / wait until a buffer is available before
		  //the task proceeds

		  xStatus = xQueueReceive( dacResponseHandle, &bufPtrToRelease, portMAX_DELAY);

		  if( xStatus == pdPASS )
		  {
			#ifdef DEBUG_TONE_SCHED
			  vPrintStringAndNumber("  Buf rel addr ", (uint32_t) bufPtrToRelease);
			#endif
			//DMA request completed, release buffer
			ReleaseBuffer(bufPtrToRelease);
		  }
		}
  }
}

void ReleaseBuffer(DAC_Sample *buf_ptr)
{
  uint8_t buf = 0;
  BUF_AVAIL_TYPE buf_found = BUF_NOT_AVAIL;

  for(buf = 0; buf < NUM_TONE_BUFFERS; buf++)
  {
    if(sampleBuf[buf] == buf_ptr)
    {
      bufInUse[buf] = 0;
      buf_found = BUF_AVAIL;
    }
  }

  if(buf_found == BUF_NOT_AVAIL)
  {
    vPrintStringAndNumber( "Unable to free buffer at address ", (uint32_t) buf_ptr);
  }
}

void cb_init(circular_buffer *cb, DAC_Sample *bfr, uint32_t capacity)
{
  cb->buffer_start = &bfr[0];
  cb->buffer_end = &bfr[capacity];
  cb->head = &bfr[0];
  cb->tail = &bfr[0];
  cb->count = 0;
  cb->capacity = capacity;
}

void cb_push_back(DAC_Sample *item, circular_buffer *cb)
{
  // wait for room in buffer to add more values
  while( cb->count >= cb->capacity )
  {
    // vTaskSuspend(NULL);
  };
  // place value in buffer
  *cb->head = *item;
  // bump pointer
  cb->head++;
  // wrap check
  if (cb->head == cb->buffer_end) cb->head = cb->buffer_start;
  cb->count++;
}

void cb_pop_front(circular_buffer *cb, DAC_Sample *item)
{
  *item = *cb->head;
  cb->tail++;
  if (cb->tail == cb->buffer_end) cb->tail = cb->buffer_start;
  cb->count--;
}

//=============================================================================
// dtmfGen() function
//=============================================================================
void dtmfGen(char btn, circular_buffer *cb)
{
  float freqA;
  float freqB;
  unsigned int num_back_fill;
  uint32_t i;
  uint16_t value;
  DAC_Sample sample;
  sample.Bias = 0;
  sample.RESERVED_SET_ZERO = 0;
  sample.RESERVED_SET_ZERO_2 = 0;
  float amp = MAX_AMPLITUDE/4;
  float offset = MAX_AMPLITUDE/2+1;

  switch (btn)
  {
    case '1':
      freqA = ROW_0_FREQ;
      freqB = COL_0_FREQ;
      break;

    case '2':
      freqA = ROW_0_FREQ;
      freqB = COL_1_FREQ;
      break;

    case '3':
      freqA = ROW_0_FREQ;
      freqB = COL_2_FREQ;
      break;

    case 'A':
      freqA = ROW_0_FREQ;
      freqB = COL_3_FREQ;
      break;

    case '4':
      freqA = ROW_1_FREQ;
      freqB = COL_0_FREQ;
      break;

    case '5':
      freqA = ROW_1_FREQ;
      freqB = COL_1_FREQ;
      break;

    case '6':
      freqA = ROW_1_FREQ;
      freqB = COL_2_FREQ;
      break;

    case 'B':
      freqA = ROW_1_FREQ;
      freqB = COL_3_FREQ;
      break;

    case '7':
      freqA = ROW_2_FREQ;
      freqB = COL_0_FREQ;
      break;

    case '8':
      freqA = ROW_2_FREQ;
      freqB = COL_1_FREQ;
      break;

    case '9':
      freqA = ROW_2_FREQ;
      freqB = COL_2_FREQ;
      break;

    case 'C':
      freqA = ROW_2_FREQ;
      freqB = COL_3_FREQ;
      break;

    case '*':
      freqA = ROW_3_FREQ;
      freqB = COL_0_FREQ;
      break;

    case '0':
      freqA = ROW_3_FREQ;
      freqB = COL_1_FREQ;
      break;

    case '#':
      freqA = ROW_3_FREQ;
      freqB = COL_2_FREQ;
      break;

    case 'D':
      freqA = ROW_3_FREQ;
      freqB = COL_3_FREQ;
      break;

    default:
      break;
  }

  // keep filling buffer until end
  for(i=0; i<cb->capacity; i++)
  {
    value = (uint16_t)(offset + sin_aft(amp, 100, t));
    //value = (uint16_t)(offset + sin_aft(amp, freqA, t)+ sin_aft(amp, freqB, t));
    sample.Value = value;
    cb_push_back(&sample, cb);
    t++;
    if (t>DAC_SAMPLE_PER_SECOND) t = 0;
#ifdef DEBUG_TONE_SAMPLE
    {
        vPrintStringAndNumber("  New sample val:  ", value);
    }
#endif
  }
}

#ifdef TONEGEN_DMA_UNIT_TEST
/* Unit Test Task Simulating DMA Request and DMA Transfer */
void vTaskDMAHandlerTest( void *pvParameters )
{
  portBASE_TYPE xStatus;
  DAC_Setup_Message dmaReq;

  for( ;; )
  {
    xStatus = xQueueReceive( xQueueDMARequest, &dmaReq, portMAX_DELAY  );

    vPrintStringAndNumber("  DMA Transfer Starting addr ", (uint32_t) dmaReq.firstSample);

    vTaskDelay(DMA_TIME_MS/portTICK_RATE_MS);

    vPrintStringAndNumber("  DMA Transfer Complete addr ", (uint32_t) dmaReq.firstSample);

    xStatus = xQueueSendToBack( dacResponseHandle, &dmaReq.firstSample, 0 );
  }
}
#endif

#ifdef TONEGEN_INPUT_UNIT_TEST
/* Unit Test Task Generating Tone On/Off */
void vTaskToneRequestTest( void *pvParameters )
{
  char tone = '1';
  char toneToSend = 0;
  uint8_t on_off = TONE_OFF;
    //double delayTick = 500.0/portTICK_RATE_MS;
    portBASE_TYPE xStatus;

  /* As per most tasks, this task is implemented in an infinite loop. */
  for( ;; )
  {
    /* As per most tasks, this task is implemented in an infinite loop. */
    for(tone = '1' ; tone < '9' /*MAX_NUM_TONES*/; tone++)
    {
      for(on_off = TONE_OFF; on_off < MAX_ON_OFF; on_off++)
      {
    	toneToSend = (on_off == TONE_OFF) ? 0 : tone;

        vPrintStringAndNumber("New request Type", toneToSend);

        xStatus = xQueueSendToBack( xQueueToneInput, &toneToSend, 0 );

        if( xStatus != pdPASS )
        {
          vPrintString( "vTaskToneRequestTest:  Could not send to the queue.\r\n" );
        }
        vTaskDelay(TONE_TIME_MS/portTICK_RATE_MS);
      }
    }
  }
}
#endif
