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
  // add 2 cols and 2 rows to void handling corner cases.
  // find connected components.
  // represent with 0s and 1s connected component into a padded frame.
  // keep track of every blob (origin, label).
  // extract information of every blob:
  //   - outside perimeter.
  //   - number of vertices.
  //   - size.
  labeled_ = Mat(frame.rows + 2, frame.cols + 2, CV_16SC1);
  labeled_.setTo(Scalar(-2));

  const short target = -1;
  for (int y = 0; y < frame.rows; ++y) {
    for (int x = 0; x < frame.cols; ++x) {
      if (frame.at<unsigned char>(y, x) == 0) {
        labeled_.at<short>(y + 1, x + 1) = target;
      }
    }
  }

  int currentLabel = 0;
  for (int y = 0; y < labeled_.rows; ++y) {
    for (int x = 0; x < labeled_.cols; ++x) {
      if (labeled_.at<short>(y, x) == target) {
        BlobInfo info;
        FloodFill(Point2d(x, y), target, currentLabel, &info);

       // if (IsValid(info, frame)) {
       //   FindVertices(info.origin, currentLabel, 10, &info.vertices);
       // }

        ++currentLabel;
        blobs_.push_back(info);
      }
    }
  }

  if (debug) {
    Mat debug = Mat(labeled_.rows, labeled_.cols, CV_8UC3);
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
    namedWindow("labeled");
    moveWindow("labeled", 0, 480);
    imshow("labeled", debug);
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
  int numPixels = 0;

  while (!q.empty()) {
    Point2d n = q.front();
    q.pop();

    if (labeled_.at<short>(n.y, n.x) == target) {
      lt.x = min(lt.x, n.x);
      lt.y = min(lt.y, n.y);
      rb.x = max(lt.x, n.x);
      rb.y = max(lt.y, n.y);

      labeled_.at<short>(n.y, n.x) = replacement;
      q.push(Point2d(n.x - 1, n.y));
      q.push(Point2d(n.x + 1, n.y));
      q.push(Point2d(n.x, n.y - 1));
      q.push(Point2d(n.x, n.y + 1));
      ++numPixels;
    }
  }

  info->numPixels = numPixels;
  info->bbox = Rect(lt.x, lt.y, rb.x - lt.x, rb.y - lt.y);
}

void BlobDetector::FindVertices(Point2d origin, short target, int delta,
                              vector<Point2d>* vertices)
{
  // TODO: Continue here.
  (void) origin;
  (void) target;
  (void) delta;
  (void) vertices;

  Point2d current = origin;

  int steps = 0;
  int move = 0;
  while (true) {
    Point2d next = StepOnPerimeter(current, target, &move);
    current = next;
    ++steps;

    if (next == origin) {
      break;
    }
  }
  cout << steps << endl;
}

// NOTE: This algorithm will fail if blobs are one pixel thick.
Point2d BlobDetector::StepOnPerimeter(Point2d origin, short target, int* move)
{
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
    *move = (i + *move) % 8;
    Point2d s = steps[*move];
    if (s.x < 0 || s.x >= labeled_.cols || s.y < 0 || s.y >= labeled_.rows) {
      continue;
    }

    if (labeled_.at<short>(s.y, s.x) != target) {
      continue;
    }

    Point2d checks[4];
    checks[0] = Point2d(s.x + 0, s.y - 1);
    checks[1] = Point2d(s.x + 1, s.y + 0);
    checks[2] = Point2d(s.x + 0, s.y + 1);
    checks[3] = Point2d(s.x - 0, s.y + 0);

    for (int k = 0; k < 4; ++k) {
      Point2d c = checks[k];
      if (c.x < 0 || c.x >= labeled_.cols || c.y < 0 || c.y >= labeled_.rows) {
        return s;
      }

      if (labeled_.at<short>(c.y, c.x) != target) {
        return s;
      }
    }
  }

  return origin;
}

bool IsValid(BlobInfo& blobInfo, Mat frame)
{
  const int minBlobSize = 16 * 16;
  const int maxBlobSize = frame.rows * frame.cols / (8 * 8);

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
