#pragma once
class FrameReader
{
public:
	FrameReader();
	~FrameReader();

	virtual HRESULT updateFrame() = 0;

};

