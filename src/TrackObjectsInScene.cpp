/*
 *  TrackObjectsInScene.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 8/3/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include "BlockTimer.h"
#include "GentleBoostCascadedClassifier.h"
#include "NMPTUtils.h"

using namespace std;
using namespace cv; 

int main (int argc, char * const argv[]) 
{	
	
	
	string WINDOW_NAME = "Object Tracker"; 
	
	Size patchSize = Size(30,30); 
	
	string GENTLEBOOST_FILE= "data/GenkiSZSLCascade.txt"; 
	
	Size minSearchSize(0,0); 
	Size maxSearchSize(0,0); 
	int numFeaturesToUse = -1;//-1; //3; //-1 means "All"
	int NMSRadius = 15; //15; 
	double patchThresh = -INFINITY;  //-INFINITY means "All", 0 means "p>.5"
	int maxObjects = 20; 
	int skipFrames = 0; 
	int useFast = 1; 
	
	Size imSize(320,240); 
	int key=0;
	
	/* Open capture */ 
	VideoCapture capture; 
	int usingCamera = NMPTUtils::getVideoCaptureFromCommandLineArgs(capture, argc, (const char**) argv); 
	if (!usingCamera--) return 0;  
	
	/* Set capture to desired width/height */ 
	if (usingCamera) {
		capture.set(CV_CAP_PROP_FRAME_WIDTH, imSize.width); 
		capture.set(CV_CAP_PROP_FRAME_HEIGHT, imSize.height); 
	}

	
    namedWindow (WINDOW_NAME, CV_WINDOW_AUTOSIZE); //Create the graphical algorithm display
    Mat current_frame, color_image, gray_image;
	
	BlockTimer bt; 
		
	GentleBoostCascadedClassifier* booster = new GentleBoostCascadedClassifier(); 
	
	ifstream in; 
	in.open(GENTLEBOOST_FILE.c_str()); 
	in >> booster; 
	in.close(); 
	booster->setSearchParams(useFast, minSearchSize, maxSearchSize,1.2, 1, 1); 
	booster->setNumFeaturesUsed(numFeaturesToUse); 
	
	int i = 0; 
    while (key != 'q' && key != 'Q') //Loop until user enters 'q'
    {
		
		//cout<< "Getting camera frame " << endl; 
		capture >> current_frame; 
		if (i++%(skipFrames+1) > 0 && !usingCamera) continue; 
		
		current_frame.copyTo(color_image); 
		cvtColor(current_frame, gray_image, CV_RGB2GRAY); 
				
		vector<SearchResult> boxes; 
		
		bt.blockRestart(2); 
		Mat img = gray_image; 
		booster->searchImage(img, boxes, NMSRadius, patchThresh); 
		cout << "Image Search Time was " << bt.getCurrTime(2)<< endl; 		
		
		if (boxes.size() > (unsigned int) maxObjects) {
			boxes.resize(maxObjects); 
		} 
		
		for (size_t i = 0; i < boxes.size(); i++) {
			Rect imgloc = boxes[i].imageLocation; 
			Point center = Point(imgloc.x + imgloc.width/2.0, imgloc.y + imgloc.height/2.0); 
			Scalar color; 
			if (boxes[i].value > 0 ) 
				color = (i== 0) ? Scalar(0,0,255): Scalar(0,255,255); 
			else color = Scalar(0,0,0); 
			circle(color_image, center, imgloc.width/2.0, color, 3); 
			circle(color_image, center, 2, color, 3); 
		}
		
		imshow(WINDOW_NAME, color_image);
		
		key = cvWaitKey(5);
	} 
	
	return 0;
}

