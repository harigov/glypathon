#pragma once

#include "opencv2/opencv.hpp"

class GlyphDetector
{
  public:
  GlyphDetector();
  ~GlyphDetector();

  bool DetectGlyph(const cv::Mat gray);
};

