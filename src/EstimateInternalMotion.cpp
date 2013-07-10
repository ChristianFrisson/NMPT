/*
 *  EstimateExternalMotion.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 1/12/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */



#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <iostream>
#include <fstream>

#include "BlockTimer.h"
#include "InternalMotionModel.h"
#include "NMPTUtils.h"

#include <vector>
#include <string>
#include <sstream>

//#include <unistd.h>

using namespace std;
using namespace cv; 

/* Fill this function with code to move your robot's eye motors to a specific servo setting */
int moveEyesAbsolute(const Mat &action) {
	
	// HERE IS WHERE YOU TELL THE ROBOT TO MOVE ITS EYES
	
	int key = cvWaitKey(1) ; 
	if (key== 'q' || key == 'Q') return 1; 
	else return 0; 
	
}


/* Fill this function with code to move your robot's eye motors relative to their current setting */
int moveEyesRelative(const Mat &actionDelta) {
	
	// HERE IS WHERE YOU TELL THE ROBOT TO MOVE ITS EYES
	
	int key = cvWaitKey(1) ; 
	if (key== 'q' || key == 'Q') return 1; 
	else return 0; 
	
}


int main (int argc, char ** argv) 
{		
	int numServos = 5;             // How many servos does your robot have?
	int cameraWidth = 320;         // Width of your camera observations
	int cameraHeight = 240;        // Height of your camera images
	int key=0;                     // Key pressed by user -- if 'q', terminate program
	int camnum=0;                  // Number of the camera used to acquire images. 
	                               // Set to 0 if only one camera is present.
	
	const char  * WINDOW_NAME  = "Estimated Appearance";         //Title of display window
	string saveFile = "myRobotsMotionParameters.txt";            //File where model will be saved
	
	BlockTimer bt;                 // A tool for tracking time.
	CvCapture* camera = NULL; 	   // The camera. 	
	int numEyeMovements = 0;       // How many eye movements have we made?
	
	CvSize imsize = cvSize(cameraWidth,cameraHeight);            //Size of images we will be working with
	IplImage* floatIm = cvCreateImage(imsize, IPL_DEPTH_64F, 1); //Floating point representation of grayscale image data
		
    cvNamedWindow (WINDOW_NAME,0); // Window to display learning progress. The contents of this will 
 	                               // be huge, so you don't want to lock its size.
	
	
	/* Try to get the camera, and fail if you can't */
	if (argc < 2) {
		camera = cvCaptureFromCAM(camnum);
	} else {
		if (argv[1][0] == '-') {
			cout << "An example program to demonstrate learning to look. A simulated robot " << endl 
			<< "learns that it has five motors, each of which do nothing." << endl << endl
			<< "Usage: " << endl << "    >> EstimateInternalMotion" << endl << endl; 
			return 0; 
		}
	}
	
    if (! camera) {
		cout << "Failed to get input from a video capture source." << endl << endl
		<< "Usage: " << endl << "    >> EstimateInternalMotion" << endl << endl; 
		return 0; 
	}
		
	/* Try to set the camera's width. If this fails, we will crop out the middle instead */
	cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, cameraWidth); 
	cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT, cameraHeight); 
	
	
	Mat actionDelta = Mat::zeros(numServos, 1, CV_64F ); // A retina-centered action representation, initially 0
	Mat newAction = Mat::ones(numServos,1,CV_64F)*.5;    // A world centered action representation, initially 0.5 (center)
	Mat action;                                          // A world centered action representation, initially 0.5 (center)
	newAction.copyTo(action); 
	Mat noise(newAction.size(), newAction.type());       // A vector for noise
	
	moveEyesAbsolute(newAction);   // Move eyes to central location (.5, .5, .5, .5, .5)... 
	cvWaitKey(1000);       // ... and wait one second for them to get there.	
	
	
	/*
	 * Find out the size of images actually coming from the camera, and how 
	 * to make them the size we want
	 */
	IplImage* frame = cvQueryFrame(camera); 
	CvRect frameROI = cvRect(frame->width/2 - cameraWidth/2, frame->height/2-cameraHeight/2, cameraWidth, cameraHeight); 	
	
	
	/* Create the internal motion model, and see how many points it wants to estimate dynamics */
	InternalMotionModel imm(numServos, imsize); 		
	int numFrames = imm.getNumDynamicsModels(); 
	
	/* Create a bank of images to estimate motion dynamics */
	vector<IplImage*> frameCopies(numFrames); 
	for (int i = 0; i < numFrames; i++) {
		frameCopies[i] = cvCreateImage(imsize, IPL_DEPTH_8U, 1); 
	}
	vector<double> frameTimes(numFrames); 
	
	
	/* This will be used to rescale the pixel values for display */
	IplImage* canvas = cvCreateImage(cvSize(imm.lambda.mu->width,imm.lambda.mu->height), IPL_DEPTH_8U, 1); 
	

	
	/* MAIN LOOP BEGINS */
	while (key != 'q' && key != 'Q') { //Loop until user enters 'q'
		
		numEyeMovements++; 
		
		/* Start a timer, move the eyes, and collect an image trajectory */		

		actionDelta = newAction - action; // Compute desired relative eye movement
		newAction.copyTo(action);         // and record the absolute position.
		
		bt.blockRestart(0);               // Start timer
		moveEyesRelative(actionDelta);    // Move Eyes relative to current position
		
		for (size_t i = 0; i < frameCopies.size(); i++) {   // For each point in the trajectory
			frame = cvQueryFrame(camera);                   // Get a camera frame
			
			frameTimes[i] = bt.getCurrTime(0);              // Record the current time
			cvSetImageROI(frame, frameROI);                 // Get the frame center
			cvCvtColor(frame, frameCopies[i], CV_BGR2GRAY); // Save it as a grayscale, 8-bit image.
			cvResetImageROI(frame);                         // and clean up.
			
		}
		
		/* Check for user input, and break if the user is done. */ 
		key = cvWaitKey(1);    
		if (key == 'q' || key == 'Q') break; 
		
		
		/* Convert each frame to a real-valued representation, and update model */ 				
		for (size_t i = 0; i < frameCopies.size(); i++) {
			cout << "Eye Movement " << numEyeMovements << ": Processing frame "<< i << endl;
			cvConvertScale(frameCopies[i], floatIm, 1.0/256, 0);
			imm.updateModelMAP(floatIm, actionDelta, i, frameTimes[i]); 
		}
				
		
		/* Visualize learning progress */ 
		cvConvertScale(imm.lambda.mu, canvas, 256); 
		cvShowImage(WINDOW_NAME, canvas); 	
		
		/* If you've been doing this for a really long time, stop */
		if (numEyeMovements == 300) break; 
		
		
		/* Compute next action: Wander around, trending toward the center */
		newAction = .9*(action-.5)+.5;  // Move a little bit back toward center
		randn(noise,0,.05);             // Get a little bit of noise
		newAction += noise;             // And add it to the current position
		
		/* Don't try to go outside the bounds of our motor limits (0 - 1) */
		Mat mask; 
		compare(newAction, 0, mask, CMP_LT);  // Find desired actions < 0
		newAction.setTo(0,mask);              // Set them to 0
		compare(newAction, 1, mask, CMP_GT);  // Find desired actions > 1
		newAction.setTo(1,mask);              // Set them to 1.
		
		/* Setting one or two servos to be steady can speed up learning, but it's not necessary */
		for (int i =0; i < 2; i++) {
			int ind = NMPTUtils::randomFloat() * numServos; 
			newAction.at<double>(ind) = action.at<double>(ind); 
		}
		
		/* Save what we've learned, so we can use it later */
		ofstream out; 
		out.open(saveFile.c_str()); 
		out << imm; 
		out.close(); 	
		
		
		/* Check for user input, and break if the user is done. */ 
		key = cvWaitKey(20); 		
		if (key == 'q' || key == 'Q') break; 
		
	}
	
	cvReleaseCapture(&camera); 
	
}
