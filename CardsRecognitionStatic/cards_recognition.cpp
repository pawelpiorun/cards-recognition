#include "cards_recognition.h"

std::vector<Card> cards_recognition::run(std::string imagePath)
{
	std::vector<Card> cards;

	Mat input = cv::imread(imagePath);
	Mat image = preliminaryProcess(input);
	Mat grayImage;
	cvtColor(image, grayImage, COLOR_BGR2GRAY);

	std::vector<std::vector<Point>> contours = findExternalContours(grayImage);
	for (int i = 0; i < contours.size(); i++)
	{
		/*create new card, zoom the contour on image, take the binary version for detecting corners
		and color version to return the original image*/
		Card newCard;
		Rect cardRoi = getExtremeRoi(contours[i]);
		Mat cardBoxBinary = separateCard(grayImage, contours, i);
		cardBoxBinary = cardBoxBinary(cardRoi);
		Mat cardBoxColor = image(cardRoi).clone();

		std::vector<Point2f> cornerPoints = getCorners(cardBoxBinary, cardBoxColor);
		newCard.imageColor = getCardImage(cardBoxColor, cornerPoints);
		newCard.imageProcessed = processCardImage(newCard.imageColor);

		//scale in order to get rid of wrong pixels on the edge of the card
		scaleCardImage(newCard, 0.95);
		cards.push_back(newCard);
	}
	recognizeCards(cards);
	return cards;
}

Mat cards_recognition::preliminaryProcess(const Mat& input)
{
	//cut borders and get rid of salt&pepper noise
	Rect roi = Rect(2, 2, input.cols - 4, input.rows - 4);
	Mat output = input(roi).clone();
	medianBlur(output, output, 7);
	return output;
}

