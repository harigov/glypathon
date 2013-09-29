#pragma once

#include "opencv2/opencv.hpp"

struct BlobInfo
{
  cv::Point2d origin;
  int size;
  std::vector<cv::Point2d> vertices;
};

class BlobDetector
{
 public:
  BlobDetector();
  ~BlobDetector();

  void DetectBlobs(cv::Mat frame, bool debug);

 private:
  cv::Mat labeled_;
  std::vector<BlobInfo> blobs_;

  void FloodFill(cv::Point2d node, short target, short replacement,
                 cv::Point2d* origin, int* size);
  void FindVertices(cv::Point2d origin, short target, int delta,
                    std::vector<cv::Point2d>* vertices);
  cv::Point2d StepOnPerimeter(cv::Point2d origin, short target);
};

int Compare(cv::Point2d lhs, cv::Point2d rhs);

// For debug porpuses mainly.
bool IsValid(BlobInfo& blobInfo, cv::Mat frame);
