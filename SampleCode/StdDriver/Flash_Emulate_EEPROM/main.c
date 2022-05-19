/******************************************************************************
 * @file     main.c
 * @version  V3.00
 * $Revision: 8 $
 * $Date: 14/02/10 2:43p $
 * @brief    FMC erase/program/read sample program for M051 series MCU
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"
#include "EEPROM_Emulate.h"
#include "malloc.h"
#define Test_data_size			255
#define Test_page_amount		5
extern uint32_t	Current_Valid_Page;
void SYS_Init(void)
{
/*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Select HCLK clock source as HIRC and and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Set PLL to Power-down mode and PLLSTB bit in CLK_STATUS register will be cleared by hardware.*/
    CLK_DisablePLL();

    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select UART module clock source as HIRC and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk))
                    |(SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);

    /* Lock protected registers */
    SYS_LockReg();
}

int main()
{
    uint32_t i;
		uint8_t u8Data;
uint32_t tcount=4000;
    /* Unlock protected registers */
    SYS_UnlockReg();
	
		SYS_Init();

    /* Init UART0 to 115200-8n1 for print message */
    UART_Open(UART0, 115200);
    my_mem_init(0); //initial memory
		/* Test Init_EEPROM() */
		if (Init_EEPROM(Test_data_size, Test_page_amount)!=0)
		{		
			while(1);
		}

		/* Test Search_Valid_Page() */
		Search_Valid_Page();
   while(tcount--)
	 {
		/* Test Write_Data() */
		for(i = 0; i < 256; i++)
		{
				Write_Data(i%Test_data_size, i%256);
        Read_Data(i%Test_data_size, &u8Data);
			  //printf("address:0x%x, w data: %d, r data: %d\n\r",i%Test_data_size,i%256, u8Data);
			  if(i%256!=u8Data)
				{
				printf("data false!\n\r");
				while(1);
				}
		}
		
		printf("Cycle_Counter%d\n\r",Get_Cycle_Counter());
		printf("Current_Valid_Page:%d\n\r",Current_Valid_Page);
	}
    while(1);
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
