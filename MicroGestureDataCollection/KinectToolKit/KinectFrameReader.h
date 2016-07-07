#pragma once
#include "stdafx.h"

class KinectFrameReader
{
public:
	KinectFrameReader();
	~KinectFrameReader();

	virtual HRESULT update() = 0;


};

