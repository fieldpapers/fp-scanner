#include <stdio.h>
#include <iostream>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"

using namespace cv;
using namespace std;

void readme();

/**
 * Adapted from:
 *  http://docs.opencv.org/doc/tutorials/features2d/feature_homography/feature_homography.html
 */
int main(int argc, char** argv) {
  if (argc != 4) {
    readme();
    return -1;
  }

  Mat img_object = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
  Mat img_scene = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

  if (!img_object.data || !img_scene.data) {
    cout << " --(!) Error reading images " << endl;
    return -1;
  }

  //-- Step 1: Detect the keypoints using SURF Detector
  int minHessian = 400;

  SurfFeatureDetector detector(minHessian);

  vector<KeyPoint> keypoints_object, keypoints_scene;

  detector.detect(img_object, keypoints_object);
  detector.detect(img_scene, keypoints_scene);

  //-- Step 2: Calculate descriptors (feature vectors)
  SurfDescriptorExtractor extractor;

  Mat descriptors_object, descriptors_scene;

  extractor.compute(img_object, keypoints_object, descriptors_object);
  extractor.compute(img_scene, keypoints_scene, descriptors_scene);

  //-- Step 3: Matching descriptor vectors using FLANN matcher
  FlannBasedMatcher matcher;
  vector<DMatch> matches;
  matcher.match(descriptors_object, descriptors_scene, matches);

  double max_dist = 0;
  double min_dist = 100;

  //-- Quick calculation of max and min distances between keypoints
  for (int i = 0; i < descriptors_object.rows; i++) {
    double dist = matches[i].distance;
    if (dist < min_dist) min_dist = dist;
    if (dist > max_dist) max_dist = dist;
  }

  //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
  std::vector<DMatch> good_matches;

  for (int i = 0; i < descriptors_object.rows; i++) {
    if (matches[i].distance < 3 * min_dist) {
      good_matches.push_back(matches[i]);
    }
  }

  //-- Localize the object
  std::vector<Point2f> obj;
  std::vector<Point2f> scene;

  for (int i = 0; i < good_matches.size(); i++) {
    //-- Get the keypoints from the good matches
    obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
    scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
  }

  Mat H = findHomography(obj, scene, CV_RANSAC);

  // read the scene in with color
  Mat img_scene_color = imread(argv[2]);

  // extract the matched object from the scene
  Mat dst;
  warpPerspective(img_scene_color, dst, H, img_object.size(), WARP_INVERSE_MAP);

  try {
    imwrite(argv[3], dst);
  } catch (std::runtime_error& ex) {
    fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
    return -1;
  }

  return 0;
}

/** @function readme */
void readme() {
  cout << " Usage: ./extract <object> <scene> <output>" << endl;
}
