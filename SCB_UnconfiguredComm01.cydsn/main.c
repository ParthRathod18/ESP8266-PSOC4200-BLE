/*******************************************************************************
* File Name: main.c
*
* Version: 1.10
*
* Description:
*  This example project demonstrates capability of the SCB component to be
*  reconfigured between multiple communication interfaces during run time.
*  This is done using the Unconfigured mode of the SCB component.
*  In this example, the component switches between I2C and UART modes to
*  execute SCB_UartComm and SCB_I2cCommSlave example projects using a
*  shared pair of communication IO’s.
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

#include <main.h>

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static cystatus ConfigurationChange(uint32 opMode);
static void RunI2CExample(void);
static void RunUartExample(void);
static uint32 ReadSwSwitch(void);
static uint8 ExecuteCommand(uint32 cmd);


/*******************************************************************************
* Common Definitions
*******************************************************************************/
/* Constants */
#define ENABLED         (1u)
#define DISABLED        (0u)
#define NON_APPLICABLE  (DISABLED)

/* Common RX and TX buffers for I2C and UART operation */
#define COMMON_BUFFER_SIZE     (16u)
uint8 bufferTx[COMMON_BUFFER_SIZE];

/* UART RX buffer requires one extra element for proper operation. One element
* remains empty while operation. Keeping this element empty simplifies
* circular buffer operation.
*/
uint8 bufferRx[COMMON_BUFFER_SIZE + 1u];


/*******************************************************************************
* I2C Configuration
*******************************************************************************/
#define I2C_SLAVE_ADDRESS       (0x08u)
#define I2C_SLAVE_ADDRESS_MASK  (0xFEu)
#define I2C_STANDARD_MODE_MAX   (100u)

#define I2C_RX_BUFFER_SIZE      (PACKET_SIZE)
#define I2C_TX_BUFFER_SIZE      (PACKET_SIZE)
#define I2C_RX_BUFER_PTR        bufferRx
#define I2C_TX_BUFER_PTR        bufferTx

/* I2C slave desired data rate is 100 kbps. The datasheet Table 1 provides a
* range of possible clock values 1.55 - 12.8 MHz. The CommCLK = 1.6 MHz is
* selected from this range. The clock divider has to be calculated to control
* clock frequency as clock component provides interface to it.
* Divider = (HFCLK / CommCLK) = (24MHz / 1.6 MHz) = 15. But the value written
* into the register has to decremented by 1. The end result is 14.
* The divider to create CommCLK for HFCLK = 12 MHz is two times less than for 
* HFCLK = 24 MHz and equal to 7. The value written into the register is 6. The 
* resulting CommCLK = 1.714MHz which meets data rate requirements.
*/
#if (24u == CYDEV_BCLK__HFCLK__MHZ)
    #define I2C_CLK_DIVIDER         (14u)
#elif (12u == CYDEV_BCLK__HFCLK__MHZ)
    #define I2C_CLK_DIVIDER         (6u)
#else
    #define I2C_CLK_DIVIDER         (0u)
#endif /* (24u == CYDEV_BCLK__HFCLK__MHZ) */

/* Comm_I2C_INIT_STRUCT provides the fields which match the selections available
* in the customizer. Refer to the I2C customizer for detailed description of
* the settings.
*/
const Comm_I2C_INIT_STRUCT configI2C =
{
    Comm_I2C_MODE_SLAVE,    /* mode: slave */
    NON_APPLICABLE,         /* oversampleLow: N/A for slave */
    NON_APPLICABLE,         /* oversampleHigh: N/A for slave */
    NON_APPLICABLE,         /* enableMedianFilter: N/A */
    I2C_SLAVE_ADDRESS,      /* slaveAddr: slave address */
    I2C_SLAVE_ADDRESS_MASK, /* slaveAddrMask: single slave address */
    DISABLED,               /* acceptAddr: disabled */
    DISABLED,               /* enableWake: disabled */
    DISABLED,               /* enableByteMode: disabled */
    I2C_STANDARD_MODE_MAX,  /* dataRate: 100 kbps */
    DISABLED,               /* acceptGeneralAddr */
};


/*******************************************************************************
* UART Configuration
*******************************************************************************/
#define UART_OVERSAMPLING       (13u)
#define UART_DATA_WIDTH         (8u)
#define UART_RX_INTR_MASK       (Comm_INTR_RX_NOT_EMPTY)
#define UART_TX_INTR_MASK       (0u)

#define UART_RX_BUFFER_SIZE     (COMMON_BUFFER_SIZE)
#define UART_TX_BUFFER_SIZE     (COMMON_BUFFER_SIZE)
#define UART_RX_BUFER_PTR       bufferRx
#define UART_TX_BUFER_PTR       bufferTx

