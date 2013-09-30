#include "blob_detector.h"

#include <sstream>
#include <queue>
#include <cassert>

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
  namedWindow("filled");
  moveWindow("filled", 700, 650);

  Mat canny = DetectGradient(grayscale);

  // add 2 cols and 2 rows to void handling corner cases.
  // find connected components.
  // represent with 0s and 1s connected component into a padded frame.
  // keep track of every blob (origin, label).
  // extract information of every blob:
  //   - outside perimeter.
  //   - number of vertices.
  //   - size.
  //

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

  // Label all target areas and extract information of the blob.
  int currentLabel = 0;
  for (int y = 0; y < labeled_.rows; ++y) {
    for (int x = 0; x < labeled_.cols; ++x) {
      if (labeled_.at<short>(y, x) == target) {
        BlobInfo info;
        FloodFill(Point2d(x, y), target, currentLabel++, &info);
        blobs_.push_back(info);
      }
    }
  }

  // Approximate each blob to a polygon.
  for (int i = 0; i < blobs_.size(); ++i) {
    BlobInfo& info = blobs_[i];
    Mat filled = FillHoles(info);
    DetectVertices(filled, &info);
  }

  if (debug) {
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

      // Ignore very large blobs.
      if (info.numPixels > labeled_.rows * labeled_.cols / (4 * 4)) {
        continue;
      }

      OverlayColor(Vec3b(0, 0, 200), info, &debug);
      stringstream ss;
      ss << "[p: " << info.numPixels << ", l: " << info.label <<
            "v: " << info.vertices.size() << "]";
      putText(debug, ss.str(), info.origin, FONT_HERSHEY_SIMPLEX, 0.35,
              Scalar(255, 255, 255), 1);

      for (int i = 0; i < info.vertices.size(); ++i) {
        circle(debug, info.vertices[i], 1, Scalar(0, 255, 255));
      }

      rectangle(debug, Point2d(info.bbox.x, info.bbox.y),
                Point2d(info.bbox.x + info.bbox.width,
                        info.bbox.y + info.bbox.height),
                Scalar(255, 0, 0));
    }

    cout << "Blobs detected: " << blobs_.size() << endl;

    namedWindow("labeled");
    moveWindow("labeled", 0, 400);
    imshow("labeled", debug);
  }
}

Mat BlobDetector::DetectGradient(Mat grayscale)
{
  Mat blur;
  const Size kernelSize(3, 3);
  cv::blur(grayscale, blur, kernelSize);

  Mat canny;
  const int lowThreshold = 10;
  const int highThreshold = 100;
  const int kernelSizeCanny = 3;
  Canny(blur, canny, lowThreshold, highThreshold, kernelSizeCanny);

  return canny;
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

void BlobDetector::DetectVertices(const Mat& blob, BlobInfo* info)
{
  const int windowSize = 16;  // Search window width and height.

  vector<Point2d> edgePoints;
  // The image is padded, skip the padding.
  for (int y = 1; y < blob.rows - 1; ++y) {
    for (int x = 1; x < blob.cols - 1; ++x) {
      // Edge is a white pixel (1) with at least one adjacent black pixel (0).
      if (blob.at<uchar>(y, x) == 1 &&
          (blob.at<uchar>(y - 1, x + 0) == 0 ||
           blob.at<uchar>(y + 0, x + 1) == 0 ||
           blob.at<uchar>(y + 1, x + 0) == 0 ||
           blob.at<uchar>(y + 0, x - 1) == 0)) {
        edgePoints.push_back(Point2d(x, y));
      }
    }
  }

  assert(!edgePoints.empty());

  // This part is not giving good results.
  const int idealValue = windowSize * windowSize / 2;
  const int tolerance = windowSize * windowSize / 6;
  const int lowThreshold = idealValue - tolerance;
  const int highThreshold = idealValue + tolerance;

  for (int i = 0; i < edgePoints.size(); ++i) {
    int sum = SumWindow(blob, edgePoints[i], windowSize);
    if (sum < lowThreshold || sum > highThreshold) {
      info->vertices.push_back(edgePoints[i] +
                               Point2d(info->bbox.x, info->bbox.y));
    }
  }
}

int BlobDetector::SumWindow(const Mat blob, const Point2d center, int window)
{
  const int width = blob.cols;
  const int height = blob.rows;
  const int halfWindow = window >> 1;

  int sum = 0;
  for (int y = -halfWindow; y < halfWindow; ++y) {
    for (int x = -halfWindow; x < halfWindow; ++x) {
      int xx = center.x + x;
      int yy = center.y + y;

      if (yy < 0 || yy >= height || xx < 0 || xx >= width) {
        continue;
      }

      sum += blob.at<uchar>(yy, xx);
    }
  }

  return sum;
}

bool IsValid(BlobInfo& blobInfo, Mat frame)
{
  const int minBlobSize = 16 * 16;
  const int maxBlobSize = frame.rows * frame.cols / (3 * 3);

  return blobInfo.numPixels <= maxBlobSize;

  return blobInfo.numPixels >= minBlobSize && blobInfo.numPixels <= maxBlobSize;
}

int Compare(cv::Point2d lhs, cv::Point2d rhs)
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
