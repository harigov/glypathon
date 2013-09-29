#include <iostream>

#include "opencv2/opencv.hpp"

#include "glyph_detector.h"
#include "glyph_validator.h"

using namespace cv;
using namespace std;

int main()
{
  VideoCapture vc(CV_CAP_ANY);
  if (!vc.isOpened()) {
    cout << "Unable to open camera, exiting..." << endl;
    return 1;
  }

  bool quit = false;

  while (!quit)
  {
    Mat frame;
    vc >> frame;

    imshow("input", frame);
  }

  return 0;
}
