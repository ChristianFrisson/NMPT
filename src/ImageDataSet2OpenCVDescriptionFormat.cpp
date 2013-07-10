/*
 *  ImageDataSet2OpenCVVectorFormat.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 8/24/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */



#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include "DetectionEvaluator.h"
#include "ImageDataSet.h"

using namespace std; 

int main (int argc, char * const argv[]) 
{	
	/*
	 const char objectfiles[]  = "data/GENKI-SZSL_files.txt"; 
	 const char objectlabels[] = "data/GENKI-R2009a/Subsets/GENKI-SZSL/GENKI-SZSL_labels.txt"; 
	 const char bgfiles[]      = "data/BGFilesList.txt"; 
	 const char saveFileName[] = "data/FacePatches"; 
	 */
	
	string objectfiles  = "AllCoinIms.txt"; 
	string objectlabels = "AllCoinLabels.txt"; 
	string saveFileName = "data/AllCoinDescriptionFormat.txt"; 
	
	//const char objectfiles[]  = "data/CoinDataFromVideo/VideoCoinNames.txt"; 
	//const char objectlabels[] = "data/CoinDataFromVideo/VideoCoinLabels.txt";  
	//const char saveFileName[] = "data/VideoCoinDescriptionFormat.txt"; 
	
	DetectionEvaluator*  evaluator = new DetectionEvaluator(objectfiles, objectlabels); 
		
	ofstream out; 
	out.open(saveFileName.c_str()); 
	out << evaluator->outputOpenCVDescriptionFormatForHaarTraining(); 
	out.close(); 	
	
	
}
