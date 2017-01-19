/*******************************************************************************
* File Name: main.h
*
* Version: 1.10
*
* Description:
*  This file provides function prototypes, constants and macros for the
*  SCB Unconfigured Mode example project.
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

#if !defined(CY_MAIN_H)
#define CY_MAIN_H

#include <project.h>


/***************************************
*            Constants
****************************************/

/* Operation mode: I2C slave or UART */
#define OP_MODE_UART    (0u)
#define OP_MODE_I2C     (1u)

/* Packet size for I2C */
#define PACKET_SIZE     (3u)

/* Byte position within the packet */
#define PACKET_SOP_POS  (0u)
#define PACKET_CMD_POS  (1u)

/* Command and status share the same offset */
#define PACKET_STS_POS  (PACKET_CMD_POS)
#define PACKET_EOP_POS  (2u)

/* Start and end of the packet markers */
#define PACKET_SOP      (0x01u)
#define PACKET_EOP      (0x17u)

/* Command execution status */
#define STS_CMD_DONE    (0x00u)
#define STS_CMD_FAIL    (0xFFu)

/* Commands */
#define CMD_SET_OFF     (0u)
#define CMD_SET_RED     (1u)
#define CMD_SET_GREEN   (2u)
#define CMD_SET_BLUE    (3u)
#define CMD_SWITCH_UART (4u)

/* Delay to unsure UART complete transmitting in milliseconds */
#define WAIT_FOR_END_UART_OUTPUT    (10u)

/* Number of debounce units to count before consider that switch is pressed */
#define SWITCH_PUSH     (80u)

/* Switch debounce delay in milliseconds */
#define SWITCH_DEBOUNCE_UNIT   (1u)


/***************************************
*               Macros
****************************************/

/* Return true if switch is pressed */
#define SWITCH_ON       (0u == SW2_Read())

/* Set LED RED color */
#define RGB_LED_ON_RED  \
                do{     \
                    LED_RED_Write  (0u); \
                    LED_GREEN_Write(1u); \
                    LED_BLUE_Write (1u); \
                }while(0)

/* Set LED GREEN color */
#define RGB_LED_ON_GREEN \
                do{      \
                    LED_RED_Write  (1u); \
                    LED_GREEN_Write(0u); \
                    LED_BLUE_Write (1u); \
                }while(0)

/* Set LED BLUE color */
#define RGB_LED_ON_BLUE \
                do{     \
                    LED_RED_Write  (1u); \
                    LED_GREEN_Write(1u); \
                    LED_BLUE_Write (0u); \
                }while(0)

/* Set LED TURN OFF */
#define RGB_LED_OFF \
                do{ \
                    LED_RED_Write  (1u); \
                    LED_GREEN_Write(1u); \
                    LED_BLUE_Write (1u); \
                }while(0)


#endif /* (CY_MAIN_H) */


/* [] END OF FILE */
