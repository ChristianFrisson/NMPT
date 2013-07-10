/*
 *  TrainNarrowFOVModel.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 4/18/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


/**
 * \ingroup ExamplesGroup
 * \page narrowfov_page TrainNarrowFOVModel
 * \brief Create a restricted field of view model to illustrate how the MIPOMDP
 * can simulate an active camera. This file is included for
 * instructional purposes -- the files that it creates are already included in 
 * the data directory (data/MIPOMDPData-21x21-3Scales-AllImages.txt).
 *
 * TrainNarrowFOVModel
 *
 * To Run: <br>
 * (1) Uncompress and Expand the included GENKI R2009a dataset. Make sure the 
 * GENKI-R2009a folder is in the data directory: <br>
 * <tt> \>\> tar -xzvf data/GENKI-R2009a.tgz -C data/<br> </tt>
 * <br>
 * (2) Run the program.<br>
 * <tt> \>\> bin/TrainNarrowFOVModel.</tt>
 *
 * \b Description: 
 *
 * This example program is contained in "TrainNarrowFOVModel.cpp". Following the 
 * proecdure in Butko and Movellan, CVPR 2009, it calculates the coefficients of
 * many multinomial distributions based on object detector performance. As a 
 * result of running this program, an MIPOMDPData text file is generated
 * and saved to the data/ directory: 
 * 
 * \li data/MIPOMDPData-21x21-4Scales-NarrowFOV.txt	- Calculates parameters 
 * using all 3500 images in the GENKI-SZSL Directory.
 *
 * The file created can be used to examine the Multinomial Observation Model
 * Parameters directly, or it can be loaded as an MIPOMDP Objects that can be 
 * used to search for objects. The included program \ref 
 * foveatedfacetracker_page uses this model. All other example programs use the 
 * AllImages model. 
 *
 * MIPOMDP is an extension of the IPOMDP Infomax Model of Eye-movment in Butko 
 * and Movellan, 2008; Najemnik and Geisler, 2005 (see \ref bib_sec).
 **/
using namespace std; 
#include "ImageDataSet.h"
#include "MIPOMDP.h"
#include <stdio.h>
#include <iostream> 
#include <opencv2/opencv.hpp>

int main(int argc, char** argv) {
	const char files[] = "data/GENKI-SZSL_files.txt"; 
	const char labels[] = "data/GENKI-R2009a/Subsets/GENKI-SZSL/GENKI-SZSL_labels.txt"; 
	char savename[5000]; 
	
	const int numScales = 3; 
	int numGridPoints = 21; 
	
	CvSize inputImageSize = cvSize (1000, 1000); 
	CvSize subImageSize = cvSize(100, 100); 
	CvSize gridSize = cvSize(numGridPoints, numGridPoints); 
	CvMat* subImageGridPoints = cvCreateMat(numScales, 2, CV_32SC1); 
	for (int i = 0; i < numScales; i++) {
		cvSetReal2D(subImageGridPoints, i, 0, 15-6*i); //15, 9, 3 
		cvSetReal2D(subImageGridPoints, i, 1, 15-6*i); //15, 9, 3
	}
	const char* haarDetectorXMLFile = "data/haarcascade_frontalface_alt2.xml";
	
	
	
	MIPOMDP* pomdp = new MIPOMDP(inputImageSize, subImageSize, gridSize, 
								 numScales, subImageGridPoints, 
								 haarDetectorXMLFile); 
	
	
	cout << "Loading." << endl; 
	ImageDataSet* train = ImageDataSet::loadFromFile(files, labels); 
	cout << "Loaded " << train->getNumEntries() << " files, with " ; 
	cout << train->numLabelsPerImage() << " labels. " << endl; 
	
	pomdp->trainObservationModel(train); 
	
	cout << "Saving parameters from whole dataset." << endl; 
	snprintf(savename, 5000, "data/MIPOMDPData-%dx%d-%dScales-AllImages.txt", 
			 gridSize.width, gridSize.height, numScales); 
	pomdp->saveToFile(savename); 
	
	
}