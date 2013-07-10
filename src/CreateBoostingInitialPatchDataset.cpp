/*
 *  CreateBoostingPatchDataset.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 8/5/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <opencv2/opencv.hpp>
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include "Feature.h"
#include "BoxFeature.h"
#include "HaarFeature.h"
#include "ImagePatch.h"
#include "BlockTimer.h"
#include "PatchDataset.h"
#include "FeatureRegressor.h"
#include "GentleBoostClassifier.h"
#include "GentleBoostCascadedClassifier.h"
#include "PatchList.h" 
#include "FastPatchList.h"

using namespace std; 
using namespace cv; 


int main (int argc, char * const argv[]) 
{	
	int verifydataset = 1; 
	
	Size patchSize = cvSize(30,30); 
	string objectfiles = "data/GENKI-SZSL_files_resolve_links.txt";//"data/GENKI-SZSL_files.txt"; 
	string objectlabels = "data/GENKI-R2009a/Subsets/GENKI-SZSL/GENKI-SZSL_labels.txt"; 
	string saveFileName = "data/GenkiSZSLFacePatches"; 
	
	cout << "Loading Patches" << endl; 
	PatchDataset dataset(patchSize, objectfiles, objectlabels);
	  
	cout << "Dataset has " << dataset.getPatches().size() << " patches ("
	<< dataset.getPosPatches().size() << " positive, " 
	<< dataset.getNegPatches().size() << " negative)." << endl; 
	
		 
	cout << "Finished Grabbing Patches, Writing to file." << endl; 
	dataset.writeToFile(saveFileName); 
	
	if (verifydataset) {
		cout << "Reading in patches from file." << endl; 
		PatchDataset* datasetPtr = PatchDataset::readFromFile(saveFileName); 
		const char* WINDOW_NAME = "Example Patches"; 
		cvNamedWindow (WINDOW_NAME, CV_WINDOW_AUTOSIZE);
		vector<ImagePatch*> posPatches = datasetPtr->getPosPatches(); 
		vector<ImagePatch*> negPatches = datasetPtr->getNegPatches(); 
		int scale = 10; 
		patchSize = datasetPtr->getPatchSize(); 
		Size bigSize(patchSize.width*scale, patchSize.height*scale); 
		Mat displayImage(Size(bigSize.width*2, bigSize.height), CV_8U); 
		int i = 0;
		int key=0; 
		while (key != 'q'  && key != 'Q') {
			int pInd = i%posPatches.size(); 
			int nInd = i%negPatches.size();
			i++; 
			Mat pIm, nIm, tmp; 
			posPatches[pInd]->getImageHeader(pIm); 
			negPatches[nInd]->getImageHeader(nIm); 
			
			tmp = displayImage(Rect(0,0,bigSize.width,bigSize.height));
			resize(pIm, tmp , bigSize, 0, 0, INTER_NEAREST); 
			tmp = displayImage(Rect(bigSize.width, 0, bigSize.width,bigSize.height));
			resize(nIm, tmp, bigSize, 0, 0,  INTER_NEAREST); 
			imshow(WINDOW_NAME, displayImage);
			key = cvWaitKey(200); 
		}
		
		delete(datasetPtr) ;
		
	}
}
