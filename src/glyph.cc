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

bool Glyph::operator==(const Glyph& glyph)
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

GlyphDB::GlyphDB(const std::string& filename)
{
  // Expects a file with the following format
  //
  //    glyph_name=glyph_schema
  //
  //  schema is specified as a sequence of b/w chars
  //  b represents that a particular cell is black
  //  w represents that a particular cell is white
  //
  // It always starts at the top-left corner of the glyph.
  std::ifstream ifile(filename.c_str());
  for (std::string line; getline(ifile, line); )
  {
    int idx = line.find_first_of("=");
    std::string glyph_name = line.substr(0, idx);
    std::string glyph_schema = line.substr(idx + 1, line.size());
    glyphs_[glyph_name] = new Glyph(glyph_schema);
    std::cout << "Read " << glyph_name << std::endl;
  }
}

GlyphDB::~GlyphDB()
{
  std::map<std::string, Glyph*>::iterator it = glyphs_.begin();
  while (it != glyphs_.end())
  {
    delete it->second;
  }
  glyphs_.clear();
}

std::string GlyphDB::GetGlyphName(const Glyph& glyph)
{
  std::map<std::string, Glyph*>::iterator it = glyphs_.begin();
  while (it != glyphs_.end())
  {
    if (*it->second == glyph)
      return it->first;
  }
  // TODO: throw an exception, maybe?
  return "";
}

