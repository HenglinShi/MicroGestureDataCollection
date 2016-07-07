#include "stdafx.h"
#include "BodyFrame.h"


BodyFrame::BodyFrame()
{
}


BodyFrame::~BodyFrame()
{
}

HRESULT BodyFrame::updateFrame(IMultiSourceFrame * multisourceFrame)
{
	if (SUCCEEDED(multisourceFrame->get_BodyFrameReference(&(this->mFrameReference)))) {
		if (SUCCEEDED(this->mFrameReference->AcquireFrame(&(this->mFrame)))) {
			return S_OK;
		}
		this->mFrameReference->Release();
	}
	return E_FAIL;
}

void BodyFrame::reset()
{
	this->filled_FrameArray_BGRA = UNFILLED;
	this->mFrame->Release();
	this->mFrameReference->Release();
	this->mJoints_CameraSpacePoint.clear();
	this->mjoints_ColorSpacePoint.clear();
	this->mJoints_DepthSpacePoint.clear();
}
