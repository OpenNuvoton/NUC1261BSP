/**************************************************************************//**
 * @file     sc.h
 * @version  V3.00
 * $Revision: 6 $
 * $Date: 16/10/25 4:25p $
 * @brief    NUC1261 series Smart Card(SC) driver header file
 *
 * @note
 *
 * @copyright SPDX-License-Identifier: Apache-2.0
 * @copyright Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __SC_H__
#define __SC_H__

#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup SC_Driver SC Driver
  @{
*/

/** @addtogroup SC_EXPORTED_CONSTANTS SC Exported Constants
  @{
*/
#define SC_INTERFACE_NUM            2                /*!< Smartcard interface numbers */
#define SC_PIN_STATE_HIGH           1                /*!< Smartcard pin status high   */
#define SC_PIN_STATE_LOW            0                /*!< Smartcard pin status low    */
#define SC_PIN_STATE_IGNORE         0xFFFFFFFF       /*!< Ignore pin status           */
#define SC_CLK_ON                   1                /*!< Smartcard clock on          */
#define SC_CLK_OFF                  0                /*!< Smartcard clock off         */

#define SC_TMR_MODE_0               (0ul << SC_TMRCTL0_OPMODE_Pos)     /*!< Timer Operation Mode 0, down count                                                      */
#define SC_TMR_MODE_1               (1ul << SC_TMRCTL0_OPMODE_Pos)     /*!< Timer Operation Mode 1, down count, start after detect start bit                        */
#define SC_TMR_MODE_2               (2ul << SC_TMRCTL0_OPMODE_Pos)     /*!< Timer Operation Mode 2, down count, start after receive start bit                       */
#define SC_TMR_MODE_3               (3ul << SC_TMRCTL0_OPMODE_Pos)     /*!< Timer Operation Mode 3, down count, use for activation, only timer 0 support this mode  */
#define SC_TMR_MODE_4               (4ul << SC_TMRCTL0_OPMODE_Pos)     /*!< Timer Operation Mode 4, down count with reload after timeout                            */
#define SC_TMR_MODE_5               (5ul << SC_TMRCTL0_OPMODE_Pos)     /*!< Timer Operation Mode 5, down count, start after detect start bit, reload after timeout  */
#define SC_TMR_MODE_6               (6ul << SC_TMRCTL0_OPMODE_Pos)     /*!< Timer Operation Mode 6, down count, start after receive start bit, reload after timeout */
#define SC_TMR_MODE_7               (7ul << SC_TMRCTL0_OPMODE_Pos)     /*!< Timer Operation Mode 7, down count, start and reload after detect start bit             */
#define SC_TMR_MODE_8               (8ul << SC_TMRCTL0_OPMODE_Pos)     /*!< Timer Operation Mode 8, up count                                                        */
#define SC_TMR_MODE_F               (0xF << SC_TMRCTL0_OPMODE_Pos)     /*!< Timer Operation Mode 15, down count, reload after detect start bit                      */

#define SC_TIMEOUT                  (SystemCoreClock)   /*!< SC time-out counter (1 second time-out) */

/*@}*/ /* end of group SC_EXPORTED_CONSTANTS */


/** @addtogroup SC_EXPORTED_FUNCTIONS SC Exported Functions
  @{
*/

/**
  * @brief      Enable Smartcard Interrupt Function
  *
  * @param[in]  sc      The pointer of smartcard module.
  * @param[in]  u32Mask Interrupt mask to be enabled. A combination of
  *                         - \ref SC_INTEN_ACERRIEN_Msk
  *                         - \ref SC_INTEN_RXTOIEN_Msk
  *                         - \ref SC_INTEN_INITIEN_Msk
  *                         - \ref SC_INTEN_CDIEN_Msk
  *                         - \ref SC_INTEN_BGTIEN_Msk
  *                         - \ref SC_INTEN_TMR2IEN_Msk
  *                         - \ref SC_INTEN_TMR1IEN_Msk
  *                         - \ref SC_INTEN_TMR0IEN_Msk
  *                         - \ref SC_INTEN_TERRIEN_Msk
  *                         - \ref SC_INTEN_TXEIEN_Msk
  *                         - \ref SC_INTEN_RDAIEN_Msk
  *
  * @return     None
  *
  * @details    The macro is used to enable Auto-convention error interrupt, Receiver buffer time-out interrupt, Initial end interrupt,
  *             Card detect interrupt, Block guard time interrupt, Timer2 interrupt, Timer1 interrupt, Timer0 interrupt,
  *             Transfer error interrupt, Transmit buffer empty interrupt or Receive data reach trigger level interrupt.
  * \hideinitializer
  */
#define SC_ENABLE_INT(sc, u32Mask)      ((sc)->INTEN |= (u32Mask))

