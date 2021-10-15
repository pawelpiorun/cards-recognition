# cards
Cards recognition using OpenCV (C++).

## Prerequisites
- OpenCV (at the moment of writing it's v4.5.3 https://opencv.org/opencv-4-5-3/)
- vcpkg (setup tutorial: https://vcpkg.io/en/getting-started.html)

When in vcpkg directory:
- .\vcpkg integrate install
- .\vcpkg install opencv opencv:x64-windows-static 
 
(or any other target that you'd like, just remember to update the project's platform target as well)

## Old project from studies - forgive me for the code :)
It was fun, I've copy pasted the code here without any refactoring (though I've been through a huge reform since then).
CardsRecognitionStatic was tested on images from 'sampleimg' folder and it worked perfectly.
I hope that someone can find some of the algorithms inside helpful some day!

TODO:
- Lots of refactoring!
- Getting live version to work properly, on not so specific images.
