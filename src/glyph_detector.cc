#include "glyph_detector.h"

#include <exception>
#include <iostream>

#include "blob_detector.h"
#include "configuration.h"

using namespace cv;
using namespace std;

GlyphDetector::GlyphDetector(string filename)
    : videoCapture_(CV_CAP_ANY)  // It has to be opened from the main thread.
    , quit_(false)
    , glyphValidator_(filename)
{
  if (!videoCapture_.isOpened()) {
    throw "Unable to open camera";
  }

  thread_ = thread(Worker, this);
}

GlyphDetector::~GlyphDetector()
{
}

void GlyphDetector::Stop()
{
  quit_ = true;
  thread_.join();
}

bool GlyphDetector::GetGlyphs(vector<Glyph>* glyphs)
{
  lock_guard<mutex> lock(mutex_);

  if (glyphs_.empty()) {
    return false;
  }

  glyphs->clear();
  glyphs->reserve(glyphs_.size());
  for (int i = 0; i < glyphs_.size(); ++i) {
    glyphs->push_back(glyphs_[i]);
  }

  return true;
}

void GlyphDetector::Worker(GlyphDetector* instance)
{
  BlobDetector blobDetector;
  Mat frame, gray;

  while (!instance->quit_) {
    instance->videoCapture_ >> frame;

    const float factor =
      Configuration::Instance().ReadFloat("frame_resize_factor");

    if (factor != 1.0f) {
      resize(frame, frame, Size(frame.cols * factor, frame.rows * factor));
    }

    if(Configuration::Instance().ReadBool("display_input_frame")) {
      namedWindow("input");
      moveWindow("input", 0, 0);
      imshow("input", frame);
    }

    cvtColor(frame, gray, CV_BGR2GRAY);

    blobDetector.Run(gray);
    int regionCount = blobDetector.GetCandidatesCount();
    while (regionCount-- > 0) {
      std::vector<cv::Point2f> vertices = blobDetector.GetVertices(regionCount);
      if (!instance->glyphValidator_.Validate(frame, vertices)) {
        continue;
      }
    }
  }
}

