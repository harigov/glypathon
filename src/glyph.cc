#include "glyph.h"

#include <cmath>
#include <exception>
#include <fstream>

Glyph::Glyph(const std::string& glyph_schema)
{
  size_t schema_len = glyph_schema.size();
  size_t side_len = sqrt(schema_len);
  if (side_len * side_len != schema_len)
  {
    /// throw new std::exception("Schema doesn't define a square glyph");
  }
  size_ = side_len;
  schema_ = new bool[size_ * size_];
  for (size_t i = 0; i < glyph_schema.size(); ++i)
  {
    schema_[i] = glyph_schema[i] == 'b' ? true : false;
  }
}

Glyph::~Glyph()
{
  delete[] schema_;
}

bool Glyph::operator==(const Glyph& glyph) const
{
  // TODO: add support for matching glyphs not
  // oriented in the same way.
  if (glyph.size_ == size_)
  {
    for(size_t i = 0; i < size_; ++i)
    {
      if (schema_[i] != glyph.schema_[i])
        return false;
    }
    return true;
  }
  return false;
}

int Glyph::Id() const
{
  return -1;
}

double Glyph::Angle() const
{
  return 0.0;
}

cv::Point2d Glyph::Center() const
{
  return cv::Point2d(300, 300);
}

