#!/bin/sh

echo "Running overlayPinConfig.sh to set pinmux and start pwm"; \
./SupportingMaterial/OverlayScripts/overlayPinConfig.sh; \
echo "Running GRB422-QVGA.sh to configure camera"; \
./SupportingMaterial/CameraManipulationScripts/GRB422-QVGA.sh; \
./SupportingMaterial/CameraManipulationScripts/SetViewport.sh; \
echo "Launching BioloidBeaglebone"; \
./BioloidBeaglebone \
	PRU_0.out \
	PRU_1.out \
	./SupportingMaterial/MotionFiles/FlexTest.mtn \
	./SupportingMaterial/MLFiles/PersonDogCatBirdMobilenetSSD/voc.names \
	./SupportingMaterial/MLFiles/PersonDogCatBirdMobilenetSSD/deploy.prototxt \
	./SupportingMaterial/MLFiles/PersonDogCatBirdMobilenetSSD/deploy.caffemodel \
	.5 \
	./SupportingMaterial/MLFiles/PersonDogCatBirdYOLO/voc.names \
	./SupportingMaterial/MLFiles/PersonDogCatBirdYOLO/deploy.cfg \
	./SupportingMaterial/MLFiles/PersonDogCatBirdYOLO/deploy.weights \
	.1 \
	.3;