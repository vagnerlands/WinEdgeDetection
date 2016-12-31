#ifndef _BITMAPHANDLER_H_
#define _BITMAPHANDLER_H_

#include "CCommonTypes.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <float.h>
#include <stdbool.h>

using namespace Types;

class CBitmapHandler
{
public:
	CBitmapHandler();
	~CBitmapHandler();

	bool load(char* filePath);
	bool save(char* filePath);
	bitmap_info_header_t getBmpHeader();
	pixel_t getPixelAt(int col, int row);
	pixel_t getOutputPixelAt(int col, int row);
	pixel_t getDistancePixelAt(int col, int row);

	pixel_t* canny_edge_detection();

	pixel_t* getBitmapImage();

	Types::EImageState getCurrentState();

	void increaseMinThreshold(int value);
	void increaseMaxThreshold(int value);
	void increaseSigma(float value);
	void increaseCurrentState(int numberOfSteps);

	char* getObjectText();

private:
	void GaussianFilter(const pixel_t *in, pixel_t *out,
		const int nx, const int ny, const float sigma);
	void convolution(const pixel_t *in, pixel_t *out, const float *kernel,
		const int nx, const int ny, const int kn,
		const bool normalize);
	bitmap_info_header_t bitmapInfoHeader;
	pixel_t* bitmapImage;
	pixel_t* outputImage;
	pixel_t *G;
	pixel_t *after_Gx;
	pixel_t *after_Gy;
	pixel_t *nms;
	char objText[132];

	Types::EImageState currentState;
	int minThreshold;
	int maxThreshold;
	float sigmaValue;
};

inline pixel_t
CBitmapHandler::getPixelAt(int col, int row)
{
	if (row > bitmapInfoHeader.width) 
		return 0;
	else
		return bitmapImage[col + (bitmapInfoHeader.width * row)];
}

inline pixel_t
CBitmapHandler::getOutputPixelAt(int col, int row)
{
	if (row > bitmapInfoHeader.width) 
		return 0;
	else
		return outputImage[col + (bitmapInfoHeader.width * row)];
}

inline pixel_t
CBitmapHandler::getDistancePixelAt(int col, int row)
{
	if (row > bitmapInfoHeader.width)
		return 0;
	else
		return G[col + (bitmapInfoHeader.width * row)];
}

inline pixel_t* 
CBitmapHandler::getBitmapImage()
{
	return bitmapImage;
}

inline Types::EImageState 
CBitmapHandler::getCurrentState()
{
	return currentState;
}

inline void CBitmapHandler::increaseMinThreshold(int value)
{
	minThreshold += value;
	if (minThreshold > maxThreshold) 
		minThreshold = maxThreshold;
}

inline void CBitmapHandler::increaseMaxThreshold(int value)
{
	maxThreshold += value;
	if (maxThreshold < minThreshold)
		maxThreshold = minThreshold;
}

inline void CBitmapHandler::increaseSigma(float value)
{
	sigmaValue += value;
}

inline void CBitmapHandler::increaseCurrentState(int numberOfSteps)
{
	currentState = static_cast<Types::EImageState>((currentState
		+ numberOfSteps) % EIMAGESTATE_FINAL);

	if (currentState < Types::EIMAGESTATE_FIRST)
		currentState = Types::EIMAGESTATE_FIRST;
}

inline char*
CBitmapHandler::getObjectText()
{
	objText[0] = 0;
	switch (currentState)
	{
	case Types::EIMAGESTATE_GAUSSIAN:
		sprintf(objText, "ST[GAUSSIAN] - Sigma %1.1f, MinT %d, MaxT %d", sigmaValue, minThreshold, maxThreshold);
		break;
	case Types::EIMAGESTATE_DISTANCE:
		sprintf(objText, "ST[DISTANCE] - Sigma %1.1f, MinT %d, MaxT %d", sigmaValue, minThreshold, maxThreshold);
		break;
	case Types::EIMAGESTATE_CLEAN:
		sprintf(objText, "ST[NOOP] - Sigma %1.1f, MinT %d, MaxT %d", sigmaValue, minThreshold, maxThreshold);
		break;
	case Types::EIMAGESTATE_CANNY:
		sprintf(objText, "ST[CANNY] - Sigma %1.1f, MinT %d, MaxT %d", sigmaValue, minThreshold, maxThreshold);
		break;
	default:
		sprintf(objText, "Error - State not found!");
		break;
	}

	return objText;
}

#endif //_BITMAPHANDLER_H_
