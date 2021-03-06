#pragma once

#include <mutex>
#include <thread>

#include "opencv2/opencv.hpp"

#include "glyph.h"
#include "glyph_validator.h"

class GlyphDetector
{
 public:
  GlyphDetector(std::string filename);
  ~GlyphDetector();

  void Stop();
  bool GetGlyphs(std::vector<Glyph>* glyphs);

 private:
  static void Worker(GlyphDetector* instance);

  cv::VideoCapture videoCapture_;
  bool quit_;
  std::thread thread_;
  std::mutex mutex_;
  std::vector<Glyph> glyphs_;
  GlyphValidator glyphValidator_;
};

