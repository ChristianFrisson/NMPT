/**
 * \ingroup ExamplesGroup
 * \page foveatedfacetracker_page FoveatedFaceTracker
 * 
 * \brief A slightly more complex program using the MIPOMDP class: takes a few 
 * more input types than the simple face tracker, and displays more information. 
 *
 * First Full-featured Example of Multinomial-Infomax-POMDP for Faster Face 
 * Tracking (MIPOMDP), from Butko and Movellan, 2009. (see \ref bib_sec).
 *
 * To Run: <br>
 * <tt> \>\> bin/FoveatedFaceTracker [optional-path-to-movie-file]</tt>
 *
 * To Quit: <br>
 * Press the 'q' key from the video window.
 *
 * \b Description: 
 *
 * This example program is contained in "FoveatedFaceTracker.cpp". It demostrates 
 * some of the internal processes of the MIPOMDP class. It takes as input an
 * attached camera (if no input arguments are provided), or any movie file that
 * OpenCV can read. 
 * 
 * It displays the result of the MIPOMDP tracking algorithm as well as optional
 * visualizations of the algorithm. 
 *
 * In the main program window, the display is controlled by the following 
 * keys: 
 * \li 'q': Quit.
 * \li 't': Toggle display of probabilities and face counts as text.
 * \li 'b': Toggle display of belief map.
 * \li 'f': Toggle display of framerate.
 * \li 'h': Toggle display of hi-res full-frame search (disables belief map).
 * \li 'r': Reset the belief about the face location.
 * 
 * Tip: If the output is not changing, select the display window 
 *         and try moving the mouse.
 **/



#include <opencv2/opencv.hpp>
#include <cassert>
#include <iostream>
#include <fstream>
#include "MIPOMDP.h"
#include "BlockTimer.h"

using namespace std;

#define showSlider 1
#define dispHeight 750

int sizeSlider = 0; 
int maxReduce = 8; 

const char  * WINDOW_NAME  = "MIPOMDP Simple Face Tracking Example" ;
IplImage* color_image = NULL; 
IplImage* current_frame = NULL; 
IplImage* gray_image = NULL ; 
IplImage* full_disp_window = NULL; 
IplImage* big_gray_float = NULL; 
MIPOMDP* facetracker = NULL; 
int usingCamera = 1; 


int imWidth, imHeight, movieWidth, movieHeight; 

