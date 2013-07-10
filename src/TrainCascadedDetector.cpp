/*
 *  TrainCascadedDetector.cpp
 *  OpenCV
 *
 *  Created by Nicholas Butko on 8/5/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include <fstream>
#include "GentleBoostCascadedClassifier.h"

using namespace std;

double costFun(const PerformanceMetrics& a) {
	return a.chisq; 
}


int main (int argc, char * const argv[]) 
{	
	int numFeaturesToAddToModel = 250;//500; 
	double maxPosRejectsPerRound = 0.0025; 
	double desiredNegRejectsPerRound = 1;
	int useFast = 1; 
	//double tau = .025; //.05; 
	//double eps = 0; //.001; 
	int boostRounds = 2;
	int patience = 10; 
	
	
	string datasetname = "data/GenkiSZSLFacePatches"; 
	string oldFileName = "";
	string newFileName = "data/GenkiSZSLCascade.txt" ; 
	
	setFeatureCostFunction(&costFun); 
	
	
	GentleBoostCascadedClassifier* booster = new GentleBoostCascadedClassifier(); 
	booster->setSearchParams(useFast);
	
	if (!oldFileName.empty()) {
		ifstream in; 
		in.open(oldFileName.c_str()); 
		in >> booster; 
		in.close(); 
		booster->setTrainingSet(datasetname); 
		cout << "Loaded existing GentleBoost cascaded with " << booster->getNumFeaturesTotal() << " features." << endl; 
		cout << "Its patch size is " << booster->getBasePatchSize().width << "x" << booster->getBasePatchSize().height << endl; 
	} else {
		booster->setTrainingSet(datasetname); 
		booster->setTrainingParams(maxPosRejectsPerRound, desiredNegRejectsPerRound); 
	}
	
	//FeatureRegressor::TAU = tau; 
	//FeatureRegressor::EPS = eps; 
	
	for (int iteration  = 0; iteration < numFeaturesToAddToModel; iteration++){
		cout << "Iteration Number " << iteration << endl; 
		
		PerformanceMetrics perf = booster->trainOneRound(patience, boostRounds); 
		cout << "Saving" << endl; 
		ofstream out; 
		out.open(newFileName.c_str()); 
		out << booster; 
		out.close(); 	
		cout << "Data Chi-Sq: " << perf.chisq  << " ; Pos Rejects: " << perf.pos_rejects 
		<< " ; Neg Rejects: " << perf.neg_rejects << endl; 
		cout << "======================================" << endl; 
		
		if (booster->exhaustedAllNegPatches()) break; 
	}
}