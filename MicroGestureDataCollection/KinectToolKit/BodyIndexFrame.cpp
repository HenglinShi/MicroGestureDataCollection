#include "stdafx.h"
#include "BodyIndexFrame.h"




BodyIndexFrame::BodyIndexFrame(IMultiSourceFrame * multisourceFrame)
{
	if (SUCCEEDED(this->initialization(multisourceFrame))) {
		this->setHealth(HEALTHY);
	}
	else {
		this->setHealth(UNHEALTHY);
	}
}


BodyIndexFrame::~BodyIndexFrame()
{
}

HRESULT BodyIndexFrame::initialization(IMultiSourceFrame * multisourceFrame)
{
	if (SUCCEEDED(this->updateFrame(multisourceFrame))) {
		if (this->mFrame->get_FrameDescription(&(this->mFrameDescription))) {
			if (SUCCEEDED(this->setFrameHeight()) && SUCCEEDED(this->setFrameWeight)) {

				if (SUCCEEDED(this->setLength_FrameArray_Index())) {

					this->length_FrameArray_BGRA = this->length_FrameArray_Index * 4;
					this->length_FrameArray_BGR = this->length_FrameArray_Index * 3;

					this->frameArray_BGRA = new BYTE[this->length_FrameArray_BGRA];
					this->frameArray_BGR = new BYTE[this->length_FrameArray_BGR];
					this->frameArray_Index = new BYTE[this->length_FrameArray_Index];

					this->filled_FrameArray_BGRA = UNFILLED;
					this->filled_FrameArray_BGR = UNFILLED;
					this->filled_FrameArray_Index = UNFILLED;

					return S_OK;
				}
			}
		}
	}
	return E_FAIL;
}

BYTE * BodyIndexFrame::getFrameArray_Index()
{
	if (this->filled_FrameArray_Index) {
		return this->frameArray_Index;	
	}
	else {
		if (this->mFrame) {
			if (SUCCEEDED(this->setFrameArray_Index())) {
				return this->frameArray_Index;
			}
				
		}
	}
	return nullptr;
}

UINT BodyIndexFrame::getLength_FrameArray_Index()
{
	if (this->filled_FrameArray_Index) {
		return this->length_FrameArray_Index;
	}
	else {
		if (this->mFrame) {
			if (SUCCEEDED(this->setFrameArray_Index()))
				return this->length_FrameArray_Index;
			else
				return 0;
		}
	}
}

HRESULT BodyIndexFrame::setLength_FrameArray_Index()
{
	if (this->frameHeight * this->frameWidth > 0) {
		this->length_FrameArray_Index = this->frameWidth * this->frameWidth;
		return S_OK;
	}
	return E_FAIL;
}

HRESULT BodyIndexFrame::setFrameArray_BGR()
{
	if (!this->filled_FrameArray_Index) {
		if (FAILED(this->setFrameArray_Index())) {
			return E_FAIL;
		}
	}
	for (UINT i = 0; i < this->length_FrameArray_Index; i++) {
		this->frameArray_BGR[i * 3] = this->frameArray_Index[i] & 0x01 ? 0x00 : 0xFF;
		this->frameArray_BGR[i * 3 + 1] = this->frameArray_Index[i] & 0x02 ? 0x00 : 0xFF;
		this->frameArray_BGR[i * 3 + 2] = this->frameArray_Index[i] & 0x04 ? 0x00 : 0xFF;
	}
	return S_OK;
}

HRESULT BodyIndexFrame::setFrameArray_Index()
{
	if (this->mFrame) {
		if (SUCCEEDED(this->mFrame->AccessUnderlyingBuffer(&this->length_FrameArray_Index, &this->frameArray_Index)))
			return S_OK;
	}
	return E_FAIL;
}

HRESULT BodyIndexFrame::updateFrame(IMultiSourceFrame * multisourceFrame)
{
	if (SUCCEEDED(multisourceFrame->get_BodyIndexFrameReference(&(this->mFrameReference)))) {
		if (SUCCEEDED(this->mFrameReference->AcquireFrame(&(this->mFrame)))) {
			return S_OK;
		}
		this->mFrameReference->Release();
	}
	return E_FAIL;
}

void BodyIndexFrame::reset()
{
	this->mFrame->Release();
	this->mFrameReference->Release();
	this->filled_FrameArray_BGRA = UNFILLED;
	this->filled_FrameArray_Index = UNFILLED;
}
