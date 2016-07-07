#include "stdafx.h"
#include "ColorFrame.h"


HRESULT ColorFrame::setFrameArray_BGRA()
{
	if (SUCCEEDED(this->mColorFrame->CopyConvertedFrameDataToArray(this->length_FrameArray_BGRA, this->frameArray_BGRA, ColorImageFormat::ColorImageFormat_Bgra))) {
		this->filled_FrameArray_BGRA = FILLED;
		return S_OK;
	}
	else {
		return E_FAIL;
	}
}

HRESULT ColorFrame::setFrameArray_BGR()
{

}

ColorFrame::ColorFrame()
{
	
}


ColorFrame::~ColorFrame()
{
}

HRESULT ColorFrame::updateFrame(IMultiSourceFrame * multisourceFrame)
{
	if (SUCCEEDED(multisourceFrame->get_ColorFrameReference(&(this->mColorFrameReference)))) {
		if (SUCCEEDED(mColorFrameReference->AcquireFrame(&(this->mColorFrame)))) {
			return S_OK;
		}
		this->mColorFrameReference->Release();
	}
	return E_FAIL;
}

BYTE * ColorFrame::getFrameArray_YUY2()
{
	if (this->filled_FrameArray_YUY2) {
		return this->frameArray_YUY2;
	}
	else {
		if (SUCCEEDED(this->mColorFrame->get_RawColorImageFormat(&(this->mColorFrameFormat)))) {

			if (mColorFrameFormat == ColorImageFormat::ColorImageFormat_Yuy2) {
				if (SUCCEEDED(this->mColorFrame->CopyRawFrameDataToArray(this->length_FrameArray_YUY2, this->frameArray_YUY2))) {
					this->filled_FrameArray_YUY2 = FILLED;
					return this->frameArray_YUY2;
				}
			}
			else {
				if (SUCCEEDED(this->mColorFrame->CopyConvertedFrameDataToArray(this->length_FrameArray_YUY2, this->frameArray_YUY2, ColorImageFormat::ColorImageFormat_Yuy2))) {
					this->filled_FrameArray_YUY2 = FILLED;
					return this->frameArray_YUY2;
				}
			}
		}
	}
	return nullptr;
}

void ColorFrame::reset()
{
	this->filled_FrameArray_BGRA = UNFILLED;
	this->filled_FrameArray_YUY2 = UNFILLED;
	this->mColorFrame->Release();
	this->mColorFrameReference->Release();

}
