#pragma once

#include <iostream>
#include <map>
#include <string>

class Glyph
{
  public:
    Glyph(const std::string& glyph_schema);
    ~Glyph();

    bool operator==(const Glyph& glyph);

  private:
    size_t size_;
    // true represents a cell with black color - false represents white
    // color cell.
    bool* schema_;

    // Hiding assignment/copying behaviors.
    Glyph(const Glyph&);
    Glyph& operator=(const Glyph&);
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

