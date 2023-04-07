/*
 * terminal_helper.c
 *
 * controller specific implementations
 *
 *  Created on: 30.12.2022
 *      Author: Martin
 */

#include "terminal_helper.h"

char dbgbuffer[256];

uint8_t aTXBuffer[TX_BUFFER_SIZE];

uint8_t aRXBufferUser[RX_BUFFER_SIZE];

/**
  * @brief Data buffers used to manage received data in interrupt routine
  */
uint8_t aRXBufferA[RX_BUFFER_SIZE];
uint8_t aRXBufferB[RX_BUFFER_SIZE];

__IO uint32_t     uwNbReceivedChars;
uint8_t *pBufferReadyForUser;
uint8_t *pBufferReadyForReception;

UART_HandleTypeDef *huart_terminal;

void Init_Terminal(UART_HandleTypeDef *handle_huart)
{
	huart_terminal = handle_huart;
    CLI_Init(TDC_Time);
	SettingsCommands_Init();
	TUSART_StartReception();
};


void _reset_fcn()
{
	NVIC_SystemReset();
}

// TODO add FIFO to aggregate char before sending and avoid waiting for empty buffer.

inline void TUSART_PutChar(char c)
{
	// check if previous transmission is ongoing
	while(huart_terminal->gState != HAL_UART_STATE_READY){}
	// copy content into reserved memory
	memcpy((uint8_t*)&aTXBuffer, (uint8_t *)&c, 1);
	while(HAL_BUSY == HAL_UART_Transmit_DMA(huart_terminal, (uint8_t*)&aTXBuffer, 1)) {}
}

// TODO add FIFO to aggregate char before sending and avoid waiting for empty buffer.
void TUSART_Print(const char* str)
{
	//determine size and limit to buffer size
	unsigned int length = min(strlen(str), TX_BUFFER_SIZE);
	// check if previous transmission is ongoing
	while(huart_terminal->gState != HAL_UART_STATE_READY){}
	// copy content into reserved memory
	memcpy((uint8_t*)&aTXBuffer, str, length);
	while(HAL_BUSY == HAL_UART_Transmit_DMA(huart_terminal, (uint8_t*)&aTXBuffer, length)){}
}

/**
  * @brief  This function initiates RX transfer
  * @retval None
  */
void TUSART_StartReception(void)
{
	/* Initializes Buffer swap mechanism (used in User callback) :
	   - 2 physical buffers aRXBufferA and aRXBufferB (RX_BUFFER_SIZE length)
	*/
	pBufferReadyForReception = aRXBufferA;
	pBufferReadyForUser      = aRXBufferB;
	uwNbReceivedChars        = 0;

   /* Initializes Rx sequence using Reception To Idle event API.
     As DMA channel associated to UART Rx is configured as Circular,
     reception is endless.
     If reception has to be stopped, call to HAL_UART_AbortReceive() could be used.
     Use of HAL_UARTEx_ReceiveToIdle_DMA service, will generate calls to
     user defined HAL_UARTEx_RxEventCallback callback for each occurrence of
     following events :
     - DMA RX Half Transfer event (HT)
     - DMA RX Transfer Complete event (TC)
     - IDLE event on UART Rx line (indicating a pause is UART reception flow)
     => make sure to enable both DMA and UART interrupts
  */
  HAL_UARTEx_ReceiveToIdle_DMA(huart_terminal, aRXBufferUser, RX_BUFFER_SIZE);
}


/**
  * @brief  This function handles buffer containing received data
  * @note   This routine is executed in Interrupt context.
  * @param  huart UART handle.
  * @param  pData Pointer on received data buffer to be processed
  * @retval Size  Nb of received characters available in buffer
  */
void TUSART_ProcessInput(uint8_t* pData, uint16_t Size)
{
  /*
   * This function might be called in any of the following interrupt contexts :
   *  - DMA TC and HT events
   *  - UART IDLE line event
   *
   * pData and Size defines the buffer where received data have been copied, in order to be processed.
   * During this processing of already received data, reception is still ongoing.
   *
   */
  uint8_t* pBuff = pData;
  uint8_t  i;
  for (i = 0; i < Size; i++)
  {
	char c = (char)*pBuff;
	CLI_EnterChar(c);
    pBuff++;
  }

}

void TUSART_UARTEx_RxEventCallback(uint16_t Size)
{
  static uint8_t old_pos = 0;
  uint8_t *ptemp;
  uint8_t i;

  /* Check if number of received data in reception buffer has changed */
  if (Size != old_pos)
  {
    /* Check if position of index in reception buffer has simply be increased
       of if end of buffer has been reached */
    if (Size > old_pos)
    {
      /* Current position is higher than previous one */
      uwNbReceivedChars = Size - old_pos;
      /* Copy received data in "User" buffer for evacuation */
      for (i = 0; i < uwNbReceivedChars; i++)
      {
        pBufferReadyForUser[i] = aRXBufferUser[old_pos + i];
      }
    }
    else
    {
      /* Current position is lower than previous one : end of buffer has been reached */
      /* First copy data from current position till end of buffer */
      uwNbReceivedChars = RX_BUFFER_SIZE - old_pos;
      /* Copy received data in "User" buffer for evacuation */
      for (i = 0; i < uwNbReceivedChars; i++)
      {
        pBufferReadyForUser[i] = aRXBufferUser[old_pos + i];
      }
      /* Check and continue with beginning of buffer */
      if (Size > 0)
      {
        for (i = 0; i < Size; i++)
        {
          pBufferReadyForUser[uwNbReceivedChars + i] = aRXBufferUser[i];
        }
        uwNbReceivedChars += Size;
      }
    }
    /* Process received data that has been extracted from Rx User buffer */
    TUSART_ProcessInput(pBufferReadyForUser, uwNbReceivedChars);

    /* Swap buffers for next bytes to be processed */
    ptemp = pBufferReadyForUser;
    pBufferReadyForUser = pBufferReadyForReception;
    pBufferReadyForReception = ptemp;
  }
  /* Update old_pos as new reference of position in User Rx buffer that
     indicates position to which data have been processed */
  old_pos = Size;
}
