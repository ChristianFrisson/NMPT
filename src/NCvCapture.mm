/*
 *  NCvCapture.mm
 *
 *  Created by Nicholas Butko on 11/3/09.
 *  Copyright 2009. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution. 
 * 3. The name of the author may not be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO 
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "NCvCapture.h"

#ifdef __APPLE__

#include <iostream>
#import <QTKit/QTKit.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>

using namespace std; 

/********************** Declaration of class headers ************************/

/*****************************************************************************
 *
 * CaptureDelegate Declaration. 
 *
 * CaptureDelegate is notified on a separate thread by the OS whenever there
 *   is a new frame. When "updateImage" is called from the main thread, it
 *   copies this new frame into an IplImage, but only if this frame has not
 *   been copied before. When "getOutput" is called from the main thread, 
 *   it gives the last copied IplImage. 
 *
 *****************************************************************************/

#ifndef QTKIT_VERSION_7_6_3
#define QTKIT_VERSION_7_6_3         70603
#define QTKIT_VERSION_7_0           70000
#endif

#ifndef QTKIT_VERSION_MAX_ALLOWED
#define QTKIT_VERSION_MAX_ALLOWED QTKIT_VERSION_7_0
#endif


@interface CaptureDelegate : NSObject
{
	int newFrame; 
    CVImageBufferRef  mCurrentImageBuffer;
	char* imagedata; 
	IplImage* image; 
	char* bgr_imagedata; 
	IplImage* bgr_image; 
	size_t currSize; 
}

- (void)captureOutput:(QTCaptureOutput *)captureOutput 
  didOutputVideoFrame:(CVImageBufferRef)videoFrame 
	 withSampleBuffer:(QTSampleBuffer *)sampleBuffer 
	   fromConnection:(QTCaptureConnection *)connection;

- (void)captureOutput:(QTCaptureOutput *)captureOutput 
didDropVideoFrameWithSampleBuffer:(QTSampleBuffer *)sampleBuffer 
	   fromConnection:(QTCaptureConnection *)connection;

- (int)updateImage; 
- (IplImage*)getOutput; 

@end


/*****************************************************************************
 *
 * NCvCapture Declaration. 
 *
 * NCvCapture is an abstract class that can represent either a camera input or
 *   a movie file input source. From the point of view of the outside, the two
 *   are indistinguishable. However, the implementations are very different, 
 *   giving different derived classes. 
 *
 *****************************************************************************/

class NCvCapture {
public:
	virtual int grabFrame() = 0; 
	virtual IplImage* retrieveFrame() = 0;
	virtual IplImage* queryFrame() = 0; 
	virtual double getCaptureProperty(int property_id) = 0; 
	virtual int setCaptureProperty(int property_id, double value) = 0; 
	virtual ~NCvCapture() {}; 
	virtual int didStart() = 0; 
}; 


/*****************************************************************************
 *
 * NCvCaptureCAM Declaration. 
 *
 * NCvCaptureCAM is the instantiation of a capture source for cameras.
 *
 *****************************************************************************/

class NCvCaptureCAM : public NCvCapture {
public:
	NCvCaptureCAM(int cameraNum = -1) ;
	~NCvCaptureCAM(); 
	virtual int grabFrame(); 
	virtual IplImage* retrieveFrame();
	virtual IplImage* queryFrame(); 
	virtual double getCaptureProperty(int property_id); 
	virtual int setCaptureProperty(int property_id, double value); 
	virtual int didStart(); 
	
	
private:
	QTCaptureSession            *mCaptureSession;
	QTCaptureDeviceInput        *mCaptureDeviceInput;
	QTCaptureDecompressedVideoOutput    *mCaptureDecompressedVideoOutput;
	CaptureDelegate* capture; 
	
	int startCaptureDevice(int cameraNum); 
	void stopCaptureDevice(); 
	
	void setWidthHeight(); 
	int grabFrame(double timeOut); 
	
	int camNum; 
	int width; 
	int height; 
	int settingWidth;
	int settingHeight;  
	int started; 
	int disableAutoRestart; 
	
}; 


/*****************************************************************************
 *
 * NCvCaptureFile Declaration. 
 *
 * NCvCaptureFile is the instantiation of a capture source for video files.
 *
 *****************************************************************************/

