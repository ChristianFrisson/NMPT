/*
 *  HaarFeature2.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 10/22/10.
 *  Copyright 2010 UC San Diego. All rights reserved.
 *
 */

#include "HaarFeature2.h"
#include "DebugGlobals.h"
#include <math.h>
#include <iostream>
#include <stdio.h>

using namespace std; 
using namespace cv; 

#define max(x,y) (x>y?x:y)
#define min(x,y) (x<y?x:y)
#define sign(x) (x<0?-1:1)

void HaarFeature2::init(cv::Size expectedPatchSize, int numBoxes) {	
	if (_HAARFEATURE_DEBUG) cout<< "HaarFeature init." << endl; 
	patchSize = expectedPatchSize; 
	prefersIntegral = 1; 
	featureName = "HaarFeature"; 
	
	double minVals[1][10] = {{1, 1, 0, 0, 0, 0, 0, 0, 0, 0}}; 
	double maxVals[1][10] = {{patchSize.width, patchSize.height, patchSize.width-1, patchSize.height-1, 6, 2, 3, 3, 2, 2}}; 
	
	parameters.create(1, 10, CV_64F); 
	minParamVals = Mat::Mat(1, 10, CV_64F, minVals).clone(); 
	maxParamVals = Mat::Mat(1, 10, CV_64F, maxVals).clone(); 
	setRandomParameterValues(); 
}

HaarFeature2::HaarFeature2() {
	if (_HAARFEATURE_DEBUG) cout<< "HaarFeature default constructor." << endl; 
	init(); 
}

HaarFeature2::HaarFeature2(const HaarFeature2 &rhs) {
	if (_HAARFEATURE_DEBUG) cout<< "HaarFeature copy constructor." << endl; 
	init(rhs.patchSize); 
	setFeatureParameters(rhs.parameters); 
}


HaarFeature2 & HaarFeature2::operator=(const HaarFeature2 &rhs) {
	if (_HAARFEATURE_DEBUG) cout<< "HaarFeature assignmnet." << endl; 
	if (this != &rhs) {
		init(rhs.patchSize); 
		setFeatureParameters(rhs.parameters); 
	}
	return *this; 
}


HaarFeature2::HaarFeature2(cv::Size expectedPatchSize)  {	
	if (_HAARFEATURE_DEBUG) cout<< "HaarFeature constructor(size)." << endl; 
	init(expectedPatchSize); 
	/*
	cvSetReal2D(minParamVals, 0, 0, 1);  // filter width - This starts at 1 to give equal range to all sizes
	cvSetReal2D(minParamVals, 0, 1, 1);  // filter height
	cvSetReal2D(minParamVals, 0, 2, 0);  // xpos
	cvSetReal2D(minParamVals, 0, 3, 0);  // ypos
	cvSetReal2D(minParamVals, 0, 4, 0);  // kind - This starts at 0 to give equal range to all types (0-1, 1-2, ...,5-6)
	cvSetReal2D(minParamVals, 0, 5, 0);  // sign
	cvSetReal2D(minParamVals, 0, 6, 0);  // hSym
	cvSetReal2D(minParamVals, 0, 7, 0);  // vSym
	cvSetReal2D(minParamVals, 0, 8, 0);  // meanSub
	cvSetReal2D(minParamVals, 0, 9, 0);  // normBrightness
	
	
	cvSetReal2D(maxParamVals, 0, 0, patchSize.width);    // filter width
	cvSetReal2D(maxParamVals, 0, 1, patchSize.height);   // filter height
	cvSetReal2D(maxParamVals, 0, 2, patchSize.width-1);  // xpos - This ends early because the min size is 2
	cvSetReal2D(maxParamVals, 0, 3, patchSize.height-1); // ypos - This ends early because the min size is 2
	cvSetReal2D(maxParamVals, 0, 4, 6);  // kind
	cvSetReal2D(maxParamVals, 0, 5, 2);  // sign
	cvSetReal2D(maxParamVals, 0, 6, 3); // hSym
	cvSetReal2D(maxParamVals, 0, 7, 3); // vSym
	cvSetReal2D(maxParamVals, 0, 8, 2);  // meanSub
	cvSetReal2D(maxParamVals, 0, 9, 2);  // normBrightness
	*/
	
}

HaarFeature2::~HaarFeature2() {
	if (_HAARFEATURE_DEBUG) cout<< "HaarFeature destructor" << endl; 
}

