/*
 *  FastSUNImage.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 11/15/10.
 *  Copyright 2010 UC San Diego. All rights reserved.
 *
 */




#include <opencv2/opencv.hpp>
#include <iostream>
#include <sstream>
#include "BlockTimer.h"
#include "FastSalience.h"
#include "LQRPointTracker.h"
#include "NMPTUtils.h"

using namespace std;
using namespace cv; 

string WINDOW_NAME1  = "FastSUN Parameters" ;
string WINDOW_NAME2  = "FastSUN Saliency Tracker" ;

/* These 8 global variables capture the value of the UI's sliders.*/
int sp_scales = 5; 
int tm_scales = 0; 
int salscale = 5; 
int tau0 = 0; 
int rad0 = 0; 
int doeFeat = 1; 
int dobFeat = 1; 
int colorFeat = 1; 
int useParams = 1; 
int estParams = 1; 
int power = 9; 

/* These 4 global variables constrain the range of the UI's sliders */
int maxPowers = 19; 
int maxScales = 6; 
int maxRad = 3; 
int maxTau = 10; 
int maxsalscale = 9; 

/* These global variables allow the saliency map's size to be set
 * as a portion of the current video source's size.
 */
int saliencyMapWidth; 
int saliencyMapHeight; 
int imwidth; 
int imheight; 

/* The saliency tracker needs to be modified by the sliders, so it's a global variable. */
FastSalience salTracker;


/* These parameters can be modified at any time without deleting the current
 * saliency tracker, so there is a separate callback for them.
 */
void modifySaliencyParams(int doNotUseMe=0, void* orMe=NULL) {
	double mypower=0.1+power/10.0;
	salTracker.setGGDistributionPower(mypower); 
	salTracker.setUseDoEFeatures(doeFeat); 
	salTracker.setUseDoBFeatures(dobFeat); 
	salTracker.setUseColorInformation(colorFeat); 
	salTracker.setUseGGDistributionParams(useParams);
	salTracker.setEstimateGGDistributionParams(estParams); 
	
	cout << "Setting Current FastSalience Parameters." << endl;
	cout << "  Generalized Gaussian Distribution Power: " << mypower << endl;
	cout << "  Use Difference of Exponential Features:  " << doeFeat << endl;
	cout << "  Use Difference of Box Features:          " << dobFeat << endl; 
	cout << "  Use Color Information:                   " << colorFeat << endl; 
	cout << "  Use Estimated GG Distribution Params:    " << useParams << endl; 
	cout << "  Estimate GG Dist Params from Data:       " << estParams << endl; 
}

/* This callback deletes the current saliency tracker and creates a new one with modified parameters
 * controlled by the UI sliders. 
 */ 
void reInitializeSaliency(int doNotUseMe=0, void* orMe=NULL) {
	
	saliencyMapWidth = imwidth/(maxsalscale+1-salscale); 
	saliencyMapHeight = imheight/(maxsalscale+1-salscale); 
	
	//Get Parameters from Global Variables Controlled by Steppers
	int nspatial = sp_scales+1;  // [2 3 4 5 ...]
	int ntemporal = tm_scales+2;  //[2 3 4 5 ...]
	float first_tau = 1.0/(tau0+1);  //[1 1/2 1/3 1/4 1/5 ...]
	int first_rad = (1<<rad0)/2; // [0 1 2 4 8 16 ...]
	
	//This is generally not thread-safe programming, but it seems to work.
	salTracker = FastSalience::FastSalience(ntemporal, nspatial, first_tau, first_rad);
	
	cout << "Created new FastSaliency object with the following parameters." << endl;
	cout << "  Width:                                   " << saliencyMapWidth << endl; 
	cout << "  Height:                                  " << saliencyMapHeight << endl;
	cout << "  # Temporal Scales:                       " << ntemporal << endl;
	cout << "  # Spatial Scales:                        " << nspatial << endl; 
	cout << "  Slowest Temporal Feature Decay:          " << first_tau << endl; 
	cout << "  Smallest Spatial Scale Radius:           " << first_rad << endl;
	
	modifySaliencyParams();
}

