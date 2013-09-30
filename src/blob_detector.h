#pragma once

#include "opencv2/opencv.hpp"

struct BlobInfo
{
  cv::Point2d origin;
  cv::Rect bbox;
  int numPixels;
  short label;
  std::vector<cv::Point2d> vertices;
};

class BlobDetector
{
 public:
  BlobDetector();
  ~BlobDetector();

  void Run(const cv::Mat frame, bool debug);

 private:
  cv::Mat labeled_;
  std::vector<BlobInfo> blobs_;

  cv::Mat DetectGradient(cv::Mat frame);
  void FloodFill(cv::Point2d node, short target, short replacement,
                 BlobInfo* info);
  cv::Mat FillHoles(const BlobInfo& info);
  void DetectVertices(const cv::Mat& blob, BlobInfo* info);
  int SumWindow(const cv::Mat blob, const cv::Point2d center, int window);
  void OverlayColor(const cv::Vec3b color, const BlobInfo& info, cv::Mat* bgr);
};

int Compare(cv::Point2d lhs, cv::Point2d rhs);

// For debug porpuses mainly.
bool IsValid(BlobInfo& blobInfo, cv::Mat frame);