/* UART desired baud rate is 115200 bps. The selected Oversampling parameter is
* 16. The CommCLK = Baud rate * Oversampling = 115200 * 13 = 1.4976 MHz.
* The clock divider has to be calculated to control clock frequency as clock
* component provides interface to it.
* Divider = (HFCLK / CommCLK) = (24 MHz / 1.4976 MHz) = 16. But the devider 
* value written into the register has to decremented by 1 therefore end value 
* is 15.
* The clock accuracy is important for UART operation. The actual CommCLK equal:
* CommCLK(actual) = (24 MHz / 16) = 1.5 MHz
* The deviation of actual CommCLK from desired must be calculated:
* Deviation = (1.5 MHz – 1.4976 MHz) / 1.4976 MHz = ~0.16%
* Taking into account HFCLK accuracy ±2%, the total error is:0.2 + 0.16 = 2.16%.
* The total error value is less than 5% and it is enough for correct
* UART operation.
* The divider to create CommCLK for HFCLK = 12 MHz is two times less than for 
* HFCLK = 24 MHz and equal to 8. The value written into the register is 7.
*/
#if (24u == CYDEV_BCLK__HFCLK__MHZ)
    #define UART_CLK_DIVIDER        (15u)
#elif (12u == CYDEV_BCLK__HFCLK__MHZ)
    #define UART_CLK_DIVIDER        (7u)
#else
    #error For correct SCB_UnconfiguredComm code example operation the HFCLK \
           frequency must be 24 or 12 MHz. Open Design Wide Resources file   \
           Clocks tab and change HFCLK to match requirement.
    
    #define UART_CLK_DIVIDER        (0u)
#endif /* (24u == CYDEV_BCLK__HFCLK__MHZ) */

/* Comm_UART_INIT_STRUCT provides the fields which match the selections
* available in the customizer. Refer to the I2C customizer for detailed
* description of the settings.
*/
const Comm_UART_INIT_STRUCT configUart =
{
    Comm_UART_MODE_STD,     /* mode: Standard */
    Comm_UART_TX_RX,        /* direction: RX + TX */
    UART_DATA_WIDTH,        /* dataBits: 8 bits */
    Comm_UART_PARITY_NONE,  /* parity: None */
    Comm_UART_STOP_BITS_1,  /* stopBits: 1 bit */
    UART_OVERSAMPLING,      /* oversample: 16 */
    DISABLED,               /* enableIrdaLowPower: disabled */
    DISABLED,               /* enableMedianFilter: disabled */
    DISABLED,               /* enableRetryNack: disabled */
    DISABLED,               /* enableInvertedRx: disabled */
    DISABLED,               /* dropOnParityErr: disabled */
    DISABLED,               /* dropOnFrameErr: disabled */
    NON_APPLICABLE,         /* enableWake: disabled */
    UART_RX_BUFFER_SIZE,    /* rxBufferSize: TX software buffer size */
    UART_RX_BUFER_PTR,      /* rxBuffer: pointer to RX software buffer */
    UART_TX_BUFFER_SIZE,    /* txBufferSize: TX software buffer size */
    UART_TX_BUFER_PTR,      /* txBuffer: pointer to TX software buffer */
    DISABLED,               /* enableMultiproc: disabled */
    DISABLED,               /* multiprocAcceptAddr: disabled */
    NON_APPLICABLE,         /* multiprocAddr: N/A */
    NON_APPLICABLE,         /* multiprocAddrMask: N/A */
    ENABLED,                /* enableInterrupt: enable internal interrupt
                             * handler for the software buffer */
    UART_RX_INTR_MASK,      /* rxInterruptMask: enable INTR_RX.NOT_EMPTY to
                             * handle RX software buffer operations */
    NON_APPLICABLE,         /* rxTriggerLevel: N/A */
    UART_TX_INTR_MASK,      /* txInterruptMask: no TX interrupts on start up */
    NON_APPLICABLE,         /* txTriggerLevel: N/A */
    DISABLED,               /* enableByteMode: disabled */
    DISABLED,               /* enableCts: disabled */
    DISABLED,               /* ctsPolarity: disabled */
    DISABLED,               /* rtsRxFifoLevel: disabled */
    DISABLED,               /* rtsPolarity: disabled */
};

/* Global variables to manage current operation mode and initialization state */
uint32 mode = OP_MODE_UART;