int main (int argc, char * const argv[]) 
{
	BlockTimer bt; 
	
	Mat im, im2, viz, sal ; 
	
	string fname = "data/HDImage_03.jpg";  // image file to analyze
	
	// Check command-line args for usage request.
	if (argc > 2 || (argc > 1 && argv[1][0] == '-')) {
		cout << argv[0] << ": A program for processing image input." << endl;
		cout << "Usage:" << endl ;
		cout << "\t" << argv[0] << "\t\t\t: Get input from the default image file, " << fname << endl; 
		cout << "\t" << argv[0] << " [path_to_image]\t: Get input from the specified image file, " << fname << endl; 
		cout << "\t" << argv[0] << " --help\t\t: Print this message and quit." << endl; 
		return 0; 
	}
	
	if (argc > 1)
		fname = argv[1];
	
	im2 = imread(fname); 
	
	if (im2.cols <= 0) {
		cout << "Warning: " << argv[0] << " could not find " << fname << endl; 
		cout << "Usage:" << endl ;
		cout << "\t" << argv[0] << "\t\t\t: Get input from the default image file, " << fname << endl; 
		cout << "\t" << argv[0] << " [path_to_image]\t: Get input from the specified image file, " << fname << endl; 
		cout << "\t" << argv[0] << " --help\t\t: Print this message and quit." << endl; 
		return 0; 
		return 0; 
	}
	
	LQRPointTracker salientSpot(2);
	vector<double> lqrpt(2,.5); 
	salientSpot.setTrackerTarget(lqrpt);
	
	
    namedWindow (WINDOW_NAME1, 0);  //Create the UI Sliders window
    namedWindow (WINDOW_NAME2, CV_WINDOW_AUTOSIZE); //Create the graphical algorithm display
	
	//Create the sliders for the intialization parameters -- callback is "reInitializeSaliency" (above) 
	createTrackbar( "Spatial Scales", WINDOW_NAME1, &sp_scales, maxScales, &reInitializeSaliency); 
	createTrackbar( "Temporal Scales", WINDOW_NAME1, &tm_scales, maxScales, &reInitializeSaliency); 
	createTrackbar( "Spatial Size", WINDOW_NAME1, &rad0, maxRad, &reInitializeSaliency); 
	createTrackbar( "Temporal Falloff", WINDOW_NAME1, &tau0, maxTau, &reInitializeSaliency); 
	createTrackbar( "Image Size", WINDOW_NAME1, &salscale, maxsalscale, &reInitializeSaliency); 
	
	//Create the sliders for non-initialization parameters -- callback is "modifySaliencyParams" (above)
	createTrackbar( "Distribution Power", WINDOW_NAME1, &power, maxPowers, &modifySaliencyParams); 
	createTrackbar( "Use Spatial Features", WINDOW_NAME1, &dobFeat, 1, &modifySaliencyParams); 
	createTrackbar( "Use Temporal Features", WINDOW_NAME1, &doeFeat, 1, &modifySaliencyParams); 
	createTrackbar( "Use Color Contrast", WINDOW_NAME1, &colorFeat, 1, &modifySaliencyParams); 
	createTrackbar( "Estimate Histogram", WINDOW_NAME1, &estParams, 1, &modifySaliencyParams); 
	createTrackbar( "Use Histogram", WINDOW_NAME1, &useParams, 1, &modifySaliencyParams); 
	
	cvResizeWindow(WINDOW_NAME1.c_str(), 350, 566); 
	
	imwidth = im2.cols; 
	imheight = im2.rows; 
	
	
	cout << "*************************************************************************" << endl;
	cout << "*  Running FastSUN Example." << endl; 
	cout << "*    TIPS: " << endl;
	cout << "*    -- Press 'q' to quit." << endl;
	cout << "*************************************************************************" << endl << endl;
	
	//Create the initial saliency tracker
	reInitializeSaliency() ; 
	
	bt.blockRestart(0);
	while (waitKey(5) <= 0) {
		
		double saltime, tottime; 
		resize(im2, im, Size(saliencyMapWidth, saliencyMapHeight), 0,0, INTER_NEAREST); 
		
		bt.blockRestart(1); 
		vector<KeyPoint> pts; 
		
		viz.create(im.rows, im.cols*2, CV_32FC3);  
		salTracker.detect(im, pts); 
		saltime = bt.getCurrTime(1) ; 
		
		
		salTracker.getSalImage(sal); 
		
		double min, max; 
		Point minloc, maxloc; 
		minMaxLoc(sal, &min, &max, &minloc, &maxloc); 
		
		lqrpt[0] = maxloc.x*1.0 / sal.cols;  
		lqrpt[1] = maxloc.y*1.0 / sal.rows; 
		salientSpot.setTrackerTarget(lqrpt); 
		
		Mat vizRect = viz(Rect(im.cols,0,im.cols, im.rows));
		cvtColor(sal, vizRect, CV_GRAY2BGR); 
		
		vizRect = viz(Rect(0, 0, im.cols, im.rows)); 
		im.convertTo(vizRect,CV_32F, 1./256.); 
		
		for (size_t i = 0; i < pts.size(); i++) {
			circle(vizRect, pts[i].pt, 2, CV_RGB(0,255,0));
		}
		
		salientSpot.updateTrackerPosition(); 
		lqrpt = salientSpot.getCurrentPosition();
		circle(vizRect, Point(lqrpt[0]*sal.cols, lqrpt[1]*sal.rows), 6, CV_RGB(0,0,255));
		circle(vizRect, Point(lqrpt[0]*sal.cols, lqrpt[1]*sal.rows), 5, CV_RGB(0,0,255));
		circle(vizRect, Point(lqrpt[0]*sal.cols, lqrpt[1]*sal.rows), 4, CV_RGB(255,255,0));
		circle(vizRect, Point(lqrpt[0]*sal.cols, lqrpt[1]*sal.rows), 3, CV_RGB(255,255,0));
		
		vizRect = viz(Rect(im.cols,0,im.cols, im.rows));
		cvtColor(sal, vizRect, CV_GRAY2BGR); 
		
		tottime = bt.getCurrTime(0); 
		bt.blockRestart(0); 
		
		stringstream text; 
		text << "FastSUN: " << (int)(saltime*1000) << " ms ; Total: " << (int)(tottime*1000) << " ms."; 
		
		putText(viz, text.str(), Point(20,20), FONT_HERSHEY_SIMPLEX, .33, Scalar(255,0,255)); 
		
		
		imshow(WINDOW_NAME2, viz); 
	}
}