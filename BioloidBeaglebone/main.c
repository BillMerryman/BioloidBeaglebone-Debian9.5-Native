/** @file main.c
 *  @brief Initialization and system starting point.
 *
 *  Right now we just initialize the PRUs and loop through looking for keyboard input
 *  to initiate a servo action (like playing a page), and taking a frame of video and
 *  processing it with OpenCV color range matching and moments recognition.
 *
 *  @author Bill Merryman
 *  @bug No known bugs.
 *
 *  Created on: Oct 16, 2015
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "cv.h"
#include "highgui.h"
#include "pru.h"
#include "PRUInterop.h"
#include "motionManager.h"
#include "visionManager.hpp"

int main (int argc, char *argv[])
{

	if(argc < 13)
	{
		fprintf(stderr, "Usage: BioloidBeaglebone PRU0Firmware PRU1Firmware MotionFile MLCaffeNamesFile MLProtoFile MLCaffeFile MLCaffeConfidence MLDarknetNamesFile MLDarknetCfgFile MLDarknetWeightsFile MLDarknetConfidence MLDarknetNMSThreshold");
		return -1;
	}

	int key = 0;

	const char *PRU_0Firmware = argv[1];
	const char *PRU_1Firmware = argv[2];
	const char *motionFile = argv[3];
	const char *caffeNamesFile = argv[4];
	const char *protoFile = argv[5];
	const char *modelFile = argv[6];
	float caffeConfidence = atof(argv[7]);
	const char *darknetNamesFile = argv[8];
	const char *cfgFile = argv[9];
	const char *weightsFile = argv[10];
	float darknetConfidence = atof(argv[11]);
	float darknetNMSThreshold = atof(argv[12]);

	initializePRU(PRU_0Firmware, PRU_1Firmware);

	motionManagerInitialize(motionFile);
	visionManagerInitialize(caffeNamesFile,
							protoFile,
							modelFile,
							caffeConfidence,
							darknetNamesFile,
							cfgFile,
							weightsFile,
							darknetConfidence,
							darknetNMSThreshold);

	while(key != 'x')
	{
		visionManagerProcess(key);
		motionManagerProcess(key);
		key = cvWaitKey(25);
	}

	visionManagerUninitialize();
	stopPRU_0();
	stopPRU_1();
}
