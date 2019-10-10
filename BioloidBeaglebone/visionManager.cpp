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

#include <iostream>
#include <fstream>
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
cv::Mat displayImage;
cv::Mat processingImage;
Net net;
int imageProcessingType = 3;

float confThreshold = 0.2;
float nmsThreshold = 0.3;


const String CLASSES[] = {"background", 
				"person",
				"dog",
				"cat",
				"bird"};

std::vector<std::string> classes;
std::vector<cv::String> outputNames;

vector<String> getOutputsNames(const Net& net);
void postprocess(Mat& frame, const vector<Mat>& outs);
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame);

void visionManagerInitialize(const char *namesFile, const char *modelFile, const char *weightsFile)
{
	CvSize inputSize;
	inputSize.width = IMAGE_COLUMNS_IN_PIXELS;
	inputSize.height = IMAGE_ROWS_IN_PIXELS;

	PRU_INTEROP_1_DATA* PRUInterop1Data = getPRUInterop1Data();

	displayImage = cv::Mat(inputSize, CV_8UC3, (void*)(PRUInterop1Data->imageData));
	processingImage = cv::Mat(inputSize, CV_8UC3);
	imageReadyFlag = ((int *)(&(PRUInterop1Data->imageReadyFlag)));
	net = cv::dnn::readNet(weightsFile, modelFile);
	outputNames = getOutputsNames(net);

	ifstream ifs(namesFile);
	string line;
	while (getline(ifs, line)) classes.push_back(line);

	cvNamedWindow("Display_Image", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Processing_Image", CV_WINDOW_AUTOSIZE);
}

void visionManagerUninitialize()
{
	cvDestroyWindow("Display_Image");
	cvDestroyWindow("Processing_Image");
}

void visionManagerProcess(char key)
{
	if(key=='t') imageProcessingType=1;
	if(key=='d') imageProcessingType=2;
	if(*imageReadyFlag == IMAGE_NOT_READY) return;
	switch(imageProcessingType)
	{
		case 1:
			visionManagerProcessThreshold();
			break;
		case 2:
			visionManagerProcessCaffe();
			break;
		case 3:
			visionManagerProcessDarknet();
			break;
	}
	*imageReadyFlag = IMAGE_NOT_READY;
}

void visionManagerProcessThreshold()
{
	double area = 0;
	CvPoint position;
	char outputMessage[50];

	/*
	 * Not sure why, but in this version (Debian 9.5), trying to pass the
	 * displayImage and processingImage directly like in previous versions
	 * crashes the entire system! Cloning the images like this works fine...
	 */

	Mat display = displayImage.clone();
	Mat processing = displayImage.clone();

	inRange(display, Scalar(60, 0, 100), Scalar(200, 80, 255), processing);
	cv::Moments moments = cv::moments(processing, false);
	area = moments.m00;
	if (area > 1000000)
	{
		position.x = moments.m10 / area;
		position.y = moments.m01 / area;
		sprintf(outputMessage, "pos: %d, %d", position.x, position.y);
		rectangle(display, cvPoint(position.x - 5, position.y - 5), cvPoint(position.x + 5, position.y + 5), cvScalar(0, 255, 0, 0), 1, 8, 0);
		putText(display, outputMessage, Point(position.x + 10, position.y + 5), CV_FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 2, 8, false);
	}
	setWindowTitle("Display_Image", "Process Image By Threshold");
	setWindowTitle("Processing_Image", "Image Moments");
	imshow("Display_Image", display);
	imshow("Processing_Image", processing);
}

void visionManagerProcessCaffe()
{
	const Size resized(320, 240);
	Point position;

	char outputMessage[64];

	if(*imageReadyFlag == IMAGE_NOT_READY) return;

	//resize(displayImage, processingImage, resized, 0, 0, CV_INTER_LINEAR);
	Mat blob = cv::dnn::blobFromImage(displayImage,
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
			rectangle(displayImage, detection, Scalar(0, 255, 0), 1, 8, 0);
			putText(displayImage, CLASSES[cls], Point(position.x, position.y + 10), CV_FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 2, 8, false);
		}
	}
	setWindowTitle("Display_Image", "Process Image By DNN");
	setWindowTitle("Processing_Image", "Not Used.");
	imshow("Display_Image", displayImage);
	imshow("Processing_Image", processingImage);
}

void visionManagerProcessDarknet()
{
	const Size resized(224, 224);
	Point position;

	char outputMessage[64];

	if(*imageReadyFlag == IMAGE_NOT_READY) return;

	//resize(displayImage, processingImage, resized, 0, 0, CV_INTER_LINEAR);
	Mat blob = cv::dnn::blobFromImage(displayImage,
						0.007843f,
						resized,
						Scalar(127.5));

	//blobFromImage(frame, 
	//		blob, 
	//		1/255.0, 
	//		cvSize(inpWidth, inpHeight), 
	//		Scalar(0,0,0), 
	//		true, 
	//		false);
     
	net.setInput(blob);
	vector<Mat> outs;
	net.forward(outs, outputNames);

	// Remove the bounding boxes with low confidence
	postprocess(displayImage, outs);
	setWindowTitle("Display_Image", "Process Image By DNN");
	setWindowTitle("Processing_Image", "Not Used.");
	imshow("Display_Image", displayImage);
	imshow("Processing_Image", processingImage);
}

vector<String> getOutputsNames(const Net& net)
{
    static vector<String> names;
    if (names.empty())
    {
        //Get the indices of the output layers, i.e. the layers with unconnected outputs
        vector<int> outLayers = net.getUnconnectedOutLayers();

        //get the names of all the layers in the network
        vector<String> layersNames = net.getLayerNames();

        // Get the names of the output layers in names
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i)
        names[i] = layersNames[outLayers[i] - 1];
    }
    return names;
}

void postprocess(Mat& frame, const vector<Mat>& outs)
{
    vector<int> classIds;
    vector<float> confidences;
    vector<Rect> boxes;

    for (size_t i = 0; i < outs.size(); ++i)
    {
        // Scan through all the bounding boxes output from the network and keep only the
        // ones with high confidence scores. Assign the box's class label as the class
        // with the highest score for the box.
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
        {
            Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            Point classIdPoint;
            double confidence;
            // Get the value and location of the maximum score
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confThreshold)
            {
                int centerX = (int)(data[0] * frame.cols);
                int centerY = (int)(data[1] * frame.rows);
                int width = (int)(data[2] * frame.cols);
                int height = (int)(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(Rect(left, top, width, height));
            }
        }
    }

    // Perform non maximum suppression to eliminate redundant overlapping boxes with
    // lower confidences
    vector<int> indices;
    NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        Rect box = boxes[idx];
        drawPred(classIds[idx], confidences[idx], box.x, box.y,
                 box.x + box.width, box.y + box.height, frame);
    }
}

void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame)
{
    //Draw a rectangle displaying the bounding box
    rectangle(frame, Point(left, top), Point(right, bottom), Scalar(0, 255, 0), 1, 8, 0);

    //Get the label for the class name and its confidence
    string label = format("%.2f", conf);
    if (!classes.empty())
    {
        CV_Assert(classId < (int)classes.size());
        label = classes[classId] + ":" + label;
    }

    //Display the label at the top of the bounding box
    int baseLine;
    Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    top = max(top, labelSize.height);
    putText(frame, label, Point(left, top), CV_FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 2, 8, false);
}
