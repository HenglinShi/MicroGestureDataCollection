#include "stdafx.h"
#include <omp.h>
#include <time.h> 
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <Windows.h>
#include <kinect.h>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/contrib/contrib.hpp>
#include "opencv2/video/video.hpp"
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iterator>
#include <fstream>
#include <sstream>
#include <vector>
#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>

using namespace std;
using namespace cv;

#define COLOR_FRAME_CHANNEL_BGR 3
#define COLOR_FRAME_CHANNEL_BGRA 4

#define TESTMODE false

//**********************************************************************************************
//**********************************************************************************************
//Kinect sdk variables																		/**/
/**/
//overall sensor variables																	/**/
IKinectSensor *kinectSensor = nullptr;														/**/
ICoordinateMapper* multisourceCoordinateMapper;												/**/
IMultiSourceFrameReader * multiSourceFrameReader = nullptr;									/**/
IMultiSourceFrame * multiSourceFrame = nullptr;												/**/
																							/**/
																							//color frame variables																		/**/
IColorFrameReference * mColorFrameReference = nullptr;										/**/
IColorFrame *mColorFrame = nullptr;															/**/
IFrameDescription * mColorFrameDescription = nullptr;										/**/
																							/**/
																							//depth frame variables																		/**/
IDepthFrameReference * mDepthFrameReference = nullptr;										/**/
IDepthFrame *mDepthFrame = nullptr;															/**/
IFrameDescription * mDepthFrameDescription = nullptr;										/**/
																							/**/
																							//body index frame variables																/**/
IBodyIndexFrameReference * mBodyIndexFrameReference = nullptr;								/**/
IBodyIndexFrame *mBodyIndexFrame = nullptr;													/**/
IFrameDescription * mBodyIndexFrameDescription = nullptr;									/**/
																							/**/
																							//body frame variables																		/**/
IBodyFrameReference * mBodyFrameReference = nullptr;										/**/
IBodyFrame * mBodyFrame = nullptr;															/**/
																							/**/
																							//**********************************************************************************************
																							//**********************************************************************************************
int height_depthFrame = 0;
int height_colorFrame = 0;
int height_bodyIndexFrame = 0;																						//Frame heights and widths
int height_bodyFrame = 0;


int width_depthFrame = 0;
int width_colorFrame = 0;
int width_bodyIndexFrame = 0;																						//Frame heights and widths
int width_bodyFrame = 0;


//Distance threashold for depth frame
USHORT depthFrameMinReliableDistance = 0;
USHORT depthFrameMaxReliableDistance = 0;

//Frame size, for OPENCV OPENCV
Size size_colorFrame(width_colorFrame, height_colorFrame);
Size size_depthFrame(width_depthFrame, height_depthFrame);
Size size_bodyIndexFrame(width_bodyIndexFrame, height_bodyIndexFrame);
Size size_bodyFrame(width_bodyFrame, height_bodyFrame);


UINT  size_colorFrameArray_BGR = 0;
UINT size_colorFrameArray_BGRA = 0;
UINT  size_bodyIndexFrameArray_BGR = 0;
UINT  size_bodyFrameArray_BGR = 0;

UINT  size_depthFrameArray_BGRA = 0;
UINT  size_depthFrameArray_BGR = 0;

BYTE * depthFrameArray_BGR = nullptr;
BYTE * colorFrameArray_BGRA = nullptr;
BYTE * colorFrameArray_YUY2 = nullptr;
BYTE * bodyIndexFrameArray_BGR = nullptr;
BYTE * bodyFrameArray_BGR = nullptr;

UINT  size_depthFrameArray_rawDepth = 0;
UINT  size_bodyIndexFrameArray_rawBodyIndex = 0;

UINT16 * depthFrameArray_rawDepth = nullptr;
BYTE * bodyIndexFrameArray_rawBodyIndex = nullptr;

Mat colorFrameMat;
Mat depthFrameMat;
Mat bodyIndexFrameMat;
Mat bodyFrameMat;


UINT colorFramePixelNum = 0;




IBody* bodies[BODY_COUNT] = { 0 };
IBody * trackedBody;
BOOLEAN isTracked = false;

Joint joints[JointType_Count];
JointOrientation jointsOrientations[JointType_Count];

