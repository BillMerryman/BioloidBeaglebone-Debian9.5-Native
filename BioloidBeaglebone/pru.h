/** @file pru.h
 *  @brief Function prototypes for initializing, loading, starting, and stopping a PRU.
 *
 *  These are the function prototypes that: initialize the PRU subsystem, load a program
 *  compiled with the PRU C compiler into the program memory of a given PRU,
 *  initiate execution of a program in a given PRU, and terminate execution of
 *  a program in a given PRU.
 *
 *  @author Bill Merryman
 *  @bug No known bugs.
 *
 *  Created on: Sep 8, 2015
 *
 */

#ifndef PRU_H_
#define PRU_H_

#include "PRUInterop.h"

/** @brief Gets a pointer to the PRU driver allocated memory
 *
 * 	Gets a pointer to the PRU driver allocated memory that will be used as shared memory
 * 	between the PRU subsystem and the application processor.
 *
 * 	@return A pointer to the PRU driver allocated memory as a structure (pointer of type PRU_INTEROP_DATA).
 *
 */
PRU_INTEROP_0_DATA *getPRUInterop0Data();
PRU_INTEROP_1_DATA *getPRUInterop1Data();

/** @brief Initializes the PRU subsystem.
 *
 * 	We start by calling some setup functions of the prussdrv TI (Texas Instruments) provided
 * 	code. This ensures that the necessary structures are set up to get the memory mappings, and
 * 	that the PRUs can access system wide resources. Importantly, this function gets the
 * 	virtual mapping to the PRU driver allocated memory for the application processor to access it,
 * 	and the physical mapping of this same PRU driver allocated memory so that the PRU subsystem
 * 	can access it. Some of this initialization may be superfluous, as we aren't really using
 * 	interrupts between the PRU subsystem and the application processor.
 *
 * 	@return	void.
 *
 */
void initializePRU(const char *PRU_0_Firmware, const char *PRU_1_Firmware);

/** @brief 	Load a program into PRU0
 *
 * 	Loads the program files into the program memory of the PRUs
 *
 * 	@return void.
 *
 */
void configurePRU_0(const char *pruFirmware);

/** @brief 	Start PRU0
 *
 * 	This starts the PRU0 microcontroller running, initiating execution of the PRU0 program.
 *
 * 	@return void.
 *
 */
void startPRU_0(void);

/** @brief 	Stop PRU0
 *
 * 	This halts the PRU0 microcontroller, stopping execution of the PRU0 program.
 *
 * 	@return void.
 *
 */
void stopPRU_0(void);

/** @brief 	Load a program into PRU1
 *
 * 	Loads a program file into the program memory of PRU1, and the accompanying data file
 * 	into the data memory of PRU1
 *
 * 	@return void.
 *
 */
void configurePRU_1(const char *pruFirmware);

/** @brief 	Start PRU1
 *
 * 	This starts the PRU1 microcontroller running, initiating execution of the PRU1 program.
 *
 * 	@return void.
 *
 */
void startPRU_1(void);

/** @brief 	Stop PRU1
 *
 * 	This halts the PRU1 microcontroller, stopping execution of the PRU1 program.
 *
 * 	@return void.
 *
 */
void stopPRU_1(void);

#endif /* PRU_H_ */
