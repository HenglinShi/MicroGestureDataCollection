#include "stdafx.h"
#include "DepthFrame.h"


DepthFrame::DepthFrame(IMultiSourceFrame * multisourceFrame)
{
	if (SUCCEEDED(this->initialization(multisourceFrame))) {
		this->setHealth(HEALTHY);
	}
	else {
		this->setHealth(UNHEALTHY);
	}
}


DepthFrame::~DepthFrame()
{
}

HRESULT DepthFrame::initialization(IMultiSourceFrame * multisourceFrame)
{
	if (SUCCEEDED(this->updateFrame(multisourceFrame))) {
		if (this->mFrame->get_FrameDescription(&(this->mFrameDescription))) {
			if (SUCCEEDED(this->setFrameHeight()) && SUCCEEDED(this->setFrameWeight)) {

				if (SUCCEEDED(this->setLength_FrameArray_Depth())) {

					this->length_FrameArray_BGRA = this->length_FrameArray_Depth * 4;
					this->frameArray_BGRA = new BYTE[this->length_FrameArray_BGRA];
					this->frameArray_Depth = new UINT16[this->length_FrameArray_Depth];

					this->filled_FrameArray_BGRA = UNFILLED;
					this->filled_FrameArray_Depth = UNFILLED;

					return S_OK;
				}
			}
		}
	}
	return E_FAIL;
}

void DepthFrame::reset()
{
	this->filled_FrameArray_BGRA = UNFILLED;
	this->frameArray_Depth = UNFILLED;
	this->mFrame->Release();
	this->mFrameReference->Release();
}

HRESULT DepthFrame::setFrameArray_Depth()
{
	if (this->mFrame) {
		if (SUCCEEDED(this->mFrame->AccessUnderlyingBuffer(&this->length_FrameArray_Depth, &this->frameArray_Depth)))
			return S_OK;
	}
	return E_FAIL;
}

HRESULT DepthFrame::setLength_FrameArray_Depth()
{
	if (this->frameHeight * this->frameWidth > 0) {
		this->length_FrameArray_Depth = this->frameWidth * this->frameWidth;
		return S_OK;
	}
	return E_FAIL;
}


HRESULT DepthFrame::updateFrame(IMultiSourceFrame * multisourceFrame)
{
	if (SUCCEEDED(multisourceFrame->get_DepthFrameReference(&(this->mFrameReference)))) {
		if (SUCCEEDED(this->mFrameReference->AcquireFrame(&(this->mFrame)))) {
			return S_OK;
		}
		this->mFrameReference->Release();
	}
	return E_FAIL;
}

UINT16 * DepthFrame::getFrameArray_Depth()
{
	if (this->filled_FrameArray_Depth) {
		return this->frameArray_Depth;
	}
	else {
		if (this->mFrame) {
			if (SUCCEEDED(this->setFrameArray_Depth()))
				return this->frameArray_Depth;

		}
	}
	return nullptr;
}

UINT DepthFrame::getLength_FrameArray_Depth()
{
	if (this->filled_FrameArray_Depth) {
		return this->length_FrameArray_Depth;
	}
	else {
		if (this->mFrame) {
			if (SUCCEEDED(this->setFrameArray_Depth()))
				return this->length_FrameArray_Depth;
			else
				return 0;
		}
	}
}




HRESULT DepthFrame::setFrameArray_BGR()
{
	if (!this->filled_FrameArray_Depth) {
		if (FAILED(this->setFrameArray_Depth())) {
			return E_FAIL;
		}
	}

	USHORT depth = 0;
	for (int i = 0; i < this->length_FrameArray_Depth; i++) {

		depth = this->frameArray_Depth[i];

		if (depth < 0) {
			this->frameArray_BGR[i * 3] = 0xFF;
			this->frameArray_BGR[i * 3 + 1] = 0;
			this->frameArray_BGR[i * 3 + 2] = 0;
		}
		else {
			memset(this->frameArray_BGR + 3 * i, (depth & 0xFF), 3);
		}
	}
	return S_OK;
}