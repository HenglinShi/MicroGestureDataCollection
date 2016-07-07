#pragma once
#define FILLED true
#define UNFILLED false

class Frame
{
private:
	Mat frameMat_BGRA;
	Mat frameMat_BGR;
	Mat frameMat_RGBA;
	Mat frameMat_RGB;
	bool filled_FrameMat_BGR;
	bool filled_FrameMat_BGRA;


	UINT length_FrameArray_RGBA;
	BYTE * frameArray_RGBA;

	UINT length_FrameArray_RGB;
	BYTE * frameArray_RGB;

	


	HRESULT setFrameMat_BGRA();
	HRESULT setFrameMat_BGR();

protected:
	UINT length_FrameArray_BGRA;
	BYTE * frameArray_BGRA;
	bool filled_FrameArray_BGRA;

	UINT length_FrameArray_BGR;
	BYTE * frameArray_BGR;
	bool filled_FrameArray_BGR;


	int frameHeight;
	int frameWidth;
	IFrameDescription * mFrameDescription;
	HRESULT setFrameHeight();
	HRESULT setFrameWeight();
	void setHealth(bool health);

	bool health;

public:
	Frame();
	~Frame();
	
	bool getHealth();
	bool isHealthy();

	BYTE * getFrameArray_BGRA();
	BYTE * getFrameArray_BGR();

	virtual HRESULT setFrameArray_BGRA() = 0;
	virtual HRESULT setFrameArray_BGR() = 0;

	virtual UINT getLength_FrameArray_BGR() = 0;
	virtual UINT getLength_FrameArray_BGRA() = 0;
	
	int getFrameHeight();
	int getFrameWidth();

	Mat getFrameMat_BGRA();
	Mat getFrameMat_BGR();



};

