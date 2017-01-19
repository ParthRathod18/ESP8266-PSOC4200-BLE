/*******************************************************************************
* File Name: main.c
*
* Version: 1.00
*
* Description:
*  This is the source code for the example project of the RTC_P4 component.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include <project.h>
#include <stdio.h>

/* Time: 02:59:50 */
#define TIME_HOUR           (0x02u)
#define TIME_MIN            (0x59u)
#define TIME_SEC            (0x50u)
#define TIME_HR_MIN_SEC     ((uint32)(TIME_HOUR << RTC_HOURS_OFFSET) | \
                            (uint32)(TIME_MIN << RTC_MINUTES_OFFSET)    | \
                             TIME_SEC)
/* Date: 03/22/2014 */
#define DATE_MONTH          (RTC_MARCH)
#define DATE_DAY            (0x22u)
#define DATE_YEAR           (0x2014u)
#define DATE_MONTH_DAY_YEAR ((uint32)(DATE_MONTH << RTC_MONTH_OFFSET)   | \
                            (uint32)(DATE_DAY << RTC_DAY_OFFSET)        | \
                             DATE_YEAR)

#define SYSTICK_EACH_10_HZ  (10u)
#define SYSTICK_RELOAD      (CYDEV_BCLK__SYSCLK__HZ / SYSTICK_EACH_10_HZ)

/* Interrupt prototype */
CY_ISR_PROTO(SysTickIsrHandler);


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  At the beginning of the main function, the Systick timer starts and 
*  initializes interrupt generation every 100 ms and sets the current Date and 
*  Time using API. After Data and Time from RTC is obtained, print them to UART 
*  in the cycle.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
int main()
{
    char timeBuffer[16u];
    char dateBuffer[16u];

    uint32 time;
    uint32 date;
    uint32 i;

    /* Starts SysTick component */
    CySysTickStart();

    /* Configure SysTick timer to generate interrupt every 100 ms */
    CySysTickSetReload(SYSTICK_RELOAD);

    /* Find unused callback slot. */
    for (i = 0u; i < CY_SYS_SYST_NUM_OF_CALLBACKS; ++i)
    {
        if (CySysTickGetCallback(i) == NULL)
        {
            /* Set callback */
            CySysTickSetCallback(i, SysTickIsrHandler);
            break;
        }
    }

    /* Starts RTC component */
    RTC_Start();

    /* Set Date and Time */
    RTC_SetDateAndTime(TIME_HR_MIN_SEC,DATE_MONTH_DAY_YEAR);

    /* Set RTC time update period */
    RTC_SetPeriod(1u, SYSTICK_EACH_10_HZ);

    /* Enable global interrupts */
    CyGlobalIntEnable;

    while(1)
    {
        /* Get Date and Time from RTC */
        time = RTC_GetTime();
        date = RTC_GetDate();

        /* Print Date and Time to UART */
        sprintf(timeBuffer, "%02lu:%02lu:%02lu", RTC_GetHours(time), RTC_GetMinutes(time), RTC_GetSecond(time));
        sprintf(dateBuffer, "%02lu/%02lu/%02lu", RTC_GetMonth(date), RTC_GetDay(date), RTC_GetYear(date));

        UART_PutString(timeBuffer);
        UART_PutString(" | ");
        UART_PutString(dateBuffer);
        UART_PutString("\r");

        CyDelay(200u);
    }
}


/*******************************************************************************
* Function Name: SysTickIsrHandler
********************************************************************************
* Summary:
*  The interrupt handler for SysTick interrupts.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void SysTickIsrHandler(void)
{
    RTC_Update();
}

/* [] END OF FILE */