/*******************************************************************************
* Function Name: Main
********************************************************************************
* Summary:
*  The main function performs the following actions:
*   1. Sets SCB configuration to UART or I2C.
*   2. Executes UART or I2C example project.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main()
{
    CyGlobalIntEnable;

    for(;;)
    {
        /* Set SCB operation mode to UART or I2C. Default mode is UART */
        (void) ConfigurationChange(mode);

        if (OP_MODE_UART == mode)
        {
            /* Execute UART example project */
            RunUartExample();
        }
        else
        {
            /* Execute I2C example project */
            RunI2CExample();
        }
    }
}


/*******************************************************************************
* Function Name: ConfigurationChange
********************************************************************************
* Summary:
*  This function reconfigures the SCB component between the I2C and UART modes
*  of operation.
*
* Parameters:
*  opMode - mode of operation to which SCB component will be configured.
*
* Return:
*  Returns CYRET_SUCCESS if no problem was encountered while operation mode
*  change or CYRET_BAD_PARAM if unknown operation mode is selected.
*  The valid operation modes are: OP_MODE_I2C and OP_MODE_UART.
*
*******************************************************************************/
static cystatus ConfigurationChange(uint32 opMode)
{
    cystatus status = CYRET_SUCCESS;

    if (OP_MODE_I2C == opMode)
    {
        /***********************************************************************
        * Configure SCB in I2C mode and enable component after completion.
        ***********************************************************************/

        /* Disable component before re-configuration */
        Comm_Stop();

        /* Set clock divider to provide clock frequency to the SCB component
        * to operated with desired data rate.
        */
        CommCLK_SetFractionalDividerRegister(I2C_CLK_DIVIDER, 0u);

        /* Configure SCB component. The configuration is stored in the I2C
        * configuration structure.
        */
        Comm_I2CInit(&configI2C);

        /* Set read and write buffers for the I2C slave */
        Comm_I2CSlaveInitWriteBuf(I2C_RX_BUFER_PTR, I2C_RX_BUFFER_SIZE);
        Comm_I2CSlaveInitReadBuf (I2C_TX_BUFER_PTR, I2C_TX_BUFFER_SIZE);

        /* Put start and end of the packet into the TX buffer */
        bufferTx[PACKET_SOP_POS] = PACKET_SOP;
        bufferTx[PACKET_EOP_POS] = PACKET_EOP;

        /* Start component after re-configuration is complete */
        Comm_Start();
    }
    else if (OP_MODE_UART == opMode)
    {
        /***********************************************************************
        * Configure SCB in UART mode and enable component after completion
        ***********************************************************************/

        /* Disable component before re-configuration */
        Comm_Stop();

        /* Set clock divider to provide clock frequency to the SCB component
        * to operated with desired data rate.
        */
        CommCLK_SetFractionalDividerRegister(UART_CLK_DIVIDER, 0u);

        /* Configure SCB component. The configuration is stored in the UART
        * configuration structure.
        */
        Comm_UartInit(&configUart);

        /* Start component after re-configuration is complete */
        Comm_Start();
    }
    else
    {
        status = CYRET_BAD_PARAM; /* Unknown operation mode - no action */
    }

    return (status);
}


/*******************************************************************************
* Function Name: RunUartExample
********************************************************************************
* Summary:
*  Executes UART example project (SCB_UartComm) until configuration change
*  occurs. The UART example project simply echoes any received character.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void RunUartExample(void)
{
    char8 ch;

    RGB_LED_OFF;

    /* Print start of the example project header in the terminal */
    Comm_UartPutString("\r\n********************************************************************************\r\n");
    Comm_UartPutString("This is SCB_UartComm datasheet example project\r\n");
    Comm_UartPutString("If you are able to read this text the terminal connection is configured\r\n");
    Comm_UartPutString("correctly. Start transmitting the characters to see an echo in the terminal.\r\n");
    Comm_UartPutString("\r\n");

    for(;;)
    {
        /***********************************************************************
        * Loopback (echo) incoming UART characters
        ***********************************************************************/

        /* Get received character or zero if nothing has been received yet */
        ch = Comm_UartGetChar();

        if (0u != ch)
        {
            /* Send the data through UART. This function is blocking and waits
            * until there is an entry into the TX FIFO to put the character.
            */
            Comm_UartPutChar(ch);
        }

        /***********************************************************************
        * Change configuration on the switch press event
        ***********************************************************************/
        if (0u != ReadSwSwitch())
        {
            /* Print end of the example project header in the terminal */
            Comm_UartPutString("\r\n********************************************************************************\r\n");
            Comm_UartPutString("SCB_UartComm datasheet example project ends its operation. The mode is changed to\r\n");
            Comm_UartPutString("I2C and datasheet example project SCB_I2cCommSlave starts operation.\r\n");
            Comm_UartPutString("Run Bridge Control Panel to communicate with I2C slave.\r\n");
            Comm_UartPutString("********************************************************************************\r\n");

            /* Wait until UART completes transfer string */
            CyDelay(WAIT_FOR_END_UART_OUTPUT);

            /* Change configuration to I2C */
            mode = OP_MODE_I2C;
            break;
        }
    }
}


