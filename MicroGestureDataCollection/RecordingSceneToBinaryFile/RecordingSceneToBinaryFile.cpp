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

//Frame heights and widths
int bodyFrameHeight = 0;
int bodyFrameWidth = 0;
int colorFrameHeight = 0;
int colorFrameWidth = 0;
int depthFrameHeight = 0;
int depthFrameWidth = 0;
int bodyIndexFrameHeight = 0;
int bodyIndexFrameWidth = 0;

//Distance threashold for depth frame
USHORT depthFrameMinReliableDistance = 0;
USHORT depthFrameMaxReliableDistance = 0;

//Frame size, for OPENCV OPENCV
Size colorFrameSize(colorFrameWidth, colorFrameHeight);
Size depthFrameSize(depthFrameWidth, depthFrameHeight);
Size bodyIndexFrame(bodyIndexFrameWidth, bodyIndexFrameHeight);
Size bodyFrame(bodyFrameWidth, bodyFrameHeight);



UINT16 * depthFrameArray = nullptr;
UINT  depthFrameArraySize = 0;



BYTE colorFrameArrayArray[1920 * 1080 * 4] = { 0 };
BYTE * colorFrameArray;
UINT colorFrameArraySize = 0;



BYTE * bodyIndexFrameArray = nullptr;
UINT bodyIndexFrameArraySize = 0;


UINT colorFramePointNum = 0;


DepthSpacePoint * colorPointCoordinateInDepthSpace = nullptr;
DepthSpacePoint * colorPointCoordinateInDepthSpaceIterator;
Mat colorFrameMat;
Mat colorFrameMatInDepth;
BYTE * colorFrameArrayInDepthSpace = nullptr;

Mat depthFrameMat;

UINT16 * depthFrameArrayEnd = nullptr;

RGBQUAD * depthFrameArrayRGBX = nullptr;
RGBQUAD * depthFrameArrayRGBXIterator = nullptr;
UINT depthFrameArrayRGBXSize = 0;


Mat bodyIndexFrameMat;

BYTE * bodyIndexArrayEnd = nullptr;

RGBQUAD * bodyIndexFrameArrayRGBX = nullptr;
RGBQUAD * bodyIndexFrameArrayRGBXIterator = nullptr;
UINT bodyIndexFrameArrayRGBXSize = 0;


Mat bodyFrameMat;
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

typedef struct BodySkeleton
{
	Mat bodyInfoMat;
	float skeletonInfo[9 * JointType_Count];
	CameraSpacePoint cameraSpacePoint[JointType_Count];
};


#define CHANNEL 4
/*Initializing Kinect Device*/


