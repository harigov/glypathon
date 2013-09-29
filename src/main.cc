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

  GlyphDetector detector;

  while (!quit)
  {
    Mat frame;
    vc >> frame;
    resize(frame, frame, Size(frame.cols / 2, frame.rows / 2));

    Mat gray;
    cvtColor(frame, gray, CV_BGR2GRAY);

    detector.DetectGlyph(gray);

    namedWindow("input");
    moveWindow("input", 0, 0);
    imshow("input", frame);
    waitKey(40);
  }

  return 0;
}
