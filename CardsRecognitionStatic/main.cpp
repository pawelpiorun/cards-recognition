#include <iostream>
#include <opencv2/opencv.hpp>
#include "cards_recognition.h"

int main()
{
    std::cout << "Hello in cards recognizer!\n";
    std::cout << "OpenCV version: " + cv::getVersionString() << std::endl << std::endl;
    std::cout << "Specify image path:\n";

    std::string path;
    std::getline(std::cin, path);

    auto recognizer = new cards_recognition();
    auto cards = recognizer->run(path);

    if (cards.empty())
        std::cout << "No card detected.\n";
    else
    {
        for (int i = 0; i < cards.size(); i++)
        {
            auto card = cards[i];
            std::cout << card.value << card.suit << std::endl;
        }
    }

    delete recognizer;
}