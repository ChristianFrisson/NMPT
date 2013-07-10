/*
 *  SimpleSalienceExample.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 11/15/10.
 *  Copyright 2010 UC San Diego. All rights reserved.
 *
 */


/*
 *  SimpleSalienceExample2.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 11/2/10.
 *  Copyright 2010 UC San Diego. All rights reserved.
 *
 */



#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <iostream>
#include "FastSalience.h"

using namespace std;
using namespace cv; 

int main (int argc, char * const argv[]) 
{
	double salWidth = 320;                  // desired width of salience map
	string fname = "data/HDMovieClip.avi";  // video file to analyze
	
	// Check command-line args for usage request.
	if (argc > 1) {
		cout << argv[0] << ": A program for processing video input." << endl;
		cout << "Usage:" << endl ;
		cout << "\t" << argv[0] << "\t\t: Get input from the default video file, " << fname << endl; 
		cout << "\t" << argv[0] << " --help\t: Print this message and quit." << endl; 
		return 0; 
	}
	
	
	// Open video for analysis.
	VideoCapture capture(fname); 
	if (!capture.isOpened()) {
		cout << "Warning: " << argv[0] << " could not find " << fname << endl; 
		return 0; 
	}
	
	FastSalience salTracker; //Initialize salience tracker
	
	/*
	 * Main loop
	 */
	Mat im, im2, sal;
	while (waitKey(5) <= 0) {
		
		capture >> im2; //Try to grab a frame, see if the video ended.
		if (im2.cols <= 0) { 
			break;
		}
		
		// Resize frame to desired size.		
		double ratio = salWidth * 1. / im2.cols; 
		resize(im2, im, Size(0,0), ratio, ratio, INTER_NEAREST); 		
		
		salTracker.updateSalience(im); // Update salience map.
		
		salTracker.getSalImage(sal); //Retrieve salience map visualization.
		
		imshow("FastSUN Salience", sal); 
		
		
	} 
}