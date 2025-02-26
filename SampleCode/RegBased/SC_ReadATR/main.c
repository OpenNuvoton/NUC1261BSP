/****************************************************************************
 * @file     main.c
 * @version  V2.0
 * $Revision: 3 $
 * $Date: 17/05/04 1:21p $
 * @brief    Read the Smartcard ATR from SC0 port.
 * @note
 * @copyright SPDX-License-Identifier: Apache-2.0
 * @copyright Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NUC1261.h"


#define PLLCTL_SETTING  CLK_PLLCTL_72MHz_HXT
#define PLL_CLOCK       72000000


#define SCLIB_MAX_ATR_LEN       32  // Max ATR length. ISO-7816 8.2.1

uint8_t ATR_Buf[SCLIB_MAX_ATR_LEN]; // Buffer holds ATR answered by smartcard
volatile int32_t g_u32SCInt = 0;

/*---------------------------------------------------------------------------------------------------------*/
/* The interrupt services routine of smartcard port 0                                                      */
/*---------------------------------------------------------------------------------------------------------*/
void SC01_IRQHandler(void)
{
    while(1)
    {
        // When SC receives one byte from external device, RXPOINT(SC_STATUS[17:16]) increases one.
        // When one byte of RX buffer is read by CPU, RXPOINT(SC_STATUS[17:16]) decreases one.
        ATR_Buf[g_u32SCInt++] = (SC0->DAT);
        if((SC0->STATUS & SC_STATUS_RXPOINT_Msk) == 0)
            break;
    }
    // RDA is the only interrupt enabled in this sample, this status bit
    // automatically cleared after Rx FIFO empty. So no need to clear interrupt
    // status here.
}

void SYS_Init(void)
{
	uint32_t u32TimeOutCnt;

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Enable HIRC clock */
    CLK->PWRCTL |= CLK_PWRCTL_HIRCEN_Msk;

    /* Waiting for HIRC clock ready */
    u32TimeOutCnt = __HIRC;
    while(!(CLK->STATUS & CLK_STATUS_HIRCSTB_Msk))
		if(--u32TimeOutCnt == 0) break;

    /* Switch HCLK clock source to HIRC */
    CLK->CLKSEL0 = CLK_CLKSEL0_HCLKSEL_HIRC;

    /* Set PLL to Power-down mode and PLLSTB bit in CLK_STATUS register will be cleared by hardware.*/
    CLK->PLLCTL |= CLK_PLLCTL_PD_Msk;

    /* Enable HXT */
    CLK->PWRCTL |= CLK_PWRCTL_HXTEN_Msk;

    /* Enable PLL and Set PLL frequency */
    CLK->PLLCTL = PLLCTL_SETTING;

    u32TimeOutCnt = __HIRC;
    while(!(CLK->STATUS & CLK_STATUS_PLLSTB_Msk))
		if(--u32TimeOutCnt == 0) break;

    /* Switch STCLK source to HCLK/2 and HCLK clock source to PLL */
    CLK->CLKSEL0 = CLK_CLKSEL0_STCLKSEL_HCLK_DIV2 | CLK_CLKSEL0_HCLKSEL_PLL;

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    SystemCoreClockUpdate();

    /* Enable peripheral clock */
    CLK->APBCLK0 |= CLK_APBCLK0_UART0CKEN_Msk;

    /* Peripheral clock source */
    CLK->CLKSEL1 = CLK_CLKSEL1_UARTSEL_PLL;

    /* Enable SC0 module clock */
    CLK->APBCLK1 |= CLK_APBCLK1_SC0CKEN_Msk;

    /* Select SC0 module clock source from HXT divide 3 */
    CLK->CLKSEL3 &= ~CLK_CLKSEL3_SC0SEL_Msk;
    CLK->CLKSEL3 |= CLK_CLKSEL3_SC0SEL_HXT;
    CLK->CLKDIV1 &= ~CLK_CLKDIV1_SC0DIV_Msk;
    CLK->CLKDIV1 |= CLK_CLKDIV1_SC0(3);

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PA multi-function pins for UART0 RXD and TXD */
    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk);
    SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA3MFP_UART0_RXD | SYS_GPA_MFPL_PA2MFP_UART0_TXD);

    /* Set PC.0 ~ PC.3 and PB.2 for SC0 interface */
    SYS->GPC_MFPL &= ~(SYS_GPC_MFPL_PC0MFP_Msk |
                       SYS_GPC_MFPL_PC1MFP_Msk |
                       SYS_GPC_MFPL_PC2MFP_Msk |
                       SYS_GPC_MFPL_PC3MFP_Msk);
    SYS->GPB_MFPL &= ~SYS_GPB_MFPL_PB2MFP_Msk;
    SYS->GPC_MFPL |= (SC0_DAT_PC0 |
                      SC0_CLK_PC1 |
                      SC0_RST_PC2 |
                      SC0_PWR_PC3);
    SYS->GPB_MFPL |= SC0_nCD_PB2;
}

