/** @file PRUInterop1.c
 *  @brief 
 *
 *
 *
 *  @author Bill
 *  @bug No known bugs.
 *
 *  Created on: Jan 16, 2018
 *
 */

#include "PRUInterop1.h"
#include "LED.h"
#include "resource_table.h"

uint32_t *PRUInterop1GetImageData(void)
{
	LED_OFF(PIN_NUMBER_FOR_LED_0);
	if(resourceTable.carveout.pa == 0)
	{
		LED_ON(PIN_NUMBER_FOR_LED_0);
		while(1);
	}
	PRU_INTEROP_1_DATA *PRUInterop1Data = (PRU_INTEROP_1_DATA *)resourceTable.carveout.pa;
	return (uint32_t *)(PRUInterop1Data->imageData);
}

uint32_t *PRUInterop1GetImageReadyFlag(void)
{
	LED_OFF(PIN_NUMBER_FOR_LED_0);
	if(resourceTable.carveout.pa == 0)
	{
		LED_ON(PIN_NUMBER_FOR_LED_0);
		while(1);
	}
	PRU_INTEROP_1_DATA *PRUInterop1Data = (PRU_INTEROP_1_DATA *)resourceTable.carveout.pa;
	return (&(PRUInterop1Data->imageReadyFlag));
}
