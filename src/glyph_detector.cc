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
  BlobDetector blobDetector;
  blobDetector.Run(gray, true);

  return true;
}

