/*
 * keypad.c
 *
 * Authors: Gennadiy Dubilirer & Michal Starosta
 * Class: RTOS
 */

#include "keypad.h"


/*****************************************************************************
 * GPIO Code
 *****************************************************************************/

#define PORT_NUM_IO 2
#define OUTPUT_PIN_MIN 0
#define OUTPUT_PIN_MAX (OUTPUT_PIN_MIN +3)
#define INPUT_PIN_MIN (OUTPUT_PIN_MAX +1)
#define INPUT_PIN_MAX (INPUT_PIN_MIN +3)
#define FALSE 0
#define TRUE 1

#define INPUT 0
#define OUTPUT 1

#define LOW 0
#define HIGH 1

#define PRESSED_SUPPRESS 2
#define PRESSED 1
#define DEPRESSED 0
#define DEBOUNCE_TIME 10
static LPC_GPIO_TypeDef (* const LPC_GPIO[5]) =
{
LPC_GPIO0,
LPC_GPIO1,
LPC_GPIO2,
LPC_GPIO3,
LPC_GPIO4
};

static char symbolDef[4][4] =
{
 '1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D'
};

static int buttonState[4][4] =
{
DEPRESSED, DEPRESSED, DEPRESSED, DEPRESSED,
DEPRESSED, DEPRESSED, DEPRESSED, DEPRESSED,
DEPRESSED, DEPRESSED, DEPRESSED, DEPRESSED,
DEPRESSED, DEPRESSED, DEPRESSED, DEPRESSED
};

/*
 * Declare a variable of type SemaphoreHandle_t. This is used
 * for interrupt gpioInterfaceTask task execution control.
 */

static SemaphoreHandle_t xButtonChangeStateSemaphore;

/* GPIOSetValue - Set pin value */
static void GPIOSetValue(uint32_t portNum, uint32_t bitPosi, uint32_t bitVal);

/* GPIOSetDir - Set pin direction */
static void GPIOSetDir(uint32_t portNum, uint32_t bitPosi, uint32_t dir);

/* GPIOGetValue - Get pin value */
static uint32_t GPIOGetValue(uint32_t portNum, uint32_t bitPosi);

/* GPIOGetValue - Set all out pins */
static void GPIOSetAllOutputPins(uint32_t dir, uint32_t setDir);

/*****************************************************************************
 * External Interrupt Code
 *****************************************************************************/

/*
 * The priority of the software interrupt. The interrupt
 * service routine uses  an (interrupt safe) FreeRTOS API
 * function, so the priority of the interrupt must be
 * equal to or lower than the priority set by
 * configMAX_SYSCALL_INTERRUPT_PRIORITY - remembering that
 * on the Cortex-M3 high  numeric values represent low priority
 * values, which can be confusing as it is counter intuitive.
 *
 */

#define EINT3_INTERRUPT_PRIORITY ( 30 )

/* Int. Handler */
void vSoftwareInterruptHandler(void);

/* Int. Handler */
void EINT3_IRQHandler(void);


void EINT3_IRQHandler(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  NVIC_DisableIRQ(EINT3_IRQn);
  /* 'Give' the semaphore to unblock the task. */
  xSemaphoreGiveFromISR(xButtonChangeStateSemaphore, &xHigherPriorityTaskWoken);

  /* Giving the semaphore may have unblocked a task - if it did and the
  unblocked task has a priority equal to or above the currently executing
  task then xHigherPriorityTaskWoken will have been set to pdTRUE and
  portEND_SWITCHING_ISR() will force a context switch to the newly
  unblocked higher priority task.*/
  portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}


/*****************************************************************************
 * GPIO Code
 *****************************************************************************/

/* GPIOSetValue - Set pin value */
static void GPIOSetValue(uint32_t portNum, uint32_t bitPosi, uint32_t bitVal)
{
  if (bitVal == 0)
  {
    LPC_GPIO[portNum]->FIOCLR = (1 << bitPosi);
  }
  else if (bitVal >= 1)
  {
    LPC_GPIO[portNum]->FIOSET = (1 << bitPosi);
  }
}

/* GPIOSetDir - Set pin direction */
static void GPIOSetDir(uint32_t portNum, uint32_t bitPosi, uint32_t dir)
{
  if (dir)
    LPC_GPIO[portNum]->FIODIR |= 1 << bitPosi;
  else
    LPC_GPIO[portNum]->FIODIR &= ~(1 << bitPosi);
}

/* GPIOGetValue - Get pin value */
static uint32_t GPIOGetValue(uint32_t portNum, uint32_t bitPosi)
{
  uint32_t val;

  LPC_GPIO[portNum]->FIOMASK = ~(1 << bitPosi);
  val = LPC_GPIO[portNum]->FIOPIN;

  val = val >> bitPosi;
  LPC_GPIO[portNum]->FIOMASK = 0x00000000;
  return val;
}

