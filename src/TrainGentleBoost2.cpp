/*
 *  TrainGentleBoost2.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 10/22/10.
 *  Copyright 2010 UC San Diego. All rights reserved.
 *
 */

#include <iostream>
#include <fstream>
#include "GentleBoostClassifier2.h"
#include "ImagePatch2.h"
#include "PatchDataset2.h"

using namespace std;
using namespace cv; 

int main (int argc, char * const argv[]) 
{	
	int numFeaturesToAddToModel = 250; 
	
	int boostRounds = 2;
	int patience = 1; 
	int startOver = 0; 
	
	
	string datasetname = "data/GenkiSZSLFacePatches"; 
	string fileName = "data/GenkiSZSLBoost.txt" ; 
	
	GentleBoostClassifier2 booster; 
	
	if (!startOver) {
		FileStorage file(fileName, FileStorage::READ); 
		if (file.isOpened()) {
			file["GentleBoostClassifier"] >> booster; 
			cout << "Adding to a GentleBoostClassifier with " 
			<< booster.getNumFeaturesTotal() << " features." << endl; 
		}
		file.release(); 
	}
	
	PatchDataset2 data; 
	FileStorage datafile(datasetname, FileStorage::READ); 
	datafile["dataset"] >> data; 
	datafile.release(); 
	
	booster.setTrainingSet(data); 

	for (int iteration  = 0; iteration < numFeaturesToAddToModel; iteration++){
		cout << "Training Iteration Number " << (iteration+1) << endl; 
		
		PerformanceMetrics perf = booster.trainOneRound(patience, boostRounds); 
		FileStorage file(fileName, FileStorage::WRITE);
		file << "GentleBoostClassifier" << booster; 
		file.release(); 
		cout << "Training Data Chi-Sq: " << perf.chisq  <<  endl; 
		cout << "======================================" << endl; 
		
	}
}