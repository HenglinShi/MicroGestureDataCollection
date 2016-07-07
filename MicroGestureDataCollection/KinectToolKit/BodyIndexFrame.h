#pragma once
#include "Frame.h"
class BodyIndexFrame :
	public Frame
{
private:
	BYTE * frameArray_Index;
	bool filled_FrameArray_Index;
	UINT length_FrameArray_Index;

	IBodyIndexFrame * mFrame = nullptr;
	IBodyIndexFrameReference * mFrameReference = nullptr;
	HRESULT setLength_FrameArray_Index();
	HRESULT setFrameArray_BGR();
	HRESULT setFrameArray_Index();
public:
	BodyIndexFrame(IMultiSourceFrame * multisourceFrame);
	~BodyIndexFrame();
	
	HRESULT initialization(IMultiSourceFrame * multisourceFrame);

	BYTE * getFrameArray_Index();
	UINT getLength_FrameArray_Index();
	HRESULT updateFrame(IMultiSourceFrame * multisourceFrame);
	void reset();
};

