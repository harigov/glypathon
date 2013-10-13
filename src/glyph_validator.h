#pragma once

#include <map>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"

#include "glyph.h"

class GlyphValidator
{
  public:
    GlyphValidator(const std::string& filename);
    ~GlyphValidator();

    bool Validate(cv::Mat image, const std::vector<cv::Point2f>& detectedPts);

  private:
    std::map<std::string, Glyph*> glyphs_;

    std::string GetGlyphName(const Glyph& glyph);
    char IdentifyCellColor(cv::Mat image, cv::Mat H, size_t r, size_t c, cv::Mat* map_image);
    cv::Point3f Transform(cv::Point3f& pt, cv::Mat H);
};

