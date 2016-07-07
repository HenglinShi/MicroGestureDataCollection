#include "stdafx.h"
#include "MultiSourcesFrameReader.h"


MultiSourcesFrameReader::MultiSourcesFrameReader(KinectSensor * kinectSensor)
{
}


MultiSourcesFrameReader::~MultiSourcesFrameReader()
{
}

HRESULT MultiSourcesFrameReader::updateFrame()
{
	if (FAILED(this->mFrameReader->AcquireLatestFrame(&(this->mFrame)))) {
		cout<<"Update multisource frame failed"<<endl;
		return E_FAIL;
	}
	
	if (FAILED(this->mColorFrame->updateFrame(this->mFrame))) {
		cout << "Update multisource frame failed" << endl;
		return E_FAIL;
	}

	if (FAILED(this->mDepthFrame->updateFrame(this->mFrame))) {
		cout << "Update multisource frame failed" << endl;
		return E_FAIL;
	}

	if (FAILED(this->mBodyFrame->updateFrame(this->mFrame))) {
		cout << "Update multisource frame failed" << endl;
		return E_FAIL;
	}

	if (FAILED(this->mBodyIndexFrame->updateFrame(this->mFrame))) {
		cout << "Update multisource frame failed" << endl;
		return E_FAIL;
	}
	
	return S_OK;
}

void MultiSourcesFrameReader::reset()
{
	this->mColorFrame->reset();
	
}