void changeImSize(int doNotUseMe=0){
	if (color_image != current_frame) cvReleaseImage(&color_image); 
	cvReleaseImage(&gray_image); 
	if (sizeSlider > 0) {
		imWidth = movieWidth*(maxReduce-sizeSlider)/maxReduce; 
		imHeight = movieHeight*(maxReduce-sizeSlider)/maxReduce; 
		color_image= cvCreateImage(cvSize (imWidth, imHeight ), IPL_DEPTH_8U, 3);
	} else {
		imWidth = movieWidth; 
		imHeight = movieHeight; 
		color_image = current_frame; 
	}
	cout << "Width: " << imWidth << "; Height: " << imHeight << endl << endl; 
	facetracker->changeInputImageSize(cvSize(imWidth, imHeight)); 
	gray_image = cvCreateImage(cvSize (imWidth, imHeight ),	 IPL_DEPTH_8U, 1);
	if (sizeSlider > 0) {
		cvResize(current_frame, color_image, CV_INTER_NN); 
	} else {
		color_image = current_frame; 
	}
	
	//Put the current frame the format expected by the MIPOMDP algorithm		
	cvCvtColor (color_image, gray_image, CV_BGR2GRAY);
	
	if (usingCamera) 	
		cvFlip( gray_image, NULL, 1);
	
	
	
	IplImage* newBig = cvCreateImage(cvSize (imWidth, imHeight ), 
											 IPL_DEPTH_32F, 1);
	IplImage* newFull= cvCreateImage(cvSize (imWidth, 2*imHeight ), 
											   IPL_DEPTH_32F, 3);
	if (big_gray_float)
		cvResize(big_gray_float, newBig);
	if (full_disp_window)
		cvResize(full_disp_window, newFull); 
	
	cvReleaseImage(&big_gray_float);
	cvReleaseImage(&full_disp_window); 
	big_gray_float = newBig; 
	full_disp_window = newFull; 

}

				   
int main (int argc, char * const argv[]) 
{
		
	//Check if there are any command line arguments -- if not, use a movie. If so, 
	//open the video. 
	CvCapture* movie = NULL; 
	if (argc < 2) {
		cout << "Getting Camera " << CV_CAP_ANY << endl; 	
		movie = cvCreateCameraCapture (CV_CAP_ANY);
	} else {
		if (argv[1][0] == '-') {
			cout << "An advanced example program illustrating the use of the MIPOMDP class that " << endl
			<< "    allows the user to select input source and view different aspects of the algorithm." << endl << endl
			<< "Usage: " << endl << "    >> bin/FoveatedFaceTracker [optional-path-to-movie-file]" << endl << endl 
			<< "If no optional path to a movie file is provided, attempts to use an attached" << endl 
			<< "    camera for input. Otherwise, attempts to use the movie file for input." << endl << endl
			<< "Note: Requires the data files \"data/haarcascade_frontalface_alt2.xml\" and" << endl
			<< "    \"data/MIPOMDPData-21x21-3Scales-AllImages.txt\" to function properly." << endl << endl; 

			return 0; 
		}
		cout << "Getting Movie " << argv[1] << endl; 
		movie = cvCreateFileCapture(argv[1]); 
		usingCamera = 0; 
	}
	
    if (! movie) {
		cout << "Failed to get input from movie or movie file." << endl << endl
		<< "Usage: " << endl << "    >> bin/FoveatedFaceTracker [optional-path-to-movie-file]" << endl << endl 
		<< "If no optional path to a movie file is provided, attempts to use an attached" << endl 
		<< "    camera for input. Otherwise, attempts to use the movie file for input." << endl << endl
		<< "Note: Requires the data files \"data/haarcascade_frontalface_alt2.xml\" and" << endl
		<< "    \"data/MIPOMDPData-21x21-3Scales-AllImages.txt\" to function properly." << endl << endl; 
		return 0; 
	}
	
	
	
	//Get an initial frame so we know what size the image will be. 
    current_frame = cvQueryFrame (movie);
//	imwidth = current_frame->width; 
//	imheight= current_frame->height;
	
	
	cout << "*************************************************************************" << endl;
	cout << "*  Running Foveated Face Tracker Example." << endl; 
	cout << "*    TIPS: " << endl;
	cout << "*    -- Press 'q' to quit." << endl;
	cout << "*    -- Press 't' to toggle display of probabilities and face counts as text." << endl; 
	cout << "*    -- Press 'b' to toggle display of belief map." << endl; 
	cout << "*    -- Press 'f' to toggle display of framerate." << endl; 
	cout << "*    -- Press 'h' to toggle display of hi-res full-frame search." << endl; 
	cout << "*    -- Press 'r' to reset the belief about the face location." << endl; 
	cout << "*    -- Press 's' to toggle small vs. full-sized window." << endl; 
	cout << "*    -- Reducing video size will make the demo more responsive." << endl;  
	cout << "*    -- If the output is not changing, select the display window " << endl;
	cout << "*         and try moving the mouse." << endl; 
	cout << "*************************************************************************" << endl << endl;
	
	
	BlockTimer timer; //The timer is used to track the frame rate. 
	timer.blockStart(0); 
	
    cvNamedWindow (WINDOW_NAME, CV_WINDOW_AUTOSIZE); //Create the graphical algorithm display
	CvFont font; 
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX, .25, .25); 
	if (showSlider)
		cvCreateTrackbar( "Reduce Video Size", WINDOW_NAME, &sizeSlider, maxReduce-1, &changeImSize); 
	
	//Get an initial frame so we know what size the image will be. 
	movieWidth = current_frame->width;//cvGetCaptureProperty(movie, CV_CAP_PROP_FRAME_WIDTH); 
	movieHeight= current_frame->height;//cvGetCaptureProperty(movie, CV_CAP_PROP_FRAME_HEIGHT); 
	
	int frameHeight = dispHeight/2; 
	int frameWidth = frameHeight*movieWidth/movieHeight; 
	
	//Create the initial saliency tracker with the default constructor
	facetracker = MIPOMDP::loadFromFile("data/MIPOMDPData-21x21-3Scales-AllImages.txt"); 
	
	//Tell the face tracker what size it needs to be. 
	changeImSize(0); 
	
	//Make some intermediate image representations: 
	
	
	//(1) The downsized representation of the original image frame
	
	
	IplImage* small_gray_int = cvCreateImage(cvSize (frameWidth, frameHeight ), 
											 IPL_DEPTH_8U, 1);
	IplImage* small_gray_float = cvCreateImage(cvSize (frameWidth,frameHeight ), 
											   IPL_DEPTH_32F, 1);
	IplImage* small_color_float = cvCreateImage(cvSize (frameWidth, frameHeight ), 
												IPL_DEPTH_32F, 3);
	IplImage* disp_window = cvCreateImage(cvSize (frameWidth, 2*frameHeight ), 
										  IPL_DEPTH_32F, 3);
	
	IplImage* logbel = cvCloneImage(facetracker->currentBelief); 
	
	
	int showProb = 1; 
	int showMap = 1; 
	int showFPS = 1; 
	int showHiRes = 0; 
	int showSmall = 1; 
	char str[1000] = {'\0'};
	
	
	
	int key=0; 
    while (key != 'q' && key != 'Q') //Loop until user enters 'q'
    {
		current_frame = cvQueryFrame (movie); 
		if (sizeSlider > 0) {
			cvResize(current_frame, color_image, CV_INTER_NN); 
		} else {
			color_image = current_frame; 
		}
		
		if (key == 'F' || key == 'f') showFPS = !showFPS; 
		if (key == 'B' || key == 'b') showMap = !showMap; 
		if (key == 'T' || key == 't') showProb = !showProb; 
		if (key == 'H' || key == 'h') showHiRes = !showHiRes; 
		if (key == 'R' || key == 'r') facetracker->resetPrior(); 
		if (key == 'S' || key == 's') showSmall = !showSmall; 
		
		if (current_frame==NULL) { //implies reached movie end
			cvSetCaptureProperty(movie, CV_CAP_PROP_POS_FRAMES, 0); //reset movie to beginning
			current_frame = cvQueryFrame (movie) ; 
			if (current_frame==NULL) //shouldn't happen
				break; 
		}		
		
		
			
		//Put the current frame the format expected by the MIPOMDP algorithm		
		cvCvtColor (color_image, gray_image, CV_BGR2GRAY);
		if (usingCamera) 	
			cvFlip( gray_image, NULL, 1);
		
		//Call the "searchNewFrame" method.
		timer.blockStart(1); 
		if (showHiRes) {
			facetracker->searchHighResImage(gray_image); 
		} else {
			facetracker->searchNewFrame(gray_image); 
		}
		timer.blockStop(1); 
		
		//Put the result into the display window
		if( current_frame->origin != IPL_ORIGIN_TL ) //on some systems, the image is upside down
			cvFlip(facetracker->foveaRepresentation, NULL, 0); 
		
		
		if (showSmall) {
			cvResize(facetracker->foveaRepresentation, small_gray_int); 
			cvCvtScale(small_gray_int, small_gray_float, 1.0/256); 
			cvCvtColor(small_gray_float, small_color_float, CV_GRAY2BGR);  
			cvSetImageROI(disp_window, cvRect(0, 0, frameWidth, frameHeight)); 
			cvCopy(small_color_float, disp_window); 
		} else {			
			cvCvtScale(facetracker->foveaRepresentation, big_gray_float, 1.0/256); 
			cvSetImageROI(full_disp_window, cvRect(0, 0, imWidth, imHeight)); 
			cvCvtColor(big_gray_float, full_disp_window, CV_GRAY2BGR); 
		}
		
		
		if (showProb) {
			//sprintf(str,"FastSUN: %03.1f MS;   Total: %03.1f MS", 1000.0*fps, 1000.0*tot); 		
			for (int i = 0; i < facetracker->currentBelief->width; i++) {
				for (int j= 0; j < facetracker->currentBelief->height; j++) {
					int c = (int)cvGetReal2D(facetracker->getCounts(),i,j); 
					if (c > 0) {
						sprintf(str, "%3d", c); 	
						CvPoint loc=facetracker->pixelForGridPoint(cvPoint(j,i)); 
						if (showSmall) {
							loc.x = loc.x*frameWidth/imWidth-7; 
							loc.y = loc.y*frameHeight/imHeight+2; 
							cvPutText( disp_window, str,loc, &font, CV_RGB(255,0,255) );
						} else {
							loc.x = loc.x -7; 
							loc.y = loc.y +2; 
							cvPutText( full_disp_window, str,loc, &font, CV_RGB(255,0,255) );
						}
					}
				}
			}
		}
		
		
		if (showMap && !showHiRes) {
			cvResetImageROI(disp_window); 
			cvResetImageROI(full_disp_window); 
			double scale = .1; 
			cvLog(facetracker->currentBelief, logbel); 
			cvScale(logbel, logbel, -scale); 
			
			if (showSmall) {
				cvResize(logbel, small_gray_float, CV_INTER_NN); 
				//cvAddS(small_gray_float, cvRealScalar(scale), small_gray_float); 
				cvCvtColor(small_gray_float, small_color_float, CV_GRAY2BGR);  
				cvSetImageROI(disp_window,cvRect(0, frameHeight, frameWidth, frameHeight)); 
				cvCopy(small_color_float, disp_window); 
				cvResetImageROI(disp_window); 
			} else {
				cvResize(logbel, big_gray_float, CV_INTER_NN); 
				cvSetImageROI(full_disp_window,cvRect(0, imHeight, imWidth, imHeight)); 
				cvCvtColor(big_gray_float, full_disp_window, CV_GRAY2BGR); 
				cvResetImageROI(full_disp_window); 
			}
			
			if (showProb) {
				//sprintf(str,"FastSUN: %03.1f MS;   Total: %03.1f MS", 1000.0*fps, 1000.0*tot); 		
				for (int i = 0; i < facetracker->currentBelief->width; i++) {
					for (int j= 0; j < facetracker->currentBelief->height; j++) {
						sprintf(str, "%5.2f", cvGetReal2D(facetracker->currentBelief,i,j)*100); 	
						CvPoint loc=facetracker->pixelForGridPoint(cvPoint(j,i)); 
						if (showSmall) {
							loc.x = loc.x*frameWidth/imWidth-10; 
							loc.y = loc.y*frameHeight/imHeight+frameHeight; 
							cvPutText( disp_window, str,loc, &font, CV_RGB(255,0,255) );
						} else {
							loc.x = loc.x - 10; 
							loc.y = loc.y + imHeight ; 
							cvPutText (full_disp_window, str, loc, &font, CV_RGB(255,0,255)); 
						}
					}
				}
			}
			
		}
		
		
		timer.blockStop(0); 
		
		//Get the time for the saliency algorithm, and for everything in total, and
		//reset the timers. 
		double tot = timer.getTotTime(0); 
		double fps = timer.getTotTime(1); 
		
		timer.blockReset(0); 
		timer.blockReset(1); 
		timer.blockStart(0); 
		
		//Print the timing information to the display image
		if (showFPS) {
		char str[1000] = {'\0'};
		sprintf(str,"MIPOMDP: %03.1f MS;   Total: %03.1f MS", 1000.0*fps, 1000.0*tot); 			
			if (showSmall) {
		cvPutText( disp_window, str, cvPoint(20,10), &font, CV_RGB(255,0,255) );
			} else {
				cvPutText( full_disp_window, str, cvPoint(20,10), &font, CV_RGB(255,0,255) );
			}
		}
		
		//Display the result to the main window		
		if (showSmall) {
		cvShowImage (WINDOW_NAME, disp_window);
		}else {
			cvShowImage(WINDOW_NAME, full_disp_window); 
		}
		
		//Check if any keys have been pressed, for quitting.
		key = cvWaitKey (5);
		
	} 
	
	// clean up
	cvReleaseCapture(&movie); 
	return 0;
}