class NCvCaptureFile : public NCvCapture {
public:
	NCvCaptureFile(const char* filename) ;
	~NCvCaptureFile(); 
	virtual int grabFrame(); 
	virtual IplImage* retrieveFrame();
	virtual IplImage* queryFrame(); 
	virtual double getCaptureProperty(int property_id); 
	virtual int setCaptureProperty(int property_id, double value); 
	virtual int didStart(); 
	
	
private:
	QTMovie *mCaptureSession;
	
	char* imagedata; 
	IplImage* image; 
	char* bgr_imagedata; 
	IplImage* bgr_image; 
	size_t currSize; 
	
	//IplImage* retrieveFrameBitmap(); 
	IplImage* retrieveFramePixelBuffer(); 
	double getFPS(); 
	
	int movieWidth; 
	int movieHeight; 
	double movieFPS; 
	double currentFPS; 
	double movieDuration; 
	int changedPos;
	
	int started; 
}; 


/*****************************************************************************
 *
 * NCvCaptureFile Declaration. 
 *
 * NCvCaptureFile is the instantiation of a capture source for video files.
 *
 *****************************************************************************/

class NCvVideoWriter {
public:
	NCvVideoWriter(const char* filename, int fourcc, 
				   double fps, CvSize frame_size, 
				   int is_color=1);
	~NCvVideoWriter(); 
	int writeFrame(const IplImage* image); 
private:
	IplImage* argbimage; 
	QTMovie* mMovie; 
	unsigned char* imagedata; 
	
	NSString* path; 
	NSString* codec; 
	double movieFPS; 
	CvSize movieSize; 
	int movieColor; 
};


/****************** Implementation of interface functions ********************/

NCvCapture* ncvCaptureFromFile(const char* filename) {
	//cout << "Warning: ncvCaptureFromFile not yet implemented." << endl; 
	return new NCvCaptureFile(filename); 
}

NCvCapture* ncvCreateFileCapture(const char* filename) {
	return ncvCaptureFromFile(filename);
}

NCvCapture* ncvCaptureFromCAM(int index) {
	NCvCapture* retval = new NCvCaptureCAM(index);
	if (!retval->didStart())
		cvReleaseCapture(&retval); 
	return retval; 
}

NCvCapture* ncvCreateCameraCapture(int index ) {
	return ncvCaptureFromCAM(index); 
}

void cvReleaseCapture(NCvCapture** capture) {
	if (*capture != NULL) {
		delete(*capture); 
	}
	*capture = NULL; 
}

int cvGrabFrame(NCvCapture* capture) {
	return capture->grabFrame(); 
}

IplImage* cvRetrieveFrame(NCvCapture* capture) {
	return  capture->retrieveFrame(); 
}

IplImage* cvQueryFrame(NCvCapture* capture) {
	return capture->queryFrame(); 
}

double cvGetCaptureProperty(NCvCapture* capture, int property_id) {
	return capture->getCaptureProperty(property_id); 
}

int cvSetCaptureProperty(NCvCapture* capture, int property_id, double value) {
	return capture->setCaptureProperty(property_id, value); 
}


NCvVideoWriter* ncvCreateVideoWriter(const char* filename, int fourcc, 
									 double fps, CvSize frame_size, 
									 int is_color) {
	return new NCvVideoWriter(filename, fourcc, fps, frame_size,is_color); 
}

void cvReleaseVideoWriter(NCvVideoWriter** writer){
	if (*writer != NULL) {
		delete(*writer); 
	}
	*writer = NULL; 
	return ; 
} 

int cvWriteFrame(NCvVideoWriter* writer, const IplImage* image){
	return writer->writeFrame(image); 
}


/********************** Implementation of Classes ****************************/ 

/*****************************************************************************
 *
 * NCvCaptureCAM Implementation. 
 *
 * NCvCaptureCAM is the instantiation of a capture source for cameras.
 *
 *****************************************************************************/

NCvCaptureCAM::NCvCaptureCAM(int cameraNum) {
	mCaptureSession = nil;
	mCaptureDeviceInput = nil;
	mCaptureDecompressedVideoOutput = nil;
	capture = nil; 
	
	width = 0; 
	height = 0; 
	settingWidth = 0; 
	settingHeight = 0; 
	disableAutoRestart = 0; 
	
	camNum = cameraNum; 
	
	if (!startCaptureDevice(camNum)) {
		cout << "Warning, camera failed to properly initialize!" << endl; 
		started = 0; 
	} else {
		started = 1; 
	}
	
}

