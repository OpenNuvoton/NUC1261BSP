/******************************************************************************
 * @file     main.c
 * @brief
 *           Demonstrate how to implement a USB Mass-Storage.
 *           It uses embedded data flash as storage.
 * @note
 * @copyright SPDX-License-Identifier: Apache-2.0
 * @copyright Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NUC1261.h"
#include "massstorage.h"

/* If crystal-less is enabled, system won't use any crystal as clock source
   If using crystal-less, system will be 48MHz, otherwise, system is 72MHz
*/
#define CRYSTAL_LESS        1
#define HIRC48_AUTO_TRIM    0x512   /* Use USB signal to fine tune HIRC 48MHz */
#define TRIM_INIT           (SYS_BASE+0x118)
#define TRIM_THRESHOLD      16      /* Each value is 0.125%, max 2% */

#define CONFIG_BASE      0x00300000
#define DATA_FLASH_BASE  MASS_STORAGE_OFFSET


#if CRYSTAL_LESS
static volatile uint32_t s_u32DefaultTrim, s_u32LastTrim;
#endif

/*--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
void SYS_Init(void)
{

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC 22.1184 MHz clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

#if (!CRYSTAL_LESS)
    /* Enable external XTAL 12 MHz clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Waiting for external XTAL clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Set core clock */
    CLK_SetCoreClock(72000000);

    /* Use HIRC as UART clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UARTSEL_HIRC, CLK_CLKDIV0_UART(1));

    /* Use PLL as USB clock source */
    CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL3_USBDSEL_PLL, CLK_CLKDIV0_USB(3));

#else
    /* Enable Internal RC 48MHz clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRC48EN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRC48STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC48, CLK_CLKDIV0_HCLK(1));

    /* Use HIRC as UART clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UARTSEL_HIRC, CLK_CLKDIV0_UART(1));

    /* Use HIRC48 as USB clock source */
    CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL3_USBDSEL_HIRC48, CLK_CLKDIV0_USB(1));
#endif

    /* Enable module clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(USBD_MODULE);


    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set PA multi-function pins for UART0 RXD and TXD */
    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk);
    SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA3MFP_UART0_RXD | SYS_GPA_MFPL_PA2MFP_UART0_TXD);
}


void UART0_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset IP */
    SYS->IPRST1 |=  SYS_IPRST1_UART0RST_Msk;
    SYS->IPRST1 &= ~SYS_IPRST1_UART0RST_Msk;

    /* Configure UART0 and set UART0 Baudrate */
    UART0->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HIRC, 115200);
    UART0->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
}


/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    uint32_t au32Config[2];

    /* Unlock protected registers */
    SYS_UnlockReg();

    SYS_Init();
    UART0_Init();

    printf("+-------------------------------------------------------+\n");
    printf("|          NuMicro USB MassStorage Sample Code          |\n");
    printf("+-------------------------------------------------------+\n");

    SYS_UnlockReg();
    /* Enable FMC ISP function */
    FMC_Open();

    /* Read User Configuration */
    FMC_ReadConfig(au32Config, 2);

    /* Check if Data Flash Size is 36K. If not, to re-define Data Flash size and to enable Data Flash function */
    if(((au32Config[0] & 0x01) == 1) || ((au32Config[0] & 0x04) == 0x4) || (au32Config[1] != DATA_FLASH_BASE))
    {
        FMC_EnableConfigUpdate();
        au32Config[0] &= ~0x5;
        au32Config[1] = DATA_FLASH_BASE;
        if(FMC_WriteConfig(au32Config, 2) < 0)
        {
            printf("Error! Fail to write User Configuration.\n");
            /* Disable FMC ISP function */
            FMC_Close();
            return -1;
        }

        FMC_ReadConfig(au32Config, 2);
        if(((au32Config[0] & 0x01) == 1) || ((au32Config[0] & 0x04) == 0x4) || (au32Config[1] != DATA_FLASH_BASE))
        {
            printf("Error! Fail to write User Configuration.\n");
            /* Disable FMC ISP function */
            FMC_Close();
            return -1;
        }

        /* Reset Chip to reload new CONFIG value */
        SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;
    }

    printf("cfg0=%08x cfg1=%08x\n", au32Config[0], au32Config[1]);

    printf("NuMicro USB MassStorage Start!\n");

    USBD_Open(&gsInfo, MSC_ClassRequest, NULL);

    USBD_SetConfigCallback(MSC_SetConfig);

    /* Endpoint configuration */
    MSC_Init();
    USBD_Start();

    /* Enable USB device interrupt */
    NVIC_EnableIRQ(USBD_IRQn);

#if CRYSTAL_LESS
    /* Backup default trim */
    s_u32DefaultTrim = M32(TRIM_INIT);
    s_u32LastTrim = s_u32DefaultTrim;
#endif

    /* Clear SOF */
    USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;

    while(1)
    {
#if CRYSTAL_LESS
        /* Start USB trim if it is not enabled. */
        if((SYS->IRCTCTL1 & SYS_IRCTCTL1_FREQSEL_Msk) != 2)
        {
            /* Start USB trim only when SOF */
            if(USBD->INTSTS & USBD_INTSTS_SOFIF_Msk)
            {
                /* Clear SOF */
                USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;

                /* Re-enable crystal-less */
                SYS->IRCTCTL1 = HIRC48_AUTO_TRIM;
            }
        }

        /* Disable USB Trim when error */
        if(SYS->IRCTISTS & (SYS_IRCTISTS_CLKERRIF1_Msk | SYS_IRCTISTS_TFAILIF1_Msk))
        {
            /* Last TRIM */
            M32(TRIM_INIT) = s_u32LastTrim;

            /* Disable crystal-less */
            SYS->IRCTCTL1 = 0;

            /* Clear error flags */
            SYS->IRCTISTS = SYS_IRCTISTS_CLKERRIF1_Msk | SYS_IRCTISTS_TFAILIF1_Msk;

            /* Clear SOF */
            USBD->INTSTS = USBD_INTSTS_SOFIF_Msk;
        }

        /* Check trim value whether it is over the threshold */
        if((M32(TRIM_INIT) > (s_u32DefaultTrim + TRIM_THRESHOLD)) || (M32(TRIM_INIT) < (s_u32DefaultTrim - TRIM_THRESHOLD)))
        {
            /* Write updated value */
            M32(TRIM_INIT) = s_u32LastTrim;
        }
        else
        {
            /* Backup trim value */
            s_u32LastTrim =  M32(TRIM_INIT);
        }
#endif

        MSC_ProcessCmd();
    }
}



/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/

