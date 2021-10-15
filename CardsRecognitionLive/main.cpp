#include "opencv2/opencv.hpp"
#include <iostream>
#include "../CardsRecognitionStatic/cards_recognition.h"

using namespace cv;

int main() {

    auto cap = VideoCapture();
    cap.open(0, cv::CAP_ANY);
    if (!cap.isOpened()) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }

    std::cout << "Press ESC to exit" << std::endl;
    Mat frame;
    while (1) {

        cap >> frame;
        if (frame.empty())
            continue;

        auto recognizer = new cards_recognition();
        auto cards = recognizer->run(frame);
        
        for (int i = 0; i < cards.size(); i++)
        {
            cv::rectangle(frame, cards[i].bbox, cv::Scalar(0, 0, 255));
        }

        imshow("Frame", frame);

        char c = (char)waitKey(25);
        if (c == 27)
            break;

        delete recognizer;
    }

    cap.release();
    destroyAllWindows();

    return 0;
}