NCvCaptureCAM::~NCvCaptureCAM() {
	stopCaptureDevice(); 
	
	cout << "Cleaned up camera." << endl; 
}

int NCvCaptureCAM::didStart() {
	return started; 
}


int NCvCaptureCAM::grabFrame() {
	return grabFrame(5); 
}

int NCvCaptureCAM::grabFrame(double timeOut) {
	
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
	double sleepTime = 0.005; 
	double total = 0; 
	NSDate *loopUntil = [NSDate dateWithTimeIntervalSinceNow:sleepTime];
	while (![capture updateImage] && (total += sleepTime)<=timeOut &&
		   [[NSRunLoop currentRunLoop] runMode: NSDefaultRunLoopMode 
									beforeDate:loopUntil])
		loopUntil = [NSDate dateWithTimeIntervalSinceNow:sleepTime]; 
	
	[localpool drain];
	return total <= timeOut; 
	
}

IplImage* NCvCaptureCAM::retrieveFrame() {
	return [capture getOutput]; 
}

IplImage* NCvCaptureCAM::queryFrame() {
	while (!grabFrame()) {
		cout << "WARNING: Couldn't grab new frame from camera!!!" << endl; 
		/*
		cout << "Attempting to restart camera; set capture property DISABLE_AUTO_RESTART to disable." << endl; 		
		stopCaptureDevice(); 
		startCaptureDevice(camNum); 
		 */
	}
	return retrieveFrame(); 
}	

void NCvCaptureCAM::stopCaptureDevice() {
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
	
	[mCaptureSession stopRunning];
	
	QTCaptureDevice *device = [mCaptureDeviceInput device];
    if ([device isOpen])  [device close];
	
	[mCaptureSession release];
    [mCaptureDeviceInput release];
	
	[mCaptureDecompressedVideoOutput setDelegate:mCaptureDecompressedVideoOutput]; 
	[mCaptureDecompressedVideoOutput release]; 
	[capture release]; 
	[localpool drain]; 
	
}

int NCvCaptureCAM::startCaptureDevice(int cameraNum) {
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
	
	capture = [[CaptureDelegate alloc] init]; 
	
	QTCaptureDevice *device; 
	NSArray* devices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeVideo];
	NSArray* mdevices = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeMuxed]; 
	devices = [devices arrayByAddingObjectsFromArray:mdevices]; 
	if ([devices count] == 0) {
		cout << "QTKit didn't find any attached Video Input Devices!" << endl; 
		[localpool drain]; 
		return 0; 
	}
	
	if (cameraNum >= 0) {
		camNum = cameraNum % [devices count]; 
		if (camNum != cameraNum) {
			cout << "Warning: Max Camera Num is " << [devices count]-1 << "; Using camera " << camNum << endl; 
		}
		device = [devices objectAtIndex:camNum] ;
	} else {
		device = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo]  ;
	}
	int success; 
	NSError* error; 
	
    if (device) {
		
		success = [device open:&error];
        if (!success) {
			cout << "QTKit failed to open a Video Capture Device" << endl; 			
			[localpool drain]; 
			return 0; 
        }
		
		mCaptureDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:device] ;
		mCaptureSession = [[QTCaptureSession alloc] init] ;
		
        success = [mCaptureSession addInput:mCaptureDeviceInput error:&error];		
		
		if (!success) {
			cout << "QTKit failed to start capture session with opened Capture Device" << endl;
			[localpool drain]; 
			return 0; 
        }
		
		
		mCaptureDecompressedVideoOutput = [[QTCaptureDecompressedVideoOutput alloc] init];
		[mCaptureDecompressedVideoOutput setDelegate:capture]; 
		NSDictionary *pixelBufferOptions ;
		if (width > 0 && height > 0) {
			pixelBufferOptions = [NSDictionary dictionaryWithObjectsAndKeys:
								  [NSNumber numberWithDouble:1.0*width], (id)kCVPixelBufferWidthKey,
								  [NSNumber numberWithDouble:1.0*height], (id)kCVPixelBufferHeightKey,
								  //[NSNumber numberWithUnsignedInt:k32BGRAPixelFormat], (id)kCVPixelBufferPixelFormatTypeKey,
								  [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA],
								  (id)kCVPixelBufferPixelFormatTypeKey,
								  nil]; 
		} else {
			pixelBufferOptions = [NSDictionary dictionaryWithObjectsAndKeys:
								  [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], 
								  (id)kCVPixelBufferPixelFormatTypeKey,
								  nil]; 
		}
		[mCaptureDecompressedVideoOutput setPixelBufferAttributes:pixelBufferOptions]; 
		