/**
  * @brief      Disable Smartcard Interrupt Function
  *
  * @param[in]  sc      The pointer of smartcard module.
  * @param[in]  u32Mask Interrupt mask to be disabled. A combination of
  *                         - \ref SC_INTEN_ACERRIEN_Msk
  *                         - \ref SC_INTEN_RXTOIEN_Msk
  *                         - \ref SC_INTEN_INITIEN_Msk
  *                         - \ref SC_INTEN_CDIEN_Msk
  *                         - \ref SC_INTEN_BGTIEN_Msk
  *                         - \ref SC_INTEN_TMR2IEN_Msk
  *                         - \ref SC_INTEN_TMR1IEN_Msk
  *                         - \ref SC_INTEN_TMR0IEN_Msk
  *                         - \ref SC_INTEN_TERRIEN_Msk
  *                         - \ref SC_INTEN_TXEIEN_Msk
  *                         - \ref SC_INTEN_RDAIEN_Msk
  *
  * @return     None
  *
  * @details    The macro is used to disable Auto-convention error interrupt, Receiver buffer time-out interrupt, Initial end interrupt,
  *             Card detect interrupt, Block guard time interrupt, Timer2 interrupt, Timer1 interrupt, Timer0 interrupt,
  *             Transfer error interrupt, Transmit buffer empty interrupt or Receive data reach trigger level interrupt.
  * \hideinitializer
  */
#define SC_DISABLE_INT(sc, u32Mask)     ((sc)->INTEN &= ~(u32Mask))

/**
  * @brief      Set Smartcard Power Pin State
  *
  * @param[in]  sc          The pointer of smartcard module.
  * @param[in]  u32State    Set pin state of power pin, valid parameters are:
  *                             - \ref SC_PIN_STATE_HIGH
  *                             - \ref SC_PIN_STATE_LOW
  *
  * @return     None
  *
  * @details    User can set PWREN (SC_PINCTL[0]) and PWRINV (SC_PINCTL[11]) to decide SC_PWR pin is in high or low level.
  * \hideinitializer
  */
#define SC_SET_VCC_PIN(sc, u32State) \
    do {\
            uint32_t u32TimeOutCnt = SC_TIMEOUT;\
            while((sc)->PINCTL & SC_PINCTL_SYNC_Msk)\
                if(--u32TimeOutCnt == 0) break;\
            if(u32State)\
                (sc)->PINCTL |= SC_PINCTL_PWREN_Msk;\
            else\
                (sc)->PINCTL &= ~SC_PINCTL_PWREN_Msk;\
    }while(0)


/**
  * @brief      Set Smartcard Clock Status
  *
  * @param[in]  sc          The pointer of smartcard module.
  * @param[in]  u32OnOff    Set clock on or off for selected smartcard module, valid values are:
  *                             - \ref SC_CLK_ON
  *                             - \ref SC_CLK_OFF
  *
  * @return     None
  *
  * @details    User can set CLKKEEP (SC_PINCTL[6]) to decide SC_CLK pin always keeps free running or not.
  * \hideinitializer
  */
#define SC_SET_CLK_PIN(sc, u32OnOff)\
    do {\
            uint32_t u32TimeOutCnt = SC_TIMEOUT;\
            while((sc)->PINCTL & SC_PINCTL_SYNC_Msk)\
                if(--u32TimeOutCnt == 0) break;\
            if(u32OnOff)\
                (sc)->PINCTL |= SC_PINCTL_CLKKEEP_Msk;\
            else\
                (sc)->PINCTL &= ~(SC_PINCTL_CLKKEEP_Msk);\
    }while(0)

/**
  * @brief      Set Smartcard I/O Pin State
  *
  * @param[in]  sc          The pointer of smartcard module.
  * @param[in]  u32State    Set pin state of I/O pin, valid parameters are:
  *                             - \ref SC_PIN_STATE_HIGH
  *                             - \ref SC_PIN_STATE_LOW
  *
  * @return     None
  *
  * @details    User can set SCDATA (SC_PINCTL[9]) to decide SC_DATA pin to high or low.
  * \hideinitializer
  */
#define SC_SET_IO_PIN(sc, u32State)\
    do {\
            uint32_t u32TimeOutCnt = SC_TIMEOUT;\
            while((sc)->PINCTL & SC_PINCTL_SYNC_Msk)\
                if(--u32TimeOutCnt == 0) break;\
            if(u32State)\
                (sc)->PINCTL |= SC_PINCTL_SCDATA_Msk;\
            else\
                (sc)->PINCTL &= ~SC_PINCTL_SCDATA_Msk;\
    }while(0)

/**
  * @brief      Set Smartcard Reset Pin State
  *
  * @param[in]  sc          The pointer of smartcard module.
  * @param[in]  u32State    Set pin state of reset pin, valid parameters are:
  *                             - \ref SC_PIN_STATE_HIGH
  *                             - \ref SC_PIN_STATE_LOW
  *
  * @return     None
  *
  * @details    User can set SCRST (SC_PINCTL[1]) to decide SC_RST pin to high or low.
  * \hideinitializer
  */
