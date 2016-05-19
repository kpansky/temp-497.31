#include <dac.h>
#include "semphr.h"

/*
 * This structure exactly matches the linked list structure for the chip
 */
typedef struct DMA_LinkedList
{
  uint32_t Src;
  uint32_t Destination;
  uint32_t NextLinkedList;
  uint32_t control;

} DMA_LinkedList;

/*
 * Semaphore used for deferred interrupt processing
 */
SemaphoreHandle_t xDacDmaSem = NULL;

#if defined(DAC_RESPONSE_QUEUE)
extern SemaphoreHandle_t DAC_RESPONSE_QUEUE;
#endif

/*
 * Sets up DMA resources on channel 0
 */
void
InitializeDMA ()
{
  LPC_SC->PCONP |= 0x20000000;
  LPC_GPDMA->DMACConfig = 1; /* Enable */
  LPC_GPDMACH0->DMACCConfig = 0;
  NVIC_SetPriority (DMA_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY);
  NVIC_EnableIRQ (DMA_IRQn);

  xDacDmaSem = xSemaphoreCreateBinary();
}

/*
 * Handles interrupt by deferring processing to task
 */
void
DMA_IRQHandler (void)
{
  portBASE_TYPE rerunScheduler = pdFALSE;
  if (LPC_GPDMA->DMACIntTCStat & 1)
    {
      LPC_GPDMA->DMACIntTCClear = 1;
      if (xDacDmaSem)
	{
	  xSemaphoreGiveFromISR(xDacDmaSem, &rerunScheduler);
	}

    }

  NVIC_ClearPendingIRQ (DMA_IRQn);

  portEND_SWITCHING_ISR(rerunScheduler);
}

/*
 * Sets DAC up (i/o pins, clock rate)
 */
void
InitializeDAC ()
{

  /*DAC is bit 22 and 23*/
  /* Clk = main clock/2, -> 50MHz tick*/
  LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & 0xFF3FFFFF) | 0x00800000;

  /* DAC is pinsel bits 21 & 20. Needs to be 0b10 to Select P0.26*/
  LPC_PINCON->PINSEL1 = (LPC_PINCON->PINSEL1 & 0xFFCFFFFF);
  LPC_PINCON->PINSEL1 |= ((0b10) << 20);

#if defined(DAC_SAMPLE_PER_SECOND)
  uint32_t period = TICKS_PER_SECOND_DAC / DAC_SAMPLE_PER_SECOND;

  LPC_DAC->DACCNTVAL = period;
#endif

  LPC_PINCON->PINMODE1 = (LPC_PINCON->PINMODE1 & 0xFFCFFFFF);
  LPC_PINCON->PINMODE1 = (LPC_PINCON->PINMODE1 | ((0b10) << 20));
}

/*
 * Main task
 * Takes as input the input message queue
 *
 * Pends on message waiting for new set of samples to send out to the DAC
 * Once receives message, sets up a transfer and then pends on a semaphore
 * When the interrupt fires and releases the semaphore, task resumes, sends
 * a message back to the originating task informing it finsihed processing.
 * Task then goes back to pending on the message queue.
 *
 * Optionally:
 *   If certain compile settings are set, it can adjust the DAC rate to match
 *   input request by controlling the DAC counter
 *   It can send a "finished" message to a desired queue in case more than one
 *   task feeding the ISR
 *   It can loop the DAC output to play for loner duration than the samples,
 *   allocating an appropriate linked list with an interrupt on total completion
 */