#if QTKIT_VERSION_MAX_ALLOWED >= QTKIT_VERSION_7_6_3
		[mCaptureDecompressedVideoOutput setAutomaticallyDropsLateVideoFrames:YES]; 
#endif
		
		
        success = [mCaptureSession addOutput:mCaptureDecompressedVideoOutput error:&error];
        if (!success) {
            cout << "QTKit failed to add Output to Capture Session" << endl; 
			[localpool drain]; 
            return 0;
        }
		
		[mCaptureSession startRunning];
		
		grabFrame(60); 
		
		return 1; 
	}
	
	[localpool drain]; 
	return 0; 
}

void NCvCaptureCAM::setWidthHeight() {	
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init]; 
	NSDictionary* pixelBufferOptions = [NSDictionary dictionaryWithObjectsAndKeys:
						  [NSNumber numberWithDouble:1.0*width], (id)kCVPixelBufferWidthKey,
						  [NSNumber numberWithDouble:1.0*height], (id)kCVPixelBufferHeightKey,
						  [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA],
						  (id)kCVPixelBufferPixelFormatTypeKey,
						  nil]; 
	
	[mCaptureDecompressedVideoOutput setPixelBufferAttributes:pixelBufferOptions];
	grabFrame(60); 
	[localpool drain]; 
}


double NCvCaptureCAM::getCaptureProperty(int property_id){
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init]; 
	
	NSArray* connections = [mCaptureDeviceInput	connections]; 
	QTFormatDescription* format = [[connections objectAtIndex:0] formatDescription]; 
	NSSize s1 = [[format attributeForKey:QTFormatDescriptionVideoCleanApertureDisplaySizeAttribute] sizeValue]; 
	
	int width=s1.width, height=s1.height; 
	switch (property_id) {
		case CV_CAP_PROP_FRAME_WIDTH:
			return width;
		case CV_CAP_PROP_FRAME_HEIGHT: 
			return height; 
		case NCV_DISABLE_AUTO_RESTART:
			return disableAutoRestart; 
		default:
			return 0; 
	}
	
	[localpool drain]; 
	
}

int NCvCaptureCAM::setCaptureProperty(int property_id, double value) {
	switch (property_id) {
		case CV_CAP_PROP_FRAME_WIDTH:
			width = value;
			settingWidth = 1; 
			if (settingWidth && settingHeight) {
				setWidthHeight(); 
				settingWidth =0; 
				settingHeight = 0; 
			}
			return 1;
		case CV_CAP_PROP_FRAME_HEIGHT:
			height = value;
			settingHeight = 1; 
			if (settingWidth && settingHeight) {
				setWidthHeight(); 
				settingWidth =0; 
				settingHeight = 0; 
			}
			return 1;
		case NCV_DISABLE_AUTO_RESTART:
			disableAutoRestart = value;
			return 1;
		default:
			return 0;;
	} 
}


/*****************************************************************************
 *
 * CaptureDelegate Implementation. 
 *
 * CaptureDelegate is notified on a separate thread by the OS whenever there
 *   is a new frame. When "updateImage" is called from the main thread, it
 *   copies this new frame into an IplImage, but only if this frame has not
 *   been copied before. When "getOutput" is called from the main thread, 
 *   it gives the last copied IplImage. 
 *
 *****************************************************************************/


@implementation CaptureDelegate 

- (id)init {
	[super init]; 
	newFrame = 0; 
	imagedata = NULL; 
	bgr_imagedata = NULL; 
	currSize = 0;
	image = NULL; 
	bgr_image = NULL; 
	return self; 
}


-(void)dealloc {
	if (imagedata != NULL) free(imagedata); 
	if (bgr_imagedata != NULL) free(bgr_imagedata); 
	cvReleaseImage(&image); 
	cvReleaseImage(&bgr_image); 
	[super dealloc]; 
}