void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART module */
    SYS->IPRST1 |=  SYS_IPRST1_UART0RST_Msk;
    SYS->IPRST1 &= ~SYS_IPRST1_UART0RST_Msk;

    /* Configure UART0 and set UART0 Baudrate */
    UART0->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(PllClock, 115200);
    UART0->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
}

void SC0_Init()
{
    uint32_t u32TimeOutCnt;

    /* Card detect level from low to high. */
    u32TimeOutCnt = SC_TIMEOUT;
    while(SC0->CTL & SC_CTL_SYNC_Msk)
        if(--u32TimeOutCnt == 0) break;
    SC0->CTL = SC_CTL_CDLV_Msk | SC_CTL_SCEN_Msk;
}

/*---------------------------------------------------------------------------------------------------------*/
/* This function reset smartcard module to its default state for activate smartcard                        */
/*---------------------------------------------------------------------------------------------------------*/
void SC0_ResetReader(void)
{
    /* Reset FIFO, enable auto de-activation while card removal */
    SC0->ALTCTL |= (SC_ALTCTL_TXRST_Msk | SC_ALTCTL_RXRST_Msk);

    /* Set Rx trigger level to 1 character, longest card detect debounce period, disable error retry */
    SC0->CTL &= ~(SC_CTL_RXTRGLV_Msk | SC_CTL_CDDBSEL_Msk | SC_CTL_TXRTY_Msk | SC_CTL_RXRTY_Msk);

    /* Enable auto convention, and all three smartcard internal timers */
    SC0->CTL |= SC_CTL_AUTOCEN_Msk | SC_CTL_TMRSEL_Msk;

    /* Disable Rx time-out */
    SC0->RXTOUT = 0;

    /* 372 clocks per ETU by default */
    SC0->ETUCTL = 371;

    /* Enable necessary interrupt for smartcard operation */
    SC0->INTEN = SC_INTEN_RDAIEN_Msk; //Enable receive data reach interrupt
}

__STATIC_INLINE void wait_SC_PINCTL_SYNC(SC_T *sc)
{
    uint32_t u32TimeOutCnt = SC_TIMEOUT;

    while(SC0->PINCTL & SC_PINCTL_SYNC_Msk)
    {
        if(--u32TimeOutCnt == 0)
        {
            printf("Wait for SC_PINCTL sync flag is cleared time-out!\n");
            break;
        }
    }
}

