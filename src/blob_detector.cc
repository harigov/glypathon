#include "blob_detector.h"
#include "configuration.h"

#include <cassert>
#include <limits>
#include <queue>
#include <sstream>

using namespace cv;
using namespace std;

typedef unsigned char uchar;

BlobDetector::BlobDetector()
{
}

BlobDetector::~BlobDetector()
{
}

void BlobDetector::Run(const Mat grayscale, bool debug = false)
{
  Mat canny = DetectGradient(grayscale);

  // Padded frame with labels of connected components.
  labeled_ = Mat(grayscale.rows + 2, grayscale.cols + 2, CV_16SC1);
  labeled_.setTo(Scalar(-2));

  // Set as target all pixels not detected as gradient areas.
  const short target = -1;
  for (int y = 0; y < grayscale.rows; ++y) {
    for (int x = 0; x < grayscale.cols; ++x) {
      if (canny.at<unsigned char>(y, x) == 0) {
        labeled_.at<short>(y + 1, x + 1) = target;
      }
    }
  }

  const int min_blob_size =
     Configuration::Instance().ReadFloat("blob_min_norm_bbox_size") *
     grayscale.cols;

  const int max_blob_size =
     Configuration::Instance().ReadFloat("blob_max_norm_bbox_size") *
     grayscale.cols;

  // Label all target areas and extract information of the blob.
  int currentLabel = 0;
  for (int y = 0; y < labeled_.rows; ++y) {
    for (int x = 0; x < labeled_.cols; ++x) {
      if (labeled_.at<short>(y, x) == target) {
        BlobInfo info;
        FloodFill(Point2d(x, y), target, currentLabel++, &info);

        // Reject blobs based on size criteria.
        if (info.bbox.width > min_blob_size &&
            info.bbox.width < max_blob_size &&
            info.bbox.height > min_blob_size &&
            info.bbox.height < max_blob_size) {
          blobs_.push_back(info);
        }
      }
    }
  }

  CornerHarrisParams params;
  params.blockSize =
      Configuration::Instance().ReadInt("corner_harris_block_size");
  params.apertureSize =
      Configuration::Instance().ReadInt("corner_harris_aperture_size");
  params.freeCoefficient =
      Configuration::Instance().ReadDouble("corner_harris_free_coefficient");
  params.threshold =
      Configuration::Instance().ReadDouble("corner_harris_threshold");

  const float verticesMergingDistance =
      Configuration::Instance().ReadFloat("vertices_merging_distance");

  const int snapWindowSize =
      Configuration::Instance().ReadInt("snap_vertices_window_size");
  const float snapSearchFactor =
      Configuration::Instance().ReadFloat("snap_vertices_search_factor");

  // Approximate each blob to a polygon.
  for (int i = 0; i < blobs_.size(); ++i) {
    BlobInfo& info = blobs_[i];
    Mat filled = FillHoles(info);
    DetectVertices(filled, params, &info);
    if (info.vertices.size() > 1) {
      vector<Point2d> reducedVertices;
      ReduceVertices(info.vertices, &info.vertices, verticesMergingDistance);

      // Only snap vertices to the edges of the blob the polygon has 4 vertices.
      if (info.vertices.size() == 4) {
        SnapVerticesToEdgesOfConvexPolygon(filled, info, snapSearchFactor,
                                           snapWindowSize, &info.vertices);
        candidates_.push_back(i);
      }
    }
  }

  if (Configuration::Instance().ReadBool("display_blob_detection")) {
    Mat debug = Mat(labeled_.rows, labeled_.cols, CV_8UC3);
    debug.setTo(Scalar(0, 0, 0));

    // Use green channel for original frame.
    for (int y = 0; y < grayscale.rows; ++y) {
      for (int x = 0; x < grayscale.cols; ++x) {
        debug.at<Vec3b>(y + 1, x + 1) = Vec3b(0, grayscale.at<uchar>(y, x), 0);
      }
    }

    for (int i = 0; i < blobs_.size(); ++i) {
      const BlobInfo& info = blobs_[i];

      OverlayColor(Vec3b(0, 0, 200), info, &debug);

      if (Configuration::Instance().ReadBool("display_text")) {
        stringstream ss;
        ss << "[v: " << info.vertices.size() << "]";
        putText(debug, ss.str(), info.origin, FONT_HERSHEY_SIMPLEX, 0.35,
                Scalar(255, 255, 255), 1);
      }

      if (Configuration::Instance().ReadBool("display_vertices")) {
        for (int i = 0; i < info.vertices.size(); ++i) {
          circle(debug, info.vertices[i], 1, Scalar(0, 255, 255));
        }
      }

      if (Configuration::Instance().ReadBool("display_bounding_boxes")) {
        rectangle(debug, Point2d(info.bbox.x, info.bbox.y),
                  Point2d(info.bbox.x + info.bbox.width,
                          info.bbox.y + info.bbox.height),
                  Scalar(255, 0, 0));
      }
    }

    namedWindow("debug");
    moveWindow("debug", 0, 0);
    imshow("debug", debug);
  } else {
    destroyWindow("debug");
  }
}