- (void)captureOutput:(QTCaptureOutput *)captureOutput 
  didOutputVideoFrame:(CVImageBufferRef)videoFrame 
	 withSampleBuffer:(QTSampleBuffer *)sampleBuffer 
	   fromConnection:(QTCaptureConnection *)connection {
	
    CVBufferRetain(videoFrame);
	CVImageBufferRef imageBufferToRelease  = mCurrentImageBuffer;
    
    @synchronized (self) {
		
        mCurrentImageBuffer = videoFrame;
		newFrame = 1; 
    }
	
	CVBufferRelease(imageBufferToRelease);
    
}
- (void)captureOutput:(QTCaptureOutput *)captureOutput 
didDropVideoFrameWithSampleBuffer:(QTSampleBuffer *)sampleBuffer 
	   fromConnection:(QTCaptureConnection *)connection {
	//cout << "Camera dropped frame!" << endl; 
}

-(IplImage*) getOutput {
	return bgr_image; 
}

-(int) updateImage {
	
	if (newFrame==0) return 0; 
	CVPixelBufferRef pixels; 
	
	@synchronized (self){
		pixels = CVBufferRetain(mCurrentImageBuffer);
		newFrame = 0; 
	}
	
	CVPixelBufferLockBaseAddress(pixels, 0);		
	uint32_t* baseaddress = (uint32_t*)CVPixelBufferGetBaseAddress(pixels);
	
	size_t width = CVPixelBufferGetWidth(pixels);
	size_t height = CVPixelBufferGetHeight(pixels);
	size_t rowBytes = CVPixelBufferGetBytesPerRow(pixels);
	
	if (rowBytes != 0) { 
		
		if (currSize != rowBytes*height*sizeof(char)) {
			currSize = rowBytes*height*sizeof(char); 
			if (imagedata != NULL) free(imagedata); 
			if (bgr_imagedata != NULL) free(bgr_imagedata); 
			imagedata = (char*)malloc(currSize); 
			bgr_imagedata = (char*)malloc(currSize); 
		}
		
		memcpy(imagedata, baseaddress, currSize);
		
		if (image == NULL) {
			image = cvCreateImageHeader(cvSize(width,height), IPL_DEPTH_8U, 4); 
		}
		image->width =width; 
		image->height = height; 
		image->nChannels = 4; 
		image->depth = IPL_DEPTH_8U; 
		image->widthStep = rowBytes; 
		image->imageData = imagedata; 
		image->imageSize = currSize; 
		
		if (bgr_image == NULL) {
			bgr_image = cvCreateImageHeader(cvSize(width,height), IPL_DEPTH_8U, 3); 
		}
		bgr_image->width =width; 
		bgr_image->height = height; 
		bgr_image->nChannels = 3; 
		bgr_image->depth = IPL_DEPTH_8U; 
		bgr_image->widthStep = rowBytes; 
		bgr_image->imageData = bgr_imagedata; 
		bgr_image->imageSize = currSize; 
		
		cvCvtColor(image, bgr_image, CV_BGRA2BGR); 
		
	}
	
	CVPixelBufferUnlockBaseAddress(pixels, 0);
	CVBufferRelease(pixels); 
	
	return 1; 
}

@end


/*****************************************************************************
 *
 * NCvCaptureFile Implementation. 
 *
 * NCvCaptureFile is the instantiation of a capture source for video files.
 *
 *****************************************************************************/

