#include <fstream>
#include <vector>

#include "glyph_validator.h"

#include "opencv2/opencv.hpp"

using namespace std;

// Specifies the width and height of the model coordinate space,
// to which glyph will be mapped to identify its schema.
static const size_t MODEL_SIZE = 100;
static const size_t GLYPH_SIZE = 5;

GlyphValidator::GlyphValidator(const std::string& filename)
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

GlyphValidator::~GlyphValidator()
{
  std::map<std::string, Glyph*>::iterator it = glyphs_.begin();
  while (it != glyphs_.end())
  {
    delete it->second;
  }
  glyphs_.clear();
}

bool GlyphValidator::Validate(cv::Mat image, const vector<cv::Point2f>& detectedPts)
{
  if (detectedPts.size() != 4)
  {
    // TODO: raise an exception.
  }

  vector<cv::Point2f> modelPts;
  modelPts.push_back(cv::Point2f(0.0f, 0.0f));
  modelPts.push_back(cv::Point2f(MODEL_SIZE, 0.0f));
  modelPts.push_back(cv::Point2f(MODEL_SIZE, MODEL_SIZE));
  modelPts.push_back(cv::Point2f(0.0f, MODEL_SIZE));
  cv::Mat H = cv::findHomography(modelPts, detectedPts);

  string glyph_schema;
  for (size_t r = 0; r < GLYPH_SIZE; ++r)
  {
    for (size_t c = 0; c < GLYPH_SIZE; ++c)
    {
      glyph_schema += IdentifyCellColor(image, H, r, c);
    }
  }

  return false;
}

std::string GlyphValidator::GetGlyphName(const Glyph& glyph)
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

char GlyphValidator::IdentifyCellColor(cv::Mat image, cv::Mat H, size_t r, size_t c)
{
  // Unless a cell is overwhelmingly of a particular color, we should not 
  // classify it to be one. This variable defines when a cell is 'overwhelmingly' 
  // of a particular color.
  float count_threshold = 0.8f;
  uint8_t color_threshold = 128;
  size_t cell_size = MODEL_SIZE / GLYPH_SIZE;
  int b_counter = 0, w_counter = 0;
  uint8_t* pixel_buf = image.ptr<uint8_t>();
  for (size_t y = 0; y < cell_size; ++y)
  {
    for (size_t x = 0; x < cell_size; ++x)
    {
      cv::Point3f model_pt(c * cell_size + x, r * cell_size + y, 1);
      cv::Point3f dest_pt = Transform(model_pt, H);
      if (pixel_buf[int(dest_pt.y) * image.cols + int(dest_pt.x)] < color_threshold)
      {
        ++b_counter;
      }
      else
      {
        ++w_counter;
      }
    }
  }
  // Ratio of number of black colored pixels should be greater than the
  // threshold defined by count_threshold for it be considered a black
  // cell. Vice versa for white cell.
  float b_ratio = b_counter / float(b_counter + w_counter);
  return (b_ratio >= count_threshold) ? 'b' 
    : (b_ratio <= (1 - count_threshold) ? 'w' : 'u');
}

cv::Point3f GlyphValidator::Transform(cv::Point3f& pt, cv::Mat H)
{
  vector<cv::Point3f> src_pts;
  src_pts.push_back(pt);
  vector<cv::Point3f> dest_pts;
  dest_pts.push_back(cv::Point3f());
  cv::transform(src_pts, dest_pts, H);
  return dest_pts[0];
}

