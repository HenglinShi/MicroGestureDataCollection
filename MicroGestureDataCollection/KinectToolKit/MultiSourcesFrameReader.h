#pragma once
#include "ColorFrame.h"
#include "DepthFrame.h"
#include "BodyIndexFrame.h"
#include "BodyFrame.h"
#include "KinectSensor.h"
class MultiSourcesFrameReader
{
private:
	
	IMultiSourceFrameReader * mFrameReader= nullptr;
	IMultiSourceFrame * mFrame = nullptr;

	ColorFrame * mColorFrame = nullptr;
	DepthFrame * mDepthFrame = nullptr;
	BodyFrame * mBodyFrame = nullptr;
	BodyIndexFrame * mBodyIndexFrame = nullptr;

public:
	MultiSourcesFrameReader(KinectSensor * kinectSensor);
	~MultiSourcesFrameReader();

	HRESULT updateFrame();
	bool getHealth();
	ColorFrame * getColorFrame();
	DepthFrame * getDepthFrame();
	BodyIndexFrame * getBodyIndexFrame();
	BodyFrame * getBodyFrame();
	void reset();

};