NCvCaptureFile::NCvCaptureFile(const char* filename) {
	
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
	
	
	mCaptureSession = nil;
	image = NULL; 
	bgr_image = NULL; 
	imagedata = NULL; 
	bgr_imagedata = NULL; 
	currSize = 0; 
	
	movieWidth = 0; 
	movieHeight = 0; 
	movieFPS = 0; 
	currentFPS = 0; 
	movieDuration = 0; 
	changedPos = 0; 
	
	started = 0; 
	
	NSError* error; 
	
	
	mCaptureSession = [[QTMovie movieWithFile:[NSString stringWithCString:filename 
																 encoding:NSASCIIStringEncoding] 
										error:&error] retain]; 
	[mCaptureSession setAttribute:[NSNumber numberWithBool:YES] 
						   forKey:QTMovieLoopsAttribute]; 
	
	if (mCaptureSession == nil) {
		cout << "WARNING: Couldn't read movie file " << filename << endl; 
		[localpool drain]; 
		started = 0; 
		return; 
	}
	
	
	[mCaptureSession gotoBeginning]; 
	
	NSSize size = [[mCaptureSession attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];
	
	movieWidth = size.width; 
	movieHeight = size.height; 
	movieFPS = getFPS(); 
	currentFPS = movieFPS; 
	
	QTTime t; 
	
	[[mCaptureSession attributeForKey:QTMovieDurationAttribute] getValue:&t]; 
	movieDuration = (t.timeValue *1000.0 / t.timeScale); 
	started = 1; 
	[localpool drain]; 
	
}

NCvCaptureFile::~NCvCaptureFile() {
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
	if (imagedata != NULL) free(imagedata); 
	if (bgr_imagedata != NULL) free(bgr_imagedata); 
	cvReleaseImage(&image);  
	cvReleaseImage(&bgr_image); 
	[mCaptureSession release]; 
	[localpool drain]; 
}

int NCvCaptureFile::didStart() {
	return started; 
}

int NCvCaptureFile::grabFrame() {
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
	double t1 = getCaptureProperty(CV_CAP_PROP_POS_MSEC); 
	[mCaptureSession stepForward]; 
	double t2 = getCaptureProperty(CV_CAP_PROP_POS_MSEC); 
	if (t2>t1 && !changedPos) {
		currentFPS = 1000.0/(t2-t1); 
	} else {
		currentFPS = movieFPS; 
	}
	changedPos = 0; 
	[localpool drain]; 
	return 1; 
}


IplImage* NCvCaptureFile::retrieveFramePixelBuffer() {
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
	
	
	NSDictionary *attributes = [NSDictionary dictionaryWithObjectsAndKeys:
								QTMovieFrameImageTypeCVPixelBufferRef, QTMovieFrameImageType,
#ifdef MAC_OS_X_VERSION_10_6
								[NSNumber numberWithBool:YES], QTMovieFrameImageSessionMode,
#endif
								nil];
	CVPixelBufferRef frame = (CVPixelBufferRef)[mCaptureSession frameImageAtTime:[mCaptureSession currentTime]
																  withAttributes:attributes 
																		   error:nil]; 
	
	CVPixelBufferRef pixels = CVBufferRetain(frame);
	CVPixelBufferLockBaseAddress(pixels, 0);		
	
	uint32_t* baseaddress = (uint32_t*)CVPixelBufferGetBaseAddress(pixels);
	size_t width = CVPixelBufferGetWidth(pixels);
	size_t height = CVPixelBufferGetHeight(pixels);
	size_t rowBytes = CVPixelBufferGetBytesPerRow(pixels);
	
	if (rowBytes != 0) { 
		
		if (currSize != rowBytes*height*sizeof(char)) {
			currSize = rowBytes*height*sizeof(char); 
			if (imagedata != NULL) free(imagedata); 
			if (bgr_imagedata != NULL) free(bgr_imagedata); 
			imagedata = (char*)malloc(currSize);  
			bgr_imagedata = (char*)malloc(currSize); 
		}
		
		memcpy(imagedata, baseaddress, currSize);
		
		//ARGB -> BGRA
		for (unsigned int i = 0; i < currSize; i+=4) {
			char temp = imagedata[i]; 
			imagedata[i] = imagedata[i+3]; 
			imagedata[i+3] = temp; 
			temp = imagedata[i+1]; 
			imagedata[i+1] = imagedata[i+2]; 
			imagedata[i+2] = temp; 
		}
		
		if (image == NULL) {
			image = cvCreateImageHeader(cvSize(width,height), IPL_DEPTH_8U, 4); 
		}
		
		image->width =width; 
		image->height = height; 
		image->nChannels = 4; 
		image->depth = IPL_DEPTH_8U; 
		image->widthStep = width*4; 
		image->imageData = imagedata; 
		image->imageSize = currSize; 
		
		
		if (bgr_image == NULL) {
			bgr_image = cvCreateImageHeader(cvSize(width,height), IPL_DEPTH_8U, 3); 
		}
		
		bgr_image->width =width; 
		bgr_image->height = height; 
		bgr_image->nChannels = 3; 
		bgr_image->depth = IPL_DEPTH_8U; 
		bgr_image->widthStep = width*4; 
		bgr_image->imageData = bgr_imagedata; 
		bgr_image->imageSize = currSize; 
		
		cvCvtColor(image, bgr_image,CV_BGRA2BGR); 
		
	}
	
	CVPixelBufferUnlockBaseAddress(pixels, 0);
	CVBufferRelease(pixels);
	
	[localpool drain]; 
	
	return bgr_image; 
}


IplImage* NCvCaptureFile::retrieveFrame() {
	return retrieveFramePixelBuffer(); 
}

IplImage* NCvCaptureFile::queryFrame() {
	grabFrame(); 
	return retrieveFrame(); 
}	

double NCvCaptureFile::getFPS() {
	if (mCaptureSession == nil) return 0; 
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
	double now = getCaptureProperty(CV_CAP_PROP_POS_MSEC); 
	double retval = 0; 
	if (now == 0) {
		[mCaptureSession stepForward]; 
		double t2 =  getCaptureProperty(CV_CAP_PROP_POS_MSEC); 
		[mCaptureSession stepBackward];
		retval = 1000.0 / (t2-now); 
	} else {
		[mCaptureSession stepBackward]; 
		double t2 = getCaptureProperty(CV_CAP_PROP_POS_MSEC); 
		[mCaptureSession stepForward]; 
		retval = 1000.0 / (now-t2); 
	}
	[localpool drain]; 
	return retval; 
}

double NCvCaptureFile::getCaptureProperty(int property_id){
	if (mCaptureSession == nil) return 0;
	
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init]; 
	
	double retval; 
	QTTime t; 
	
	switch (property_id) {
		case CV_CAP_PROP_POS_MSEC:
			[[mCaptureSession attributeForKey:QTMovieCurrentTimeAttribute] getValue:&t]; 
			retval = t.timeValue * 1000.0 / t.timeScale; 
			break; 
		case CV_CAP_PROP_POS_FRAMES:
			retval = movieFPS * getCaptureProperty(CV_CAP_PROP_POS_MSEC) / 1000;
			break; 
		case CV_CAP_PROP_POS_AVI_RATIO:
			retval = (getCaptureProperty(CV_CAP_PROP_POS_MSEC)) / (movieDuration ); 
			break; 
		case CV_CAP_PROP_FRAME_WIDTH:
			retval = movieWidth;
			break; 
		case CV_CAP_PROP_FRAME_HEIGHT:  
			retval = movieHeight; 
			break;
		case CV_CAP_PROP_FPS:
			retval = currentFPS;  
			break; 
		case CV_CAP_PROP_FOURCC:
		default:
			retval = 0; 
	}
	
	[localpool drain]; 
	return retval; 
}