/*******************************************************************************
* Function Name: RunI2CExample
********************************************************************************
* Summary:
*  Executes I2C example project (SCB_I2cCommSlave) until configuration change
*  occurs. The I2C example project receives a single command byte from an
*  I2C master to control the state of the RGB LED using the ExecuteCommand()
*  function.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void RunI2CExample(void)
{
    uint8 status;

    /* Loop until switch is pressed to change configuration */
    for(;;)
    {
        /***********************************************************************
        * Handle master write completion event
        ***********************************************************************/
        if (0u != (Comm_I2CSlaveStatus() & Comm_I2C_SSTAT_WR_CMPLT))
        {
            status = STS_CMD_FAIL;

            /* Check packet length */
            if (PACKET_SIZE == Comm_I2CSlaveGetWriteBufSize())
            {
                /* Check start and end of the packet markers */
                if ((bufferRx[PACKET_SOP_POS] == PACKET_SOP) &&
                    (bufferRx[PACKET_EOP_POS] == PACKET_EOP))
                {
                    status = ExecuteCommand(bufferRx[PACKET_CMD_POS]);
                }
            }

            /* Clear slave write buffer and status */
            Comm_I2CSlaveClearWriteBuf();
            (void) Comm_I2CSlaveClearWriteStatus();

            /* Update read buffer */
            bufferTx[PACKET_STS_POS] = status;
        }

        /***********************************************************************
        * Handle master read completion event
        ***********************************************************************/
        if (0u != (Comm_I2CSlaveStatus() & Comm_I2C_SSTAT_RD_CMPLT))
        {
            /* Clear slave read buffer and status */
            Comm_I2CSlaveClearReadBuf();
            (void) Comm_I2CSlaveClearReadStatus();
        }

        /***********************************************************************
        * Change configuration on the switch press event
        ***********************************************************************/
        if (0u != ReadSwSwitch())
        {
            /* Change configuration to UART */
            mode = OP_MODE_UART;
            break;
        }
    }
}


/*******************************************************************************
* Function Name: ReadSwSwitch
********************************************************************************
* Summary:
*  Reads and returns the current status of the switch.
*
* Parameters:
*  None
*
* Return:
*  Returns non-zero value if switch is pressed and zero otherwise.
*
*******************************************************************************/
static uint32 ReadSwSwitch(void)
{
    uint32 heldDown;
    uint32 swStatus;

    swStatus = 0u;  /* Switch is not active */
    heldDown = 0u;  /* Reset debounce counter */

    /* Wait for debounce period before determining that the switch is pressed */
    while (SWITCH_ON)
    {
        /* Count debounce period */
        CyDelay(SWITCH_DEBOUNCE_UNIT);
        ++heldDown;

        if (heldDown > SWITCH_PUSH)
        {
            swStatus = 1u; /* Switch is pressed */
            break;
        }
    }

    return (swStatus);
}


/*******************************************************************************
* Function Name: ExecuteCommand
********************************************************************************
* Summary:
*  Executes received command to control the LED color and returns status.
*  If the command is unknown, the LED color is not changed.
*
* Parameters:
*  cmd: command to execute. Available commands:
*   - CMD_SET_RED:   set red color of the LED.
*   - CMD_SET_GREEN: set green color of the LED.
*   - CMD_SET_BLUE:  set blue color of the LED.
*   - CMD_SET_OFF:   turn off the LED.
*
* Return:
*  Returns status of command execution. There are two statuses
*  - STS_CMD_DONE: command is executed successfully.
*  - STS_CMD_FAIL: Unknown command
*
*******************************************************************************/
static uint8 ExecuteCommand(uint32 cmd)
{
    uint8 status;

    status = STS_CMD_DONE;

    /* Execute received command */
    switch (cmd)
    {
        case CMD_SET_RED:
            RGB_LED_ON_RED;
            break;

        case CMD_SET_GREEN:
            RGB_LED_ON_GREEN;
            break;

        case CMD_SET_BLUE:
            RGB_LED_ON_BLUE;
            break;

        case CMD_SET_OFF:
            RGB_LED_OFF;
            break;

        default:
            status = STS_CMD_FAIL;
            break;
    }

    return (status);
}


/* [] END OF FILE */
