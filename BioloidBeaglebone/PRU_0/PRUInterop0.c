/** @file PRUInterop0.c
 *  @brief 
 *
 *
 *
 *  @author Bill
 *  @bug No known bugs.
 *
 *  Created on: Jan 14, 2018
 *
 */

#include "PRUInterop0.h"
#include "resource_table.h"
#include "LED.h"

MOTION_PAGE* PRUInterop0GetMotionPages(void)
{
	LED_OFF(PIN_NUMBER_FOR_LED_0);
	if(resourceTable.carveout.pa == 0)
	{
		LED_ON(PIN_NUMBER_FOR_LED_0);
		while(1);
	}
	PRU_INTEROP_0_DATA *PRUInterop0Data = (PRU_INTEROP_0_DATA *)resourceTable.carveout.pa;
	return PRUInterop0Data->motionPages;
}

uint8_t* PRUInterop0GetMotionInstruction(void)
{
	LED_OFF(PIN_NUMBER_FOR_LED_0);
	if(resourceTable.carveout.pa == 0)
	{
		LED_ON(PIN_NUMBER_FOR_LED_0);
		while(1);
	}
	PRU_INTEROP_0_DATA *PRUInterop0Data = (PRU_INTEROP_0_DATA *)resourceTable.carveout.pa;
	return &(PRUInterop0Data->motionInstruction);
}

uint8_t* PRUInterop0GetMotionArgument(void)
{
	LED_OFF(PIN_NUMBER_FOR_LED_0);
	if(resourceTable.carveout.pa == 0)
	{
		LED_ON(PIN_NUMBER_FOR_LED_0);
		while(1);
	}
	PRU_INTEROP_0_DATA *PRUInterop0Data = (PRU_INTEROP_0_DATA *)resourceTable.carveout.pa;
	return &(PRUInterop0Data->motionArgument);
}
