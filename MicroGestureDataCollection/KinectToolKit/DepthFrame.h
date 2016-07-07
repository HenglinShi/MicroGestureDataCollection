#pragma once
#include "Frame.h"
class DepthFrame :
	public Frame
{
private:
	UINT16 * frameArray_Depth;
	UINT length_FrameArray_Depth;
	bool filled_FrameArray_Depth;
	
	IDepthFrame * mFrame = nullptr;
	IDepthFrameReference * mFrameReference = nullptr;

	HRESULT setFrameArray_Depth();
	HRESULT setLength_FrameArray_Depth();
	HRESULT setFrameArray_BGR();
	HRESULT initialization(IMultiSourceFrame * multisourceFrame);

public:
	DepthFrame(IMultiSourceFrame * multisourceFrame);
	~DepthFrame();
	HRESULT updateFrame(IMultiSourceFrame * multisourceFrame);


	UINT16 * getFrameArray_Depth();
	UINT getLength_FrameArray_Depth();
	
	void reset();

};

