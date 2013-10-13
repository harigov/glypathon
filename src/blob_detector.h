#pragma once

#include "opencv2/opencv.hpp"

struct BlobInfo
{
  cv::Point2f origin;
  cv::Rect bbox;
  int numPixels;
  short label;
  std::vector<cv::Point2f> vertices;
};

class BlobDetector
{
 public:
  BlobDetector();
  ~BlobDetector();

  void Run(const cv::Mat frame);
  int GetCandidatesCount() const;
  const std::vector<cv::Point2f>& GetVertices(const int index) const;

 private:
  struct CornerHarrisParams
  {
    int blockSize;
    int apertureSize;
    double freeCoefficient;
    int threshold;
  };

  cv::Mat labeled_;
  std::vector<BlobInfo> blobs_;
  std::vector<int> candidates_;

  cv::Mat DetectGradient(cv::Mat frame);
  void ReduceVertices(const std::vector<cv::Point2f>& vertices,
                      std::vector<cv::Point2f>* reducedVertices,
                      const float mergingDistance);
  void SnapVerticesToEdgesOfConvexPolygon(const cv::Mat& blob,
                                          const BlobInfo& info,
                                          const float snapSearchFactor,
                                          const int windowSize,
                                          std::vector<cv::Point2f>* vertices);
  void FloodFill(cv::Point2f node, short target, short replacement,
                 BlobInfo* info);
  cv::Mat FillHoles(const BlobInfo& info);
  void DetectVertices(const cv::Mat& blob, const CornerHarrisParams& params,
                      BlobInfo* info);
  int SumBlock(const cv::Mat img, const int x, const int y,
               const int halfWindowSize, const int earlyTerminationSum);
  int SumWindow(const cv::Mat blob, const cv::Point2f center, int window);
  void OverlayColor(const cv::Vec3b color, const BlobInfo& info, cv::Mat* bgr);
};

int Compare(const cv::Point2f& lhs, const cv::Point2f& rhs);
float SquaredDistance(const cv::Point2f& lhs, const cv::Point2f& rhs);
int RootNode(const std::vector<int>& labels, int idx);

// For debug porpuses mainly.
bool IsValid(BlobInfo& blobInfo, cv::Mat frame);
