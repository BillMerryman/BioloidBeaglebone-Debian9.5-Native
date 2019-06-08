/** @file visionManager.c
 *  @brief Functions for managing images/vision.
 *
 *  These functions currently setup the pointers to where the PRU will write image data
 *  and an image data ready flag, process and display the image data from the shared memory
 *  in OpenCV windows, and garbage collect the image data when the program shuts down.
 *  The image ready flag is a cheap way to deal with the fact
 *  that the PRU can write data to the shared memory much faster than the application
 *  processor can consume it. We will want to come up with a better way of working with images
 *  on the application processor side. Image processing is currently just using OpenCV
 *  cvInRangeS function to mask a certain color (aibo ball pink) and cvMoments to identify
 *  instances of it in the image. The initialize function creates two windows, one for the
 *  image and one for the thresholded mask. The main loop updates these windows when the image
 *  ready flag is set.
 *
 *  @author Bill Merryman
 *  @bug No known bugs.
 *
 *  Created on: Nov 19, 2017
 *
 */

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/dnn.hpp"
#include "opencv2/dnn/shape_utils.hpp"
#include "visionManager.hpp"

extern "C"
{
#include "PRUInterop.h"
#include "pru.h"
}

using namespace std;
using namespace cv;
using namespace cv::dnn;

volatile int *imageReadyFlag;
cv::Mat sourceImage;
cv::Mat maskImage;
Net net;
Size resized(320, 240);

const String CLASSES[] = {"background",
							"aeroplane",
							"bicycle",
							"bird",
							"boat",
							"bottle",
							"bus",
							"car",
							"cat",
							"chair",
							"cow",
							"diningtable",
							"dog",
							"horse",
							"motorbike",
							"person",
							"pottedplant",
							"sheep",
							"sofa",
							"train",
							"tvmonitor"};

void visionManagerInitialize(const char *protoFile, const char *caffeFile)
{
	CvSize inputSize;
	inputSize.width = IMAGE_COLUMNS_IN_PIXELS;
	inputSize.height = IMAGE_ROWS_IN_PIXELS;

	PRU_INTEROP_1_DATA* PRUInterop1Data = getPRUInterop1Data();

	sourceImage = cv::Mat(inputSize, CV_8UC3, (void*)(PRUInterop1Data->imageData));
	maskImage = cv::Mat(inputSize, CV_8UC3);
	imageReadyFlag = ((int *)(&(PRUInterop1Data->imageReadyFlag)));
	net = cv::dnn::readNetFromCaffe(protoFile, caffeFile);
	cvNamedWindow("main", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("mask", CV_WINDOW_AUTOSIZE);
}

void visionManagerUninitialize()
{
	cvDestroyWindow("main");
	cvDestroyWindow("mask");
}

void visionManagerProcess()
{
	if(*imageReadyFlag == IMAGE_NOT_READY) return;
	visionManagerProcessDNN();
	*imageReadyFlag = IMAGE_NOT_READY;
}

void visionManagerProcessThreshold()
{
	double area = 0;
	CvPoint position;
	char outputMessage[50];

	inRange(sourceImage, Scalar(60, 0, 100, 0), Scalar(200, 80, 255, 0), maskImage);
	cv::Moments moments = cv::moments(maskImage, false);
	area = moments.m00;
	if (area > 1000000)
	{
		position.x = moments.m10 / area;
		position.y = moments.m01 / area;
		sprintf(outputMessage, "pos: %d, %d", position.x, position.y);
		rectangle(sourceImage, cvPoint(position.x - 5, position.y - 5), cvPoint(position.x + 5, position.y + 5), cvScalar(0, 255, 0, 0), 1, 8, 0);
		putText(sourceImage, outputMessage, Point(position.x + 10, position.y + 5), CV_FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0), 2, 8, false);
	}
	imshow("main", sourceImage);
	imshow("mask", maskImage);
}

void visionManagerProcessDNN()
{
	Mat resizedImage;
	Point position;
	static double outputColor = 0;

	char outputMessage[64];

	if(*imageReadyFlag == IMAGE_NOT_READY) return;

	(outputColor == 0) ? outputColor = 255 : outputColor = 0;

	resize(sourceImage, resizedImage, resized, 0, 0, CV_INTER_LINEAR);
	Mat blob = cv::dnn::blobFromImage(resizedImage,
						0.007843f,
						resized,
						Scalar(127.5));
	net.setInput(blob);
	Mat detections = net.forward();
	for(int i = 0; i < detections.size[2]; i++)
	{
		int idxConf[4] = {0, 0, i, 2};
		float conf = detections.at<float>(idxConf);

		if(conf > 0.5f)
		{
			int idxCls[4] = {0, 0, i, 1};
			int cls = detections.at<float>(idxCls);

			int leftPercent[4] = {0, 0, i, 3};
			int topPercent[4] = {0, 0, i, 4};
			int widthPercent[4] = {0, 0, i, 5};
			int heightPercent[4] = {0, 0, i, 6};

			position.x = detections.at<float>(leftPercent) * resized.width;
			position.y = detections.at<float>(topPercent) * resized.height;
			int width = (detections.at<float>(widthPercent) * resized.width) - position.x;
			int height = (detections.at<float>(heightPercent) * resized.height) - position.y;

			Rect detection(position.x, position.y, width, height);
			rectangle(resizedImage, detection, Scalar(0, outputColor, outputColor), 1, 8, 0);
			putText(resizedImage, CLASSES[cls], Point(position.x, position.y + 10), CV_FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, outputColor, outputColor), 2, 8, false);
		}
	}
	imshow("main", resizedImage);
}
