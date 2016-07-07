#include "stdafx.h"
#include "KinectSensor.h"
#include "MultiSourcesFrameReader.h"
#include <time.h> 

string DOUBLE_SLASH = "\\";
string DESTINATION_DIR_PATH = "C:\\Users\\chaoyu\\Desktop\\SampleOutPut";
string COLOR_FRAME_DIR_NAME = "color";
string DEPTH_FRAME_DIR_NAME = "depth";
string BODY_INDEX_FRAME_DIR_NAME = "bodyIndex";
string BODY_FRAME_DIR_NAME = "body";

ofstream colorFrameWriter;
ofstream depthFrameWriter;
ofstream bodyIndexFrameWriter;
ofstream bodyFrameWriter;

int userOperation =  -1;
bool recording = false;

//TIMER
time_t startTimer;
time_t endTimer;
double diffSeconds;
int relativeSeconds = 1;



int main() { 
	//INITIALIZING KINECT
	KinectSensor * mKinectSensor = new KinectSensor();
	if (!mKinectSensor->getHealth()) {
		cout<<"Kinect Sensor Initializing Failed!"<<endl;
		return -1;
	}
	MultiSourcesFrameReader * mFrameReader = new MultiSourcesFrameReader(mKinectSensor);

	if (!mFrameReader->getHealth()) {
		cout << "MultiSourcesFrameReader Initializing Failed!" << endl;
		return -1;
	}

	cout << "put sample number here (please include all the zeros):" << endl;

	string sampleNO;
	cin >> sampleNO;

	
	string SAMPLE_DIR_PATH = DESTINATION_DIR_PATH + DOUBLE_SLASH + "sample" + sampleNO;

	string COLOR_FRAME_DIR_PATH = SAMPLE_DIR_PATH + DOUBLE_SLASH + COLOR_FRAME_DIR_NAME;
	string DEPTH_FRAME_DIR_PATH = SAMPLE_DIR_PATH + DOUBLE_SLASH + DEPTH_FRAME_DIR_NAME;
	string BODY_INDEX_FRAME_DIR_PATH = SAMPLE_DIR_PATH + DOUBLE_SLASH + BODY_INDEX_FRAME_DIR_NAME;
	string BODY_FRAME_DIR_PATH = SAMPLE_DIR_PATH + DOUBLE_SLASH + BODY_FRAME_DIR_NAME;

	_mkdir(SAMPLE_DIR_PATH.c_str());
	_mkdir(DEPTH_FRAME_DIR_PATH.c_str());
	_mkdir(BODY_INDEX_FRAME_DIR_PATH.c_str());
	_mkdir(BODY_INDEX_FRAME_DIR_PATH.c_str());
	_mkdir(BODY_FRAME_DIR_PATH.c_str());

	string COLOR_FRAME_FILE_PATH;
	string DEPTH_FRAME_FILE_PATH;
	string BODY_INDEX_FRAME_FILE_PATH;
	string BODY_FRAME_FILE_PATH;

	namedWindow("COLOR FRAME", CV_WINDOW_AUTOSIZE);
	namedWindow("DEPTH FRAME", CV_WINDOW_AUTOSIZE);
	namedWindow("BODY INDEX FRAME", CV_WINDOW_AUTOSIZE);
	namedWindow("BODY FRAME", CV_WINDOW_AUTOSIZE);


	//TIMER
	time(&startTimer);
	int frameNum = 0;

	//UPDATE FRAME
	while (SUCCEEDED(mFrameReader->updateFrame())) {

		//UPDATE TIMER
		frameNum++;
		time(&endTimer);
		diffSeconds = difftime(endTimer, startTimer);

		if (diffSeconds >= 1) {
			relativeSeconds++;
			cout << relativeSeconds << "  " << frameNum << endl;
			time(&startTimer);
			frameNum = 0;
		}

		//OUT PUT FRAMES 

		imshow("color image", mFrameReader->getColorFrame()->getFrameMat_BGRA());
		imshow("depth image", mFrameReader->getDepthFrame()->getFrameMat_BGR());
		imshow("bodyIndex image", mFrameReader->getBodyIndexFrame()->getFrameMat_BGR());
		imshow("body image", mFrameReader->getBodyFrame()->getFrameMat_BGR());


		userOperation = waitKey(1);

		if (userOperation == 27 || userOperation == 'q' || userOperation == 'Q')
			break;
		if (userOperation == 'r' || userOperation == 'R') {
			recording = true;
			cout << "recoridng" << endl << "use s-key to stop" << endl;
		}
		if (userOperation == 's' || userOperation == 'S') {
			recording = false;
			cout << "stoped" << endl;
		}
		if (recording == true) {

			COLOR_FRAME_FILE_PATH = COLOR_FRAME_DIR_PATH + "\\" + to_string(relativeSeconds) + "_" + to_string(frameNum) + ".dat";
			DEPTH_FRAME_FILE_PATH = DEPTH_FRAME_DIR_PATH + "\\" + to_string(relativeSeconds) + "_" + to_string(frameNum) + ".dat";
			BODY_FRAME_FILE_PATH = BODY_FRAME_DIR_PATH + "\\" + to_string(relativeSeconds) + "_" + to_string(frameNum) + ".dat";
			BODY_INDEX_FRAME_FILE_PATH = BODY_INDEX_FRAME_DIR_PATH + "\\" + to_string(relativeSeconds) + "_" + to_string(frameNum) + ".dat";

			//Writing data to file 

			colorFrameWriter.open(COLOR_FRAME_FILE_PATH.c_str(), ios_base::out | ios_base::binary);

			colorFrameWriter.write((char *)(mFrameReader->getColorFrame()->getFrameArray_YUY2()), sizeof(BYTE) * (mFrameReader->getColorFrame()->getLenght_FrameArray_YUY2()));
			colorFrameWriter.flush();
			colorFrameWriter.close();

			depthFrameWriter.open(DEPTH_FRAME_FILE_PATH.c_str(), ios_base::out | ios_base::binary);
			depthFrameWriter.write((char *)mFrameReader->getDepthFrame()->getFrameArray_Depth(), (sizeof(UINT16) * mFrameReader->getDepthFrame()->getLength_FrameArray_Depth()));
			depthFrameWriter.flush();
			depthFrameWriter.close();

			bodyIndexFrameWriter.open(BODY_INDEX_FRAME_FILE_PATH.c_str(), ios_base::out | ios_base::binary);
			bodyIndexFrameWriter.write((char *)mFrameReader->getBodyIndexFrame()->getFrameArray_Index(), (sizeof(BYTE) * mFrameReader->getBodyIndexFrame()->getLength_FrameArray_Index()));
			bodyIndexFrameWriter.flush();
			bodyIndexFrameWriter.close();

			bodyFrameWriter.open(BODY_FRAME_FILE_PATH.c_str(), ios_base::out | ios_base::binary);
			bodyFrameWriter.write((char *)&mFrameReader->getBodyFrame()->getJoints_CameraSpacePoint(), sizeof(mFrameReader->getBodyFrame()->getJoints_CameraSpacePoint()));
			bodyFrameWriter.flush();
			bodyFrameWriter.close();
		}
	}
}