#include <iostream>

#include "blob_detector.h"
#include "glyph_detector.h"

using namespace cv;
using namespace std;

GlyphDetector::GlyphDetector()
{
}

GlyphDetector::~GlyphDetector()
{
}

bool GlyphDetector::DetectGlyph(const Mat gray)
{
  Mat blur;
  const Size kernelSize(3, 3);
  cv::blur(gray, blur, kernelSize);

  Mat canny;
  const int lowThreshold = 10;
  const int highThreshold = 100;
  const int kernelSizeCanny = 3;
  Canny(blur, canny, lowThreshold, highThreshold, kernelSizeCanny);

  BlobDetector blobDetector;
  blobDetector.DetectBlobs(canny, true);

  namedWindow("edges");
  moveWindow("edges", 640, 0);
  imshow("edges", canny);

  return true;
}

