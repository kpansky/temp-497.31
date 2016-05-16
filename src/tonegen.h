#ifndef TONEGEN_H
#define TONEGEN_H

#include "main.h"
//#include "dma.h"
#include "dac.h"

#define MAX_AMPLITUDE       1023    //
#define SILENCE_LENGTH      (.1*DAC_SAMPLE_PER_SECOND) // silence gen after each tone in samples
#define TWO_PI (3.14159 * 2)

/* Buffer Definitions */
#define NUM_TONE_BUFFERS 3           //CONFIGURABLE - Number of Tone Buffer Sizes
#define TONE_BUFFER_SIZE 0x100       //CONFIGURABLE - Tone Buffer Size

/* Queue Sizes */
#define DTMF_REQ_QUEUE_SIZE     10
#define DMA_REQ_QUEUE_SIZE      NUM_TONE_BUFFERS
#define DMA_COMP_QUEUE_SIZE     NUM_TONE_BUFFERS

/* Sample Timing Parameters */
#define SEC_TO_MSEC             1000
//#define SAMPLE_SET_DELAY_MS   SEC_TO_MSEC * TONE_BUFFER_SAMPLE_SET / DAC_SAMPLE_PER_SECOND
#define TONE_BUFFER_DELAY_MS    SEC_TO_MSEC * TONE_BUFFER_SIZE / DAC_SAMPLE_PER_SECOND

/* Unit Test Parameters */
#define DMA_TIME_MS				SEC_TO_MSEC * TONE_BUFFER_SIZE / DAC_SAMPLE_PER_SECOND
#define TONE_TIME_MS            SEC_TO_MSEC * TONE_PERIODS * TONE_BUFFER_SIZE / DAC_SAMPLE_PER_SECOND
#define TONE_PERIODS            500  //CONFIGURABLE - Number of Times Tone Buffer is filled before sending new tone

/////////////////////////////////////////////////
// Tone Table
// 	      1209 Hz 	1336 Hz 	1477 Hz 	1633 Hz
//697 Hz 	1 	      2 	      3 	      A
//770 Hz 	4 	      5 	      6 	      B
//852 Hz 	7 	      8 	      9 	      C
//941 Hz 	* 	      0 	      # 	      D


#define ROW_0_FREQ  697
#define ROW_1_FREQ  770
#define ROW_2_FREQ  852
#define ROW_3_FREQ  941

#define COL_0_FREQ  1209
#define COL_1_FREQ  1336
#define COL_2_FREQ  1477
#define COL_3_FREQ  1633

typedef enum
{
	BUF_NOT_AVAIL,
	BUF_AVAIL
} BUF_AVAIL_TYPE;

typedef enum
{
	TONE_OFF,
	TONE_ON,
	MAX_ON_OFF
} TONE_ON_OFF_TYPE;

typedef struct
{
  DAC_Sample *buffer_start; // start of data buffer
  DAC_Sample *buffer_end;   // end of data buffer
  DAC_Sample *head;         // pointer to head
  DAC_Sample *tail;         // pointer to tail
  uint32_t capacity;      // maximum number of items in the buffer
  uint32_t count;         // number of items in the buffer
} circular_buffer;

void dtmfGen(char btn, circular_buffer *cb);
void cb_init(circular_buffer *cb, DAC_Sample *bfr, uint32_t capacity);
void cb_free(circular_buffer *cb);
void cb_push_back(DAC_Sample *item, circular_buffer *cb);
void cb_pop_front(circular_buffer *cb, DAC_Sample *item);
void FillBuffer(char btn, uint8_t *activeBuffer);
void SendSamplesToDMA(uint8_t *activeBuffer);
void GetNextAvailableBuffer(uint8_t *activeBuffer);
void ReleaseBuffer(DAC_Sample *buf_ptr);
/* Tone Generator Tasks */
void vTaskToneGenerator( void *pvParameters );
/* Unit Test Tasks */
void vTaskToneRequestTest( void *pvParameters );
void vTaskDMAHandlerTest( void *pvParameters );

#endif

