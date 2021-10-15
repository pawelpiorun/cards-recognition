#pragma once
#include <opencv2/opencv.hpp>

struct Card {
	bool isBlack;
	int value;
	char suit;
	cv::Mat imageColor;
	cv::Mat imageProcessed;
};
