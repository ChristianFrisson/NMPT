/*
 *  CreateBoostingInitialPatchDataset2.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 10/25/10.
 *  Copyright 2010 UC San Diego. All rights reserved.
 *
 */


#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include "ImagePatch2.h"
#include "PatchDataset2.h"

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
	PatchDataset2 dataset(patchSize, objectfiles, objectlabels);
	
	cout << "Dataset has " << dataset.getPatches().size() << " patches ("
	<< dataset.getPosPatches().size() << " positive, " 
	<< dataset.getNegPatches().size() << " negative)." << endl; 
	
	
	cout << "Finished Grabbing Patches, Writing to file." << endl; 
	FileStorage file; 
	file.open(saveFileName, FileStorage::WRITE); 
	file << "dataset" << dataset; 
	file.release();
	
	if (verifydataset) {
		PatchDataset2 data2;
		cout << "Reading in patches from file." << endl; 
		file.open(saveFileName, FileStorage::READ); 
		file["dataset"] >> data2; 
		file.release(); 
		
		vector<ImagePatch2> posPatches = data2.getPosPatches(); 
		vector<ImagePatch2> negPatches = data2.getNegPatches(); 
		
		cout << "Read dataset with " << posPatches.size() << " positive and " << negPatches.size() << " negative patches." << endl; 
		int scale = 10; 
		patchSize = data2.getPatchSize(); 
		Size bigSize(patchSize.width*scale, patchSize.height*scale); 
		Mat displayImage(Size(bigSize.width*2, bigSize.height), CV_8U); 
		int i = 0;
		int key=0; 
		while (key <= 0) {
			int pInd = i%posPatches.size(); 
			int nInd = i%negPatches.size();
			i++; 
			Mat pIm, nIm, tmp; 
			posPatches[pInd].createRepIfNeeded(1); 
			negPatches[nInd].createRepIfNeeded(1); 
			pIm = posPatches[pInd].getImageHeader(); 
			nIm = negPatches[nInd].getImageHeader(); 
			
			tmp = displayImage(Rect(0,0,bigSize.width,bigSize.height));
			resize(pIm, tmp , bigSize, 0, 0, INTER_NEAREST); 
			tmp = displayImage(Rect(bigSize.width, 0, bigSize.width,bigSize.height));
			resize(nIm, tmp, bigSize, 0, 0,  INTER_NEAREST); 
			imshow("Example Patches", displayImage);
			key = waitKey(200); 
		}				
	}
}
