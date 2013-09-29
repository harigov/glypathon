#include "blob_detector.h"

#include <queue>

using namespace cv;
using namespace std;

BlobDetector::BlobDetector()
{
}

BlobDetector::~BlobDetector()
{
}

void BlobDetector::DetectBlobs(cv::Mat frame, bool debug = false)
{
  labeled_ = Mat(frame.rows, frame.cols, CV_16SC1);
  labeled_.setTo(Scalar(-2));

  const short target = -1;
  for (int y = 0; y < frame.rows; ++y) {
    for (int x = 0; x < frame.cols; ++x) {
      if (frame.at<unsigned char>(y, x) == 0) {
        labeled_.at<short>(y, x) = target;
      }
    }
  }

  int currentLabel = 0;
  for (int y = 0; y < labeled_.rows; ++y) {
    for (int x = 0; x < labeled_.cols; ++x) {
      if (labeled_.at<short>(y, x) == target) {
        BlobInfo info;
        FloodFill(Point2d(x, y), target, currentLabel++,
                  &info.origin, &info.size);

        blobs_.push_back(info);
      }
    }
  }

  if (debug) {
    Mat debug = Mat(frame.rows, frame.cols, CV_8UC3);
    debug.setTo(Scalar(0, 0, 0));
    const int colorOffset = 30;
    const int step = 0xffffff / (currentLabel + colorOffset);
    for (int y = 0; y < labeled_.rows; ++y) {
      for (int x = 0; x < labeled_.cols; ++x) {
        const short value = labeled_.at<short>(y, x);
        if (value >= 0) {
          if (IsValid(blobs_[value], frame)) {
            const int i = (value + colorOffset) * step;
            Vec3b c(i & 0xff, i >> 8 & 0xff, i >> 12 & 0xff);
            debug.at<Vec3b>(y, x) = c;
          }
        }
      }
    }

    cout << "Blobs detected: " << blobs_.size() << endl;
    imshow("labeled", debug);
  }
}

void BlobDetector::FloodFill(Point2d node, short target, short replacement,
                             cv::Point2d* origin, int* size)
{
  queue<Point2d> q;
  q.push(node);

  *origin = node;
  *size = 0;

  while (!q.empty()) {
    Point2d n = q.front();
    q.pop();

    if (Compare(n, *origin) == -1) {
      *origin = n;
    }

    if (n.x >= 0 && n.x < labeled_.cols && n.y >= 0 && n.y < labeled_.rows &&
        labeled_.at<short>(n.y, n.x) == target) {
      labeled_.at<short>(n.y, n.x) = replacement;
      q.push(Point2d(n.x - 1, n.y));
      q.push(Point2d(n.x + 1, n.y));
      q.push(Point2d(n.x, n.y - 1));
      q.push(Point2d(n.x, n.y + 1));
      ++*size;
    }
  }
}

void BlobDetector::FindVertices(Point2d origin, short target, int delta,
                              vector<Point2d>* vertices)
{
  // TODO: Continue here.
  (void) origin;
  (void) target;
  (void) delta;
  (void) vertices;
}

Point2d BlobDetector::StepOnPerimeter(Point2d origin, short target)
{
  (void) target;
  Point2d steps[8];
  steps[0] = Point2d(origin.x + 0, origin.y - 1);
  steps[1] = Point2d(origin.x + 1, origin.y - 1);
  steps[2] = Point2d(origin.x + 1, origin.y + 0);
  steps[3] = Point2d(origin.x + 1, origin.y + 1);
  steps[4] = Point2d(origin.x + 0, origin.y + 1);
  steps[5] = Point2d(origin.x - 1, origin.y + 1);
  steps[6] = Point2d(origin.x - 1, origin.y + 0);
  steps[7] = Point2d(origin.x - 1, origin.y - 1);

  for (int i = 0; i < 8; ++i) {
    Point2d s = steps[i];
    if (s.x < 0 || s.x >= labeled_.cols || s.y < 0 || s.y >= labeled_.rows) {
      continue;
    }
    // TODO: Continue here.
  }
  return Point2d();
}

bool IsValid(BlobInfo& blobInfo, Mat frame)
{
  const int minBlobSize = 16 * 16;
  const int maxBlobSize = frame.rows * frame.cols / (8 * 8);

  return blobInfo.size >= minBlobSize && blobInfo.size <= maxBlobSize;
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