int BlobDetector::GetCandidatesCount() const
{
  return candidates_.size();
}

const std::vector<cv::Point2d>& BlobDetector::GetVertices(const int index) const
{
  return blobs_[candidates_[index]].vertices;
}

Mat BlobDetector::DetectGradient(Mat grayscale)
{
  const int blurKernelSize =
      Configuration::Instance().ReadInt("canny_blur_kernel_size");

  Mat blur;
  cv::blur(grayscale, blur, Size(blurKernelSize, blurKernelSize));

  const int lowThreshold =
      Configuration::Instance().ReadInt("canny_low_threshold");
  const int highThreshold =
      Configuration::Instance().ReadInt("canny_high_threshold");
  const int kernelSizeCanny =
      Configuration::Instance().ReadInt("canny_kernel_size");

  Mat canny;
  Canny(blur, canny, lowThreshold, highThreshold, kernelSizeCanny);

  return canny;
}

void BlobDetector::ReduceVertices(const vector<Point2d>& vertices,
                                  vector<Point2d>* reducedVertices,
                                  const float mergingDistance)
{
  const float squaredMergingDistance = mergingDistance * mergingDistance;
  const int size = vertices.size();

  vector<int> labels;
  labels.resize(size);
  for (int i = 0; i < size; ++i) {
    labels[i] = i;
  }

  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      if (i == j) {
        continue;
      }

      if (SquaredDistance(vertices[i], vertices[j]) < squaredMergingDistance) {
        labels[i] = RootNode(labels, j);
      }
    }
  }

  map<int, vector<int>> clusters;
  for (int i = 0; i < size; ++i) {
    clusters[RootNode(labels, i)].push_back(i);
  }

  vector<Point2d> means;
  for (auto cluster : clusters) {
    Point2d acc(0, 0);

    for (auto idx : cluster.second) {
      acc += vertices[idx];
    }

    const int numItems = cluster.second.size();
    means.push_back(Point2d(acc.x / numItems, acc.y / numItems));
  }

  *reducedVertices = means;
}

int RootNode(const vector<int>& labels, int idx)
{
  while (labels[idx] != idx) {
    idx = labels[idx];
  }

  return idx;
}

void BlobDetector::OverlayColor(const Vec3b color, const BlobInfo& info,
                                Mat* bgr)
{
  // Lazy approach.
  int ox = info.bbox.x;
  int oy = info.bbox.y;
  for (int y = 0; y < info.bbox.height; ++y) {
    for (int x = 0; x < info.bbox.width; ++x) {
      if (labeled_.at<short>(oy + y, ox + x) == info.label) {
        bgr->at<Vec3b>(oy + y, ox + x) = color;
      }
    }
  }

}

void BlobDetector::FloodFill(Point2d node, short target, short replacement,
                             BlobInfo* info)
{
  queue<Point2d> q;
  q.push(node);

  // Left-Top and Right-Bottom points for the bounding box.
  Point2d lt(labeled_.cols, labeled_.rows);
  Point2d rb(0, 0);
  Point2d origin = node;
  int numPixels = 0;

  while (!q.empty()) {
    Point2d n = q.front();
    q.pop();

    if (labeled_.at<short>(n.y, n.x) == target) {
      // Update bounding box.
      lt.x = min(lt.x, n.x);
      lt.y = min(lt.y, n.y);
      rb.x = max(rb.x, n.x);
      rb.y = max(rb.y, n.y);

      // Update origin.
      if (Compare(n, origin) == -1) {
        origin = n;
      }

      labeled_.at<short>(n.y, n.x) = replacement;
      q.push(Point2d(n.x - 1, n.y));
      q.push(Point2d(n.x + 1, n.y));
      q.push(Point2d(n.x, n.y - 1));
      q.push(Point2d(n.x, n.y + 1));

      ++numPixels;
    }
  }

  info->numPixels = numPixels;
  info->bbox = Rect(lt.x, lt.y, rb.x - lt.x + 1, rb.y - lt.y + 1);
  info->origin = origin;
  info->label = replacement;
}

