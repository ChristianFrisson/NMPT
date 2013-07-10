/**
 * \ingroup ExamplesGroup
 * \page cvprtrain_page CVPRTrainModels 
 * \brief Reproduce the Multinomial Observation Models used to generate results 
 * in Butko and Movellan, CVPR 09 on your own machine. This file is included for
 * instructional purposes -- the files that it creates are already included in 
 * the data directory (data/MIPOMDPData-21x21-4Scales-*.txt).
 *
 * CVPRTrainModels
 *
 * To Run: <br>
 * (1) Uncompress and Expand the included GENKI R2009a dataset. Make sure the 
 * GENKI-R2009a folder is in the data directory: <br>
 * <tt> \>\> tar -xzvf data/GENKI-R2009a.tgz -C data/<br> </tt>
 * <br>
 * (2) Run the program.<br>
 * <tt> \>\> bin/CVPRTrainModels.</tt>
 *
 * \b Description: 
 *
 * This example program is contained in "CVPRTrainModels.cpp". Following the 
 * proecdure in Butko and Movellan, CVPR 2009, it calculates the coefficients of
 * many multinomial distributions based on object detector performance. As a 
 * result of running this program, several MIPOMDPData text files are generated
 * and saved to the data/ directory: 
 * 
 * \li data/MIPOMDPData-21x21-4Scales-AllImages.txt	- Calculates parameters 
 * using all 3500 images in the GENKI-SZSL Directory.
 * \li data/MIPOMDPData-21x21-4Scales-ImageSet[0-6].txt - Calculates parameters 
 * using non-overlapping blocks of 500 images each.
 * \li data/MIPOMDPData-21x21-4Scales-HoldoutSet[0-6].txt - Calculates 
 * parameters using 3000 images (all but the 500 included in ImageSet[0-6].txt ;
 * I.e. HoldoutSet0 uses all but the first 500 images, which are used in 
 * ImageSet0. HoldoutSet6 uses all but the last 500 images, which are used in 
 * ImageSet6.
 * \li data/MIPOMDPData-21x21-4Scales-NoTraining.txt - Uses multionomials with
 * parameters set by a heuristic function. These parameters come from the 
 * default MultinomialObservationModel constructor. 
 *
 * The files created can be used to examine the Multinomial Observation Model
 * Parameters directly, or they can be loaded as MIPOMDP Objects that can be 
 * used to search for objects. The included program \ref cvprspeed_page uses the
 * HoldoutSet[0-6] models. All other example programs use the AllImages model. 
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
	

	MIPOMDP* pomdp = new MIPOMDP(); 
	CvSize gridSize = pomdp->getGridSize(); 
	int numScales = pomdp->getNumScales(); 
	
	cout << "Loading." << endl; 
	ImageDataSet* train = ImageDataSet::loadFromFile(files, labels); 
	cout << "Loaded " << train->getNumEntries() << " files, with " ; 
	cout << train->numLabelsPerImage() << " labels. " << endl; 
	
	char savename[5000]; 
	snprintf(savename, 5000, "data/MIPOMDPData-%dx%d-%dScales-NoTraining.txt", 
			 gridSize.width, gridSize.height, numScales);  
	pomdp->saveToFile(savename); 
	
	for (int i = 0; i < 7; i++) {
		cout << "Getting counts for image set " << i << endl; 
		ImageDataSet* test = train->split(0, 499); 
		pomdp->trainObservationModel(test); 
		cout << "Trained on " << test->getNumEntries() << " images. " ; 
		cout << train->getNumEntries() << " images remain." << endl; 
		snprintf(savename, 5000, 
				 "data/MIPOMDPData-%dx%d-%dScales-ImageSet%d.txt", 
				 gridSize.width, gridSize.height, numScales,i); 
		pomdp->saveToFile(savename); 
		delete(test); 
	}
	
	for (int i = 0; i <=7; i++) {
		pomdp->resetModel(); 
		cout << "Combining counts for holdout set " << i << endl; 
		for (int j = 0; j < 7; j++) {
			if (i==j) continue; 
			snprintf(savename, 5000, 
					 "data/MIPOMDPData-%dx%d-%dScales-ImageSet%d.txt", 
					 gridSize.width, gridSize.height, numScales,j); 
			MIPOMDP* other = MIPOMDP::loadFromFile(savename); 
			cout << savename << endl; 
			pomdp->combineModels(other); 
			delete(other); 
		}
		if (i < 7) {
			snprintf(savename, 5000, 
					 "data/MIPOMDPData-%dx%d-%dScales-HoldoutSet%d.txt", 
					 gridSize.width, gridSize.height, numScales,i);
			pomdp->saveToFile(savename); 
		}
	}
	
	cout << "Saving parameters from whole dataset." << endl; 
	snprintf(savename, 5000, "data/MIPOMDPData-%dx%d-%dScales-AllImages.txt", 
			 gridSize.width, gridSize.height, numScales); 
	pomdp->saveToFile(savename); 
	
	
}