/* GPIOGetValue - Set all out pins */
static void GPIOSetAllOutputPins(uint32_t dir, uint32_t setDir)
{
  uint32_t outputpin;
  for (outputpin = OUTPUT_PIN_MIN; outputpin <= OUTPUT_PIN_MAX; outputpin++)
  {
    if(setDir)
      GPIOSetDir(PORT_NUM_IO, outputpin, OUTPUT);
    GPIOSetValue(PORT_NUM_IO, outputpin, dir);
  }
}

/*
 * GPIOGetValue - Task - process keypad key press/release upon
 * external interrupt.  Send key value pressed, or 0 for release.
 *
 */

void gpioInterfaceTask(void *pvParameters)
{

  xData xSendStructure;

  xSendStructure.ucSource = KEYPAD_TASK;
  xSendStructure.ucValue = 0;
#if DEBUG_KEYPAD
  static int intCounter = 0;
#endif
  portBASE_TYPE xStatus;

  TickType_t xSleepTime = 0;
  xSleepTime = 100 / portTICK_RATE_MS;
  TickType_t xDebounceTime = DEBOUNCE_TIME / portTICK_RATE_MS;

  uint32_t var;
  uint32_t outputpin, outputpin2, inputpin, inputpin2;
  char string[256];
  int buttonXPressedIndex = 0xFF;
  int buttonYPressedIndex = 0xFF;

  xQueueHandle * xKeypadQueue = (xQueueHandle*) pvParameters;

  vSemaphoreCreateBinary(xButtonChangeStateSemaphore);

  xSemaphoreTake(xButtonChangeStateSemaphore, portMAX_DELAY);

  /* Disable the interrupt while we configure input pins. */
  NVIC_DisableIRQ(EINT3_IRQn);
  GPIOSetAllOutputPins(LOW,TRUE);

  for (inputpin = INPUT_PIN_MIN; inputpin <= INPUT_PIN_MAX; inputpin++)
  {
    GPIOSetDir(PORT_NUM_IO, inputpin, INPUT);
    // Enable P2.X interrupts for input pins rising and falling edge
    // Also clear interrupts which were set previously
    LPC_GPIOINT->IO2IntClr |= (1 << inputpin);
    LPC_GPIOINT->IO2IntEnF |= (1 << inputpin);
    LPC_GPIOINT->IO2IntEnR &= ~(1 << inputpin);
  }

  /* The interrupt service routine uses an (interrupt safe) FreeRTOS API
  function so the interrupt priority must be at or below the priority
  defined by configSYSCALL_INTERRUPT_PRIORITY. */
  NVIC_SetPriority(EINT3_IRQn, EINT3_INTERRUPT_PRIORITY);

  /* Enable the interrupt. */
  NVIC_EnableIRQ(EINT3_IRQn);

  while (1)
  {
    xSemaphoreTake(xButtonChangeStateSemaphore, portMAX_DELAY);
    GPIOSetAllOutputPins(HIGH, FALSE);
#if DEBUG_KEYPAD
    intCounter++;
#endif
    for (outputpin = OUTPUT_PIN_MIN; outputpin <= OUTPUT_PIN_MAX; outputpin++)
    {
      GPIOSetValue(PORT_NUM_IO, outputpin, LOW);
      // Debounce wait to ensure the input pin stays at the same level
      vTaskDelay(xDebounceTime);
      for (inputpin = INPUT_PIN_MIN; inputpin <= INPUT_PIN_MAX; inputpin++)
      {
        LPC_GPIOINT->IO2IntClr |= (1 << inputpin);

        var = GPIOGetValue(PORT_NUM_IO, inputpin); // read the button value

        if ((var == 0) && (GPIOGetValue(PORT_NUM_IO, inputpin) == 0))
        // the button is pushed
        {
          if ((buttonXPressedIndex == 0xFF) && (buttonState[outputpin - OUTPUT_PIN_MIN][inputpin - INPUT_PIN_MIN] == DEPRESSED))
          {
            xSendStructure.ucValue = symbolDef[outputpin - OUTPUT_PIN_MIN][inputpin - INPUT_PIN_MIN];

#if DEBUG_KEYPAD
            sprintf(string,"Pressed %c @ column %d , row =%d, intCounter= %d \n", symbolDef[outputpin - OUTPUT_PIN_MIN][inputpin - INPUT_PIN_MIN],(inputpin - INPUT_PIN_MIN), (outputpin - OUTPUT_PIN_MIN), intCounter);
            vPrintString(string);
#endif
            xStatus = xQueueSendToBack(*xKeypadQueue, (char*)&xSendStructure.ucValue, xSleepTime);
            buttonState[outputpin - OUTPUT_PIN_MIN][inputpin - INPUT_PIN_MIN] = PRESSED;
            buttonXPressedIndex = outputpin - OUTPUT_PIN_MIN;
            buttonYPressedIndex = inputpin - INPUT_PIN_MIN;
            // Disable falling edge & enable rising edge
            // interrupt for this input pin
            LPC_GPIOINT->IO2IntEnF &= ~(1 << inputpin);
            LPC_GPIOINT->IO2IntEnR |= (1 << inputpin);
          }
        }
        else if (((outputpin - OUTPUT_PIN_MIN) == buttonXPressedIndex) && ((inputpin - INPUT_PIN_MIN) == buttonYPressedIndex) &&
                  ((buttonState[outputpin - OUTPUT_PIN_MIN][inputpin - INPUT_PIN_MIN] == PRESSED)))
        {
          // if another pin is pressed in the same column then drive all
          // output pins low to ensure that no other button in the same column
          // is pressed.
          int suppressedButtons = FALSE;
          GPIOSetAllOutputPins(LOW, FALSE);
          vTaskDelay(xDebounceTime);
          if(GPIOGetValue(PORT_NUM_IO, inputpin) == 1)
          {
#if DEBUG_KEYPAD
            sprintf(string,"Depressed %c @ column %d , row =%d, intCounter= %d\n", symbolDef[outputpin - OUTPUT_PIN_MIN][inputpin - INPUT_PIN_MIN], (inputpin - INPUT_PIN_MIN), (outputpin - OUTPUT_PIN_MIN), intCounter);
            vPrintString(string);
#endif
            if(buttonState[outputpin - OUTPUT_PIN_MIN][inputpin - INPUT_PIN_MIN] == PRESSED)
            {
              // Now check if any other pin is pressed
              GPIOSetAllOutputPins(HIGH, FALSE);
              for (outputpin2 = OUTPUT_PIN_MIN; outputpin2 <= OUTPUT_PIN_MAX; outputpin2++)
              {
                GPIOSetValue(PORT_NUM_IO, outputpin2, LOW);
                for (inputpin2 = INPUT_PIN_MIN; inputpin2 <= INPUT_PIN_MAX; inputpin2++)
                {
                  if (GPIOGetValue(PORT_NUM_IO, inputpin2) == 0)
                  {
                    // the button is pushed
#if DEBUG_KEYPAD
                    sprintf(string,"Suppress Pressed %c @ column %d , row =%d\n", symbolDef[outputpin2 - OUTPUT_PIN_MIN][inputpin2 - INPUT_PIN_MIN],(inputpin2 - INPUT_PIN_MIN), (outputpin2 - OUTPUT_PIN_MIN));
                    vPrintString(string);
#endif
                    buttonState[outputpin2 - OUTPUT_PIN_MIN][inputpin2 - INPUT_PIN_MIN] = PRESSED_SUPPRESS;
                    suppressedButtons = TRUE;
                  }
                }
              }
              // No other button is pressed
              if(suppressedButtons == FALSE)
              {
                buttonXPressedIndex = 0xFF;
                buttonYPressedIndex = 0xFF;
                buttonState[outputpin - OUTPUT_PIN_MIN][inputpin - INPUT_PIN_MIN] = DEPRESSED;
                xSendStructure.ucValue = 0;
                xStatus = xQueueSendToBack(*xKeypadQueue, (char*)&xSendStructure.ucValue, xSleepTime);
                // Re-enable falling edge disable rising edge interrupt for this pin
                LPC_GPIOINT->IO2IntEnF |= (1 << inputpin);
                LPC_GPIOINT->IO2IntEnR &= ~(1 << inputpin);
              }
            }

          }
          GPIOSetAllOutputPins(HIGH, FALSE);
        }
        else if (buttonState[outputpin - OUTPUT_PIN_MIN][inputpin - INPUT_PIN_MIN] == PRESSED_SUPPRESS)
        {
          // button is no longer pressed suppressed
          buttonState[outputpin - OUTPUT_PIN_MIN][inputpin - INPUT_PIN_MIN] = DEPRESSED;
        }
      }
      GPIOSetValue(PORT_NUM_IO, outputpin, HIGH);
    }

    GPIOSetAllOutputPins(LOW, FALSE);
    for (inputpin = INPUT_PIN_MIN; inputpin <= INPUT_PIN_MAX; inputpin++)
    {
      // Also clear interrupts which were set previously
      LPC_GPIOINT->IO2IntClr |= (1 << inputpin);
    }
    NVIC_ClearPendingIRQ( EINT3_IRQn);
    // Reenable IRQ
    NVIC_EnableIRQ(EINT3_IRQn);
	taskYIELD();
  }
}

