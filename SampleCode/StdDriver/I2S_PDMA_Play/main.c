/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * $Revision: 2 $
 * $Date: 16/10/25 4:29p $
 * @brief    NUC1261 SPI Driver Sample Code
 *           This is a I2S demo for playing data and demonstrate how I2S works with PDMA.
 *
 * @note
 * @copyright SPDX-License-Identifier: Apache-2.0
 *
 * @copyright Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "NUC1261.h"

#define I2S_TX_DMA_CH 1
#define I2S_RXData_DMA_CH 2

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#define BUFF_LEN 4
#define CHECK_BUFF_LEN 32

typedef struct
{
    uint32_t CTL;
    uint32_t SA;
    uint32_t DA;
    uint32_t FIRST;
} DESC_TABLE_T;

DESC_TABLE_T g_asDescTable_TX[2], g_asDescTable_DataRX[1];

/* Function prototype declaration */
void SYS_Init(void);

/* Global variable declaration */
volatile uint8_t u8TxIdx = 0;
uint32_t PcmRxDataBuff[1][CHECK_BUFF_LEN] = {0};
uint32_t PcmTxBuff[2][BUFF_LEN] = {0};

/* Once PDMA has transferred, software need to reset Scatter-Gather table */
void PDMA_ResetTxSGTable(uint8_t id)
{
    g_asDescTable_TX[id].CTL |= PDMA_OP_SCATTER;
    g_asDescTable_TX[id].CTL |= ((BUFF_LEN - 1) << PDMA_DSCT_CTL_TXCNT_Pos);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    uint32_t u32InitValue, u32DataCount;

    /* Unlock protected registers */
    SYS_UnlockReg();
    /* Init system, peripheral clock and multi-function I/O */
    SYS_Init();
    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    printf("\n");
    printf("+----------------------------------------------+\n");
    printf("|         I2S + PDMA  Play Sample Code         |\n");
    printf("+----------------------------------------------+\n");
    printf("  I2S configuration:\n");
    printf("      Sample rate 16 kHz\n");
    printf("      Word width 16 bits\n");
    printf("      Stereo mode\n");
    printf("      I2S format\n");
    printf("      TX 1/2 value: 0x50005000/0xA000A000, 0x50015001/0xA001A001, ... \n");
    printf("  The I/O connection for I2S:\n");
    printf("      I2S_LRCLK (PD.12)\n      I2S_BCLK (PD.15)\n");
    printf("      I2S_DI (PD.14)\n      I2S_DO (PD.13)\n\n");
    printf("      This sample code will transmit and receive %d data with PDMA transfer.\n", CHECK_BUFF_LEN);
    printf("      Connect I2S_DI and I2S_DO to check if the received values and its' sequence\n are the same with the data which stored in two transmit buffers.\n");
    printf("      After PDMA transfer is finished, the received values will be printed.\n\n");

    /* Select PCLK as the clock source of SPI1 */
    CLK_SetModuleClock(SPI1_MODULE, CLK_CLKSEL2_SPI1SEL_PCLK0, MODULE_NoMsk);

    /* Enable I2S TX and RX functions */
    /* Sampling rate 16000 Hz; bit clock rate 512 kHz. */
    /* Master mode, 16-bit word width, stereo mode, I2S format. */
    SPII2S_Open(SPI1, SPII2S_MODE_MASTER, 16000, SPII2S_DATABIT_16, SPII2S_STEREO, SPII2S_FORMAT_I2S);

    /* Data initiation */
    u32InitValue = 0x50005000;
    for(u32DataCount = 0; u32DataCount < BUFF_LEN; u32DataCount++)
    {
        PcmTxBuff[0][u32DataCount] = u32InitValue;
        PcmTxBuff[1][u32DataCount] = u32InitValue + 0x50005000;
        u32InitValue += 0x00010001;
    }

    /* Enable PDMA channels */
    PDMA_Open((1 << I2S_TX_DMA_CH) | (1 << I2S_RXData_DMA_CH));

    /* Tx(Play) description */
    g_asDescTable_TX[0].CTL = ((BUFF_LEN - 1) << PDMA_DSCT_CTL_TXCNT_Pos) | PDMA_WIDTH_32 | PDMA_SAR_INC | PDMA_DAR_FIX | PDMA_REQ_SINGLE | PDMA_OP_SCATTER;
    g_asDescTable_TX[0].SA = (uint32_t)&PcmTxBuff[0];
    g_asDescTable_TX[0].DA = (uint32_t)&SPI1->TX;
    g_asDescTable_TX[0].FIRST = (uint32_t)&g_asDescTable_TX[1] - (PDMA->SCATBA);

    g_asDescTable_TX[1].CTL = ((BUFF_LEN - 1) << PDMA_DSCT_CTL_TXCNT_Pos) | PDMA_WIDTH_32 | PDMA_SAR_INC | PDMA_DAR_FIX | PDMA_REQ_SINGLE | PDMA_OP_SCATTER;
    g_asDescTable_TX[1].SA = (uint32_t)&PcmTxBuff[1];
    g_asDescTable_TX[1].DA = (uint32_t)&SPI1->TX;
    g_asDescTable_TX[1].FIRST = (uint32_t)&g_asDescTable_TX[0] - (PDMA->SCATBA);   //link to first description

    /* Rx description */
    g_asDescTable_DataRX[0].CTL = ((CHECK_BUFF_LEN - 1) << PDMA_DSCT_CTL_TXCNT_Pos) | PDMA_WIDTH_32 | PDMA_SAR_FIX | PDMA_DAR_INC | PDMA_REQ_SINGLE | PDMA_OP_BASIC;
    g_asDescTable_DataRX[0].SA = (uint32_t)&SPI1->RX;
    g_asDescTable_DataRX[0].DA = (uint32_t)&PcmRxDataBuff[0];

    PDMA_SetTransferMode(1, PDMA_SPI1_TX, 1, (uint32_t)&g_asDescTable_TX[0]);
    PDMA_SetTransferMode(2, PDMA_SPI1_RX, 1, (uint32_t)&g_asDescTable_DataRX[0]);

    /* Enable PDMA channel 1 interrupt */
    PDMA_EnableInt(1, PDMA_INT_TRANS_DONE);

    NVIC_EnableIRQ(PDMA_IRQn);

    /* Clear RX FIFO */
    SPII2S_CLR_RX_FIFO(SPI1);

    /* Enable RX function and TX function */
    SPII2S_ENABLE_RX(SPI1);
    SPII2S_ENABLE_TX(SPI1);

    /* Enable RX PDMA and TX PDMA function */
    SPII2S_ENABLE_TXDMA(SPI1);
    SPII2S_ENABLE_RXDMA(SPI1);

    /* Print the received data */
    for(u32DataCount = 0; u32DataCount < CHECK_BUFF_LEN; u32DataCount++)
    {
        printf("%d:\t0x%X\n", u32DataCount, PcmRxDataBuff[0][u32DataCount]);
    }

    printf("\n\nExit I2S sample code.\n");

    while(1);
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

    /* Enable HXT */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Wait for HXT clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* System optimization when CPU runs at 72 MHz */
    FMC->FTCTL |= 0x50;

    /* Set core clock rate as 72MHz from PLL */
    CLK_SetCoreClock(72000000);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UARTSEL_Msk, CLK_CLKDIV0_UART(1));

    /* Enable IP peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(SPI1_MODULE);
    /* Enable PDMA peripheral clock */
    CLK_EnableModuleClock(PDMA_MODULE);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
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

void PDMA_IRQHandler(void)
{
    uint32_t u32Status = PDMA_GET_INT_STATUS();

    if(u32Status & 0x1)    /* abort */
    {
        if(PDMA_GET_ABORT_STS() & 0x4)
            PDMA_CLR_ABORT_FLAG(PDMA_ABTSTS_ABTIF1_Msk);
    }
    else if(u32Status & 0x2)
    {
        if(PDMA_GET_TD_STS() & 0x2)             /* channel 1 done */
        {
            /* Reset PDMA Scatter-Gather table */
            PDMA_ResetTxSGTable(u8TxIdx);
            u8TxIdx ^= 1;
        }
        PDMA_CLR_TD_FLAG(PDMA_TDSTS_TDIF1_Msk);
    }
    else
        printf("unknown interrupt, status=0x%x!!\n", u32Status);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
