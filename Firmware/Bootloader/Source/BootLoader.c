/*********************************************************************
 *
 *                  PIC32 Boot Loader
 *
 *********************************************************************
 * FileName:        Bootloader.c
 * Dependencies:
 * Processor:       PIC32
 *
 * Compiler:        MPLAB C32
 *                  MPLAB IDE
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the �Company�) for its PIC32 Microcontroller is intended
 * and supplied to you, the Company�s customer, for use solely and
 * exclusively on Microchip PIC32 Microcontroller products.
 * The software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *
 * $Id:  $
 * $Name: $
 *
 **********************************************************************/
#include "Include\GenericTypeDefs.h"
#include "Include\HardwareProfile\HardwareProfile.h"
#include "Include\Bootloader.h"
#include "Include\FrameWork\Framework.h"
#include <stdlib.h>
#include <plib.h>


// Configuring the Device Configuration Registers
// 80Mhz Core/Periph, Pri Osc w/PLL, Write protect Boot Flash
    #pragma config PMDL1WAY = OFF       //Allow multiple reconfigurations of peripherial mod disable
    #pragma config IOL1WAY = OFF        //Allow multiple PPS reconfigs
    #pragma config UPLLEN   = ON        // USB PLL Enabled 
    #pragma config UPLLIDIV = DIV_1     // USB PLL Input Divider = Divide by 2
    #pragma config FPLLMUL  = MUL_20        // PLL Multiplier
    #pragma config FPLLIDIV = DIV_1         // PLL Input Divider
    #pragma config FPLLODIV = DIV_2         // PLL Output Divider
    #pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
    #pragma config FWDTEN   = OFF           // Watchdog Timer
    #pragma config WDTPS    = PS1           // Watchdog Timer Postscale
    #pragma config FCKSM    = CSECMD        // Clock Switching ON & Fail Safe Clock Monitor OFF
    #pragma config OSCIOFNC = OFF           // CLKO Enable
    #pragma config POSCMOD  = XT            // Primary Oscillator
    #pragma config IESO     = OFF           // Internal/External Switch-over
    #pragma config FSOSCEN  = ON           // Secondary Oscillator Enable
    #pragma config FNOSC    = PRIPLL        // Oscillator Selection
    #pragma config CP       = OFF           // Code Protect
    #pragma config BWP      = ON           // Boot Flash Write Protect Boot load flash right protect
    #pragma config PWP      = PWP12K           // Program Flash Write Protect first 12K for boor loader
    #pragma config ICESEL   = ICS_PGx1      // ICE/ICD Comm Channel Select
    #pragma config DEBUG    = OFF           // Debugger Disabled for Starter
    #pragma config JTAGEN   = OFF


void JumpToApp(void);
BOOL ValidAppPresent(void);
/********************************************************************
* Function: 	main()
*
* Precondition: 
*
* Input: 		None.
*
* Output:		None.
*
* Side Effects:	None.
*
* Overview: 	Main entry function. If there is a trigger or 
*				if there is no valid application, the device 
*				stays in firmware upgrade mode.
*
*			
* Note:		 	None.
********************************************************************/
INT main(void)
{
        bootLoadTimeOut = BOOTLOAD_TIMEOUT;
	UINT pbClk;

	// Setup configuration
	pbClk = SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

        ANSELBbits.ANSB0 = 0;
        //mInitSwitch2();
        PGD_TRIS=1; //input
        PGD_PU=1;	//pullup on

        PGC_OUT=0;
        PGC_TRIS=0;

        int i = 1000;
        while(i > 0)
        {
            i--;
        }

        if(PGD_IN==0)//If jumper, skip bootloader
        {
            JumpToApp();
        }
        else
        {
            ANSELBbits.ANSB0 = 1;
            PGD_TRIS=1; //input
            PGD_PU=0;	//pullup on
            PGC_TRIS=1;
        }




	// Enter firmware upgrade mode if there is a trigger or if the application is not valid
	//if(!ValidAppPresent())
	//{
		// Initialize the transport layer - UART/USB/Ethernet
		TRANS_LAYER_Init(pbClk);

		while(bootLoadTimeOut > 0) // Be in loop till framework recieves "run application" command from PC
		{
                    if(ValidAppPresent())
                    {
                        bootLoadTimeOut--;
                    }
			// Enter firmware upgrade mode.
			// Be in loop, looking for commands from PC
			TRANS_LAYER_Task(); // Run Transport layer tasks
			FRAMEWORK_FrameWorkTask(); // Run frame work related tasks (Handling Rx frame, process frame and so on)
			// Blink LED (Indicates the user that bootloader is running).
                       
		}
		// Close trasnport layer.
		TRANS_LAYER_Close();
	//}


	// No trigger + valid application = run application.
	JumpToApp();

	return 0;
}

void JumpToApp(void)
{
	void (*fptr)(void);
	fptr = (void (*)(void))USER_APP_RESET_ADDRESS;
	fptr();
}


/********************************************************************
* Function: 	ValidAppPresent()
*
* Precondition:
*
* Input: 		None.
*
* Output:		TRUE: If application is valid.
*
* Side Effects:	None.
*
* Overview: 	Logic: Check application vector has
				some value other than "0xFFFFFFFF"
*
*
* Note:		 	None.
********************************************************************/
BOOL ValidAppPresent(void)
{
	DWORD *AppPtr;
	AppPtr = (DWORD *)USER_APP_RESET_ADDRESS;
	if(*AppPtr == 0xFFFFFFFF)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


/*********************End of File************************************/