int NCvCaptureFile::setCaptureProperty(int property_id, double value) {

	if (mCaptureSession == nil) return 0;
	
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init]; 
	
	double retval = 0; 
	QTTime t; 
	
	double ms; 
	
	switch (property_id) {
		case CV_CAP_PROP_POS_MSEC:
			[[mCaptureSession attributeForKey:QTMovieCurrentTimeAttribute] getValue:&t]; 
			t.timeValue = value * t.timeScale / 1000; 
			[mCaptureSession setCurrentTime:t]; 
			changedPos = 1; 
			retval = 1; 
			break; 
		case CV_CAP_PROP_POS_FRAMES:
			ms = (value*1000.0 -5)/ currentFPS; 
			setCaptureProperty(CV_CAP_PROP_POS_MSEC, ms); 
			retval = 1; 
			break; 
		case CV_CAP_PROP_POS_AVI_RATIO:
			ms = value * movieDuration; 
			setCaptureProperty(CV_CAP_PROP_POS_MSEC, ms); 
			retval = 1; 
			break; 
		case CV_CAP_PROP_FRAME_WIDTH:
			//retval = movieWidth;
			break; 
		case CV_CAP_PROP_FRAME_HEIGHT:  
			//retval = movieHeight; 
			break;
		case CV_CAP_PROP_FPS:
			//etval = currentFPS;  
			break; 
		case CV_CAP_PROP_FOURCC:
		default:
			retval = 0; 
	}
	
	[localpool drain]; 
	return retval; 
}


/*****************************************************************************
 *
 * NCvVideoWriter Implementation. 
 *
 * NCvVideoWriter is the instantiation of a video output class
 *
 *****************************************************************************/


