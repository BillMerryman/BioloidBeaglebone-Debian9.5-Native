/** @file motionManager.h
 *  @brief Function prototypes for managing motions.
 *
 *  These are the prototypes for functions that: setup the pointers used to
 *  communicate with the memory shared between the application processor and PRU,
 *  place requests in the shared memory to the PRU motion worker, and to load a
 *  motion file from the file system into the shared memory so the PRU motion worker
 *  can retrieve and use them.
 *
 *  @author Bill Merryman
 *  @bug No known bugs.
 *
 *  Created on: Nov 25, 2017
 *
 */

#ifndef MOTIONMANAGER_H_
#define MOTIONMANAGER_H_

#include "PRUInterop.h"

/** @brief Initializes the motion subsystem
 *
 * 	Currently gets pointers to the portion of PRU driver allocated memory that will
 * 	be used for storing pages and communicating between the application processor and
 * 	the PRU that will be managing the actuators. It also currently loads the motion page
 * 	file into the portion of PRU allocated memory we set aside for that.
 *
 *	@param	motionFile the file containing motion pages.
 * 	@return void.
 *
 */
void motionManagerInitialize(const char *motionFile);

/** @brief Un-initializes the motion subsystem
 *
 * 	Currently this function is not implemented, as it is not necessary.
 *
 * 	@return void.
 *
 */
void motionManagerUninitialize();

/** @brief Processes a command initiated by a key press
 *
 * 	Currently takes a character that is interpreted as a command so we can command
 * 	the motion subsystem to perform a motion page.
 *
 *	@param	key	The character entered at the keyboard representing the requested page
 *				to be performed.
 * 	@return void.
 *
 */
void motionManagerProcess(char key);

/** @brief A function to load a motion file from file system
 *
 * 	This function opens a motion file in the file system and does a binary read
 * 	directly from the file to the portion of PRU driver allocated memory that we
 * 	reserve for holding a motion file. Currently no attempt is made to validate
 * 	the file or its contents.
 *
 *	@param	filename	The full path in the file system of a motion file to load
 *						that can be read and performed by the motion subsystem.
 * 	@return void.
 *
 */
void motionManagerLoadFile(const char *filename);

#endif /* MOTIONPROCESSOR_H_ */
