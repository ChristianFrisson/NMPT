/**
 * \ingroup ExamplesGroup
 * \page simplefacetracker_page SimpleFaceTracker
 * 
 * \brief An example the simplest program using of the MIPOMDP class. 
 *
 * First Simple Example of Multinomial-Infomax-POMDP for Faster Face Tracking (MIPOMDP),
 * from Butko and Movellan, 2009. (see \ref bib_sec).
 *
 * To Run: <br>
 * <tt> \>\> bin/SimpleFaceTracker</tt>
 *
 * To Quit: <br>
 * Press the 'q' key from the video window.
 *
 * \b Description: 
 *
 * This example program is contained in "SimpleFaceTracker.cpp". It demostrates 
 * the simplest usage of the MIPOMDP class. It takes as input one of the included example
 * movies. On each frame, it chooses a different region of the video to fixate in a way that
 * is designed to rapidly gather information about the location of the face. 
 *
 * The major steps in creating and using an MIPOMDP object are: 
 * \li Load an MIPOMDP data file. These data files can be generated using the program "bin/CVPRTrainModels", 
 * but examples are already included in the data directory. 
 * \li Tell the MIPOMDP data structure what size frame to expect.
 * \li Use OpenCV to load an image or a frame of video into memory.
 * \li Use OpenCV to convert the image to grayscale.
 * \li Call the MIPOMDP's "searchNewFrame" method on the grayscale image. 
 * \li Access the result via the "foveaRepresentation" member variable.
 *
 * Here are examples of code that this program uses to accomplish these tasks: 
 *
 * <b>"Load an MIPOMDP data file."</b>
 *
 * <tt>MIPOMDP* facetracker = MIPOMDP::loadFromFile("data/MIPOMDPData-21x21-4Scales-AllImages.txt"); </tt>
 *
 * <b>"Tell the MIPOMDP data structure what size frame to expect."</b>
 *
 * <tt>facetracker->changeInputImageSize(cvSize(movieWidth, movieHeight));  </tt>
 *
 * <b>"Use OpenCV to load an image or a frame of video into memory."</b> First we load the movie into a
 * cvCapture object by
 *
 * <tt>CvCapture* movie = cvCreateFileCapture("data/HDMovieClip.avi");</tt>
 *
 * Then on every iteration of the main program loop, we query the next frame: 
 *
 * <tt>current_frame = cvQueryFrame(movie);</tt>
 *
 * <b>"Use OpenCV to convert the image to grayscale."</b> 
 *
 * <tt>cvCvtColor (current_frame, gray_image, CV_BGR2GRAY);</tt>
 *
 * <b>"Call the MIPOMDP's "searchNewFrame" method on the grayscale image."</b> 
 *
 * <tt>facetracker->searchNewFrame(gray_image); </tt>	
 *
 * <b>"Access the result via the 'foveaRepresentation' member variable."</b> 
 *
 * <tt>cvShowImage (WINDOW_NAME, facetracker->foveaRepresentation); </tt>
 *
 * This concludes the tutorial of the simplest usage of the MIPOMDP class.
 *
 **/



#include<opencv2/core/core_c.h>
#include<opencv2/highgui/highgui_c.h>
#include<opencv2/imgproc/imgproc_c.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include "MIPOMDP.h"
#include "BlockTimer.h"

using namespace std;

const char  * WINDOW_NAME  = "MIPOMDP Simple Face Tracking Example" ;


int main (int argc, char * const argv[]) 
{
	if (argc > 1) {
		cout << endl << "A Simple Demonstration of the usage of the MIPOMDP class. Must be called" << endl
		<< "    with no arguments." << endl << endl 
		<< "Usage:" << endl
		<< "    >>bin/SimpleFaceTracker" << endl << endl
		<< "Searches for the movie \"HDMovieClip.avi\" in the folder \"data\" and aborts if not" << endl
		<< "    found. Should be run from the root NMPT directory." << endl << endl; 
		return 0; 
	}
	
	//Get input from the included video
	CvCapture* movie = cvCreateFileCapture("data/HDMovieClip.avi"); 
	
    if (! movie) {
		cout<< "Couldn't find movie: data/HDMovieClip.avi - Try running from the NMPT directory." << endl << endl
		<< "Usage:" << endl
		<< "    >>bin/SimpleSaliencyExample" << endl << endl
		<< "Searches for the movie \"HDMovieClip.avi\" in the folder \"data\" and aborts if not" << endl
		<< "    found. Should be run from the root NMPT directory." << endl << endl; 
        return 0;
	}
	
	
	
	cout << "*************************************************************************" << endl;
	cout << "*  Running Simple Face Tracker Example." << endl; 
	cout << "*    TIPS: " << endl;
	cout << "*    -- Press 'q' to quit." << endl;
	cout << "*    -- If the output is not changing, select the display window " << endl;
	cout << "*         and try moving the mouse." << endl; 
	cout << "*************************************************************************" << endl << endl;
	
    cvNamedWindow (WINDOW_NAME, CV_WINDOW_AUTOSIZE); //Create the graphical algorithm display
	
	//Get an initial frame so we know what size the image will be. 
	int movieWidth = cvGetCaptureProperty(movie, CV_CAP_PROP_FRAME_WIDTH); 
	int movieHeight= cvGetCaptureProperty(movie, CV_CAP_PROP_FRAME_HEIGHT); 
	
	//Create the initial saliency tracker with the default constructor
	MIPOMDP* facetracker = MIPOMDP::loadFromFile("data/MIPOMDPData-21x21-4Scales-AllImages.txt"); 
	
	//Tell the face tracker what size it needs to be. 
	facetracker->changeInputImageSize(cvSize(movieWidth, movieHeight)); 
	
	//Make some intermediate image representations: 
	
	//(0) The frame we will get from the movie
    IplImage *  current_frame = NULL; 
	
	//(1) The downsized representation of the original image frame
	IplImage* gray_image = cvCreateImage(cvSize (movieWidth, movieHeight ), 
												IPL_DEPTH_8U, 1);
	
	
	
	int key=0; 
    while (key != 'q' && key != 'Q') //Loop until user enters 'q'
    {
		current_frame = cvQueryFrame (movie); 
		
		if (current_frame==NULL) { //implies reached movie end
			cvSetCaptureProperty(movie, CV_CAP_PROP_POS_FRAMES, 0); //reset movie to beginning
			current_frame = cvQueryFrame (movie) ; 
			if (current_frame==NULL) //shouldn't happen
				break; 
		}		
		
		//Put the current frame the format expected by the MIPOMDP algorithm		
		cvCvtColor (current_frame, gray_image, CV_BGR2GRAY);
		
		//Call the "updateSaliency" method.
		facetracker->searchNewFrame(gray_image); 
		
		//Display the result to the main window
		if( current_frame->origin != IPL_ORIGIN_TL ) //on some systems, the image is upside down
			cvFlip(facetracker->foveaRepresentation, NULL, 0); 
		cvShowImage (WINDOW_NAME, facetracker->foveaRepresentation);
		
		//Check if any keys have been pressed, for quitting.
		key = cvWaitKey (5);
		
	} 
	
	// clean up
	cvReleaseCapture(&movie); 
	return 0;
}
