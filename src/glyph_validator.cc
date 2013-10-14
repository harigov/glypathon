#include <fstream>
#include <vector>

#include "glyph_validator.h"

#include "opencv2/opencv.hpp"

using namespace std;

// Specifies the width and height of the model coordinate space,
// to which glyph will be mapped to identify its schema.
static const size_t MODEL_SIZE = 100;
static const size_t GLYPH_SIZE = 5;

GlyphValidator::GlyphValidator(std::string filename)
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
  if (detectedPts.size() != 4 || !AreValidPoints(image, detectedPts))
  {
    // TODO: raise an exception.
    return false;
  }

  vector<cv::Point2f> modelPts;
  modelPts.push_back(cv::Point2f(0.0f, 0.0f));
  modelPts.push_back(cv::Point2f(MODEL_SIZE, 0.0f));
  modelPts.push_back(cv::Point2f(MODEL_SIZE, MODEL_SIZE));
  modelPts.push_back(cv::Point2f(0.0f, MODEL_SIZE));

  vector<cv::Point2f> reorderPts = ReorderPoints(detectedPts);
  cv::Mat H = cv::findHomography(modelPts, reorderPts);

  cv::Mat mapImg(cv::Size(MODEL_SIZE, MODEL_SIZE), CV_8UC1);

  string glyph_schema;
  for (size_t r = 0; r < GLYPH_SIZE; ++r)
  {
    for (size_t c = 0; c < GLYPH_SIZE; ++c)
    {
      char color = IdentifyCellColor(image, H, r, c, &mapImg);
      if (color == 'b' || color == 'w')
      {
        glyph_schema.push_back(color);
      }
      else
      {
        return false;
      }
    }
  }

  cv::imshow("debug", mapImg);
  cout << "Schema is " << glyph_schema << endl;
  return true;
}

bool GlyphValidator::AreValidPoints(cv::Mat image, const vector<cv::Point2f>& detectedPts)
{
  int min_size_factor = 6;
  int min_width = image.cols / min_size_factor, min_height = image.rows / min_size_factor;

  float minx = numeric_limits<float>::max(), miny = numeric_limits<float>::max(), 
        maxx = numeric_limits<float>::min(), maxy = numeric_limits<float>::min();
  for (size_t i = 0; i < detectedPts.size(); ++i)
  {
    if (detectedPts[i].x < minx) minx = detectedPts[i].x;
    if (detectedPts[i].y < miny) miny = detectedPts[i].y;
    if (detectedPts[i].x > maxx) maxx = detectedPts[i].x;
    if (detectedPts[i].y > maxy) maxy = detectedPts[i].y;
  }

  return (maxx - minx) > min_width && (maxy - miny) > min_height;
}

vector<cv::Point2f> GlyphValidator::ReorderPoints(const vector<cv::Point2f>& detectedPts)
{
  // Find clockwise orientation order of points starting with top-left corner.
  vector<cv::Point2f> reorderedPts;
  // Find the points that are closest and farthest from the origin. Those will
  // be the top-left and bottom-right points.
  cv::Point2f origin(0, 0);
  cv::Point2f top_left, top_right, bottom_right, bottom_left;
  double min_dist = std::numeric_limits<double>::max();
  double max_dist = std::numeric_limits<double>::min();
  float maxx = 0;
  for (size_t i = 0; i < detectedPts.size(); ++i)
  {
    double dist = cv::norm(detectedPts[i] - origin);
    if (dist < min_dist) 
    {
      top_left = detectedPts[i];
      min_dist = dist;
    }
    if (dist > max_dist) 
    {
      bottom_right = detectedPts[i];
      max_dist = dist;
    }
    if (detectedPts[i].x > maxx)
    {
      maxx = detectedPts[i].x;
    }
  }

  // Finding the closest point to the top-right corner of the image.
  // This gives top-right and bottom-right corner of the four points.
  cv::Point2f maxx_pt(maxx, 0);
  min_dist = std::numeric_limits<double>::max();
  max_dist = std::numeric_limits<double>::min();
  for (size_t i = 0; i < detectedPts.size(); ++i)
  {
    double dist = cv::norm(detectedPts[i] - maxx_pt);
    if (dist < min_dist) 
    {
      top_right = detectedPts[i];
      min_dist = dist;
    }
    if (dist > max_dist) 
    {
      bottom_left = detectedPts[i];
      max_dist = dist;
    }
  }
  reorderedPts.push_back(top_left);
  reorderedPts.push_back(top_right);
  reorderedPts.push_back(bottom_right);
  reorderedPts.push_back(bottom_left);
  return reorderedPts;
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

char GlyphValidator::IdentifyCellColor(cv::Mat image, cv::Mat H, size_t r, size_t c, cv::Mat* map_image)
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
      uint8_t pixel_color = image.at<uint8_t>(int(dest_pt.y / dest_pt.z), int(dest_pt.x / dest_pt.z));
      if (pixel_color < color_threshold)
      {
        ++b_counter;
      }
      else
      {
        ++w_counter;
      }
      if (map_image)
      {
        map_image->at<uint8_t>(model_pt.y, model_pt.x) = pixel_color;
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
  dest_pts.push_back(cv::Point3f(0, 0, 1));
  cv::perspectiveTransform(src_pts, dest_pts, H);
  return dest_pts[0];
}

void PrintPoints(vector<cv::Point2f> pts)
{
  cout << "Following points are in the list:" << endl;
  for (size_t i = 0; i < pts.size(); ++i)
  {
    cout << pts[i].x << ", " << pts[i].y << endl;
  }
}
