#pragma once
#include <opencv2/opencv.hpp>
#include "Card.h"

using namespace cv;

class cards_recognition {

public:
	std::vector<Card> run(std::string imagePath);

private:
	Mat preliminaryProcess(const Mat& input);
	std::vector<std::vector<Point>> findExternalContours(const Mat& cardImage);
	Rect getExtremeRoi(std::vector<Point> contour);
	Mat separateCard(const Mat& grayImage, std::vector<std::vector<Point>> cardContours, int cardNumber);
	std::vector<Point2f> getCorners(Mat& grayImage, Mat& colorImage);
	std::vector<Point2f> findExtremePoints(std::vector<Point> contour);
	std::vector<Point2f> findCornerPoints(const Mat& cardBox);
	bool wrongCorners(const Mat& cardBox, const std::vector<Point2f>& cornerPoints);
	Point2f getGravityCenter(std::vector<Point> contour);
	void rotateCard(Mat& cardImage, double angle);
	Mat getCardImage(const Mat& input, std::vector<Point2f> corners);
	Mat processCardImage(Mat cardImage);
	void scaleCardImage(Card& card, double scale);
	int calculateThreshold(const Mat& cardImage, int minValue = 0, int constant = 0);
	void recognizeCards(std::vector<Card>& cards);
	int getCardValue(const std::vector<std::vector<Point>>& contours, int connected);
	int findConnected(const std::vector<std::vector<Point>>& contours, int threshold);
	Mat getCardSymbol(const Mat& cardImage, const std::vector<std::vector<Point>>& contours);
	bool getCardColor(const Mat& cardSymbol, double redPercent);
	char getCardSuit(Mat cardSymbol, bool isBlack, int blackThreshold, double redTolerance);
	int getMaximumContour(std::vector<std::vector<Point>> contours);
};