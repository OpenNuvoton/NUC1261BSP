/**************************************************************************//**
 * @file     main.c
 * @version  V0.10
 * $Revision: 2 $
 * $Date: 16/10/25 4:29p $
 * @brief
 *           Configure SPI1 as I2S Slave mode and demonstrate how I2S works in Slave mode.
 *           This sample code needs to work with I2S_Master.
 * @note
 * @copyright SPDX-License-Identifier: Apache-2.0
 *
 * @copyright Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "NUC1261.h"

#define PLL_CLOCK           72000000

volatile uint32_t g_u32TxValue;
volatile uint32_t g_u32DataCount;

/* Function prototype declaration */
void SYS_Init(void);

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    uint32_t u32RxValue1, u32RxValue2;

    /* Unlock protected registers */
    SYS_UnlockReg();
    /* Init System, IP clock and multi-function I/O. */
    SYS_Init();
    /* Lock protected registers */
    SYS_LockReg();

    /* Reset UART0 module */
    SYS_ResetModule(UART0_RST);
    /* Init UART0 to 115200-8n1 for printing messages */
    UART_Open(UART0, 115200);

    printf("+----------------------------------------------------------+\n");
    printf("|            I2S Driver Sample Code (slave mode)           |\n");
    printf("+----------------------------------------------------------+\n");
    printf("  I2S configuration:\n");
    printf("      Word width 16 bits\n");
    printf("      Stereo mode\n");
    printf("      I2S format\n");
    printf("      TX value: 0xAA00AA01, 0xAA02AA03, ..., 0xAAFEAAFF, wraparound\n");
    printf("  The I/O connection for I2S1 (SPI1):\n");
    printf("      I2S1_LRCLK (PD.12)\n      I2S1_BCLK(PD.15)\n");
    printf("      I2S1_DI (PD.14)\n      I2S1_DO (PD.13)\n\n");
    printf("  NOTE: Connect with a I2S master.\n");
    printf("        This sample code will transmit a TX value 50000 times, and then change to the next TX value.\n");
    printf("        When TX value or the received value changes, the new TX value or the current TX value and the new received value will be printed.\n");
    printf("  Press any key to start ...");
    getchar();
    printf("\n");

    /* Slave mode, 16-bit word width, stereo mode, I2S format. Set TX FIFO threshold to 2 and RX FIFO threshold to 1. */
    /* I2S peripheral clock rate is equal to PCLK1 clock rate. */
    SPII2S_Open(SPI1, SPII2S_MODE_SLAVE, 0, SPII2S_DATABIT_16, SPII2S_STEREO, SPII2S_FORMAT_I2S);

    /* Initiate data counter */
    g_u32DataCount = 0;
    /* Initiate TX value and RX value */
    g_u32TxValue = 0xAA00AA01;
    u32RxValue1 = 0;
    u32RxValue2 = 0;
    /* Enable TX threshold level interrupt */
    SPII2S_EnableInt(SPI1, SPII2S_FIFO_TXTH_INT_MASK);
    NVIC_EnableIRQ(SPI1_IRQn);

    printf("Start I2S ...\nTX value: 0x%X\n", g_u32TxValue);

    while(1)
    {
        /* Check RX FIFO empty flag */
        if((SPI1->I2SSTS & SPI_I2SSTS_RXEMPTY_Msk) == 0)
        {
            /* Read RX FIFO */
            u32RxValue2 = SPII2S_READ_RX_FIFO(SPI1);
            if(u32RxValue1 != u32RxValue2)
            {
                u32RxValue1 = u32RxValue2;
                /* If received value changes, print the current TX value and the new received value. */
                printf("TX value: 0x%X;  RX value: 0x%X\n", g_u32TxValue, u32RxValue1);
            }
        }
        if(g_u32DataCount >= 50000)
        {
            g_u32TxValue = 0xAA00AA00 | ((g_u32TxValue + 0x00020002) & 0x00FF00FF); /* g_u32TxValue: 0xAA00AA01, 0xAA02AA03, ..., 0xAAFEAAFF */
            printf("TX value: 0x%X\n", g_u32TxValue);
            g_u32DataCount = 0;
        }
    }
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable HIRC clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Switch HCLK clock source to HIRC */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Enable HXT clock (external XTAL 12MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Wait for HXT clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Configure PLL */
    CLK_EnablePLL(CLK_PLLCTL_PLLSRC_HXT, PLL_CLOCK);

    /* Switch HCLK clock source to PLL */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_PLL, CLK_CLKDIV0_HCLK(1));

    /* Select UART module clock source as HXT and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UARTSEL_HXT, CLK_CLKDIV0_UART(1));

    /* Select PCLK0 as the clock source of SPI1 */
    CLK_SetModuleClock(SPI1_MODULE, CLK_CLKSEL2_SPI1SEL_PCLK0, MODULE_NoMsk);

    /* Enable peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(SPI1_MODULE);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CyclesPerUs automatically. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PA multi-function pins for UART0 RXD and TXD */
    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk);
    SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA3MFP_UART0_RXD | SYS_GPA_MFPL_PA2MFP_UART0_TXD);

    /* Configure SPI1 related multi-function pins. */
    /* GPD[15:12] : SPI1_CLK (I2S1_BCLK), SPI1_MISO (I2S1_DI), SPI1_MOSI (I2S1_DO), SPI1_SS (I2S1_LRCLK). */
    SYS->GPD_MFPH &= ~(SYS_GPD_MFPH_PD12MFP_Msk | SYS_GPD_MFPH_PD13MFP_Msk | SYS_GPD_MFPH_PD14MFP_Msk | SYS_GPD_MFPH_PD15MFP_Msk);
    SYS->GPD_MFPH |= (SYS_GPD_MFPH_PD12MFP_SPI1_SS | SYS_GPD_MFPH_PD13MFP_SPI1_MOSI | SYS_GPD_MFPH_PD14MFP_SPI1_MISO | SYS_GPD_MFPH_PD15MFP_SPI1_CLK);
}

void SPI1_IRQHandler()
{
    /* Write 2 TX values to TX FIFO */
    SPII2S_WRITE_TX_FIFO(SPI1, g_u32TxValue);
    SPII2S_WRITE_TX_FIFO(SPI1, g_u32TxValue);
    g_u32DataCount += 2;
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