void HaarFeature2::setFeatureParameters(const cv::Mat &paramVec) {
	
	if (_HAARFEATURE_DEBUG) cout<< "Starting to Set HaarFeature parameters." << endl; 
	assert(paramVec.cols == parameters.cols && paramVec.rows == parameters.rows); 
	if (_HAARFEATURE_DEBUG) cout<< "Param Vec is the right size." << endl ;
	
	if (!(&paramVec == &parameters)) {
		parameters = paramVec.clone(); 
	}
	
	if (_HAARFEATURE_DEBUG) cout<< "Making sure parameters are in proper range." << endl; 
	checkParameterBounds(parameters); 
	
	int ind = 0; 
	MatConstIterator_<double> param = parameters.begin<double>(); 
	
	fullWidth = ceil(*param);
	param++; 
	fullWidth = max(fullWidth,2);     //2:width
	if (_HAARFEATURE_DEBUG) cout << "fullWidth="<<fullWidth << " ; ind = " << ind << endl;
	
	fullHeight =  ceil(*param);   //2:height
	param++; 
	fullHeight = max(fullHeight,2); 
	if (_HAARFEATURE_DEBUG) cout << "fullHeight="<<fullHeight << " ; ind = " << ind << endl; 
	
	fullX = ceil(*param);         //0:width-2
	param++; 
	fullX =  max(fullX,1)-1;	
	if (_HAARFEATURE_DEBUG) cout << "fullX="<<fullX << " ; ind = " << ind << endl; 
	
	fullY = ceil(*param); 
	param++; 
	fullY = max(fullY,1)-1;         //0:height-2
	if (_HAARFEATURE_DEBUG) cout << "fullY="<<fullY << " ; ind = " << ind << endl; 
	
	waveletType = ceil(*param); 
	param++; 
	waveletType = max(waveletType,1);   //1:6	
	if (_HAARFEATURE_DEBUG) cout << "waveletType="<<waveletType <<" ; ind = " << ind <<  endl; 
	
	waveletSign = ceil(*param); 
	param++; 
	waveletSign = 2*(max(waveletSign,1)- 1.5); //-1,1
	if (_HAARFEATURE_DEBUG) cout << "waveletSign="<<waveletSign << " ; ind = " << ind << endl; 
	
	hasHSym = ceil(*param); 
	param++; 
	hasHSym =  max(hasHSym,1)-2;     //-1:1
	if (_HAARFEATURE_DEBUG) cout << "hasHSym="<<hasHSym << " ; ind = " << ind << endl; 
	
	hasVSym = ceil(*param); 
	param++; 
	hasVSym = max(hasVSym,1)-2;     //-1:1
	if (_HAARFEATURE_DEBUG) cout << "hasVSym="<<hasVSym << " ; ind = " << ind << endl; 
	
	meanSub = ceil(*param); 
	param++; 
	meanSub = max(meanSub,1)-1;     //0:1
	if (_HAARFEATURE_DEBUG) cout << "meanSub="<<meanSub << " ; ind = " << ind << endl; 
	
	//cvSetReal2D(parameters,0,ind,2); 
	normBrightness =ceil(*param); 
	param++; 
	normBrightness =max(normBrightness,1)-1; //0:1
	if (_HAARFEATURE_DEBUG) cout << "normBrightness="<<normBrightness << " ; ind = " << ind << endl; 
	
	if (_HAARFEATURE_DEBUG) cout << "Finished reading " << ind << " parameters." << endl; 
	
	//hasHSym = 0; 
	//hasVSym = 0; 
	//meanSub = 0; 
	//normBrightness = 1; 
	
	int leftX1, rightX1, leftX2, rightX2, topY1, bottomY1, topY2, bottomY2; 
	int centerWidth, centerHeight; 
	
	pt1.clear(); 
	pt2.clear(); 
	switch (waveletType) {
		case 1: //left-right
			//leftX1 = 19 - 1 = 18
			//rightX2= 19 + 1 = 20
			//leftX2 = 19 + 0 = 19
			//rightX1= 19 + 1 = 18
			
			leftX1 = fullX - (floor(1.0*(fullWidth-1)/2.0)); 			
			leftX2 = fullX; 
			rightX1 = fullX+1; 
			rightX2 = fullX + ceil(1.0*(fullWidth-1)/2.0); 
			
			topY1 = fullY - floor(1.0*(fullHeight-1)/2.0); 
			topY2 = fullY + ceil(1.0*(fullHeight-1)/2.0); 
			bottomY1 = topY1; 
			bottomY2 = topY2; 
			
			
			numBoxes = 2; 
			weights.create(1,numBoxes, CV_64F); 
			
			pt1.push_back(Point(leftX1, topY1)); 
			pt2.push_back(Point(leftX2, topY2)); 
			weights.at<double>(0,0) = waveletSign; 
			
			pt1.push_back(Point(rightX1, bottomY1)); 
			pt2.push_back(Point(rightX2, bottomY2)); 
			weights.at<double>(0,1) = -waveletSign; 
			
			/*
			cvSetReal2D(boxX1, 0, 0, leftX1); 
			cvSetReal2D(boxX2, 0, 0, leftX2); 
			cvSetReal2D(boxY1, 0, 0, topY1); 
			cvSetReal2D(boxY2, 0, 0, topY2); 
			cvSetReal2D(weights, 0, 0, waveletSign); 
			cvSetReal2D(boxX1, 0, 1, rightX1); 
			cvSetReal2D(boxX2, 0, 1, rightX2); 			
			cvSetReal2D(boxY1, 0, 1, bottomY1); 
			cvSetReal2D(boxY2, 0, 1, bottomY2); 
			cvSetReal2D(weights, 0, 1, -waveletSign); 
			
			cvSetReal2D(boxX1, 0, 2, 1); 
			cvSetReal2D(boxX2, 0, 2, 1); 
			cvSetReal2D(boxY1, 0, 2, 1); 
			cvSetReal2D(boxY2, 0, 2, 1); 
			cvSetReal2D(weights, 0, 2, 0);
			
			cvSetReal2D(boxX1, 0, 3, 1); 
			cvSetReal2D(boxX2, 0, 3, 1); 
			cvSetReal2D(boxY1, 0, 3, 1); 
			cvSetReal2D(boxY2, 0, 3, 1); 
			 cvSetReal2D(weights, 0, 3, 0);		
			 */
			
			
			
			break;			
		case 2: //up-down
			
			
			
			topY1 = fullY - floor(1.0*(fullHeight-1)/2.0);			
			topY2 = fullY; 
			bottomY1 = fullY+1; 
			bottomY2 = fullY + ceil(1.0*(fullHeight-1)/2.0);  
			
			leftX1 = fullX - (floor(1.0*(fullWidth-1)/2.0));
			leftX2 = fullX + ceil(1.0*(fullWidth-1)/2.0); 
			rightX1 = leftX1; 
			rightX2 = leftX2; 
			
			
			numBoxes = 2; 
			weights.create(1,numBoxes, CV_64F); 
			
			pt1.push_back(Point(leftX1, topY1)); 
			pt2.push_back(Point(leftX2, topY2)); 
			weights.at<double>(0,0) = waveletSign; 
			
			pt1.push_back(Point(rightX1, bottomY1)); 
			pt2.push_back(Point(rightX2, bottomY2)); 
			weights.at<double>(0,1) = -waveletSign; 
			
			/*
			cvSetReal2D(boxX1, 0, 0, leftX1); 
			cvSetReal2D(boxX2, 0, 0, leftX2); 
			cvSetReal2D(boxY1, 0, 0, topY1); 
			cvSetReal2D(boxY2, 0, 0, topY2); 
			cvSetReal2D(weights, 0, 0, waveletSign); 
			
			cvSetReal2D(boxX1, 0, 1, rightX1); 
			cvSetReal2D(boxX2, 0, 1, rightX2); 			
			cvSetReal2D(boxY1, 0, 1, bottomY1); 
			cvSetReal2D(boxY2, 0, 1, bottomY2); 
			cvSetReal2D(weights, 0, 1, -waveletSign); 
			
			cvSetReal2D(boxX1, 0, 2, 1); 
			cvSetReal2D(boxX2, 0, 2, 1); 
			cvSetReal2D(boxY1, 0, 2, 1); 
			cvSetReal2D(boxY2, 0, 2, 1); 
			cvSetReal2D(weights, 0, 2, 0);
			
			cvSetReal2D(boxX1, 0, 3, 1); 
			cvSetReal2D(boxX2, 0, 3, 1); 
			cvSetReal2D(boxY1, 0, 3, 1); 
			cvSetReal2D(boxY2, 0, 3, 1); 
			cvSetReal2D(weights, 0, 3, 0);		
			
			numBoxes = 2; 
			 
			 */
			
			break;
			
		case 3: //center-surround
			//left/top is surround, right/bottom is center
			centerWidth = floor((fullWidth)*1.0/2); 
			centerHeight = floor((fullHeight)*1.0/2); 
			leftX1 = fullX - (floor(1.0*(fullWidth-1)/2.0)); 
			leftX2 = fullX + ceil(1.0*(fullWidth-1)/2.0);
			rightX1 =  fullX - (floor(1.0*(centerWidth-1)/2.0)); 
			rightX2 =  fullX + ceil(1.0*(centerWidth-1)/2.0);
			
			topY1 = fullY - floor(1.0*(fullHeight-1)/2.0); 
			topY2 = fullY + ceil(1.0*(fullHeight-1)/2.0);  
			bottomY1 = fullY - floor(1.0*(centerHeight-1)/2.0); 
			bottomY2 = fullY + ceil(1.0*(centerHeight-1)/2.0); 
			
			
			
			numBoxes = 3; 
			weights.create(1,numBoxes, CV_64F); 
			
			pt1.push_back(Point(leftX1, topY1)); 
			pt2.push_back(Point(leftX2, topY2)); 
			weights.at<double>(0,0) = waveletSign; 
			
			pt1.push_back(Point(rightX1, bottomY1)); 
			pt2.push_back(Point(rightX2, bottomY2)); 
			weights.at<double>(0,1) = -waveletSign; 
			
			pt1.push_back(Point(rightX1, bottomY1)); 
			pt2.push_back(Point(rightX2, bottomY2)); 
			weights.at<double>(0,2) = -waveletSign; 
			
			/*
			cvSetReal2D(boxX1, 0, 0, leftX1); 
			cvSetReal2D(boxX2, 0, 0, leftX2); 
			cvSetReal2D(boxY1, 0, 0, topY1); 
			cvSetReal2D(boxY2, 0, 0, topY2); 
			cvSetReal2D(weights, 0, 0, waveletSign); 
			
			cvSetReal2D(boxX1, 0, 1, rightX1); 
			cvSetReal2D(boxX2, 0, 1, rightX2); 			
			cvSetReal2D(boxY1, 0, 1, bottomY1); 
			cvSetReal2D(boxY2, 0, 1, bottomY2); 
			cvSetReal2D(weights, 0, 1, -waveletSign); 
			
			cvSetReal2D(boxX1, 0, 2, rightX1); 
			cvSetReal2D(boxX2, 0, 2, rightX2); 			
			cvSetReal2D(boxY1, 0, 2, bottomY1); 
			cvSetReal2D(boxY2, 0, 2, bottomY2); 
			cvSetReal2D(weights, 0, 2, -waveletSign); 
			
			cvSetReal2D(boxX1, 0, 3, 1); 
			cvSetReal2D(boxX2, 0, 3, 1); 
			cvSetReal2D(boxY1, 0, 3, 1); 
			cvSetReal2D(boxY2, 0, 3, 1); 
			cvSetReal2D(weights, 0, 3, 0);		
			
			numBoxes = 3; 
			 */
			
			break;
			
		case 4: //left-center-right
			
			centerWidth = floor((fullWidth)*1.0/2); 
			
			leftX1 = fullX - (floor(1.0*(fullWidth-1)/2.0)); 
			leftX2 = fullX + ceil(1.0*(fullWidth-1)/2.0);
			rightX1 =  fullX - (floor(1.0*(centerWidth-1)/2.0)); 
			rightX2 =  fullX + ceil(1.0*(centerWidth-1)/2.0);
			
			topY1 = fullY - floor(1.0*(fullHeight-1)/2.0); 
			topY2 = fullY + ceil(1.0*(fullHeight-1)/2.0);  
			bottomY1 = topY1; 
			bottomY2 = topY2; 
			
			numBoxes = 3; 
			weights.create(1,numBoxes, CV_64F); 
			
			pt1.push_back(Point(leftX1, topY1)); 
			pt2.push_back(Point(leftX2, topY2)); 
			weights.at<double>(0,0) = waveletSign; 
			
			pt1.push_back(Point(rightX1, bottomY1)); 
			pt2.push_back(Point(rightX2, bottomY2)); 
			weights.at<double>(0,1) = -waveletSign; 
			
			pt1.push_back(Point(rightX1, bottomY1)); 
			pt2.push_back(Point(rightX2, bottomY2)); 
			weights.at<double>(0,2) = -waveletSign; 
			/*
			
			cvSetReal2D(boxX1, 0, 0, leftX1); 
			cvSetReal2D(boxX2, 0, 0, leftX2); 
			cvSetReal2D(boxY1, 0, 0, topY1); 
			cvSetReal2D(boxY2, 0, 0, topY2); 
			cvSetReal2D(weights, 0, 0, waveletSign); 
			
			cvSetReal2D(boxX1, 0, 1, rightX1); 
			cvSetReal2D(boxX2, 0, 1, rightX2); 			
			cvSetReal2D(boxY1, 0, 1, bottomY1); 
			cvSetReal2D(boxY2, 0, 1, bottomY2); 
			cvSetReal2D(weights, 0, 1, -waveletSign); 
			
			cvSetReal2D(boxX1, 0, 2, rightX1); 
			cvSetReal2D(boxX2, 0, 2, rightX2); 			
			cvSetReal2D(boxY1, 0, 2, bottomY1); 
			cvSetReal2D(boxY2, 0, 2, bottomY2); 
			cvSetReal2D(weights, 0, 2, -waveletSign); 
			
			cvSetReal2D(boxX1, 0, 3, 1); 
			cvSetReal2D(boxX2, 0, 3, 1); 
			cvSetReal2D(boxY1, 0, 3, 1); 
			cvSetReal2D(boxY2, 0, 3, 1); 
			cvSetReal2D(weights, 0, 3, 0);		
			numBoxes = 3; 
			 */
			break;
			
		case 5: //diagonal
			
			leftX1 = fullX - (floor(1.0*(fullWidth-1)/2.0)); 			
			leftX2 = fullX; 
			rightX1 = fullX+1; 
			rightX2 = fullX + ceil(1.0*(fullWidth-1)/2.0); 
			
			topY1 = fullY - floor(1.0*(fullHeight-1)/2.0);			
			topY2 = fullY; 
			bottomY1 = fullY+1; 
			bottomY2 = fullY + ceil(1.0*(fullHeight-1)/2.0);  
			
			
			numBoxes = 4; 
			weights.create(1,numBoxes, CV_64F); 
			
			pt1.push_back(Point(leftX1, topY1)); 
			pt2.push_back(Point(leftX2, topY2)); 
			weights.at<double>(0,0) = waveletSign; 
			
			pt1.push_back(Point(rightX1, bottomY1)); 
			pt2.push_back(Point(rightX2, bottomY2)); 
			weights.at<double>(0,1) = waveletSign; 
			
			pt1.push_back(Point(leftX1, bottomY1)); 
			pt2.push_back(Point(leftX2, bottomY2)); 
			weights.at<double>(0,2) = -waveletSign; 
			
			
			pt1.push_back(Point(rightX1, topY1)); 
			pt2.push_back(Point(rightX2, topY2)); 
			weights.at<double>(0,3) = -waveletSign; 
			
			/*
			 leftX1 = fullX - ceil(1.0*(fullWidth-1)/2.0) - 1; 
			 rightX2 = fullX + floor(1.0*(fullWidth-1)/2.0) - 1; 
			 leftX2 = leftX1 + floor(1.0*(rightX2-leftX1)/2.0) ; 
			 rightX1 = leftX2 + 1; 
			 
			 topY1 = fullY - ceil(1.0*(fullHeight-1)/2.0) - 1; 
			 bottomY2 = fullY + floor(1.0*(fullHeight-1)/2.0) - 1; 
			 topY2 = topY1 + floor(1.0*(bottomY2-topY1)/2.0) ;  
			 bottomY1 = topY2 +1; 
			 */			
			/*
			cvSetReal2D(boxX1, 0, 0, leftX1); 
			cvSetReal2D(boxX2, 0, 0, leftX2); 
			cvSetReal2D(boxY1, 0, 0, topY1); 
			cvSetReal2D(boxY2, 0, 0, topY2); 
			cvSetReal2D(weights, 0, 0, waveletSign); 
			
			cvSetReal2D(boxX1, 0, 1, rightX1); 
			cvSetReal2D(boxX2, 0, 1, rightX2); 			
			cvSetReal2D(boxY1, 0, 1, bottomY1); 
			cvSetReal2D(boxY2, 0, 1, bottomY2); 
			cvSetReal2D(weights, 0, 1, waveletSign); 
			
			cvSetReal2D(boxX1, 0, 2, leftX1); 
			cvSetReal2D(boxX2, 0, 2, leftX2); 
			cvSetReal2D(boxY1, 0, 2, bottomY1); 
			cvSetReal2D(boxY2, 0, 2, bottomY2); 
			cvSetReal2D(weights, 0, 2, -waveletSign); 
			
			cvSetReal2D(boxX1, 0, 3, rightX1); 
			cvSetReal2D(boxX2, 0, 3, rightX2); 			
			cvSetReal2D(boxY1, 0, 3, topY1); 
			cvSetReal2D(boxY2, 0, 3, topY2); 
			cvSetReal2D(weights, 0, 3, -waveletSign); 
			
			numBoxes = 4; 
			*/
			
			break;
			
		case 6: //up-center-down
			
			
			centerHeight = floor((fullHeight)*1.0/2);
			
			leftX1 = fullX - (floor(1.0*(fullWidth-1)/2.0));
			leftX2 = fullX + ceil(1.0*(fullWidth-1)/2.0); 
			rightX1 = leftX1; 
			rightX2 = leftX2; 
			
			topY1 = fullY - floor(1.0*(fullHeight-1)/2.0); 
			topY2 = fullY + ceil(1.0*(fullHeight-1)/2.0);  
			bottomY1 = fullY - floor(1.0*(centerHeight-1)/2.0); 
			bottomY2 = fullY + ceil(1.0*(centerHeight-1)/2.0); 
			
			
			numBoxes = 3; 
			weights.create(1,numBoxes, CV_64F); 
			
			pt1.push_back(Point(leftX1, topY1)); 
			pt2.push_back(Point(leftX2, topY2)); 
			weights.at<double>(0,0) = waveletSign; 
			
			pt1.push_back(Point(rightX1, bottomY1)); 
			pt2.push_back(Point(rightX2, bottomY2)); 
			weights.at<double>(0,1) = -waveletSign; 
			
			pt1.push_back(Point(rightX1, bottomY1)); 
			pt2.push_back(Point(rightX2, bottomY2)); 
			weights.at<double>(0,2) = -waveletSign; 
			
			/*
			 leftX1 = fullX - ceil(1.0*(fullWidth-1)/2.0) - 1; 
			 leftX2 = fullX + floor(1.0*(fullWidth-1)/2.0)-1;
			 rightX1 =  fullX - ceil(1.0*(fullWidth-1)/2.0) - 1; 
			 rightX2 =  fullX + floor(1.0*(fullWidth-1)/2.0)-1;
			 
			 topY1 = fullY - ceil(1.0*(fullHeight-1)/2.0) - 1; 
			 topY2 = fullY + floor(1.0*(fullHeight-1)/2.0)-1; 
			 bottomY1 = fullY - ceil(1.0*(fullHeight - 1)/4.0) - 1; 
			 bottomY2 = fullY + floor(1.0*(fullHeight-1)/4.0)-1; 
			 */
			/*
			cvSetReal2D(boxX1, 0, 0, leftX1); 
			cvSetReal2D(boxX2, 0, 0, leftX2); 
			cvSetReal2D(boxY1, 0, 0, topY1); 
			cvSetReal2D(boxY2, 0, 0, topY2); 
			cvSetReal2D(weights, 0, 0, waveletSign); 
			
			cvSetReal2D(boxX1, 0, 1, rightX1); 
			cvSetReal2D(boxX2, 0, 1, rightX2); 			
			cvSetReal2D(boxY1, 0, 1, bottomY1); 
			cvSetReal2D(boxY2, 0, 1, bottomY2); 
			cvSetReal2D(weights, 0, 1, -waveletSign); 
			
			cvSetReal2D(boxX1, 0, 2, rightX1); 
			cvSetReal2D(boxX2, 0, 2, rightX2); 			
			cvSetReal2D(boxY1, 0, 2, bottomY1); 
			cvSetReal2D(boxY2, 0, 2, bottomY2); 
			cvSetReal2D(weights, 0, 2, -waveletSign); 
			
			cvSetReal2D(boxX1, 0, 3, 1); 
			cvSetReal2D(boxX2, 0, 3, 1); 
			cvSetReal2D(boxY1, 0, 3, 1); 
			cvSetReal2D(boxY2, 0, 3, 1); 
			cvSetReal2D(weights, 0, 3, 0);		
			
			numBoxes = 3; 
			 */
			break;
			
		default:
			break;
	}
	
	if (_HAARFEATURE_DEBUG) cout << "Updating Kernel visualization." << endl; 
	keepBoxesInPatch(); 
	expandSymmetries(); 
	setKernelVisualization(); 
	if (_HAARFEATURE_DEBUG) cout << "Box Parameters are set." << endl; 
}
