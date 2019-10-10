#!/bin/sh

echo "Running overlayPinConfig.sh to set pinmux and start pwm"; \
./SupportingMaterial/OverlayScripts/overlayPinConfig.sh; \
echo "Running GRB422-QVGA.sh to configure camera"; \
./SupportingMaterial/CameraManipulationScripts/GRB422-QVGA.sh; \
./SupportingMaterial/CameraManipulationScripts/SetViewport.sh; \
echo "Launching BioloidBeaglebone"; \
./BioloidBeaglebone PRU_0.out PRU_1.out ./SupportingMaterial/MotionFiles/FlexTest.mtn ./SupportingMaterial/MLFiles/PersonDogCatBirdYOLO/voc.names ./SupportingMaterial/MLFiles/PersonDogCatBirdYOLO/yolov3-tiny_obj.cfg ./SupportingMaterial/MLFiles/PersonDogCatBirdYOLO/yolov3-tiny_obj_final.weights;
