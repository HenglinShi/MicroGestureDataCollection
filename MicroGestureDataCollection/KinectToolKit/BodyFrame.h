#pragma once
#include "stdafx.h"
#include "Frame.h"
class BodyFrame :
	public Frame
{
private:
	IBodyFrameReference * mFrameReference = nullptr;
	IBodyFrame * mFrame = nullptr;
	vector<CameraSpacePoint> mJoints_CameraSpacePoint;
	vector<DepthSpacePoint> mJoints_DepthSpacePoint;
	vector<ColorSpacePoint> mjoints_ColorSpacePoint;
public:
	BodyFrame();
	~BodyFrame();

	vector<CameraSpacePoint> getJoints_CameraSpacePoint();
	HRESULT updateFrame(IMultiSourceFrame * multisourceFrame);
	void reset();
};

