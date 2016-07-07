#pragma once
#include "Frame.h"
#define FILLED true
#define UNFILLED false

class ColorFrame :
	public Frame
{
private:

	BYTE * frameArray_YUY2;
	UINT length_FrameArray_YUY2;
	bool filled_FrameArray_YUY2;

	HRESULT setFrameArray_BGRA();
	HRESULT setFrameArray_BGR();

	

	IColorFrame * mColorFrame = nullptr;
	IColorFrameReference * mColorFrameReference = nullptr;
	ColorImageFormat mColorFrameFormat = ColorImageFormat::ColorImageFormat_None;

public:
	ColorFrame();
	~ColorFrame();

	HRESULT updateFrame(IColorFrameReader * colorFrameReader);
	HRESULT updateFrame(IMultiSourceFrame * multisourceFrame);

	BYTE * getFrameArray_YUY2();
	int getLenght_FrameArray_YUY2();

	void reset();

	
};

