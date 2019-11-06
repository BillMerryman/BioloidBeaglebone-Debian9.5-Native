#!/bin/sh

echo "Running overlayPinConfig.sh to set pinmux and start pwm"; \
./SupportingMaterial/OverlayScripts/overlayPinConfig.sh; \
echo "Running GRB422-QVGA.sh to configure camera"; \
./SupportingMaterial/CameraManipulationScripts/GRB422-QVGA.sh; \
./SupportingMaterial/CameraManipulationScripts/SetViewport.sh; \
echo "Launching BioloidBeaglebone"; \
./BioloidBeaglebone PRU_0.out PRU_1.out ./SupportingMaterial/MotionFiles/FlexTest.mtn ./SupportingMaterial/MLFiles/PersonDogCatBirdMobilenetSSD/voc.names ./SupportingMaterial/MLFiles/PersonDogCatBirdMobilenetSSD/deploy.prototxt ./SupportingMaterial/MLFiles/PersonDogCatBirdMobilenetSSD/VGG_VOC0712_SSD_300x300_iter_100000_detection_eval-p792.caffemodel;
