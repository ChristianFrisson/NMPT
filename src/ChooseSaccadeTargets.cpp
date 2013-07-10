/*
 *  ChooseSaccadeTargets.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 8/26/10.
 *  Copyright 2010 UC San Diego. All rights reserved.
 *
 */



#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "BlockTimer.h"
#include "InternalMotionModel.h"
#include "NMPTUtils.h"

using namespace cv; 
using namespace std; 

Size imSize = Size(640,480);      //Size of the visual field -- can be different from the one used in training.

double lastFrameTime;             //Timinig info that allows the dynamics model to correct
InternalMotionModel imm;          //The model of how the robot's motors worked, learned in EstimateInternalMotion.cpp
vector<actionRecord> pastActions; //A record of past eye-movements, used in the dynamics model
int useDynamics = 1;              //A flag that turns the dynamics model off, for comparison
BlockTimer bt;                    //Timers

/* Fill this function with code to move your robot's eye motors relative to their current setting */
int moveEyesRelative(const Mat &actionDelta) {
	
	// HERE IS WHERE YOU TELL THE ROBOT TO MOVE ITS EYES
	
	int key = cvWaitKey(1) ; 
	if (key== 'q' || key == 'Q') return 1; 
	else return 0; 
	
}

/* Get a desired looking location from the user (mouse click), and decide on action */
void respondToClick(int event, int x, int y, int flags, void* param) {		
	
	/* Ignore events that aren't clicks */
	if (event == CV_EVENT_MOUSEMOVE || event == CV_EVENT_LBUTTONUP || 
		event == CV_EVENT_RBUTTONUP || event == CV_EVENT_MBUTTONUP) return; 
	
	cout << "User clicked on point " << x << ", " << y << endl; 
	
	/* Convert to a centered coordinate system */
	double xoffset, yoffset;	
	xoffset = (x - imSize.width / 2.0) ;
	yoffset = (y - imSize.height / 2.0);	
	cout << "XOffset: " << xoffset << " - YOffset: " << yoffset << endl;
	
	/* Scale to match what the IMM expects, and construct desired transform.  */ 
	Size s = imm.getEyeMovementUnitsSize(); 
	xoffset = xoffset*s.width/imSize.width; 
	yoffset = yoffset*s.height/imSize.height;
	
	double translation[2][1] = {{xoffset}, {yoffset}};
	Mat tau(2, 1, CV_64F, translation);
	
	/* Get desired transform, either taking dynamics into account or not. */
	cout << "Finding recommended action for desired transform " << NMPTUtils::commaSeparatedFlattenedMat(tau) << endl;
	Mat actionDelta; 
	
	if (useDynamics) {		
		imm.recommendActionForDesiredTransform(tau, pastActions, bt.getCurrTime(1)-lastFrameTime, actionDelta); 		
	} else {
		imm.recommendActionForDesiredTransform(tau, actionDelta);
	}
	
	cout << "The recommended action is " << NMPTUtils::commaSeparatedFlattenedMat(actionDelta) << endl; 
	
	/* Move the eyes */ 
	moveEyesRelative(actionDelta); 
	
	/* Erase past actions that are too far away to matter, and update action record for future dynamics */
	if (pastActions.size() > (size_t) 30) {
		pastActions.erase(pastActions.begin()); 
	}	
	actionRecord r; 	
	r.actionVals = actionDelta.clone(); 
	r.timeStamp = bt.getCurrTime(1); 
	pastActions.push_back(r); 
	
	
}


int main (int argc, char ** argv) 
{
	/* Read data from file saved by EstimateInternalMotion.cpp */	
	ifstream in; 
	in.open("myRobotsMotionParameters.txt"); 
	in >> imm; 
	in.close(); 
	
	/* Open camera */ 
	VideoCapture capture; 
	int usingCamera = NMPTUtils::getVideoCaptureFromCommandLineArgs(capture, argc, (const char**) argv); 
	if (!usingCamera--) return 0;  
	
	/* Set camera to desired width/height */ 
	if (usingCamera) {
		capture.set(CV_CAP_PROP_FRAME_WIDTH, imSize.width); 
		capture.set(CV_CAP_PROP_FRAME_HEIGHT, imSize.height); 
	}
	
	/* Set up mouse call back to get user input, and start global timer */ 
	const string windowName = "Camera View"; 
	namedWindow(windowName); 
	cvSetMouseCallback(windowName.c_str(), respondToClick); 
	bt.blockRestart(1); 
	
	while (waitKey(10) < 0) {
		Mat frame, dispFrame; 
		bt.blockRestart(0);    //For the dynamics model, it's important to know when the frame was acquired
		/* Get frame and draw a target on it */
		capture >> frame; 
		frame.copyTo(dispFrame); 
		imSize = dispFrame.size(); 
		Point center(imSize.width/2, imSize.height/2); 
		circle(dispFrame, center, 5, CV_RGB(255,0,0),3);
		imshow(windowName, dispFrame); 
		lastFrameTime = bt.getCurrTime(0);  //When the frame was acquired is in relation when the signal "moveEyes" is sent
	}
	
	
    return 0;
}