Joint jointTmp;
JointOrientation jointOrientationTmp;
DepthSpacePoint depthSpacePointTmp[JointType_Count];
CvPoint depthSkeletonPoint[JointType_Count] = { cvPoint(-1.0, -1.0) };


float skeletonJoints[9 * JointType_Count] = { 0 };
//Mat drawAperson(CvPoint *jointsPoints, CvScalar skeletonColor, int skeletonThickness, Mat tmpSkeletonMat);

//HRESULT initializing();










HRESULT initializingKinectDevice()
{
	if (SUCCEEDED(GetDefaultKinectSensor(&kinectSensor))) {

		if (SUCCEEDED(kinectSensor->get_CoordinateMapper(&multisourceCoordinateMapper))) {
			kinectSensor->Open();
			if (SUCCEEDED(kinectSensor->OpenMultiSourceFrameReader(
				FrameSourceTypes::FrameSourceTypes_Depth |
				FrameSourceTypes::FrameSourceTypes_Color |
				FrameSourceTypes::FrameSourceTypes_Body |
				FrameSourceTypes::FrameSourceTypes_BodyIndex,
				&multiSourceFrameReader)))
				return S_OK;

		}
	}
	return E_FAIL;
}

HRESULT initializingBodyFrame() {

	if (SUCCEEDED(multiSourceFrame->get_BodyFrameReference(&mBodyFrameReference))) {
		if (SUCCEEDED(mBodyFrameReference->AcquireFrame(&mBodyFrame))) {

			mBodyFrameReference->Release();
			mBodyFrame->Release();
			return S_OK;
		}
		mBodyFrameReference->Release();
	}
	return E_FAIL;
}


HRESULT initializingColorFrame() {
	if (SUCCEEDED(multiSourceFrame->get_ColorFrameReference(&mColorFrameReference))) {
		if (SUCCEEDED(mColorFrameReference->AcquireFrame(&mColorFrame))) {
			if (SUCCEEDED(mColorFrame->get_FrameDescription(&mColorFrameDescription))) {
				if (SUCCEEDED(mColorFrameDescription->get_Height(&height_colorFrame)) &&
					SUCCEEDED(mColorFrameDescription->get_Width(&width_colorFrame))) {

					colorFramePixelNum = height_colorFrame * width_colorFrame;
					size_colorFrameArray_BGRA = colorFramePixelNum * COLOR_FRAME_CHANNEL_BGRA;

					if (size_colorFrameArray_BGRA > 0) {

						colorFrameArray_BGRA = new BYTE[size_colorFrameArray_BGRA];

						mColorFrameDescription->Release();
						mColorFrame->Release();
						mColorFrameReference->Release();
						return S_OK;
					}
				}
				mColorFrameDescription->Release();
			}
			mColorFrame->Release();
		}
		mColorFrameReference->Release();
	}
	return E_FAIL;
}

HRESULT initializingDepthFrame() {

	if (SUCCEEDED(multiSourceFrame->get_DepthFrameReference(&mDepthFrameReference))) {
		if (SUCCEEDED(mDepthFrameReference->AcquireFrame(&mDepthFrame))) {
			if (SUCCEEDED(mDepthFrame->get_FrameDescription(&mDepthFrameDescription))) {

				if (SUCCEEDED(mDepthFrameDescription->get_Height(&height_depthFrame)) &&
					SUCCEEDED(mDepthFrameDescription->get_Width(&width_depthFrame))) {

					size_depthFrameArray_rawDepth = height_depthFrame * width_depthFrame;
					if (size_depthFrameArray_rawDepth) {

						if (SUCCEEDED(mDepthFrame->get_DepthMaxReliableDistance(&depthFrameMaxReliableDistance)) &&
							SUCCEEDED(mDepthFrame->get_DepthMinReliableDistance(&depthFrameMinReliableDistance))) {

							depthFrameArray_rawDepth = new UINT16[size_depthFrameArray_rawDepth];

							mDepthFrameDescription->Release();
							mDepthFrame->Release();
							mDepthFrameReference->Release();
							return S_OK;
						}
					}
				}
				mDepthFrameDescription->Release();
			}
			mDepthFrame->Release();
		}
		mDepthFrameReference->Release();
	}

	return E_FAIL;
}

