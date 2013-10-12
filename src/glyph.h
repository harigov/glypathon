#pragma once

#include <iostream>
#include <map>
#include <string>

#include "opencv2/opencv.hpp"

class Glyph
{
  public:
    Glyph(const std::string& glyph_schema);
    ~Glyph();

    bool operator==(const Glyph& glyph) const;

    int Id() const;
    double Angle() const;
    cv::Point2d Center() const;

  private:
    size_t size_;
    // true represents a cell with black color - false represents white
    // color cell.
    bool* schema_;
};

class GlyphDB
{
  public:
    GlyphDB(const std::string& filename);
    ~GlyphDB();

    std::string GetGlyphName(const Glyph& glyph);

  private:
    std::map<std::string, Glyph*> glyphs_;
};

