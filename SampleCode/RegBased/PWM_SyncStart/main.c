/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * $Revision: 2 $
 * $Date: 16/10/25 4:27p $
 * @brief    Demonstrate how to use PWM counter synchronous start function.
 * @note
 * @copyright SPDX-License-Identifier: Apache-2.0
 *
 * @copyright Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NUC1261.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define PLLCTL_SETTING  CLK_PLLCTL_144MHz_HXT

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

/**
 * @brief       PWM0 IRQ Handler
 *
 * @param       None
 *
 * @return      None
 *
 * @details     ISR to handle PWM0 interrupt event
 */
void PWM0_IRQHandler(void)
{

}

void SYS_Init(void)
{
	uint32_t u32TimeOutCnt;

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable HIRC clock (Internal RC 22.1184MHz) */
    CLK->PWRCTL |= CLK_PWRCTL_HIRCEN_Msk;

    /* Waiting for HIRC clock ready */
    u32TimeOutCnt = __HIRC;
    while(!(CLK->STATUS & CLK_STATUS_HIRCSTB_Msk))
		if(--u32TimeOutCnt == 0) break;

    /* Select HCLK clock source as HIRC and HCLK clock divider as 1 */
    CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk) | CLK_CLKSEL0_HCLKSEL_HIRC;
    CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_HCLKDIV_Msk) | CLK_CLKDIV0_HCLK(1);

    /* Enable HXT clock (external XTAL 12MHz) */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;

    /* Waiting for HXT clock ready */
    u32TimeOutCnt = __HIRC;
    while(!(CLK->STATUS & CLK_STATUS_HXTSTB_Msk))
		if(--u32TimeOutCnt == 0) break;

    /* Enable PLL and Set PLL frequency */
    CLK->PLLCTL = PLLCTL_SETTING;
    u32TimeOutCnt = __HIRC;
    while(!(CLK->STATUS & CLK_STATUS_PLLSTB_Msk))
		if(--u32TimeOutCnt == 0) break;
    CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLKSEL_Msk) | CLK_CLKSEL0_HCLKSEL_PLL;

    /* Enable PWM0 module clock */
    CLK->APBCLK0 |= CLK_APBCLK0_PWM0CKEN_Msk;

    /* Enable PWM1 module clock */
    CLK->APBCLK0 |= CLK_APBCLK0_PWM1CKEN_Msk;

    /*---------------------------------------------------------------------------------------------------------*/
    /* PWM clock frequency configuration                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Select HCLK clock divider as 2 */
    CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_HCLKDIV_Msk) | CLK_CLKDIV0_HCLK(2);

    /* PWM clock frequency can be set equal or double to HCLK by choosing case 1 or case 2 */
    /* case 1.PWM clock frequency is set equal to HCLK: select PWM module clock source as PCLK */
    CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_PWM0SEL_Msk) | CLK_CLKSEL1_PWM0SEL_PCLK0;
    CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_PWM1SEL_Msk) | CLK_CLKSEL1_PWM1SEL_PCLK1;

    /* case 2.PWM clock frequency is set double to HCLK: select PWM module clock source as PLL */
    //CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_PWM0SEL_Msk) | CLK_CLKSEL1_PWM0SEL_PLL;
    //CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_PWM1SEL_Msk) | CLK_CLKSEL1_PWM1SEL_PLL;
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable UART module clock */
    CLK->APBCLK0 |= CLK_APBCLK0_UART0CKEN_Msk;

    /* Select UART module clock source as HXT */
    CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_UARTSEL_Msk) | CLK_CLKSEL1_UARTSEL_HXT;

    /* Reset PWM0 module */
    SYS->IPRST1 |= SYS_IPRST1_PWM0RST_Msk;
    SYS->IPRST1 &= ~SYS_IPRST1_PWM0RST_Msk;

    /* Reset PWM1 module */
    SYS->IPRST1 |= SYS_IPRST1_PWM1RST_Msk;
    SYS->IPRST1 &= ~SYS_IPRST1_PWM1RST_Msk;

    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PA multi-function pins for UART0 RXD and TXD */
    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk);
    SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA3MFP_UART0_RXD | SYS_GPA_MFPL_PA2MFP_UART0_TXD);

    /* Set PC multi-function pins for PWM0 Channel 0~5 */
    SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC0MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk |
                       SYS_GPC_MFPL_PC2MFP_Msk | SYS_GPC_MFPL_PC3MFP_Msk |
                       SYS_GPC_MFPL_PC4MFP_Msk | SYS_GPC_MFPL_PC5MFP_Msk);
    SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC0MFP_PWM0_CH0 | SYS_GPC_MFPL_PC1MFP_PWM0_CH1 |
                      SYS_GPC_MFPL_PC2MFP_PWM0_CH2 | SYS_GPC_MFPL_PC3MFP_PWM0_CH3 |
                      SYS_GPC_MFPL_PC4MFP_PWM0_CH4 | SYS_GPC_MFPL_PC5MFP_PWM0_CH5);

    /* Set PA, PC and PD multi-function pins for PWM1 Channel 0~5 */
    SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC6MFP_Msk | SYS_GPC_MFPL_PC7MFP_Msk);
    SYS->GPC_MFPL |= (SYS_GPC_MFPL_PC6MFP_PWM1_CH0 | SYS_GPC_MFPL_PC7MFP_PWM1_CH1);
    SYS->GPD_MFPH &= ~(SYS_GPD_MFPH_PD14MFP_Msk | SYS_GPD_MFPH_PD15MFP_Msk);
    SYS->GPD_MFPH |= (SYS_GPD_MFPH_PD14MFP_PWM1_CH2 | SYS_GPD_MFPH_PD15MFP_PWM1_CH3);
    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA1MFP_Msk | SYS_GPA_MFPL_PA0MFP_Msk);
    SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA1MFP_PWM1_CH4 | SYS_GPA_MFPL_PA0MFP_PWM1_CH5);
}

