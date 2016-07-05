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


#include <Strsafe.h>

using namespace cv;

using namespace std;
int yuy2ArraySize = 1920 * 1080 * 2;
int rgbArraySize = 1920 * 1080 * 4;
int colorPixNum = 1920 * 1080;

typedef struct fileName {
	int time;
	int idx;
	int flag;
};

vector<string> TraverseDirectory(wchar_t Dir[MAX_PATH], string fileList)
{
	vector<string> fileNames;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	wchar_t DirSpec[MAX_PATH];                  //定义要遍历的文件夹的目录  
	DWORD dwError;
	StringCchCopy(DirSpec, MAX_PATH, Dir);
	StringCchCat(DirSpec, MAX_PATH, TEXT("\\*"));   //定义要遍历的文件夹的完整路径\*  



	hFind = FindFirstFile(DirSpec, &FindFileData);          //找到文件夹中的第一个文件  

	if (hFind == INVALID_HANDLE_VALUE)                               //如果hFind句柄创建失败，输出错误信息  
	{
		FindClose(hFind);
	}
	else
	{
		char ch[260];
		char DefChar = ' ';
		string sss;
		ofstream csvFile(fileList.c_str());
		while (FindNextFile(hFind, &FindFileData) != 0)                            //当文件或者文件夹存在时  
		{

			if ((FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == 0)    //如果不是文件夹  
			{

				if (WideCharToMultiByte(CP_ACP, 0, FindFileData.cFileName, -1, ch, 260, &DefChar, NULL) == 0) {
					cout << "error on handling wchat to string" << endl;
					system("pause");
				}
				sss = string(ch);
				fileNames.push_back(sss);
				memset(&ch, 0, sizeof(ch));
				csvFile << sss << endl;

				wcout << Dir << "\\" << FindFileData.cFileName << endl;            //输出完整路径  
			}
		}
		FindClose(hFind);

		csvFile.flush();
		csvFile.close();
	}
	return fileNames;

}


BYTE ClipToByte(int n) {
	n &= -(n >= 0);
	return n | ((255 - n) >> 31);

}


void convertingYUYV2RGB(BYTE * yuy2, BYTE * rgb) {
	int yuy2Ite = 0;
	int rgbIte = 0;

	int Y1 = 0;
	int U = 0;
	int Y2 = 0;
	int V = 0;

	for (int i = 0; i < colorPixNum / 2; i++) {

		int _Y0 = yuy2[(i << 2) + 0] - 16;
		int _U = yuy2[(i << 2) + 1] - 128;
		int _Y1 = yuy2[(i << 2) + 2] - 16;
		int _V = yuy2[(i << 2) + 3] - 128;

		byte _R = ClipToByte((298 * _Y0 + 409 * _V + 128) >> 8);
		byte _G = ClipToByte((298 * _Y0 - 100 * _U - 208 * _V + 128) >> 8);
		byte _B = ClipToByte((298 * _Y0 + 516 * _U + 128) >> 8);

		rgb[(i * 6) + 0] = _B;
		rgb[(i * 6) + 1] = _G;
		rgb[(i * 6) + 2] = _R;
		//_OutputImage[(_Index << 3) + 3] = 0xFF; // A

		_R = ClipToByte((298 * _Y1 + 409 * _V + 128) >> 8);
		_G = ClipToByte((298 * _Y1 - 100 * _U - 208 * _V + 128) >> 8);
		_B = ClipToByte((298 * _Y1 + 516 * _U + 128) >> 8);

		rgb[(i * 6) + 3] = _B;
		rgb[(i * 6) + 4] = _G;
		rgb[(i * 6) + 5] = _R;
	}

}












int main()
{
	string dirPath = "C:\\Users\\hshi\\Desktop\\SampleOutPut\\Sampleknhvkjhv\\";

	int slashPos = 0;
	int dotPos = 0;
	string tmpFileName = "";
	fileName mFileName;
	mFileName.time = -1;
	mFileName.idx = -1;

	string tmpTime = "";
	string tmpIdx = "";

	vector<string> fileNames = TraverseDirectory(L"C:\\Users\\hshi\\Desktop\\SampleOutPut\\Sampleknhvkjhv\\color", "C:\\Users\\hshi\\Desktop\\SampleOutPut\\Sampleknhvkjhv\\fileList.csv");
	vector<fileName> fileNameStructs;


	for (int i = 0; i < fileNames.size(); i++) {

		tmpFileName = fileNames.at(i);

		slashPos = tmpFileName.find("_");
		dotPos = tmpFileName.find(".");
		tmpTime = tmpFileName.substr(0, (slashPos));
		tmpIdx = tmpFileName.substr(slashPos + 1, (dotPos - slashPos - 1));

		mFileName.time = atoi(tmpTime.c_str());
		mFileName.idx = atoi(tmpIdx.c_str());
		mFileName.flag = mFileName.time * 31 + mFileName.idx;
		cout << mFileName.idx << endl << mFileName.time << endl;
		fileNameStructs.push_back(mFileName);
		mFileName.time = -1;
		mFileName.idx = -1;
	}

	//Sorting
	for (int i = 0; i < fileNameStructs.size() - 1; i++) {
		for (int j = 0; j < fileNameStructs.size() - i - 1; j++) {
			if (fileNameStructs.at(j).flag > fileNameStructs.at(j + 1).flag) {
				mFileName = fileNameStructs.at(j + 1);
				fileNameStructs.at(j + 1) = fileNameStructs.at(j);
				fileNameStructs.at(j) = mFileName;

				tmpFileName = fileNames.at(j + 1);
				fileNames.at(j + 1) = fileNames.at(j);
				fileNames.at(j) = tmpFileName;

			}
		}
	}

	//processing pixels;
	ifstream colorDataIn;
	ifstream depthDataIn;
	ifstream bodyIndexDataIn;
	ifstream bodyDataIn;

	BYTE * yuy2DataArray = nullptr;
	BYTE *rgbDataArray = nullptr;

	UINT16 * depthFrameArray_rawDepth = nullptr;
	BYTE * depthFrameArray_BGR = nullptr;

	BYTE * bodyIndexFrameArray_rawBodyIndex = nullptr;
	BYTE * bodyIndexFrameArray_BGR = nullptr;

	UINT size_depthFrameArray_rawDepth = 512 * 424;
	UINT size_depthFrameArray_BGR = 512 * 424 * 3;


	rgbDataArray = new BYTE[rgbArraySize];
	yuy2DataArray = new BYTE[yuy2ArraySize];

	depthFrameArray_rawDepth = new UINT16[size_depthFrameArray_rawDepth];
	depthFrameArray_BGR = new BYTE[size_depthFrameArray_BGR];

	bodyIndexFrameArray_rawBodyIndex = new BYTE[size_depthFrameArray_rawDepth];
	bodyIndexFrameArray_BGR = new BYTE[size_depthFrameArray_BGR];



	memset(yuy2DataArray, 0, yuy2ArraySize);
	memset(rgbDataArray, 0, rgbArraySize);

	memset(depthFrameArray_rawDepth, 0, size_depthFrameArray_rawDepth);
	memset(depthFrameArray_BGR, 0, size_depthFrameArray_BGR);

	memset(bodyIndexFrameArray_rawBodyIndex, 0, size_depthFrameArray_rawDepth);
	memset(bodyIndexFrameArray_BGR, 0, size_depthFrameArray_BGR);


	int fourcc = -1;
	Size colorFrameSize(1920, 1080);
	Size depthFrameSize(512, 424);
	

	namedWindow("color image", CV_WINDOW_AUTOSIZE);
	namedWindow("depth image", CV_WINDOW_AUTOSIZE);
	namedWindow("bodyIndex image", CV_WINDOW_AUTOSIZE);
	namedWindow("body image", CV_WINDOW_AUTOSIZE);

	Mat colorFrameMat;
	Mat depthFrameMat;
	Mat bodyIndexFrameMat;
	Mat bodyFrameMat;

	string colorVideoPath = dirPath + "colorVideo.avi";
	string depthVideoPath = dirPath + "depthVideo.avi";
	string bodyIndexVideoPath = dirPath + "bodyIndexVideo.avi";
	string bodyVideoPath = dirPath + "bodyVideo.avi";

	VideoWriter colorVideoWriter(colorVideoPath.c_str(), fourcc, 28, colorFrameSize, true);
	VideoWriter depthVideoWriter(depthVideoPath.c_str(), fourcc, 28, depthFrameSize, true);
	VideoWriter bodyIndexVideoWriter(bodyIndexVideoPath.c_str(), fourcc, 28, depthFrameSize, true);
	VideoWriter bodyVideoWriter(bodyVideoPath.c_str(), fourcc, 28, depthFrameSize, true);

	if (!colorVideoWriter.isOpened() || !depthVideoWriter.isOpened() || !bodyIndexVideoWriter.isOpened() || !bodyVideoWriter.isOpened()) {
		cout << "!!! Output video could not be opened" << std::endl;
		return -1;
	}

	for (int i = 0; i < fileNames.size(); i++) {
		cout << dirPath + "\\color\\" + fileNames.at(i) << endl;
		colorDataIn.open((dirPath + "\\color\\" + fileNames.at(i)).c_str(), ios_base::in | ios_base::binary);
		colorDataIn.read(reinterpret_cast<char*> (yuy2DataArray), sizeof(BYTE) * yuy2ArraySize);


		convertingYUYV2RGB(yuy2DataArray, rgbDataArray);
		colorFrameMat = Mat(1080, 1920, CV_8UC3, rgbDataArray);
		imshow("color image", colorFrameMat);
		waitKey(1);
		colorVideoWriter.write(colorFrameMat);
		colorDataIn.close();
		memset(rgbDataArray, 0, rgbArraySize);
		memset(yuy2DataArray, 10, yuy2ArraySize);
	}



	for (int i = 0; i < fileNames.size(); i++) {
		cout << dirPath + "\\color\\" + fileNames.at(i) << endl;

		depthDataIn.open((dirPath + "\\depth\\" + fileNames.at(i)).c_str(), ios_base::in | ios_base::binary);
		depthDataIn.read(reinterpret_cast<char*> (depthFrameArray_rawDepth), sizeof(UINT16) * size_depthFrameArray_rawDepth);

		bodyIndexDataIn.open((dirPath + "\\bodyIndex\\" + fileNames.at(i)).c_str(), ios_base::in | ios_base::binary);
		bodyIndexDataIn.read(reinterpret_cast<char*> (bodyIndexFrameArray_rawBodyIndex), sizeof(BYTE) * size_depthFrameArray_rawDepth);



		bodyIndexDataIn.open((dirPath + "\\body\\" + fileNames.at(i)).c_str(), ios_base::in | ios_base::binary);
		bodyIndexDataIn.read(reinterpret_cast<char*> (bodyIndexFrameArray_rawBodyIndex), sizeof(BYTE) * size_depthFrameArray_rawDepth);

		for (UINT i = 0; i < size_depthFrameArray_rawDepth; i++) {
			bodyIndexFrameArray_BGR[i * 3] = bodyIndexFrameArray_rawBodyIndex[i] & 0x01 ? 0x00 : 0xFF;
			bodyIndexFrameArray_BGR[i * 3 + 1] = bodyIndexFrameArray_rawBodyIndex[i] & 0x02 ? 0x00 : 0xFF;
			bodyIndexFrameArray_BGR[i * 3 + 2] = bodyIndexFrameArray_rawBodyIndex[i] & 0x04 ? 0x00 : 0xFF;


		}
		bodyIndexFrameMat = Mat(424, 512, CV_8UC3, bodyIndexFrameArray_BGR);


		for (int i = 0; i < size_depthFrameArray_rawDepth; i++) {
			USHORT depth = depthFrameArray_rawDepth[i];

			if (depth < 0) {
				depthFrameArray_BGR[i * 3] = 0xFF;
				depthFrameArray_BGR[i * 3 + 1] = 0;
				depthFrameArray_BGR[i * 3 + 2] = 0;
			}

			else {
				memset(depthFrameArray_BGR + 3 * i, (depth & 0xFF), 3);
			}
		}
		depthFrameMat = Mat(424, 512, CV_8UC3, depthFrameArray_BGR);

		imshow("depth image", depthFrameMat);
		imshow("bodyIndex image", bodyIndexFrameMat);
		imshow("body image", bodyFrameMat);

		depthVideoWriter.write(colorFrameMat);
		bodyIndexVideoWriter.write(bodyIndexFrameMat);
		bodyVideoWriter.write(bodyFrameMat);

		waitKey(1);


		depthDataIn.close();
		bodyIndexDataIn.close();
		bodyIndexDataIn.close();


		
		memset(depthFrameArray_rawDepth, 0, size_depthFrameArray_rawDepth);
		memset(depthFrameArray_BGR, 0, size_depthFrameArray_BGR);

		memset(depthFrameArray_rawDepth, 0, size_depthFrameArray_rawDepth);
		memset(bodyIndexFrameArray_BGR, 0, size_depthFrameArray_BGR);
	}





	colorVideoWriter.release();
	cvDestroyWindow("color image");
	string colorPathNew = dirPath + "color.mp4";
	rename(colorVideoPath.c_str(), colorPathNew.c_str());
}