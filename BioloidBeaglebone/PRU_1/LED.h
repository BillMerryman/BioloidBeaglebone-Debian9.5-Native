/*
 * LED.h
 *
 *  Created on: Nov 4, 2017
 *      Author: Bill
 */

#ifndef LED_H_
#define LED_H_

#include "TI_Headers/soc_AM335x.h"
#include "TI_Headers/gpio_v2.h"
#include "TI_Headers/hw_types.h"

#define BANK_ADDRESS_FOR_LED_GPIOS		(SOC_GPIO_1_REGS)
#define PIN_NUMBER_FOR_LED_0        	(21)
#define PIN_NUMBER_FOR_LED_1        	(22)
#define PIN_NUMBER_FOR_LED_2        	(23)
#define PIN_NUMBER_FOR_LED_3        	(24)

#define LED_ON(LED_PIN_NUMBER)			HWREG(BANK_ADDRESS_FOR_LED_GPIOS + GPIO_DATAOUT) |= (1 << LED_PIN_NUMBER)
#define LED_OFF(LED_PIN_NUMBER)			HWREG(BANK_ADDRESS_FOR_LED_GPIOS + GPIO_DATAOUT) &= ~(1 << LED_PIN_NUMBER)
#define LED_TOGGLE(LED_PIN_NUMBER)		HWREG(BANK_ADDRESS_FOR_LED_GPIOS + GPIO_DATAOUT) ^= (1 << LED_PIN_NUMBER)

#endif /* LED_H_ */