int32_t SC0_ActivationCmd(void)
{
    uint32_t u32TimeOutCnt;

    /* Disable Timer1 interrupt and use polling method to check time-out happened */
    SC0->INTEN &= ~SC_INTEN_TMR1IEN_Msk;

    /* VCC high */
    SC_SET_VCC_PIN(SC0, SC_PIN_STATE_HIGH);

    /* Select operation mode 0 and give a count value */
    SC0-> TMRCTL1 = SC_TMR_MODE_0 | (SC_TMRCTL1_CNT_Msk & (1 - 1));;
    SC0->ALTCTL |= SC_ALTCTL_CNTEN1_Msk; //Start a smartcard timer1

    u32TimeOutCnt = SC_TIMEOUT;
    while(((SC0->INTSTS & SC_INTSTS_TMR1IF_Msk) == 0x00))
    {
        if(--u32TimeOutCnt == 0)
        {
            printf("Wait for SC timer1 interrupt flag time-out!\n");
            return -1;
        }
    }
    SC0->INTSTS = SC_INTSTS_TMR1IF_Msk; // Clear time-out flag

    /* Due to synchronization, user should check SYNC (SC_PINCTL[30]) bit when writing a new value to SC_PINCTL register */
    wait_SC_PINCTL_SYNC(SC0);
    /* Start clock */
    SC_SET_CLK_PIN(SC0, SC_CLK_ON);

    /* Due to synchronization, user should check SYNC (SC_PINCTL[30]) bit when writing a new value to SC_PINCTL register */
    wait_SC_PINCTL_SYNC(SC0);
    /* I/O pin high */
    SC_SET_IO_PIN(SC0, SC_PIN_STATE_HIGH);

    /* Select operation mode 0  and give a count value */
    SC0-> TMRCTL1 = SC_TMR_MODE_0 | (SC_TMRCTL1_CNT_Msk & (109 - 1));;
    SC0->ALTCTL |= SC_ALTCTL_CNTEN1_Msk; //Start a smartcard timer1

    u32TimeOutCnt = SC_TIMEOUT;
    while(((SC0->INTSTS & SC_INTSTS_TMR1IF_Msk) == 0x00))
    {
        if(--u32TimeOutCnt == 0)
        {
            printf("Wait for SC timer1 interrupt flag time-out!\n");
            return -1;
        }
    }
    SC0->INTSTS = SC_INTSTS_TMR1IF_Msk; // Clear time-out flag

    /* Due to synchronization, user should check SYNC (SC_PINCTL[30]) bit when writing a new value to SC_PINCTL register */
    wait_SC_PINCTL_SYNC(SC0);
    /* RST pin high */
    SC_SET_RST_PIN(SC0, SC_PIN_STATE_HIGH);

    /* Select operation mode 0  and give a count value */
    SC0-> TMRCTL0 = SC_TMR_MODE_0 | (SC_TMRCTL0_CNT_Msk & (22 * 32 + 22)); // Wait 726 ETU for ATR
    SC0->ALTCTL |= SC_ALTCTL_CNTEN0_Msk; //Start a smartcard timer0

    return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{
    uint32_t i, u32TimeOutCnt;

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf */
    UART0_Init();

    /* Init SC0 */
    SC0_Init();

    printf("\n\nCPU @ %dHz\n", SystemCoreClock);
    printf("+-------------------------------------+\n");
    printf("|    Smart Card Driver Sample Code    |\n");
    printf("+-------------------------------------+\n");
    printf("# This sample code will get Smart Card ATR data via SC0 port.\n");
    printf("# I/O configuration:\n");
    printf("    SC0CLK (PC.1) <--> smart card slot clock pin\n");
    printf("    SC0DAT (PC.0) <--> smart card slot data pin\n");
    printf("    SC0PWR (PC.3) <--> smart card slot power pin\n");
    printf("    SC0RST (PC.2) <--> smart card slot reset pin\n");
    printf("    SC0CD  (PB.2) <--> smart card slot card detect pin\n");
    printf("\nThis sample code reads ATR from smartcard...\n");

    NVIC_EnableIRQ(SC01_IRQn);

    /* Wait 'til card insert */
    while(((SC0->STATUS & SC_STATUS_CDPINSTS_Msk) >> SC_STATUS_CDPINSTS_Pos) !=
            ((SC0->CTL & SC_CTL_CDLV_Msk) >> SC_CTL_CDLV_Pos));

    /* Reset smartcard module to its default state for activate smartcard */
    SC0_ResetReader();

    /* Activation sequence generator */
    if( SC0_ActivationCmd() < 0 ) goto lexit;

    u32TimeOutCnt = SC_TIMEOUT;
    while(1)
    {
        if(((SC0->INTSTS & SC_INTSTS_TMR0IF_Msk) == SC_INTSTS_TMR0IF_Msk) &&
                (SC0->ALTCTL & SC_ALTCTL_CNTEN0_Msk) == 0)
        {
            printf("\nATR: ");
            for(i = 0; i < g_u32SCInt; i++)
                printf("%x ", ATR_Buf[i]);
            printf("\n");
            break;
        }

        if(--u32TimeOutCnt == 0)
        {
            printf("Read ATR time-out!\n");
            break;
        }
    }

lexit:

    while(1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