// Returns padded image with 0s for background and 1s for filled blob.
Mat BlobDetector::FillHoles(const BlobInfo& info)
{
  const Rect bbox = info.bbox;

  // There are internal colors; output colors are 0s and 1s.
  const uchar backgroundColor = 255;
  const uchar blobColor = 0;
  const uchar replacementColor = 127;

  Mat filled(bbox.height + 2, bbox.width + 2, CV_8UC1);
  filled.setTo(Scalar(backgroundColor));

  int checkSum = 0;  // For sanity check.
  for (int y = 0; y < bbox.height; ++y) {
    for (int x = 0; x < bbox.width; ++x) {
      short value = labeled_.at<short>(bbox.y + y, bbox.x + x);
      if (value == info.label) {
        filled.at<uchar>(y + 1, x + 1) = blobColor;
        ++checkSum;
      }
    }
  }

  assert(checkSum == info.numPixels);

  queue<Point2d> q;
  q.push(Point2d(0, 0));  // Start at top-left background pixel.

  while (!q.empty()) {
    Point2d n = q.front();
    q.pop();

    if (n.x >= 0 && n.x < filled.cols &&
        n.y >= 0 && n.y < filled.rows &&
        filled.at<uchar>(n.y, n.x) == backgroundColor) {
      filled.at<uchar>(n.y, n.x) = replacementColor;
      q.push(Point2d(n.x - 1, n.y));
      q.push(Point2d(n.x + 1, n.y));
      q.push(Point2d(n.x, n.y - 1));
      q.push(Point2d(n.x, n.y + 1));
    }
  }

  for (int y = 0; y < filled.rows; ++y) {
    for (int x = 0; x < filled.cols; ++x) {
        filled.at<uchar>(y, x) =
            filled.at<uchar>(y, x) == replacementColor ? 0 : 1;
    }
  }

  return filled;
}

void BlobDetector::DetectVertices(const Mat& blob,
    const CornerHarrisParams& params, BlobInfo* info)
{
  Mat harris(blob.size(), CV_32FC1);
  cornerHarris(blob, harris, params.blockSize, params.apertureSize,
               params.freeCoefficient, BORDER_REPLICATE);
  Mat norm, scaled;
  normalize(harris, norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
  convertScaleAbs(norm, scaled);

  const Point2d offset(info->bbox.x - 1, info->bbox.y - 1);
  vector<Point2d> edges;
  // The image is padded, skip the padding.
  for (int y = 1; y < scaled.rows - 1; ++y) {
    for (int x = 1; x < scaled.cols - 1; ++x) {
      if (scaled.at<uchar>(y, x) > params.threshold) {
        Point2d p(x, y);
        edges.push_back(p);
        info->vertices.push_back(p + offset);
      }
    }
  }
}

void BlobDetector::SnapVerticesToEdgesOfConvexPolygon(
    const Mat& blob,
    const BlobInfo& info,
    const float snapSearchFactor,
    const int windowSize,
    vector<Point2d>* vertices)
{
  const int halfWindowSize = windowSize >> 1;
  const int halfSearchSize = (max(blob.rows, blob.cols) * snapSearchFactor) / 2;
  const Point2d offset(info.bbox.x - 1, info.bbox.y - 1);

  for (auto& vertice : *vertices)
  {
    const int xini =
      max(static_cast<int>(vertice.x - offset.x - halfSearchSize), 0);
    const int xend =
      min(static_cast<int>(vertice.x - offset.x + halfSearchSize), blob.cols);
    const int yini =
      max(static_cast<int>(vertice.y - offset.y - halfSearchSize), 0);
    const int yend =
      min(static_cast<int>(vertice.y - offset.y + halfSearchSize), blob.rows);

    Point2d best = vertice;
    int minSum = numeric_limits<int>::max();

    for (int y = yini; y < yend; ++y) {
      for (int x = xini; x < xend; ++x) {
        if (blob.at<uchar>(y, x) == 1) {
          int sum = SumBlock(blob, x, y, halfWindowSize, minSum);
          if (sum < minSum) {
            vertice = Point2d(x, y) + offset;
            minSum = sum;
          }
        }
      }
    }
  }
}

int BlobDetector::SumBlock(const Mat img, const int x, const int y,
                           const int halfWindowSize,
                           const int earlyTerminationSum)
{
  const int xini = max(x - halfWindowSize, 0);
  const int xend = min(x + halfWindowSize, img.cols);
  const int yini = max(y - halfWindowSize, 0);
  const int yend = min(y + halfWindowSize, img.rows);

  int acc = 0;
  for (int y = yini; y < yend; ++y) {
    for (int x = xini; x < xend; ++x) {
      acc += img.at<uchar>(y, x);
      if (acc >= earlyTerminationSum) {
        return numeric_limits<int>::max();
      }
    }
  }

  return acc;
}


float SquaredDistance(const Point2d& lhs, const Point2d& rhs)
{
  return (lhs.x - rhs.x) * (lhs.x - rhs.x) + (lhs.y - rhs.y) * (lhs.y - rhs.y);
}

int Compare(const Point2d& lhs, const Point2d& rhs)
{
  if (lhs.y < rhs.y) {
    return -1;
  } else if (lhs.y > rhs.y) {
    return 1;
  }

  if (lhs.x < rhs.x) {
    return -1;
  } else if (lhs.x > rhs.x) {
    return 1;
  }

  return 0;
}
