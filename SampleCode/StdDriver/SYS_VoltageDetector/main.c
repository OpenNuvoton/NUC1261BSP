/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * $Revision: 2 $
 * $Date: 16/10/25 4:30p $
 * @brief
 *           Show how to use voltage detector to detect pin input voltage.
 *
 * @note
 * @copyright SPDX-License-Identifier: Apache-2.0
 *
 * @copyright Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "stdio.h"
#include "NUC1261.h"


#define PLL_CLOCK           72000000


/*---------------------------------------------------------------------------------------------------------*/
/*  Brown Out Detector IRQ Handler                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
void BOD_IRQHandler(void)
{
    /* Check if voltage detector interrupt happen */
    if( SYS->BODCTL & SYS_BODCTL_VDETIF_Msk )
    {
        /* Clear voltage detector interrupt flag */
        SYS->BODCTL |= SYS_BODCTL_VDETIF_Msk;

        if(SYS->BODCTL & SYS_BODCTL_VDETOUT_Msk)
            printf("The input voltage is lower than Bandgap.\n");
        else
            printf("The input voltage is higher than Bandgap.\n");
    }

}


void SYS_Init(void)
{

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable HIRC clock (Internal RC 22.1184MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Select HCLK clock source as HIRC and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Enable HXT clock (external XTAL 12MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Wait for HXT clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(PLL_CLOCK);

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select UART module clock source as HXT and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UARTSEL_HXT, CLK_CLKDIV0_UART(1));

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set PA multi-function pins for UART0 RXD and TXD */
    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk);
    SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA3MFP_UART0_RXD | SYS_GPA_MFPL_PA2MFP_UART0_TXD);

    /* Set PB multi-function pins for VDET_P0(PB.0) */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB0MFP_Msk)) | SYS_GPB_MFPL_PB0MFP_VDET_P0;

}

void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART0 */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf */
    UART0_Init();

    printf("\n\nCPU @ %d Hz\n", SystemCoreClock);
    printf("+------------------------------------------------+\n");
    printf("|      NUC1261 Voltage Detector Sample Code      |\n");
    printf("+------------------------------------------------+\n");

    printf("Change VDET_P0(PB.0) input voltage.\n");
    printf("The voltage detector interrupt is requested when the input voltage \nis dropped down or raised up through the Bandgap voltage(1.2V).\n\n");
    UART_WAIT_TX_EMPTY(DEBUG_PORT);

    /* Select voltage detector external input voltage pin as VDET_P0(PB.0) */
    SYS->BODCTL &= ~SYS_BODCTL_VDETPINSEL_Msk;

    /* Enable voltage detector function */
    SYS->BODCTL |= SYS_BODCTL_VDETEN_Msk;

    /* Enable voltage detector interrupt function */
    SYS->BODCTL |= SYS_BODCTL_VDETIEN_Msk;
    NVIC_EnableIRQ(BOD_IRQn);

    /* Wait for voltage detector interrupt happen */
    while(1);

}
