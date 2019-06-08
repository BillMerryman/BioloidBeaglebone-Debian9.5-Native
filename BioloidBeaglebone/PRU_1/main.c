#include <stdint.h>
#include <stdio.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>
#include "LED.h"
#include "image.h"
#include "PRUInterop1.h"

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 */
#define TO_ARM_HOST			18	
#define FROM_ARM_HOST			19

volatile unsigned int *imageReadyFlag;

int main()
{

	/* Allow OCP master port access by the PRU so the PRU can read external memories */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	/* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
	CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

	imageInitialize();
	imageReadyFlag = PRUInterop1GetImageReadyFlag();
	*imageReadyFlag = IMAGE_NOT_READY;

	while(1)
	{
		while(*imageReadyFlag == IMAGE_READY);
		GET_IMAGE;
		*imageReadyFlag = IMAGE_READY;
		LED_TOGGLE(PIN_NUMBER_FOR_LED_0);
	}
}
