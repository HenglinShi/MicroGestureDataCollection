#include "stdafx.h"
#include "Frame.h"

Frame::Frame()
{
}


Frame::~Frame()
{
}

bool Frame::getHealth()
{
	return this->health;
}

bool Frame::isHealthy()
{
	return this->health;;
}

BYTE * Frame::getFrameArray_BGRA()
{
	if (!this->filled_FrameArray_BGRA) {
		if (FAILED(this->setFrameArray_BGRA())) {
			return nullptr;
		}
	}

	return this->frameArray_BGRA;

}

BYTE * Frame::getFrameArray_BGR()
{
	if (!this->filled_FrameArray_BGR) {
		if (FAILED(this->setFrameArray_BGR())) {
			return nullptr;
		}
	}

	return this->frameArray_BGR;
}


HRESULT Frame::setFrameHeight()
{
	return this->mFrameDescription->get_Height(&(this->frameHeight));
}

HRESULT Frame::setFrameWeight()
{
	return this->mFrameDescription->get_Width(&(this->frameWidth));
}

void Frame::setHealth(bool health)
{
	this->health = health;
}


HRESULT Frame::setFrameMat_BGRA()
{
	if (this->filled_FrameArray_BGRA) {
		this->frameMat_BGRA = Mat(this->frameHeight, this->frameWidth, CV_8UC4,	this->frameArray_BGRA);
		return S_OK;
	}
	else {
		if (SUCCEEDED(this->setFrameArray_BGRA())) {
			this->frameMat_BGRA = Mat(this->frameHeight, this->frameWidth, CV_8UC4,	this->frameArray_BGRA);
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT Frame::setFrameMat_BGR()
{
	if (this->filled_FrameArray_BGR) {
		this->frameMat_BGR = Mat(this->frameHeight, this->frameWidth, CV_8UC3, this->frameArray_BGR);
		return S_OK;
	}
	else {
		if (SUCCEEDED(this->setFrameArray_BGR())) {
			this->frameMat_BGR = Mat(this->frameHeight,	this->frameWidth, CV_8UC3, this->frameArray_BGR);
			return S_OK;
		}
	}
	return E_FAIL;
}



int Frame::getFrameHeight()
{
	return this->frameHeight;
}

int Frame::getFrameWidth()
{
	return this->frameWidth;
}

Mat Frame::getFrameMat_BGRA()
{
	if (this->filled_FrameMat_BGRA) {
		return this->frameMat_BGRA;
	}
	else {
		if(SUCCEEDED(this->setFrameMat_BGRA())){
			return this->frameMat_BGRA;
		}
	}
	return Mat::zeros(this->frameHeight, this->frameWidth, CV_8UC4);
}

Mat Frame::getFrameMat_BGR()
{
	if (this->filled_FrameMat_BGR) {
		return this->frameMat_BGR;
	}
	else {
		if (SUCCEEDED(this->setFrameMat_BGR())) {
			return this->frameMat_BGR;
		}
	}
	return Mat::zeros(this->frameHeight, this->frameWidth, CV_8UC3);
}