HRESULT initializingBodyIndexFrame() {

	if (SUCCEEDED(multiSourceFrame->get_BodyIndexFrameReference(&mBodyIndexFrameReference))) {
		if (SUCCEEDED(mBodyIndexFrameReference->AcquireFrame(&mBodyIndexFrame))) {
			if (SUCCEEDED(mBodyIndexFrame->get_FrameDescription(&mBodyIndexFrameDescription))) {
				if (SUCCEEDED(mBodyIndexFrameDescription->get_Height(&height_bodyIndexFrame)) &&
					SUCCEEDED(mBodyIndexFrameDescription->get_Width(&width_bodyIndexFrame))) {

					size_bodyIndexFrameArray_rawBodyIndex = height_bodyIndexFrame * width_bodyIndexFrame;
					if (size_bodyIndexFrameArray_rawBodyIndex > 0) {

						bodyIndexFrameArray_rawBodyIndex = new BYTE[size_bodyIndexFrameArray_rawBodyIndex];

						mBodyIndexFrameDescription->Release();
						mBodyIndexFrame->Release();
						mBodyIndexFrameReference->Release();
						return S_OK;
					}
				}
				mBodyIndexFrameDescription->Release();
			}
			mBodyIndexFrame->Release();
		}
		mBodyIndexFrameReference->Release();
	}
	return E_FAIL;
}


HRESULT updateBodyFrame() {

	if (SUCCEEDED(multiSourceFrame->get_BodyFrameReference(&mBodyFrameReference))) {
		if (SUCCEEDED(mBodyFrameReference->AcquireFrame(&mBodyFrame))) {


			if (SUCCEEDED(mBodyFrame->GetAndRefreshBodyData(BODY_COUNT, bodies))) {
				for (int i = 0; i < BODY_COUNT; i++) {
					if (bodies[i]) {
						if (SUCCEEDED(bodies[i]->get_IsTracked(&isTracked)) && isTracked) {
							//isTracked = false;		
							trackedBody = bodies[i];


							if (SUCCEEDED(trackedBody->GetJoints(JointType_Count, joints) &&
								SUCCEEDED(trackedBody->GetJointOrientations(JointType_Count, jointsOrientations)))) {

								for (int jointIte = 0; jointIte < JointType_Count; jointIte++) {

									jointTmp = joints[jointIte];
									jointOrientationTmp = jointsOrientations[jointIte];

									if (jointTmp.TrackingState > 0) {
										skeletonJoints[0 + jointIte * 9] = jointTmp.Position.X;
										skeletonJoints[1 + jointIte * 9] = jointTmp.Position.Y;
										skeletonJoints[2 + jointIte * 9] = jointTmp.Position.Z;

										//Get space point;

										skeletonJoints[3 + jointIte * 9] = jointOrientationTmp.Orientation.w;
										skeletonJoints[4 + jointIte * 9] = jointOrientationTmp.Orientation.x;
										skeletonJoints[5 + jointIte * 9] = jointOrientationTmp.Orientation.y;
										skeletonJoints[6 + jointIte * 9] = jointOrientationTmp.Orientation.z;

										if (SUCCEEDED(multisourceCoordinateMapper->MapCameraPointToDepthSpace(jointTmp.Position, &depthSpacePointTmp[jointIte]))) {
											skeletonJoints[7 + jointIte * 9] = depthSpacePointTmp[jointIte].X;
											skeletonJoints[8 + jointIte * 9] = depthSpacePointTmp[jointIte].Y;

										}
									}
								}
							}

							mBodyFrame->Release();
							mBodyFrameReference->Release();
							return S_OK;
						}
					}
				}
			}
			mBodyFrame->Release();
		}
		mBodyFrameReference->Release();
	}
	return E_FAIL;
}

HRESULT updateDepthFrame() {

	//IFrameDescription * mDepthFrameDescription = nullptr;

	if (SUCCEEDED(multiSourceFrame->get_DepthFrameReference(&mDepthFrameReference))) {
		if (SUCCEEDED(mDepthFrameReference->AcquireFrame(&mDepthFrame))) {

			//Put depth frame data to buffer
			if (SUCCEEDED(mDepthFrame->CopyFrameDataToArray(size_depthFrameArray_rawDepth, depthFrameArray_rawDepth))) {
				//cout << "accessing depth frame buffer failed" << endl;
				mDepthFrame->Release();
				mDepthFrameReference->Release();
				return S_OK;
			}
			mDepthFrame->Release();
		}
		mDepthFrameReference->Release();
	}
	return E_FAIL;
}