//BUFFERS FOR FRAME BUFFERS










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
				if (SUCCEEDED(mColorFrameDescription->get_Height(&colorFrameHeight)) &&
					SUCCEEDED(mColorFrameDescription->get_Width(&colorFrameWidth))) {

					ColorImageFormat imageFormat = ColorImageFormat::ColorImageFormat_None;
					mColorFrame->get_RawColorImageFormat(&imageFormat);
					mColorFrame->AccessRawUnderlyingBuffer(&colorFrameArraySize, &colorFrameArray);

					cout << imageFormat << endl;

					colorFramePointNum = colorFrameHeight * colorFrameWidth;
					//colorFrameArraySize = colorFramePointNum * CHANNEL;

					if (colorFrameArraySize > 0) {

						//colorFrameBuffer.resize(colorFrameArraySize);
						colorFrameArray = new BYTE[colorFrameArraySize];
						//vector<DepthSpacePoint> =
						colorPointCoordinateInDepthSpace = new DepthSpacePoint[colorFramePointNum];
						memset(colorPointCoordinateInDepthSpace, 0, colorFramePointNum);

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

				if (SUCCEEDED(mDepthFrameDescription->get_Height(&depthFrameHeight)) &&
					SUCCEEDED(mDepthFrameDescription->get_Width(&depthFrameWidth))) {

					depthFrameArraySize = depthFrameHeight * depthFrameWidth;
					if (depthFrameArraySize) {

						if (SUCCEEDED(mDepthFrame->get_DepthMaxReliableDistance(&depthFrameMaxReliableDistance)) &&
							SUCCEEDED(mDepthFrame->get_DepthMinReliableDistance(&depthFrameMinReliableDistance))) {

							//depthFrameArrayRGBXSize = depthFrameWidth * depthFrameHeight;
							//depthFrameArrayRGBX = new RGBQUAD[depthFrameArrayRGBXSize];
							//colorFrameArrayInDepthSpace = new BYTE[depthFrameHeight * depthFrameWidth * CHANNEL];

							depthFrameArray = new UINT16[depthFrameArraySize];

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
				if (SUCCEEDED(mBodyIndexFrameDescription->get_Height(&bodyIndexFrameHeight)) &&
					SUCCEEDED(mBodyIndexFrameDescription->get_Width(&bodyIndexFrameWidth))) {

					bodyIndexFrameArraySize = bodyIndexFrameHeight * bodyIndexFrameWidth;
					if (bodyIndexFrameArraySize > 0) {

						//bodyIndexFrameArrayRGBXSize = bodyIndexFrameHeight * bodyIndexFrameWidth;
						//bodyIndexFrameArrayRGBX = new RGBQUAD[bodyIndexFrameArrayRGBXSize];

						bodyIndexFrameArray = new BYTE[bodyIndexFrameArraySize];

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
			if (SUCCEEDED(mDepthFrame->CopyFrameDataToArray(depthFrameArraySize, depthFrameArray))) {
				//cout << "accessing depth frame buffer failed" << endl;

				if (SUCCEEDED(multisourceCoordinateMapper->MapColorFrameToDepthSpace(
					depthFrameArraySize,
					depthFrameArray,
					colorFramePointNum,
					colorPointCoordinateInDepthSpace))) {

					mDepthFrame->Release();
					mDepthFrameReference->Release();
					return S_OK;
				}

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

			if (SUCCEEDED(mColorFrame->CopyRawFrameDataToArray(colorFrameArraySize, colorFrameArray))) {

				//colorFrameBufferQueue.push_back(colorFrameBuffer);
				//colorFrameBuffer.clear();
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

			if (SUCCEEDED(mBodyIndexFrame->CopyFrameDataToArray(bodyIndexFrameArraySize, bodyIndexFrameArray))) {

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
}















int main() {


	initializing();

	time_t startTimer;
	time_t endTimer;
	double diffSeconds;
	int relativeSeconds = 1;
	time(&startTimer);
	int frameNum = 0;


	string DESTINATIONPATH = "C:\\Users\\chaoyu\\Desktop\\SampleOutPut";

	string colorMappintToDepthDirPath = "\\colorMappingToDepth";
	string colorDataDirPath = "\\color";
	string depthDataDirPath = "\\depth";
	string bodyIndexDataDirPath = "\\bodyIndex";
	string bodyDataDirPath = "\\body";

	string sampleNum;

	if (TESTMODE) {

		sampleNum = "test";

	}

	else {

		cout << "put sample number here (please include all the zeros):" << endl;
		cin >> sampleNum;
	}


	string path = DESTINATIONPATH + "\\Sample" + sampleNum;

	colorMappintToDepthDirPath = path + colorMappintToDepthDirPath;
	colorDataDirPath = path + colorDataDirPath;
	depthDataDirPath = path + depthDataDirPath;
	bodyIndexDataDirPath = path + bodyIndexDataDirPath;
	bodyDataDirPath = path + bodyDataDirPath;

	_mkdir(path.c_str());
	_mkdir(colorMappintToDepthDirPath.c_str());
	_mkdir(colorDataDirPath.c_str());
	_mkdir(depthDataDirPath.c_str());
	_mkdir(bodyIndexDataDirPath.c_str());
	_mkdir(bodyDataDirPath.c_str());

	string colorMappingToDepthDataPath;
	string colorDataPath;
	string depthDataPath;
	string bodyIndexDataPath;
	string bodyDataPath;

	// no attr. template


	ofstream colorMappingToDepthFStreamOut;
	ofstream colorFStreamOut;
	ofstream depthFStreamOut;
	ofstream bodyFStreamOut;
	ofstream bodyIndexFStreamOut;


	colorFrameArrayInDepthSpace = new BYTE[depthFrameArraySize * 4];


	while (1)
	{
		if (SUCCEEDED(multiSourceFrameReader->AcquireLatestFrame(&multiSourceFrame))) {


			if (SUCCEEDED(update())) {


				colorPointCoordinateInDepthSpaceIterator = colorPointCoordinateInDepthSpace;
				//#pragma omp parallel for
				//for (int i = 0; i < colorFramePointNum; i++) {

				//	int depthY = static_cast<int>((*colorPointCoordinateInDepthSpaceIterator).Y + 0.5f);
				//	int depthX = static_cast<int>((*colorPointCoordinateInDepthSpaceIterator).X + 0.5f);

				//	if (depthY >= 0 && depthY < depthFrameHeight &&
				//		depthX >= 0 && depthX < depthFrameWidth) {
				//		colorFrameArrayInDepthSpace[(depthY * depthFrameWidth + depthX) * 4] = colorFrameArray[i * 4];
				//		colorFrameArrayInDepthSpace[(depthY * depthFrameWidth + depthX) * 4 + 1] = colorFrameArray[i * 4 + 1];
				//		colorFrameArrayInDepthSpace[(depthY * depthFrameWidth + depthX) * 4 + 2] = colorFrameArray[i * 4 + 2];
				//		colorFrameArrayInDepthSpace[(depthY * depthFrameWidth + depthX) * 4 + 3] = colorFrameArray[i * 4 + 3];
				//	//	memcpy(&(colorFrameArrayInDepthSpace[(depthY * depthFrameWidth + depthX) * 4]), &(colorFrameArray[i * 4]), 4 * sizeof(BYTE));

				//	}

				//	colorPointCoordinateInDepthSpaceIterator++;
				//}

				//colorFrameMatInDepth = Mat(depthFrameHeight, depthFrameWidth, CV_8UC4, colorFrameArrayInDepthSpace);
				//namedWindow("COLOR_IN_DEPTH", CV_WINDOW_AUTOSIZE);
				//imshow("COLOR_IN_DEPTH", colorFrameMatInDepth);
				//waitKey(1);




				frameNum++;
				time(&endTimer);
				diffSeconds = difftime(endTimer, startTimer);

				if (diffSeconds >= 1) {
					relativeSeconds++;

					cout << relativeSeconds << "  " << frameNum << endl;

					time(&startTimer);
					frameNum = 0;
				}

				colorMappingToDepthDataPath = colorMappintToDepthDirPath + "\\" + to_string(relativeSeconds) + "_" + to_string(frameNum) + ".dat";
				colorDataPath = colorDataDirPath + "\\" + to_string(relativeSeconds) + "_" + to_string(frameNum) + ".dat";
				depthDataPath = depthDataDirPath + "\\" + to_string(relativeSeconds) + "_" + to_string(frameNum) + ".dat";
				bodyDataPath = bodyDataDirPath + "\\" + to_string(relativeSeconds) + "_" + to_string(frameNum) + ".dat";
				bodyIndexDataPath = bodyIndexDataDirPath + "\\" + to_string(relativeSeconds) + "_" + to_string(frameNum) + ".dat";

				//Writing data to file 

				colorFStreamOut.open(colorDataPath.c_str(), ios_base::out | ios_base::binary);
				colorFStreamOut.write((char *)colorFrameArray, (sizeof(BYTE) * colorFrameArraySize));
				colorFStreamOut.flush();
				colorFStreamOut.close();

				mColorFrame->Release();
				mColorFrameReference->Release();

				colorFStreamOut.open(depthDataPath.c_str(), ios_base::out | ios_base::binary);
				colorFStreamOut.write((char *)depthFrameArray, (sizeof(UINT16) * depthFrameArraySize));
				colorFStreamOut.flush();
				colorFStreamOut.close();

				colorFStreamOut.open(bodyIndexDataPath.c_str(), ios_base::out | ios_base::binary);
				colorFStreamOut.write((char *)bodyIndexFrameArray, (sizeof(BYTE) * bodyIndexFrameArraySize));
				colorFStreamOut.flush();
				colorFStreamOut.close();


				//int skeletonSize = sizeof(skeletonJoints);

				colorFStreamOut.open(bodyDataPath.c_str(), ios_base::out | ios_base::binary);
				colorFStreamOut.write((char *)&skeletonJoints, sizeof(skeletonJoints));
				colorFStreamOut.flush();
				colorFStreamOut.close();


				//colorFStreamOut.open(colorMappingToDepthDataPath.c_str(), ios_base::out | ios_base::binary);		
				//colorFStreamOut.write((char *)colorPointCoordinateInDepthSpace, (sizeof(DepthSpacePoint) * colorFramePointNum));
				//colorFStreamOut.flush();
				//colorFStreamOut.close();




				if (TESTMODE) {

					BYTE *colorFrameArrayRead = new BYTE[colorFrameArraySize];
					memset(colorFrameArrayRead, 0, colorFrameArraySize);

					UINT16 *depthFrameArrayRead = new UINT16[depthFrameArraySize];
					memset(depthFrameArrayRead, 0, depthFrameArraySize);

					BYTE *bodyIndexFrameArrayRead = new BYTE[bodyIndexFrameArraySize];
					memset(bodyIndexFrameArrayRead, 0, bodyIndexFrameArraySize);

					float bodyFrameRead[225];
					memset(bodyFrameRead, 0, 225);

					DepthSpacePoint * colorPointCoordinateIndepthSpaceRead = new DepthSpacePoint[colorFramePointNum];
					memset(colorPointCoordinateIndepthSpaceRead, 0, colorFramePointNum);




					ifstream colorStreamIn;
					ifstream depthStreamIn;
					ifstream bodyIndexStreamIn;
					ifstream bodyStreanIn;
					ifstream colorMappingStreamIn;

					colorStreamIn.open(colorDataPath.c_str(), ios_base::in | ios_base::binary);
					colorStreamIn.read(reinterpret_cast<char*>(colorFrameArrayRead), sizeof(BYTE) * colorFrameArraySize);

					depthStreamIn.open(depthDataPath.c_str(), ios_base::in | ios_base::binary);
					depthStreamIn.read(reinterpret_cast<char*>(depthFrameArrayRead), sizeof(UINT16) * depthFrameArraySize);

					bodyIndexStreamIn.open(bodyIndexDataPath.c_str(), ios_base::in | ios_base::binary);
					bodyIndexStreamIn.read(reinterpret_cast<char*>(bodyIndexFrameArrayRead), sizeof(BYTE) * bodyIndexFrameArraySize);

					bodyStreanIn.open(bodyDataPath.c_str(), ios_base::in | ios_base::binary);
					//bodyStreanIn.read(reinterpret_cast<char*>(bodyFrameRead), skeletonSize);

					//colorMappingStreamIn.open(colorMappingToDepthDataPath.c_str(), ios_base::in | ios_base::binary);
					//colorMappingStreamIn.read(reinterpret_cast<char*>(colorPointCoordinateIndepthSpaceRead), sizeof(DepthSpacePoint) * colorFramePointNum);
					UINT ERROTTAG = 0;

					for (int i = 0; i < colorFrameArraySize; i++) {
						if (colorFrameArray[i] - colorFrameArrayRead[i] != 0)
							ERROTTAG++;
					}

					for (int i = 0; i < depthFrameArraySize; i++) {
						if (depthFrameArray[i] - depthFrameArrayRead[i] != 0)
							ERROTTAG++;
					}

					for (int i = 0; i < bodyIndexFrameArraySize; i++) {
						if (bodyIndexFrameArray[i] - bodyIndexFrameArrayRead[i] != 0)
							ERROTTAG++;
					}

					for (int i = 0; i < 225; i++) {
						if (skeletonJoints[i] - bodyFrameRead[i] != 0)
							ERROTTAG++;
					}

					//for (int i = 0; i < colorFramePointNum; i ++) {
					//	bool o = false;
					//	if ((colorPointCoordinateIndepthSpaceRead[i].X >= 0 && colorPointCoordinateIndepthSpaceRead[i].X < depthFrameWidth) &&
					//		(colorPointCoordinateIndepthSpaceRead[i].Y >= 0 && colorPointCoordinateIndepthSpaceRead[i].Y < depthFrameHeight))
					//		o = true;

					//	bool r = false;
					//	if ((colorPointCoordinateInDepthSpace[i].X >= 0 && colorPointCoordinateInDepthSpace[i].X < depthFrameWidth) &&
					//		(colorPointCoordinateInDepthSpace[i].Y >= 0 && colorPointCoordinateInDepthSpace[i].Y < depthFrameHeight))
					//		r = true;


					//	if (o && r) {
					//		if ((colorPointCoordinateIndepthSpaceRead[i].X - colorPointCoordinateInDepthSpace[i].X != 0) ||
					//			(colorPointCoordinateIndepthSpaceRead[i].Y - colorPointCoordinateInDepthSpace[i].Y != 0))
					//			ERROTTAG ++;
					//	}
					//	else {

					//		if (o || r)
					//			ERROTTAG ++;

					//	}
					//		
					//}


					//colorFrameArrayInDepthSpace = new  BYTE[depthFrameArraySize * 4];
					//for (int i = 0; i < colorFramePointNum; i++) {
					//	colorPointCoordinateInDepthSpaceIterator = colorPointCoordinateIndepthSpaceRead[i];

					//	int depthX = static_cast<int>(colorPointCoordinateInDepthSpaceIterator.X + 0.5f);
					//	int depthY = static_cast<int>(colorPointCoordinateInDepthSpaceIterator.Y + 0.5f);


					//	if (colorPointCoordinateInDepthSpaceIterator.X >= 0 && colorPointCoordinateInDepthSpaceIterator.X < depthFrameWidth) {
					//		if (colorPointCoordinateInDepthSpaceIterator.Y >= 0 && colorPointCoordinateInDepthSpaceIterator.Y < depthFrameHeight) {
					//			colorFrameArrayInDepthSpace[(depthY * depthFrameWidth + depthX) * 4] = colorFrameArray[i * 4];
					//			colorFrameArrayInDepthSpace[(depthY * depthFrameWidth + depthX) * 4 + 1] = colorFrameArray[i * 4 + 1];
					//			colorFrameArrayInDepthSpace[(depthY * depthFrameWidth + depthX) * 4 + 2] = colorFrameArray[i * 4 + 2];
					//			colorFrameArrayInDepthSpace[(depthY * depthFrameWidth + depthX) * 4 + 3] = colorFrameArray[i * 4 + 3];

					//		}
					//	}
					//}
					//colorFrameMat = Mat(depthFrameHeight, depthFrameWidth, CV_8UC4, colorFrameArrayInDepthSpace);


					//namedWindow("POST_COLOR_IN_DEPTH", CV_WINDOW_AUTOSIZE);
					//imshow("POST_COLOR_IN_DEPTH", colorFrameMat);
					//waitKey();


				}
			}

		}
		if (multiSourceFrame)
			multiSourceFrame->Release();



	}

}



