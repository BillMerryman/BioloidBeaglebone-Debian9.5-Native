/** @file clock.c
 *  @brief Functions for initializing, starting, and stopping a timer
 *
 *  These functions initialize, set the interval of, start, and stop the
 *  Industrial Ethernet Peripheral (IEP) timer to be used to time the updating
 *  of position information and transmitting that updated information to the attached AX-12s.
 *
 *  @author Bill Merryman
 *  @bug No known bugs.
 *
 *  Created on: Jan 23, 2016
 *
 */

#include <stdint.h>

#include "common.h"

#include "TI_Headers/hw_types.h"
#include "TI_Headers/pru_iep.h"

#include "clock.h"

extern volatile pruIep CT_IEP;

void clockInitialize(void)
{

	clockStop();

	/* Reset Count register */
	CT_IEP.TMR_CNT = 0x0;

	/* Clear overflow status register */
	CT_IEP.TMR_GLB_STS_bit.CNT_OVF = 0x1;

	/* Disable compensation */
	CT_IEP.TMR_COMPEN_bit.COMPEN_CNT = 0x0;

	/* Enable CMP0 and reset on event */
	CT_IEP.TMR_CMP_CFG_bit.CMP0_RST_CNT_EN = 0x1;
	CT_IEP.TMR_CMP_CFG_bit.CMP_EN = 0x1;

	/* Clear compare status */
	CT_IEP.TMR_CMP_STS_bit.CMP_HIT = 0xFF;

}

void clockSet(uint32_t interval)
{

	/* Set compare value */
	CT_IEP.TMR_CMP0 = interval;

}

void clockStart(void)
{

	/* Enable counter */
	CT_IEP.TMR_GLB_CFG = 0x11;

}

void clockStop(void)
{

	/* Disable counter */
	CT_IEP.TMR_GLB_CFG_bit.CNT_EN = 0;

}

bool clockIsExpired(void)
{
	if(CT_IEP.TMR_CMP_STS_bit.CMP_HIT & 0x01)
	{
		CT_IEP.TMR_CMP_STS_bit.CMP_HIT |= 0x01;
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