HRESULT updateColorFrame() {

	//mColorFrameReference = nullptr;
	if (SUCCEEDED(multiSourceFrame->get_ColorFrameReference(&mColorFrameReference))) {
		//mColorFrame = nullptr;
		//if (mColorFrame) mColorFrame->Release();
		if (SUCCEEDED(mColorFrameReference->AcquireFrame(&mColorFrame))) {

			//Acquire color framea

			if (SUCCEEDED(mColorFrame->CopyConvertedFrameDataToArray(size_colorFrameArray_BGRA, 
																	 colorFrameArray_BGRA,
																	 ColorImageFormat::ColorImageFormat_Bgra))) {

				return S_OK;
			}
			mColorFrame->Release();
		}
		mColorFrameReference->Release();
	}
	return E_FAIL;
}



HRESULT updateBodyIndexFrame() {

	if (SUCCEEDED(multiSourceFrame->get_BodyIndexFrameReference(&mBodyIndexFrameReference))) {


		if (SUCCEEDED(mBodyIndexFrameReference->AcquireFrame(&mBodyIndexFrame))) {

			if (SUCCEEDED(mBodyIndexFrame->CopyFrameDataToArray(size_bodyIndexFrameArray_rawBodyIndex, bodyIndexFrameArray_rawBodyIndex))) {

				mBodyIndexFrame->Release();
				mBodyIndexFrameReference->Release();
				return S_OK;
			}
			mBodyIndexFrame->Release();
		}
		mBodyIndexFrameReference->Release();
	}
	return E_FAIL;
}


HRESULT update()
{

	if (FAILED(updateBodyFrame())) {
		cout << "update body frame failed" << endl;
		return E_FAIL;
	}
	if (FAILED(updateBodyIndexFrame())) {
		cout << "update body index frame failed" << endl;
		return E_FAIL;
	}
	if (FAILED(updateDepthFrame())) {
		cout << "update depth frame failed" << endl;
		return E_FAIL;
	}
	if (FAILED(updateColorFrame())) {
		cout << "update color frame failed" << endl;
		return E_FAIL;
	}
	return S_OK;
}

void initializing() {
	while (FAILED(initializingKinectDevice())) {}

	while (FAILED(multiSourceFrameReader->AcquireLatestFrame(&multiSourceFrame))) {
		if (multiSourceFrame) multiSourceFrame->Release();
	}

	cout << "Initializing painters ---- First try" << endl;

	while (FAILED(initializingBodyFrame())) {
		cout << "BodyFramePainter initializing failed" << endl;
		cout << "fixing" << endl;

		multiSourceFrame->Release();

		while (FAILED(multiSourceFrameReader->AcquireLatestFrame(&multiSourceFrame))) {
		}
	}

	while (FAILED(initializingDepthFrame())) {
		cout << "DepthFramePainter initializing failed" << endl;
		cout << "fixing" << endl;

		if (multiSourceFrame) multiSourceFrame->Release();
		while (FAILED(multiSourceFrameReader->AcquireLatestFrame(&multiSourceFrame))) {
			if (multiSourceFrame) multiSourceFrame->Release();
		}
	}

	while (FAILED(initializingColorFrame())) {
		cout << "ColorFramePainter initializing failed" << endl;
		cout << "fixing" << endl;

		if (multiSourceFrame) multiSourceFrame->Release();
		while (FAILED(multiSourceFrameReader->AcquireLatestFrame(&multiSourceFrame))) {
			if (multiSourceFrame) multiSourceFrame->Release();
		}
	}

	while (FAILED(initializingBodyIndexFrame())) {
		cout << "initializingBodyIndexFrame initializing failed" << endl;
		cout << "fixing" << endl;

		if (multiSourceFrame) multiSourceFrame->Release();
		while (FAILED(multiSourceFrameReader->AcquireLatestFrame(&multiSourceFrame))) {
			if (multiSourceFrame) multiSourceFrame->Release();
		}
	}

	if (multiSourceFrame) multiSourceFrame->Release();

	depthFrameArray_BGR = new BYTE[height_depthFrame * width_depthFrame * COLOR_FRAME_CHANNEL_BGR];
	bodyIndexFrameArray_BGR = new BYTE[height_bodyIndexFrame * width_bodyIndexFrame * COLOR_FRAME_CHANNEL_BGR];
	bodyFrameArray_BGR = new BYTE[height_bodyIndexFrame * width_bodyIndexFrame * COLOR_FRAME_CHANNEL_BGR];

	size_colorFrame = Size(width_colorFrame, height_colorFrame);
	size_depthFrame = Size(width_depthFrame, height_depthFrame);
	size_bodyIndexFrame = Size(width_bodyIndexFrame, height_bodyIndexFrame);
	size_bodyFrame = Size(width_bodyFrame, height_bodyFrame);
	
}