void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART IP */
    SYS->IPRST1 |=  SYS_IPRST1_UART0RST_Msk;
    SYS->IPRST1 &= ~SYS_IPRST1_UART0RST_Msk;

    /* Configure UART0 and set UART0 baud rate */
    UART0->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HXT, 115200);
    UART0->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    /* Init System, IP clock and multi-function I/O
       In the end of SYS_Init() will issue SYS_LockReg()
       to lock protected register. If user want to write
       protected register, please issue SYS_UnlockReg()
       to unlock protected register if necessary */

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART to 115200-8n1 for print message */
    UART0_Init();

    printf("\n\nCPU @ %dHz(PLL@ %dHz)\n", SystemCoreClock, PllClock);
    printf("PWM0 clock is from %s\n", (CLK->CLKSEL1 & CLK_CLKSEL1_PWM0SEL_Msk) ? "CPU" : "PLL");
    printf("PWM1 clock is from %s\n", (CLK->CLKSEL1 & CLK_CLKSEL1_PWM1SEL_Msk) ? "CPU" : "PLL");
    printf("+------------------------------------------------------------------------+\n");
    printf("|                          PWM Driver Sample Code                        |\n");
    printf("|                                                                        |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("  This sample code will output waveform with PWM0 and PWM1 channel 0~5 at the same time.\n");
    printf("  I/O configuration:\n");
    printf("    waveform output pin: PWM0_CH0(PC.0), PWM0_CH1(PC.1), PWM0_CH2(PC.2), PWM0_CH3(PC.3), PWM0_CH4(PC.4), PWM0_CH5(PC.5)\n");
    printf("                         PWM1_CH0(PC.6), PWM1_CH1(PC.7), PWM1_CH2(PD.14), PWM1_CH3(PD.15), PWM1_CH4(PA.1), PWM1_CH5(PA.0)\n");

    /* PWM0 and PWM1 channel 0~5 frequency and duty configuration are as follows */

    /*
      Configure PWM0 channel 0 init period and duty(up counter type).
      Period is PLL / (prescaler * (CNR + 1))
      Duty ratio = (CMR) / (CNR + 1)
      Period = 72 MHz / (2 * (199 + 1)) = 180000 Hz
      Duty ratio = (100) / (199 + 1) = 50%
    */

    /* Set PWM to up counter type(edge aligned) */
    PWM0->CTL1 &= ~PWM_CTL1_CNTTYPE0_Msk;

    /* PWM0 channel 0 frequency and duty configuration */
    /* Set PWM Timer clock prescaler */
    PWM_SET_PRESCALER(PWM0, 0, 0); /* Divided by 1 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM0, 0, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM0, 0, 399);

    /* PWM0 channel 1 frequency and duty configuration */
    /* PWM Timer clock prescaler of Channel 1 is share with Channel 0 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM0, 1, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM0, 1, 399);

    /* PWM0 channel 2 frequency and duty configuration */
    /* Set PWM Timer clock prescaler */
    PWM_SET_PRESCALER(PWM0, 2, 0); /* Divided by 1 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM0, 2, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM0, 2, 399);

    /* PWM0 channel 3 frequency and duty configuration */
    /* PWM Timer clock prescaler of Channel 3 is share with Channel 2 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM0, 3, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM0, 3, 399);

    /* PWM0 channel 4 frequency and duty configuration */
    /* Set PWM Timer clock prescaler */
    PWM_SET_PRESCALER(PWM0, 4, 0); /* Divided by 1 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM0, 4, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM0, 4, 399);

    /* PWM0 channel 5 frequency and duty configuration */
    /* PWM Timer clock prescaler of Channel 5 is share with Channel 4 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM0, 5, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM0, 5, 399);

    /* Set waveform generation */
    PWM_SET_OUTPUT_LEVEL(PWM0, 0x3F, PWM_OUTPUT_HIGH, PWM_OUTPUT_LOW, PWM_OUTPUT_NOTHING, PWM_OUTPUT_NOTHING);

    /* Enable output of PWM0 channel 0 ~ 5 */
    PWM0->POEN |= 0x3F;

    /* Set PWM to up counter type(edge aligned) */
    PWM1->CTL1 &= ~PWM_CTL1_CNTTYPE0_Msk;

    /* PWM1 channel 0 frequency and duty configuration */
    /* Set PWM Timer clock prescaler */
    PWM_SET_PRESCALER(PWM1, 0, 0); /* Divided by 1 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM1, 0, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM1, 0, 399);

    /* PWM1 channel 1 frequency and duty configuration */
    /* PWM Timer clock prescaler of Channel 1 is share with Channel 0 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM1, 1, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM1, 1, 399);

    /* PWM1 channel 2 frequency and duty configuration */
    /* Set PWM Timer clock prescaler */
    PWM_SET_PRESCALER(PWM1, 2, 0); /* Divided by 1 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM1, 2, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM1, 2, 399);

    /* PWM1 channel 3 frequency and duty configuration */
    /* PWM Timer clock prescaler of Channel 3 is share with Channel 2 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM1, 3, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM1, 3, 399);

    /* PWM1 channel 4 frequency and duty configuration */
    /* Set PWM Timer clock prescaler */
    PWM_SET_PRESCALER(PWM1, 4, 0); /* Divided by 1 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM1, 4, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM1, 4, 399);

    /* PWM1 channel 5 frequency and duty configuration */
    /* PWM Timer clock prescaler of Channel 5 is share with Channel 4 */
    /* Set PWM Timer duty */
    PWM_SET_CMR(PWM1, 5, 200);
    /* Set PWM Timer period */
    PWM_SET_CNR(PWM1, 5, 399);

    /* Set waveform generation */
    PWM_SET_OUTPUT_LEVEL(PWM1, 0x3F, PWM_OUTPUT_HIGH, PWM_OUTPUT_LOW, PWM_OUTPUT_NOTHING, PWM_OUTPUT_NOTHING);

    /* Enable output of PWM1 channel 0 ~ 5 */
    PWM1->POEN |= 0x3F;

    /* Enable counter synchronous start function for PWM0 and PWM1 channel 0~5 */
    PWM0->SSCTL = 0x3F | PWM_SSCTL_SSRC_PWM0;
    PWM1->SSCTL = 0x3F | PWM_SSCTL_SSRC_PWM0;

    printf("Press any key to start.\n");
    getchar();

    /* Trigger PWM counter synchronous start by PWM0 */
    PWM0->SSTRG = PWM_SSTRG_CNTSEN_Msk;

    printf("Done.");
    while(1);

}