std::vector<std::vector<Point>> cards_recognition::findExternalContours(const Mat& cardImage)
{
	Mat contoursImage;
	//get binary image of cards externals
	threshold(cardImage, contoursImage, 40, 255, THRESH_BINARY);
	std::vector<std::vector<Point>> contours;
	std::vector<Vec4i> hierarchy;

	//find external contours
	findContours(contoursImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	return contours;
}

Rect cards_recognition::getExtremeRoi(std::vector<Point> contour)
{
	//find max and min coordinates, which contains maximum values (binary image), create ROI
	std::vector<Point2f> extremePoints = findExtremePoints(contour);
	return Rect(extremePoints[0].x, extremePoints[1].y, extremePoints[2].x - extremePoints[0].x + 1, extremePoints[3].y - extremePoints[1].y + 1);
}

Mat cards_recognition::separateCard(const Mat& grayImage, std::vector<std::vector<Point>> cardContours, int cardNumber)
{
	//draw mask of a card, add empty image and grayscale cards image with a card mask, in order to get single card
	Mat contourMask = Mat::zeros(grayImage.size(), CV_8U);
	drawContours(contourMask, cardContours, cardNumber, (255), FILLED);
	Mat singleCard = Mat::zeros(grayImage.size(), CV_8U);
	add(singleCard, grayImage, singleCard, contourMask);
	threshold(singleCard, singleCard, 50, 255, THRESH_BINARY);
	return singleCard;
}

std::vector<Point2f> cards_recognition::getCorners(Mat& grayImage, Mat& colorImage)
{
	std::vector<Point2f> corners;
	Mat cardBox, cardBoxColor;
	do
	{
		std::vector<std::vector<Point>> cardContour = findExternalContours(grayImage);

		//cut empty edges
		Rect cardRoi = getExtremeRoi(cardContour[0]);
		cardBox = grayImage(cardRoi);
		cardBoxColor = colorImage(cardRoi);

		corners = findCornerPoints(cardBox);

		//if corners are wrongy detected, rotate card and detect again
		if (wrongCorners(cardBox, corners))
		{
			rotateCard(cardBox, 3);
			rotateCard(cardBoxColor, 3);
			threshold(cardBox, cardBox, 50, 255, THRESH_BINARY);
		}
	} while (wrongCorners(cardBox, corners));
	return corners;
}

std::vector<Point2f> cards_recognition::findExtremePoints(std::vector<Point> contour)
{
	std::vector<Point2f> extremePoints;
	int  xMinIndex = 0, xMaxIndex = 0, yMinIndex = 0, yMaxIndex = 0;
	for (int j = 0; j < contour.size(); j++)
	{
		if (contour[j].x < contour[xMinIndex].x)
		{
			xMinIndex = j;
		}
		if (contour[j].x > contour[xMaxIndex].x)
		{
			xMaxIndex = j;
		}
		if (contour[j].y < contour[yMinIndex].y)
		{
			yMinIndex = j;
		}
		if (contour[j].y > contour[yMaxIndex].y)
		{
			yMaxIndex = j;
		}
	}
	extremePoints.push_back(Point2f((float)contour[xMinIndex].x, (float)contour[xMinIndex].y));
	extremePoints.push_back(Point2f((float)contour[yMinIndex].x, (float)contour[yMinIndex].y));
	extremePoints.push_back(Point2f((float)contour[xMaxIndex].x, (float)contour[xMaxIndex].y));
	extremePoints.push_back(Point2f((float)contour[yMaxIndex].x, (float)contour[yMaxIndex].y));
	return extremePoints;
}

std::vector<Point2f> cards_recognition::findCornerPoints(const Mat& cardBox)
{
	std::vector<Point2f> cornerPoints;
	Point2f upLeft, upRight, downLeft, downRight;
	for (int i = 0; i < cardBox.cols; i++)
	{
		if (cardBox.at<uchar>(0, i) == 255)
		{
			upRight = Point2f(float(i), 0);
		}
		if (cardBox.at<uchar>(cardBox.rows - 1, cardBox.cols - 1 - i) == 255)
		{
			downLeft = Point2f(float(cardBox.cols - 1 - i), float(cardBox.rows - 1));
		}
	}
	for (int i = cardBox.rows - 1; i >= 0; i--)
	{
		if (cardBox.at<uchar>(i, 0) == 255)
		{
			upLeft = Point2f(0, float(i));
		}
		if (cardBox.at<uchar>(cardBox.rows - 1 - i, cardBox.cols - 1) == 255)
		{
			downRight = Point2f(float(cardBox.cols - 1), float(cardBox.rows - 1 - i));
		}
	}
	//if Points are rotated backwards, because of the algorithm, rotate it clockwise
	if (upLeft.y > downRight.y)
	{
		Point2f tmp = upRight;
		upRight = downRight;
		downRight = downLeft;
		downLeft = upLeft;
		upLeft = tmp;
	}
	cornerPoints.push_back(upLeft);
	cornerPoints.push_back(upRight);
	cornerPoints.push_back(downRight);
	cornerPoints.push_back(downLeft);
	return cornerPoints;
}

bool cards_recognition::wrongCorners(const Mat& cardBox, const std::vector<Point2f>& cornerPoints)
{
	std::vector<std::vector<Point>> contours;
	std::vector<Vec4i> hierarchy;
	findContours(cardBox, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	Point2f gravityCenter = getGravityCenter(contours[0]);

	//if card is more horizontal than vertical
	if ((cornerPoints[0].x - gravityCenter.x) * (cornerPoints[1].x - gravityCenter.x) > 0
		and (cornerPoints[2].x - gravityCenter.x) * (cornerPoints[3].x - gravityCenter.x) > 0
		and (cornerPoints[0].y - gravityCenter.y) * (cornerPoints[1].y - gravityCenter.y) > 0) return false;

	//if corners are not in the right place given the gravityCenter
	if (cornerPoints[0].x > gravityCenter.x or cornerPoints[0].y > gravityCenter.y
		or cornerPoints[1].x < gravityCenter.x or cornerPoints[1].y > gravityCenter.y
		or cornerPoints[2].x < gravityCenter.x or cornerPoints[2].y < gravityCenter.y
		or cornerPoints[3].x > gravityCenter.x or cornerPoints[3].y < gravityCenter.y) return true;

	return false;
}

Point2f cards_recognition::getGravityCenter(std::vector<Point> contour)
{
	int xSum = 0, ySum = 0;
	for (int i = 0; i < contour.size(); i++)
	{
		xSum += contour[i].x;
		ySum += contour[i].y;
	}
	Point2f gravityCenter = Point2f(float(xSum / contour.size()), float(ySum / contour.size()));
	return gravityCenter;
}

void cards_recognition::rotateCard(Mat& cardImage, double angle)
{
	Mat rotation = getRotationMatrix2D(Point2f(float(cardImage.rows / 2), float(cardImage.cols / 2)), angle, 1);
	warpAffine(cardImage, cardImage, rotation, cardImage.size());
}

Mat cards_recognition::getCardImage(const Mat& input, std::vector<Point2f> corners)
{
	Mat cardImage;
	//get destined card size
	float cardWidth = sqrt((corners[1].y - corners[0].y) * (corners[1].y - corners[0].y)
		+ (corners[1].x - corners[0].x) * (corners[1].x - corners[0].x));
	float cardHeight = sqrt((corners[3].y - corners[0].y) * (corners[3].y - corners[0].y)
		+ (corners[3].x - corners[0].x) * (corners[3].x - corners[0].x));

	//calculate and make transformation
	std::vector<Point2f> srcPoints = { corners[0], corners[1], corners[2], corners[3] };
	std::vector<Point2f> dstPoints = { Point2f(0,0), Point2f(cardWidth,0),
		Point2f(cardWidth,cardHeight), Point2f(0,cardHeight) };
	Mat T = getPerspectiveTransform(srcPoints, dstPoints);

	warpPerspective(input, cardImage, T, Size(cardWidth, cardHeight));
	return cardImage;
}

Mat cards_recognition::processCardImage(Mat cardImage)
{
	//get rid of salt&pepper noise, get grayscale image and then threshold to get binary image
	medianBlur(cardImage, cardImage, 7);
	cvtColor(cardImage, cardImage, COLOR_BGR2GRAY);
	int treshold = calculateThreshold(cardImage, 50, 35);
	threshold(cardImage, cardImage, treshold, 255, THRESH_BINARY);
	return cardImage;
}

void cards_recognition::scaleCardImage(Card& card, double scale)
{
	int roiWidth = int(scale * card.imageProcessed.cols);
	int roiHeight = int(scale * card.imageProcessed.rows);
	Rect scaledRoi = Rect((card.imageProcessed.cols - roiWidth) / 2, (card.imageProcessed.rows - roiHeight) / 2, roiWidth, roiHeight);

	card.imageProcessed = card.imageProcessed(scaledRoi);
	card.imageColor = card.imageColor(scaledRoi);
}

int cards_recognition::calculateThreshold(const Mat& cardImage, int minValue, int constant)
{
	//get mean value of pixels above minValue
	int accumulator = 0, pixelNumber = 0;
	for (int i = 0; i < cardImage.cols * cardImage.rows; i++)
	{
		if (cardImage.data[i] > minValue)
		{
			pixelNumber++;
			accumulator += cardImage.data[i];
		}
	}
	//substract constant
	return accumulator / pixelNumber - constant;;
}

void cards_recognition::recognizeCards(std::vector<Card>& cards)

{
	for (int i = 0; i < cards.size(); i++)
	{
		Mat card = cards[i].imageProcessed;
		Mat cardUnprocessed = cards[i].imageColor;

		//find external contours inside the card
		Mat cardInverted = 255 - card;
		std::vector<std::vector<Point>> symbolContours = findExternalContours(cardInverted);

		//get card value, symbol, color and suit
		cards[i].value = getCardValue(symbolContours, findConnected(symbolContours, 166));
		Mat cardSymbolBlack = getCardSymbol(card, symbolContours);
		Mat cardSymbolColor = getCardSymbol(cardUnprocessed, symbolContours);
		cards[i].isBlack = getCardColor(cardSymbolColor, 0.68);
		cards[i].suit = getCardSuit(cardSymbolBlack, cards[i].isBlack, 241, 0.3);
	}
}

int cards_recognition::getCardValue(const std::vector<std::vector<Point>>& contours, int connected)
{
	//if size is > 13 - it's a 10
	if (contours.size() > 13) return 10;
	else return contours.size() - 4 + connected;
}

int cards_recognition::findConnected(const std::vector<std::vector<Point>>& contours, int threshold)
{
	//first and last contour which area is greater than theshold are connected
	int connected = 0;
	if (contours[contours.size() - 1].size() > threshold) connected++;
	if (contours[0].size() > threshold) connected++;
	return connected;
}

Mat cards_recognition::getCardSymbol(const Mat& cardImage, const std::vector<std::vector<Point>>& contours)
{
	//get maximum contour of a symbol form the upper half of the card and zoom it
	int k = getMaximumContour(contours);
	Rect symbolRoi = getExtremeRoi(contours[k]);
	return cardImage(symbolRoi);
}

bool cards_recognition::getCardColor(const Mat& cardSymbol, double redPercent)
{
	//get red channel and count red values - if the percentage is ok, then its red
	Mat imgChannels[3];
	split(cardSymbol, imgChannels);
	Mat redChannel = imgChannels[2];
	int sumRed = 0, sum = 0;
	for (int i = 0; i < cardSymbol.cols * cardSymbol.rows - 1; i++)
	{
		sumRed += redChannel.data[i];
		sum += (int)((cardSymbol.data[3 * i] + cardSymbol.data[3 * i + 1] + cardSymbol.data[3 * i + 2]) / 3);
	}
	sumRed /= redChannel.cols * redChannel.rows;
	sum /= cardSymbol.cols * cardSymbol.rows;
	if (sumRed > redPercent * sum) return false;
	return true;
}

char cards_recognition::getCardSuit(Mat cardSymbol, bool isBlack, int blackThreshold, double redTolerance)
{
	char suit = 0;
	threshold(cardSymbol, cardSymbol, 125, 255, THRESH_BINARY);
	if (isBlack)
	{
		//if contour area is grater than threshold, its C, otherwise its S
		int sum = 0;
		int k = 0;
		for (int i = 0; i < cardSymbol.cols; i++)
		{
			for (int j = 0; j < cardSymbol.rows / 5; j++)
			{
				if (cardSymbol.at<uchar>(j, i) == 0) sum++;
			}
		}
		if (sum > blackThreshold) suit = 'C';
		else suit = 'S';
	}
	else
	{
		//if zero pixels in upper half are more or less the same as in the lower half - its D, otherwise H
		int sum1 = 0, sum2 = 0;
		for (int i = 0; i < cardSymbol.cols; i++)
		{
			for (int j = 0; j < cardSymbol.rows / 2; j++)
			{
				if (cardSymbol.at<uchar>(j, i) == 0) sum1++;
			}
		}
		for (int i = 0; i < cardSymbol.cols; i++)
		{
			for (int j = cardSymbol.rows / 2; j < cardSymbol.rows; j++)
			{
				if (cardSymbol.at<uchar>(j, i) == 0) sum2++;
			}
		}
		if (sum2 > (1 - redTolerance) * sum1 and sum2 < (1 + redTolerance) * sum1) suit = 'D';
		else suit = 'H';
	}

	return suit;
}

int cards_recognition::getMaximumContour(std::vector<std::vector<Point>> contours)
{
	int maxContourNumber = contours.size() / 2 + contours.size() % 2;
	for (int k = maxContourNumber; k <= contours.size() - 2; k++)
	{
		if (contours[k].size() > contours[maxContourNumber].size()) maxContourNumber = k;
	}
	return maxContourNumber;
}