Mat drawAperson(CvPoint *jointsPoints, CvScalar skeletonColor, int skeletonThickness, Mat tmpSkeletonMat) {


	//HEAD + NECK
	if ((jointsPoints[JointType_Head].x >= 0) && (jointsPoints[JointType_Neck].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_Head], jointsPoints[JointType_Neck], skeletonColor, skeletonThickness);

	//NECK + SPINE SHOULDER;
	if ((jointsPoints[JointType_Neck].x >= 0) && (jointsPoints[JointType_SpineShoulder].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_Neck], jointsPoints[JointType_SpineShoulder], skeletonColor, skeletonThickness);

	//SPINE SHOULDER + RIGHT SHOULDER
	if ((jointsPoints[JointType_SpineShoulder].x >= 0) && (jointsPoints[JointType_ShoulderRight].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_SpineShoulder], jointsPoints[JointType_ShoulderRight], skeletonColor, skeletonThickness);

	//SPINE SHOULDER + LEFT SHOULDER
	if ((jointsPoints[JointType_SpineShoulder].x >= 0) && (jointsPoints[JointType_ShoulderLeft].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_SpineShoulder], jointsPoints[JointType_ShoulderLeft], skeletonColor, skeletonThickness);
	//RIGHT SHOULDER + RIGHT ELBOW

	if ((jointsPoints[JointType_ShoulderRight].x >= 0) && (jointsPoints[JointType_ElbowRight].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_ShoulderRight], jointsPoints[JointType_ElbowRight], skeletonColor, skeletonThickness);

	//LEFT SHOULDER + LEFT ELBOW
	if ((jointsPoints[JointType_ShoulderLeft].x >= 0) && (jointsPoints[JointType_ElbowLeft].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_ShoulderLeft], jointsPoints[JointType_ElbowLeft], skeletonColor, skeletonThickness);

	// RIGHT ELBOW + RIGHT WRIST
	if ((jointsPoints[JointType_ElbowRight].x >= 0) && (jointsPoints[JointType_WristRight].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_ElbowRight], jointsPoints[JointType_WristRight], skeletonColor, skeletonThickness);

	//LEFT ELBOW + LEFT WRIST
	if ((jointsPoints[JointType_ElbowLeft].x >= 0) && (jointsPoints[JointType_WristLeft].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_ElbowLeft], jointsPoints[JointType_WristLeft], skeletonColor, skeletonThickness);

	//RIGHT WRIST + RIGHT HAND
	if ((jointsPoints[JointType_WristRight].x >= 0) && (jointsPoints[JointType_HandRight].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_WristRight], jointsPoints[JointType_HandRight], skeletonColor, skeletonThickness);

	//LEFT WRIST + LEFT HAND
	if ((jointsPoints[JointType_WristLeft].x >= 0) && (jointsPoints[JointType_HandLeft].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_WristLeft], jointsPoints[JointType_HandLeft], skeletonColor, skeletonThickness);

	//RIGHT HAND + RIGHT THUMB
	if ((jointsPoints[JointType_HandRight].x >= 0) && (jointsPoints[JointType_ThumbRight].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_HandRight], jointsPoints[JointType_ThumbRight], skeletonColor, skeletonThickness);

	//LEFT HAND + LEFT THUMB
	if ((jointsPoints[JointType_HandLeft].x >= 0) && (jointsPoints[JointType_ThumbLeft].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_HandLeft], jointsPoints[JointType_ThumbLeft], skeletonColor, skeletonThickness);

	//RIGHT HAND + RIGHT HANDTIP
	if ((jointsPoints[JointType_HandRight].x >= 0) && (jointsPoints[JointType_HandTipRight].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_HandRight], jointsPoints[JointType_HandTipRight], skeletonColor, skeletonThickness);

	//LEFT HAND + LEFT HANDTIP
	if ((jointsPoints[JointType_HandLeft].x >= 0) && (jointsPoints[JointType_HandTipLeft].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_HandLeft], jointsPoints[JointType_HandTipLeft], skeletonColor, skeletonThickness);

	//SPINE SHOULDER + SPINE MID
	if ((jointsPoints[JointType_SpineShoulder].x >= 0) && (jointsPoints[JointType_SpineMid].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_SpineShoulder], jointsPoints[JointType_SpineMid], skeletonColor, skeletonThickness);

	//SPINE MID + SPINE BASE
	if ((jointsPoints[JointType_SpineMid].x >= 0) && (jointsPoints[JointType_SpineBase].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_SpineMid], jointsPoints[JointType_SpineBase], skeletonColor, skeletonThickness);

	//SPINE BASE + RIGHT HIP
	if ((jointsPoints[JointType_SpineBase].x >= 0) && (jointsPoints[JointType_HipRight].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_SpineBase], jointsPoints[JointType_HipRight], skeletonColor, skeletonThickness);

	//SPINE BASE + LEFT HIP
	if ((jointsPoints[JointType_SpineBase].x >= 0) && (jointsPoints[JointType_HipLeft].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_SpineBase], jointsPoints[JointType_HipLeft], skeletonColor, skeletonThickness);

	//RIGHT HIP + RIGHT KNEE
	if ((jointsPoints[JointType_HipRight].x >= 0) && (jointsPoints[JointType_KneeRight].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_HipRight], jointsPoints[JointType_KneeRight], skeletonColor, skeletonThickness);

	//LEFT HIP + LEFT KNEE
	if ((jointsPoints[JointType_HipLeft].x >= 0) && (jointsPoints[JointType_KneeLeft].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_HipLeft], jointsPoints[JointType_KneeLeft], skeletonColor, skeletonThickness);

	//RIGHT KNEE + RIGHT ANKLE
	if ((jointsPoints[JointType_KneeRight].x >= 0) && (jointsPoints[JointType_AnkleRight].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_KneeRight], jointsPoints[JointType_AnkleRight], skeletonColor, skeletonThickness);

	//LEFT KNEE + LEFT ANKLE
	if ((jointsPoints[JointType_KneeLeft].x >= 0) && (jointsPoints[JointType_AnkleLeft].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_KneeLeft], jointsPoints[JointType_AnkleLeft], skeletonColor, skeletonThickness);

	//RIGHT ANKLE + RIGHT FOOT
	if ((jointsPoints[JointType_AnkleRight].x >= 0) && (jointsPoints[JointType_FootRight].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_AnkleRight], jointsPoints[JointType_FootRight], skeletonColor, skeletonThickness);

	//LEFT ANKLE + LEFT FOOT
	if ((jointsPoints[JointType_AnkleLeft].x >= 0) && (jointsPoints[JointType_FootLeft].x >= 0))
		line(tmpSkeletonMat, jointsPoints[JointType_AnkleLeft], jointsPoints[JointType_FootLeft], skeletonColor, skeletonThickness);

	return tmpSkeletonMat;

}