void
DAC_Handler (void* queue)
{
#if 0
    {
      DAC_Sample testCode;
      testCode.RESERVED_SET_ZERO=0;
      testCode.Value=1;
      testCode.Bias=0;
      testCode.RESERVED_SET_ZERO_2=0;

      if(0x40 != *((uint32_t*)&testCode))
	{
	  printf("Bad layout DAC sample\n");
	}
      testCode.Value=0xffff;
    }
#endif

  xQueueHandle inboundQueue = (xQueueHandle) queue;
  DAC_Setup_Message message;
  for (;;)
    {
      if (pdFAIL == xQueueReceive(inboundQueue, &message, portMAX_DELAY))
	{
	  continue;
	}
      LPC_DAC->DACCTRL = 0;

#if !defined(DAC_SAMPLE_PER_SECOND)
      uint32_t period = TICKS_PER_SECOND_DAC/message.samplesPerSecond;

      LPC_DAC->DACCNTVAL = period;
#endif

#if defined(ALLOW_LOOPING_DAC_DMA)
      uint32_t numberCompleteCycles = floor((message.samplesPerSecond*message.playTimeSeconds)/message.numberSamples);
      uint32_t samplesExtraPass = (message.samplesPerSecond*message.playTimeSeconds)-(numberCompleteCycles*message.numberSamples);
      uint32_t dmaListToAllocate = numberCompleteCycles;
      if(samplesExtraPass)
	{
	  ++dmaListToAllocate;
	}

      DMA_LinkedList* linkedList = calloc(dmaListToAllocate,sizeof(DMA_LinkedList));
      for(uint32_t ii=0;ii<dmaListToAllocate;++ii)
	{
	  linkedList[ii].Src = (uint32_t)message.firstSample;
	  linkedList[ii].Destination = (uint32_t)&LPC_DAC->DACR;
	  if(ii != (dmaListToAllocate-1))
	    {
	      linkedList[ii].NextLinkedList = (uint32_t)&linkedList[ii+1].NextLinkedList;
	    }
	  /*
	   * Burst size 1 word | Burst size 1 word | width 4 bytes | width 4 bytes | increment source | do not increment dest
	   */
	  linkedList[ii].control = message.numberSamples|(0<<12)|(0<<15)|(0x2<<18)|(0x2<<21)|(1<<26)|(0<<27);
	}
      uint32_t samplesLastPass = (samplesExtraPass>0)?samplesExtraPass:message.numberSamples;
      /*
       * Add in interrupt on completion
       */
      linkedList[dmaListToAllocate-1].control = samplesLastPass|(0<<12)|(0<<15)|(0x2<<18)|(0x2<<21)|(1<<26)|(0<<27)|(1<<31);
      /* Program DMA controller (to copy of first entry)*/
      LPC_GPDMACH0->DMACCControl = linkedList[0].control;
      LPC_GPDMACH0->DMACCSrcAddr = linkedList[0].Src;
      LPC_GPDMACH0->DMACCDestAddr = linkedList[0].Destination;
      LPC_GPDMACH0->DMACCLLI = linkedList[0].NextLinkedList;

#else
      /*
       * Burst size 1 word | Burst size 1 word | width 4 bytes | width 4 bytes | increment source | do not increment dest | interrupt on complete
       */
      LPC_GPDMACH0->DMACCControl = message.numberSamples | (0 << 12) | (0 << 15)
	  | (0x2 << 18) | (0x2 << 21) | (1 << 26) | (0 << 27) | (1 << 31);
      LPC_GPDMACH0->DMACCSrcAddr = (uint32_t) message.firstSample;
      LPC_GPDMACH0->DMACCDestAddr = (uint32_t) &LPC_DAC->DACR;
      LPC_GPDMACH0->DMACCLLI = 0;
#endif

      /*
       * Enable | DAC destination | memory to peripheral | terminal interrupt
       */
      LPC_GPDMACH0->DMACCConfig = 0x1 | (7 << 6) | (1 << 11) | (1 << 15);

      /* Start the DAC/DMA*/
      LPC_DAC->DACCTRL = 1 << 3 | 1 << 2;

      xSemaphoreTake(xDacDmaSem, portMAX_DELAY);
#if defined(ALLOW_LOOPING_DAC_DMA)
      free(linkedList);
      linkedList = NULL;
#endif

#if !defined(DAC_RESPONSE_QUEUE)
      if(message.completionResponseQueue)
#else
#endif
	{
	  DAC_Complete_Message complete;
	  complete.firstSample = message.firstSample;
	  complete.numberSamples = message.numberSamples;

#if !defined(DAC_RESPONSE_QUEUE)
	  xQueueSend(message.completionResponseQueue,&complete,portMAX_DELAY);
#else
	  xQueueSend(DAC_RESPONSE_QUEUE, &complete, portMAX_DELAY);
#endif

	}

    }
}
