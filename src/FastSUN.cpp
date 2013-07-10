/*
 *  FastSUN.cpp
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

int main (int argc, char * const argv[]) 
{
	Size imSize(320,240); 
	BlockTimer bt; 
	
	/* Open capture */ 
	VideoCapture capture; 
	int usingCamera = NMPTUtils::getVideoCaptureFromCommandLineArgs(capture, argc, (const char**) argv); 
	if (!usingCamera--) return 0;  
	
	FastSalience salTracker; 
	
	LQRPointTracker salientSpot(2);
	vector<double> lqrpt(2,.5); 
	salientSpot.setTrackerTarget(lqrpt);
	
	/* Set capture to desired width/height */ 
	if (usingCamera) {
		capture.set(CV_CAP_PROP_FRAME_WIDTH, imSize.width); 
		capture.set(CV_CAP_PROP_FRAME_HEIGHT, imSize.height); 
	}
	bt.blockRestart(1);
	Mat im, im2, viz, sal ; 
	while (waitKey(5) <= 0) {
		double saltime, tottime; 
		capture >> im2; 
		
		if (usingCamera) {
			im = im2; 
		} else {
			double ratio = imSize.width * 1. / im2.cols; 
			resize(im2, im, Size(0,0), ratio, ratio, INTER_NEAREST); 		
		}
		
		viz.create(im.rows, im.cols*2, CV_32FC3); 
		
		bt.blockRestart(0); 
		vector<KeyPoint> pts; 
		salTracker.detect(im, pts); 
		saltime = bt.getCurrTime(0) ; 
		
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
		if (usingCamera) flip(viz, viz, 1); 
		
		tottime = bt.getCurrTime(1); 
		bt.blockRestart(1); 
		
		stringstream text; 
		text << "FastSUN: " << (int)(saltime*1000) << " ms ; Total: " << (int)(tottime*1000) << " ms."; 
		
		putText(viz, text.str(), Point(20,20), FONT_HERSHEY_SIMPLEX, .33, Scalar(255,0,255)); 
		
		
		imshow("FastSUN Salience", viz); 
	}
}