int main() {

	initializing();


	CvPoint jointsPoints[JointType_Count] = { cvPoint(-1,-1) };

	time_t startTimer;
	time_t endTimer;
	double diffSeconds;
	int relativeSeconds = 1;
	time(&startTimer);
	int frameNum = 0;


	string DESTINATIONPATH = "C:\\Users\\HenglinShi\\Desktop\\SampleOutPut";



	string sampleNum;
	cout << "put sample number here (please include all the zeros):" << endl;
	cin >> sampleNum;
	string path = DESTINATIONPATH + "\\Sample" + sampleNum;
	_mkdir(path.c_str());
	string colorPath = path + "\\Sample" + sampleNum + "_color.avi";
	string depthPath = path + "\\Sample" + sampleNum + "_depth.avi";
	string dataPath = path + "\\Sample" + sampleNum + "_data.csv";
	string skeletonPath = path + "\\Sample" + sampleNum + "_skeleton.csv";
	string userPath = path + "\\Sample" + sampleNum + "_user.avi";
	string skeletonFPath = path + "\\Sample" + sampleNum + "_skeleton.avi";

	bool r = false;
	int fourcc = -1;

	VideoWriter colorVideoWriter(colorPath.c_str(), fourcc, 20, size_colorFrame, true);
	VideoWriter depthVideoWriter(depthPath.c_str(), fourcc, 20, size_depthFrame, true);
	VideoWriter bodyIndexVideoWriter(userPath.c_str(), fourcc, 20, size_bodyIndexFrame, true);
	VideoWriter bodyVideoWriter(skeletonFPath.c_str(), fourcc, 20, size_bodyFrame, true);
	ofstream csvFile(skeletonPath.c_str());

	namedWindow("depth image", CV_WINDOW_AUTOSIZE);
	namedWindow("color image", CV_WINDOW_AUTOSIZE);
	namedWindow("mask image", CV_WINDOW_AUTOSIZE);
	namedWindow("skeleton image", CV_WINDOW_AUTOSIZE);

	

	if (!colorVideoWriter.isOpened() || 
		!depthVideoWriter.isOpened() || 
		!bodyIndexVideoWriter.isOpened() || 
		!bodyVideoWriter.isOpened())
	{
		cout << "!!! Output video could not be opened" << std::endl;
		return -1;
	}
	else 
		cout << "video opened successfully" << endl << "use r-key to record" << endl;

	CvScalar skeletonColor = cvScalar(100.0,100.0,0.0);
	while (1)
	{
		if (SUCCEEDED(multiSourceFrameReader->AcquireLatestFrame(&multiSourceFrame))) {


			if (SUCCEEDED(update())) {

				frameNum++;
				time(&endTimer);
				diffSeconds = difftime(endTimer, startTimer);

				if (diffSeconds >= 1) {
					relativeSeconds++;

					cout << relativeSeconds << "  " << frameNum << endl;

					time(&startTimer);
					frameNum = 0;
				}

				//Processing color Frame
				//Processing color Frame
				//Processing color Frame
				colorFrameMat = Mat(height_colorFrame, width_colorFrame, CV_8UC4, colorFrameArray_BGRA);
		

				//Processing body Frame

				for (UINT i = 0; i < size_bodyIndexFrameArray_rawBodyIndex; i++) {
					bodyIndexFrameArray_BGR[i * 3] = bodyIndexFrameArray_rawBodyIndex[i] & 0x01 ? 0x00 : 0xFF;
					bodyIndexFrameArray_BGR[i * 3 + 1] = bodyIndexFrameArray_rawBodyIndex[i] & 0x02 ? 0x00 : 0xFF;
					bodyIndexFrameArray_BGR[i * 3 + 2] = bodyIndexFrameArray_rawBodyIndex[i] & 0x04 ? 0x00 : 0xFF;


				}
				bodyIndexFrameMat = Mat(height_bodyIndexFrame, width_bodyIndexFrame, CV_8UC3, bodyIndexFrameArray_BGR);


				//Processing depth Frame
				//Processing depth Frame
				//Processing depth Frame

				for (int i = 0; i < size_depthFrameArray_rawDepth; i ++) {
					USHORT depth = depthFrameArray_rawDepth[i];

					if (depth < 0) {
						depthFrameArray_BGR[i * 3] = 0xFF;
						depthFrameArray_BGR[i * 3 + 1] = 0;
						depthFrameArray_BGR[i * 3 + 2] = 0;
					}
					else if (depth < depthFrameMinReliableDistance) {
						depthFrameArray_BGR[i * 3] = 0;
						depthFrameArray_BGR[i * 3 + 1] = depth & 0x7F + 0x80;
						depthFrameArray_BGR[i * 3 + 2] = 0;
					}
					else if (depth < depthFrameMaxReliableDistance) {
						memset(depthFrameArray_BGR + 3 * i, (depth & 0xFF), 3);
					}
					else {
						depthFrameArray_BGR[i * 3] = 0;
						depthFrameArray_BGR[i * 3 + 1] = 0;
						depthFrameArray_BGR[i * 3 + 2] = depth & 0x7F + 0x80;
					}
				}
				depthFrameMat = Mat(height_depthFrame, width_depthFrame, CV_8UC3, depthFrameArray_BGR);

				//multisourceCoordinateMapper->MapCameraPointsToDepthSpace(JointType_Count, &joints[1].Position,JointType_Count, &(depthSpacePointTmp[1]));

				for (int i = 0; i < JointType_Count; i++) {
					//multisourceCoordinateMapper->MapCameraPointToDepthSpace(joints[i].Position, &depthSpacePointTmp[i]);
					jointsPoints[i].x = depthSpacePointTmp[i].X;
					jointsPoints[i].y = depthSpacePointTmp[i].Y;
				}
				
				//bodyFrameMat = Mat::zeros(height_depthFrame, width_depthFrame, CV_8UC3);
				
				bodyFrameMat = drawAperson(jointsPoints, skeletonColor,  10, depthFrameMat);






				imshow("depth image", depthFrameMat);
				imshow("mask image", bodyIndexFrameMat);
				imshow("color image", colorFrameMat);
				imshow("skeleton image", bodyFrameMat);

	

			

				int c = cvWaitKey(1);
				if (c == 27 || c == 'q' || c == 'Q')
					break;
				if (c == 'r' || c == 'R') {
					r = true;
					cout << "recoridng" << endl << "use s-key to stop" << endl;
				}
				if (c == 's' || c == 'S') {
					r = false;
					cout << "stoped" << endl;
				}

				if (r == true) {

					colorVideoWriter.write(colorFrameMat);
					depthVideoWriter.write(depthFrameMat);
					bodyIndexVideoWriter.write(bodyIndexFrameMat);
					bodyVideoWriter.write(bodyFrameMat);

					for (int i = 0; i < JointType_Count - 1; i++) {
						csvFile
							<< to_string(skeletonJoints[9 * i + 0]) << ","
							<< to_string(skeletonJoints[9 * i + 1]) << ","
							<< to_string(skeletonJoints[9 * i + 2]) << ","
							<< to_string(skeletonJoints[9 * i + 3]) << ","
							<< to_string(skeletonJoints[9 * i + 4]) << ","
							<< to_string(skeletonJoints[9 * i + 5]) << ","
							<< to_string(skeletonJoints[9 * i + 6]) << ","
							<< to_string(skeletonJoints[9 * i + 7]) << ","
							<< to_string(skeletonJoints[9 * i + 8]) << ",";
					}
					csvFile
						<< to_string(skeletonJoints[9 * (JointType_Count - 1) + 0]) << ","
						<< to_string(skeletonJoints[9 * (JointType_Count - 1) + 1]) << ","
						<< to_string(skeletonJoints[9 * (JointType_Count - 1) + 2]) << ","
						<< to_string(skeletonJoints[9 * (JointType_Count - 1) + 3]) << ","
						<< to_string(skeletonJoints[9 * (JointType_Count - 1) + 4]) << ","
						<< to_string(skeletonJoints[9 * (JointType_Count - 1) + 5]) << ","
						<< to_string(skeletonJoints[9 * (JointType_Count - 1) + 6]) << ","
						<< to_string(skeletonJoints[9 * (JointType_Count - 1) + 7]) << ","
						<< to_string(skeletonJoints[9 * (JointType_Count - 1) + 8]) << endl;
				}
				mColorFrame->Release();
				mColorFrameReference->Release();
			}
			
		}

		if (multiSourceFrame) {
			multiSourceFrame->Release();
		}
	}
	colorVideoWriter.release();
	depthVideoWriter.release();
	bodyIndexVideoWriter.release();
	bodyVideoWriter.release();


	cvDestroyWindow("depth image");
	cvDestroyWindow("color image");
	cvDestroyWindow("mask image");
	cvDestroyWindow("skeleton image");
		
	//NuiShutdown();
	string colorPathNew = path + "\\Sample" + sampleNum + "_color.mp4";
	string depthPathNew = path + "\\Sample" + sampleNum + "_depth.mp4";
	string userPathNew = path + "\\Sample" + sampleNum + "_user.mp4";
	string skeletonFPathNew = path + "\\Sample" + sampleNum + "_skeleton.mp4";
	rename(colorPath.c_str(), colorPathNew.c_str());
	rename(depthPath.c_str(), depthPathNew.c_str());
	rename(userPath.c_str(), userPathNew.c_str());
	rename(skeletonFPath.c_str(), skeletonFPathNew.c_str());

}