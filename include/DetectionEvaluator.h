/*
 *  DetectionEvaluator.h
 *  OpenCV
 *
 *  Created by Nicholas Butko on 8/20/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef DETECTIONEVALUATOR_H
#define DETECTIONEVALUATOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>
#include <vector>
#include <string>
#include "PatchList.h"
#include "ImageDataSet.h"
#include "StructTypes.h"


bool operator<(const EvaluationMetrics& a, const EvaluationMetrics& b); 


/**
 *\ingroup AuxGroup
 * \brief  <tt>Auxilliary Tool:</tt> A tool for evaluating the performance 
 * of a whole-image-object-detector, such as a GentleBoostCascadedClassifier,
 * for performance on an ImageDataSet. 
 * 
 * Specifically, the evaluation method is fashioned after the "miss-rate as
 * a function of false positives per image" introduced by Dollar et al. in
 * P. Dollar, C. Wojek, B. Schiele, and P. Perona. Pedestrian detection: 
 * A benchmark. In CVPR, June 2009.
 *
 * \author Nicholas Butko
 * \date 2010
 * \version 0.4
 */
class DetectionEvaluator {
public: 
	
	/**
	 * \brief Match parameter: defines the overlap requirement between
	 * detected objects and known dataset objects. Should be (0-1].
	 *
	 * Specifically, a "hit" is recorded if the area union of a reported 
	 * bounding box and a dataset object bounding box is acceptArea * intersection.
	 * 1 means the overlap must be perfect. The default value, .5, means the 
	 * union must be at least half of the intersection. 
	 */
	static double acceptArea; 
	
	/**
	 * \brief Constructor. 
	 * 
	 * @param imageDataSet The labeled images that we want to use to evaluate
	 * for object detector performance. 
	 */
	DetectionEvaluator(ImageDataSet* imageDataSet); 
	
	/**
	 * \brief Constructor. 
	 * 
	 * @param fileListFile Path to a file containing file names of images
	 * in the dataset. One file name corresponds to the location of one object,
	 * so the same filename may be listed multiple times. 
	 *
	 * @param labelFile Path to a file containing object locations in the images
	 * listed in fileListFile. Each line is a triple containing object center x,
	 * object center y, and object width/height. This is the same format used
	 * by ImageDataSet. 
	 */
	DetectionEvaluator(const std::string &fileListFile, const std::string &labelFile); 
	
	/**
	 * \brief Destructor.
	 */
	~DetectionEvaluator();
	
	
	/**
	 * \brief Get the names of the images used for evaluation. In contrast to
	 * the ImageDataSet format, these names are unique. 
	 * 
	 * @return A list of file names containing images in the dataset, one entry
	 * per image.
	 */
 	std::vector<std::string> getFileNames(); 
	
	
	/**
	 * \brief Get the location of the images used for evaluation. There can be
	 * several in each image; there is one vector per file name, and possibly
	 * several elements per vector.
	 * 
	 * @return A list of target location lists. 
	 */
	std::vector<std::vector<cv::Rect> > getTargetLocations(); 
	
	
	/**
	 * \brief Evaluate the number of hits, misses, and false alarms on a given
	 * image with a given threshold. The object detector output must be supplied
	 * in the form of a vector of SearchResults.
	 * 
	 * @param imNum Index of image in the dataset.
	 * @param boxes Output of an object detector, such as a 
	 * GentleBoostCascadedClassifier, evaluated on the image whose path is
	 * getFileNames()[imNum].
	 * @param threshold Detection threshold. SearchResult data with value less
	 * than threshold are not considered.
	 *
	 * @return Performance on a single image for a given threshold, in terms
	 * of hits, misses, and false_alarms. 
	 */
	EvaluationMetrics evaluateImagePerformanceWithThreshold(int imNum, 
															std::vector<SearchResult> boxes,
															double threshold); 
	
	/**
	 * \brief Evaluate the number of hits, misses, and false alarms on a given
	 * image across a range of thresholds. The object detector output must be 
	 * supplied in the form of a vector of SearchResults.
	 * 
	 * @param imNum Index of image in the dataset.
	 * @param boxes Output of an object detector, such as a 
	 * GentleBoostCascadedClassifier, evaluated on the image whose path is
	 * getFileNames()[imNum].
	 *
	 * @return Performance on a single image for a range of thresholds, in terms
	 * of hits, misses, and false_alarms. 
	 */
	std::vector<EvaluationMetrics> evaluateImagePerformance(int imNum, const std::vector<SearchResult>& boxes); 
	
	/**
	 * \brief Evaluate the number of hits, misses, and false alarms on all
	 * dataset images across a range of thresholds. The object detector output 
	 * must be supplied in the form of a vector of SearchResults vectors, one
	 * per image.
	 * 
	 * @param imBoxes Output of an object detector, such as a 
	 * GentleBoostCascadedClassifier, evaluated on every image in the dataset.
	 *
	 * @return Performance on all images for a range of thresholds, in terms
	 * of hits, misses, and false_alarms. 
	 */
	std::vector<EvaluationMetrics> evaluatePerformance(const std::vector<std::vector<SearchResult> >& imBoxes); 
	
	/**
	 * \brief Get the "Description Format" used by OpenCV for Haar Training.
	 *
	 * This is useful for converting between an ImageDataset and OpenCV's format
	 * which is more closely mirrored by DetectionEvaluator's internal data
	 * structures. 
	 * 
	 * @return A string, suitable for writing to a file, that can be used to
	 * specify the data needed to train an OpenCV Haar Cascade. 
	 */
	std::string outputOpenCVDescriptionFormatForHaarTraining(); 
	
protected:	
	
	
	std::vector<std::string> fileNames; 
	std::vector<std::vector<cv::Rect> > targets; 
	EvaluationMetrics getPerformanceAtThreshold(const std::vector<EvaluationMetrics>& ms, 
														double threshold); 
	void init(ImageDataSet* labeledImages) ; 
};

#endif
