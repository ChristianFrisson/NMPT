/**
 * \ingroup ExamplesGroup
 * \page cvprspeed_page CVPRTestSpeed 
 * \brief Reproduce the speed results from Butko and Movellan, CVPR 09 on your 
 * own machine. 
 *
 * CVPRTestSpeed
 *
 * To Run: <br>
 * (1) Uncompress and Expand the included GENKI R2009a dataset. Make sure the 
 * GENKI-R2009a folder is in the data directory: <br>
 * <tt> \>\> tar -xzvf data/GENKI-R2009a.tgz -C data/<br> </tt>
 *<br>
 * (2) Run the program.<br>
 * <tt> \>\> bin/CVPRTestSpeed.</tt>
 *
 * \b Description: 
 *
 * This example program is contained in "CVPRTestSpeed.cpp". Following the 
 * proecdure in Butko and Movellan, CVPR 2009, it calculates the speed and 
 * accuracy of plain Viola-Jones search, and of MIPOMDP-wrapped Viola-Jones 
 * search on the GENKI-SZSL subset of the GENKI data set. The results are 
 * computed using 7-Fold cross-validation. The included Multionomial observation
 * models (data/MIPOMDPData-21x21-4Scales-Holdout[0-6].txt) were compted using
 * 3000/3500 of the GENKI-SZSL images. Each file has a different 500 images held
 * out. This program evaluates each image using the model that was created when
 * this image was held out -- i.e. it was not used to fit the model parameters.
 * 
 * After each image is searched, several statistics of performance for the
 * current image are printed, separated by commas: 
 *
 * \li MIPOMDP Search Time
 * \li VJ Search Time
 * \li MIPOMDP Distance from Most Likely Face Location to True Face Location
 * \li VJ Distance from Most Likely Face Location to True Face Location
 * \li Image Width
 * \li Image Height
 * \li Estimated Probability that Face is really at Face Location
 * \li Posterior Belief Distribution Negative Entropy.
 *
 * Then, statistics of average performance are printed. 
 *
 * MIPOMDP is an extension of the IPOMDP Infomax Model of Eye-movment in Butko 
 * and Movellan, 2008; Najemnik and Geisler, 2005 (see \ref bib_sec).
 *
 **/


using namespace std; 
#include "ImageDataSet.h"
#include "MIPOMDP.h"
#include "BlockTimer.h"
#include <stdio.h>
#include <iostream> 
#include <string>

#include <opencv2/opencv.hpp>


int main(int argc, char** argv) {
	const char files[] = "data/GENKI-SZSL_files.txt"; 
	const char labels[] = "data/GENKI-R2009a/Subsets/GENKI-SZSL/GENKI-SZSL_labels.txt"; 
	
	
	CvSize gridSize = cvSize(21,21); 
	int numScales =4; 
	
	ImageDataSet* train = ImageDataSet::loadFromFile(files, labels); 
	cout << "Loaded dataset of  " << train->getNumEntries() << " files, with ";  
	cout << train->numLabelsPerImage() << " labels. " << endl; 
	
	
	BlockTimer timer; 

	
	double totalSearchTime=0, totalSearchGridErr=0, totalHiResTime=0; 
	double totalHiResGridErr=0; 
	int numImages = 0; 
	long long numPixels = 0; 
	
	for (int i = 0; i < 7; i++) {
		ImageDataSet* test = train->split(0, 499); 
		char pomdpfile[5000]; 
		snprintf(pomdpfile, 5000, 
				 "data/MIPOMDPData-%dx%d-%dScales-HoldoutSet%d.txt", 
				 gridSize.width, gridSize.height, numScales, i); 
		MIPOMDP* pomdp = MIPOMDP::loadFromFile(pomdpfile);
		pomdp->setGeneratePreview(0); 
		cout << "Holdout Set " << i << "; Loaded File " << pomdpfile << endl; 
		for (int j = 0; j < test->getNumEntries(); j++) {
			double stopconf = 0.125; 			
			string imageFile = test->getFileName(j); 
			vector<double> imageLabels = test->getFileLabels(j); 
			CvPoint faceLocation = cvPoint(imageLabels[0], imageLabels[1]); 
			
			IplImage* current_frame = cvLoadImage(imageFile.c_str(), 
												  CV_LOAD_IMAGE_GRAYSCALE ); 
			pomdp->changeInputImageSize(cvSize(current_frame->width, 
											   current_frame->height)); 
			CvPoint faceGridLocation = pomdp->gridPointForPixel(faceLocation); 			
			
			timer.blockStart(1); 
			CvPoint searchPoint = pomdp->searchFrameUntilConfident(current_frame
																   , stopconf);		
			timer.blockStop(1); 
			double searchProb = pomdp->getProb(); 
			double searchRew = pomdp->getReward(); 
			CvPoint searchGridPoint = pomdp->gridPointForPixel(searchPoint);
			pomdp->resetPrior(); 
			
			timer.blockStart(2); 
			CvPoint hiresPoint = pomdp->searchHighResImage(current_frame);			
			timer.blockStop(2); 
			CvPoint hiresGridPoint = pomdp->gridPointForPixel(hiresPoint);
			pomdp->resetPrior(); 
			
			
			double distGridSearch = sqrt((searchGridPoint.x-faceGridLocation.x)*
										 (searchGridPoint.x-faceGridLocation.x)+
										 (searchGridPoint.y-faceGridLocation.y)*
										 (searchGridPoint.y-faceGridLocation.y)
			); 
			double distGridHires = sqrt((hiresGridPoint.x-faceGridLocation.x)*
										(hiresGridPoint.x-faceGridLocation.x)+
										(hiresGridPoint.y-faceGridLocation.y)*
										(hiresGridPoint.y-faceGridLocation.y)); 
			
			double searchTime = timer.getTotTime(1); 
			double hiresTime = timer.getTotTime(2); 
			
			totalSearchTime = totalSearchTime+searchTime; 
			totalSearchGridErr = totalSearchGridErr+distGridSearch; 
			totalHiResTime = totalHiResTime+hiresTime; 
			totalHiResGridErr = totalHiResGridErr+distGridHires; 
			numImages++; 
			numPixels = numPixels + current_frame->width*current_frame->height; 
			
						
			cout << searchTime << ", " << hiresTime << ", " << distGridSearch ;
			cout << ", " << distGridHires  << ", " << current_frame->width ; 
			cout << ", " << current_frame->height << ", " << searchProb << ", ";
			cout << searchRew <<  endl; 
			
			cout << "Mean MIPOMDP Search Time: " ;
			cout << totalSearchTime / numImages << " Seconds" << endl; 
			cout << "Mean VJ Search Time     : " ; 
			cout << totalHiResTime / numImages << " Seconds" << endl; 
			cout << "Mean MIPOMDP Search Rate: " ; 
			cout << totalSearchTime*1000000/numPixels <<" ms / 1000 px" << endl; 
			cout << "Mean VJ Search Rate     : " ; 
			cout << totalHiResTime*1000000/numPixels << " ms / 1000 px" << endl; 
			cout << "Mean MIPOMDP Grid Error : " ; 
			cout << totalSearchGridErr / numImages << endl; 
			cout << "Mean VJ Grid Error      : " ; 
			cout << totalHiResGridErr / numImages << endl; 
			
			cvReleaseImage(&current_frame); 
			
			timer.blockReset(1); 
			timer.blockReset(2); 	
			
			
		}		
		delete(test); 
		delete(pomdp); 
	}
}