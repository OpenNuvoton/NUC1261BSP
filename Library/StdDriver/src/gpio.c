/**************************************************************************//**
 * @file     gpio.c
 * @version  V3.00
 * $Revision: 7 $
 * $Date: 16/10/25 4:25p $
 * @brief    NUC1261 series General Purpose I/O (GPIO) driver source file
 *
 * @note
 *
 * @copyright SPDX-License-Identifier: Apache-2.0
 * @copyright Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "NUC1261.h"


/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup GPIO_Driver GPIO Driver
  @{
*/

/** @addtogroup GPIO_EXPORTED_FUNCTIONS GPIO Exported Functions
  @{
*/

/**
 * @brief       Set GPIO operation mode
 *
 * @param[in]   port        GPIO port. It could be PA, PB, PC, PD, PE or PF.
 * @param[in]   u32PinMask  The single or multiple pins of specified GPIO port. \n
 *                          It could be BIT0 ~ BIT3 for PA. \n
 *                          It could be BIT0 ~ BIT8, BIT11, BIT15 for PB. \n
 *                          It could be BIT0 ~ BIT7 for PC. \n
 *                          It could be BIT0 ~ BIT3, BIT7 ~ BIT9, BIT12 ~ BIT15 for PD. \n
 *                          It could be BIT0, BIT2, BIT6 ~ BIT13 for PE. \n
 *                          It could be BIT0 ~ BIT4, BIT7 for PF.
 * @param[in]   u32Mode     Operation mode.  It could be
 *                          - \ref GPIO_MODE_INPUT
 *                          - \ref GPIO_MODE_OUTPUT
 *                          - \ref GPIO_MODE_OPEN_DRAIN
 *                          - \ref GPIO_MODE_QUASI
 *
 * @return      None
 *
 * @details     This function is used to set specified GPIO operation mode.
 */
void GPIO_SetMode(GPIO_T *port, uint32_t u32PinMask, uint32_t u32Mode)
{
    uint32_t i;

    for(i = 0; i < GPIO_PIN_MAX; i++)
    {
        if(u32PinMask & (1 << i))
        {
            port->MODE = (port->MODE & ~(0x3 << (i << 1))) | (u32Mode << (i << 1));
        }
    }
}

/**
 * @brief       Enable GPIO interrupt
 *
 * @param[in]   port            GPIO port. It could be PA, PB, PC, PD, PE or PF.
 * @param[in]   u32Pin          The pin of specified GPIO port. \n
 *                              It could be 0 ~ 3 for PA GPIO port. \n
 *                              It could be 0 ~ 8, 11, 15 for PB GPIO port. \n
 *                              It could be 0 ~ 7 for PC GPIO port. \n
 *                              It could be 0 ~ 3, 7 ~ 9, 12 ~ 15 for PD GPIO port. \n
 *                              It could be 0, 2, 6 ~ 13 for PE GPIO port. \n
 *                              It could be 0 ~ 4, 7 for PF GPIO port.
 * @param[in]   u32IntAttribs   The interrupt attribute of specified GPIO pin. It could be
 *                              - \ref GPIO_INT_RISING
 *                              - \ref GPIO_INT_FALLING
 *                              - \ref GPIO_INT_BOTH_EDGE
 *                              - \ref GPIO_INT_HIGH
 *                              - \ref GPIO_INT_LOW
 *
 * @return      None
 *
 * @details     This function is used to enable specified GPIO pin interrupt.
 */
void GPIO_EnableInt(GPIO_T *port, uint32_t u32Pin, uint32_t u32IntAttribs)
{
    /* Configure interrupt mode of specified pin */
    port->INTTYPE = (port->INTTYPE & ~(1ul << u32Pin)) | (((u32IntAttribs >> 24) & 0xFFUL) << u32Pin);

    /* Enable interrupt function of specified pin */
    port->INTEN = (port->INTEN & ~(0x00010001ul << u32Pin)) | ((u32IntAttribs & 0xFFFFFFUL) << u32Pin);
}


/**
 * @brief       Disable GPIO interrupt
 *
 * @param[in]   port        GPIO port. It could be PA, PB, PC, PD, PE or PF.
 * @param[in]   u32Pin      The pin of specified GPIO port. \n
 *                          It could be 0 ~ 3 for PA GPIO port. \n
 *                          It could be 0 ~ 8, 11, 15 for PB GPIO port. \n
 *                          It could be 0 ~ 7 for PC GPIO port. \n
 *                          It could be 0 ~ 3, 7 ~ 9, 12 ~ 15 for PD GPIO port. \n
 *                          It could be 0, 2, 6 ~ 13 for PE GPIO port. \n
 *                          It could be 0 ~ 4, 7 for PF GPIO port.
 *
 * @return      None
 *
 * @details     This function is used to disable specified GPIO pin interrupt.
 */
void GPIO_DisableInt(GPIO_T *port, uint32_t u32Pin)
{
    /* Configure interrupt mode of specified pin */
    port->INTTYPE &= ~(1UL << u32Pin);

    /* Disable interrupt function of specified pin */
    port->INTEN &= ~((0x00010001UL) << u32Pin);
}


/*@}*/ /* end of group GPIO_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group GPIO_Driver */

/*@}*/ /* end of group Standard_Driver */

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
