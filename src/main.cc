#include <iostream>

#include "opencv2/opencv.hpp"

#include "configuration.h"
#include "glyph_detector.h"
#include "glyph_validator.h"

using namespace cv;
using namespace std;

int main()
{
  Configuration::Instance().Load("configuration.txt");

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

    const float factor =
      Configuration::Instance().ReadFloat("frame_resize_factor");

    if (factor != 1.0f) {
      resize(frame, frame, Size(frame.cols * factor, frame.rows * factor));
    }

    Mat gray;
    cvtColor(frame, gray, CV_BGR2GRAY);

    detector.DetectGlyph(gray);

    if(Configuration::Instance().ReadBool("display_input_frame")) {
      namedWindow("input");
      moveWindow("input", 0, 0);
      imshow("input", frame);
    }

    waitKey(5);
  }

  return 0;
}