#define SC_SET_RST_PIN(sc, u32State)\
    do {\
            uint32_t u32TimeOutCnt = SC_TIMEOUT;\
            while((sc)->PINCTL & SC_PINCTL_SYNC_Msk)\
                if(--u32TimeOutCnt == 0) break;\
            if(u32State)\
                (sc)->PINCTL |= SC_PINCTL_SCRST_Msk;\
            else\
                (sc)->PINCTL &= ~SC_PINCTL_SCRST_Msk;\
    }while(0)

/**
  * @brief      Read One Byte Data
  *
  * @param[in]  sc      The pointer of smartcard module.
  *
  * @return     One byte read from receive FIFO
  *
  * @details    By reading DAT register, the SC will return an 8-bit received data.
  * \hideinitializer
  */
#define SC_READ(sc)             ((char)((sc)->DAT))

/**
  * @brief      Write One Byte Data
  *
  * @param[in]  sc      The pointer of smartcard module.
  * @param[in]  u8Data  Data to write to transmit FIFO.
  *
  * @return     None
  *
  * @details    By writing data to DAT register, the SC will send out an 8-bit data.
  * \hideinitializer
  */
#define SC_WRITE(sc, u8Data)    ((sc)->DAT = (u8Data))

/**
  * @brief      Set Smartcard Stop Bit Length
  *
  * @param[in]  sc      The pointer of smartcard module.
  * @param[in]  u32Len  Stop bit length, ether 1 or 2.
  *
  * @return     None
  *
  * @details    Stop bit length must be 1 for T = 1 protocol and 2 for T = 0 protocol.
  * \hideinitializer
  */
#define SC_SET_STOP_BIT_LEN(sc, u32Len) ((sc)->CTL = ((sc)->CTL & ~SC_CTL_NSB_Msk) | (((u32Len) == 1)? SC_CTL_NSB_Msk : 0))

/**
  * @brief      Set Tx Error Retry Count
  *
  * @param[in]  sc          The pointer of smartcard module.
  * @param[in]  u32Count    The number of Tx error retry count, between 0~8 and 0 means disable Tx error retry function.
  *
  * @return     None
  *
  * @details    This function is used to enable/disable transmitter retry function when parity error has occurred, and set error retry count.
  * @note       Set error retry count to 0 will disable Tx error retry function.
  */
__STATIC_INLINE void SC_SetTxRetry(SC_T *sc, uint32_t u32Count)
{
    uint32_t u32TimeOutCnt = SC_TIMEOUT;

    while((sc)->CTL & SC_CTL_SYNC_Msk)
        if(--u32TimeOutCnt == 0) break;

    if(u32Count == 0)         // Disable Tx error retry
    {
        (sc)->CTL &= ~(SC_CTL_TXRTY_Msk | SC_CTL_TXRTYEN_Msk);
    }
    else
    {
        (sc)->CTL = ((sc)->CTL & ~SC_CTL_TXRTY_Msk) | ((u32Count - 1) << SC_CTL_TXRTY_Pos) | SC_CTL_TXRTYEN_Msk;
    }
}

/**
  * @brief      Set Rx Error Retry Count
  *
  * @param[in]  sc          The pointer of smartcard module.
  * @param[in]  u32Count    The number of Rx error retry count, between 0~8 and 0 means disable Rx error retry function.
  *
  * @return     None
  *
  * @details    This function is used to enable/disable receiver retry function when parity error has occurred, and set error retry count.
  * @note       Set error retry count to 0 will disable Rx error retry function.
  */
__STATIC_INLINE void  SC_SetRxRetry(SC_T *sc, uint32_t u32Count)
{
    uint32_t u32TimeOutCnt = SC_TIMEOUT;

    while((sc)->CTL & SC_CTL_SYNC_Msk)
        if(--u32TimeOutCnt == 0) break;

    if(u32Count == 0)         // Disable Rx error retry
    {
        (sc)->CTL &= ~(SC_CTL_RXRTY_Msk | SC_CTL_RXRTYEN_Msk);
    }
    else
    {
        (sc)->CTL = ((sc)->CTL & ~SC_CTL_RXRTY_Msk) | ((u32Count - 1) << SC_CTL_RXRTY_Pos) | SC_CTL_RXRTYEN_Msk;
    }
}


uint32_t SC_IsCardInserted(SC_T *sc);
void SC_ClearFIFO(SC_T *sc);
void SC_Close(SC_T *sc);
void SC_Open(SC_T *sc, uint32_t u32CardDet, uint32_t u32PWR);
void SC_ResetReader(SC_T *sc);
void SC_SetBlockGuardTime(SC_T *sc, uint32_t u32BGT);
void SC_SetCharGuardTime(SC_T *sc, uint32_t u32CGT);
void SC_StopAllTimer(SC_T *sc);
void SC_StartTimer(SC_T *sc, uint32_t u32TimerNum, uint32_t u32Mode, uint32_t u32ETUCount);
void SC_StopTimer(SC_T *sc, uint32_t u32TimerNum);
uint32_t SC_GetInterfaceClock(SC_T *sc);

/*@}*/ /* end of group SC_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group SC_Driver */

/*@}*/ /* end of group Standard_Driver */

#ifdef __cplusplus
}
#endif

#endif //__SC_H__

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