NCvVideoWriter::NCvVideoWriter(const char* filename, int fourcc, 
							   double fps, CvSize frame_size, 
							   int is_color) {
	
	
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
	
	movieFPS = fps; 
	movieSize = frame_size; 
	movieColor = is_color; 
	mMovie = nil; 
	path = [[[NSString stringWithCString:filename encoding:NSASCIIStringEncoding] stringByExpandingTildeInPath] retain];
	
	argbimage = cvCreateImage(movieSize, IPL_DEPTH_8U, 4); 
	
	
	char cc[5]; 
	cc[0] = fourcc & 255; 
	cc[1] = (fourcc >> 8) & 255; 
	cc[2] = (fourcc >> 16) & 255; 
	cc[3] = (fourcc >> 24) & 255; 
	cc[4] = 0; 
	int cc2 = CV_FOURCC(cc[0], cc[1], cc[2], cc[3]); 
	if (cc2!=fourcc) {
		cout << "WARNING: Didn't properly encode FourCC. Expected " << fourcc 
		<< " but got " << cc2 << "." << endl;
	}
	
	codec = [[NSString stringWithCString:cc encoding:NSASCIIStringEncoding] retain]; 
	
	NSError *error = nil;
    if (!mMovie) {
		
		NSFileManager* files = [NSFileManager defaultManager]; 
		if ([files fileExistsAtPath:path]) {
			if (![files removeItemAtPath:path error:nil]) {
				cout << "WARNING: Failed to remove existing file " << [path cStringUsingEncoding:NSASCIIStringEncoding] << endl; 
			}
		}
		
        mMovie = [[QTMovie alloc] initToWritableFile:path error:&error];
        if (!mMovie) {
            cout << "WARNING: Could not create empty movie file container." << endl; 
			[localpool drain]; 
            return;
        }
    }
	
	[mMovie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieEditableAttribute];
	
	[localpool drain]; 
}


NCvVideoWriter::~NCvVideoWriter() {
	cvReleaseImage(&argbimage); 	
	
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
	[mMovie release]; 
	[path release]; 
	[codec release]; 
	[localpool drain]; 
}

int NCvVideoWriter::writeFrame(const IplImage* image) {
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init]; 
	
	cvCvtColor(image, argbimage, CV_BGR2BGRA); 
	
	
	unsigned char* imagedata = (unsigned char*)argbimage->imageData; 
	//BGRA --> ARGB 
	
	for (int j = 0; j < argbimage->height; j++) {
		int rowstart = argbimage->widthStep * j; 
		for (int i = rowstart; i < rowstart+argbimage->widthStep; i+=4) {
			unsigned char temp = imagedata[i];
			imagedata[i] = 255; 
			imagedata[i+3] = temp;   
			temp = imagedata[i+2];
			imagedata[i+2] = imagedata[i+1];
			imagedata[i+1] = temp; 
		}
	}
	
	NSBitmapImageRep* imageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&imagedata 
																		 pixelsWide:movieSize.width 
																		 pixelsHigh:movieSize.height 
																	  bitsPerSample:8 
																	samplesPerPixel:4 
																		   hasAlpha:YES
																		   isPlanar:NO
																	 colorSpaceName:NSDeviceRGBColorSpace
																	   bitmapFormat:NSAlphaFirstBitmapFormat
																		bytesPerRow:argbimage->widthStep
																	   bitsPerPixel:32]  ; 
	
	
	NSImage* nsimage = [[NSImage alloc] init]; 
	
	[nsimage addRepresentation:imageRep];
	
	/*  
	 codecLosslessQuality          = 0x00000400,
	 codecMaxQuality               = 0x000003FF,
	 codecMinQuality               = 0x00000000,
	 codecLowQuality               = 0x00000100,
	 codecNormalQuality            = 0x00000200,
	 codecHighQuality              = 0x00000300
	 */
	
	[mMovie addImage:nsimage forDuration:QTMakeTime(100,100*movieFPS) withAttributes:[NSDictionary dictionaryWithObjectsAndKeys:
																					  codec, QTAddImageCodecType, 
																					  //[NSNumber numberWithInt:codecLowQuality], QTAddImageCodecQuality,
																					  [NSNumber numberWithInt:100*movieFPS], QTTrackTimeScaleAttribute,nil]];
	
	if (![mMovie updateMovieFile]) {
		cout << "Didn't successfully update movie file." << endl; 
	}
	
	[imageRep release]; 
	[nsimage release]; 
	[localpool drain]; 
	
	return 1; 
}

#endif
