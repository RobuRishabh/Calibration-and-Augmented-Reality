#include <iostream>
#include <bits/stdc++.h>
#include "utils.h"

enum mode{
  HARRIS = 1,
  SIFT = 2,
};
int main(int argc, char *argv[]) {
  mode m = HARRIS;

  cv::VideoCapture *capdev;
  // open the video device
  capdev = new cv::VideoCapture(0);
  if (!capdev->isOpened()) {
    printf("Unable to open video device\n");
    return (-1);
  }
  // retrieves the width and height of the frames from the video capture device using the get method and cv::CAP_PROP_FRAME_WIDTH and cv::CAP_PROP_FRAME_HEIGHT constants. 
  //The obtained values are stored in a cv::Size object named refS
  cv::Size refS((int) capdev->get(cv::CAP_PROP_FRAME_WIDTH),
                (int) capdev->get(cv::CAP_PROP_FRAME_HEIGHT));
  printf("Expected size: %d %d\n", refS.width, refS.height);
  cv::namedWindow("Video", 1); // identifies a window
  cv::Mat frame;
  cv::Mat grey_scale;
  cv::Mat dst = cv::Mat::zeros(refS.width, refS.height, CV_32FC1);
  cv::Mat dst_norm, dst_norm_scaled;

  for (;;) {
    *capdev >> frame; // get a new frame from the camera, treat as a stream
    if (frame.empty()) {
      printf("frame is empty\n");
      break;
    }

    cv::cvtColor(frame, grey_scale, cv::COLOR_BGR2GRAY);

    //corner detection process. 
    if (m == HARRIS) {
      int blockSize = 5;
      int apertureSize = 3;
      double k = 0.04;
      //cornerHarris function takes the grayscale image grey_scale, the output matrix dst, the block size, aperture size, and Harris parameter k.
      cornerHarris(grey_scale, dst, blockSize, apertureSize, k);
      //The corner response matrix is then normalized using normalize 
      normalize(dst, dst_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
      //converted to an 8-bit representation using convertScaleAbs.
      convertScaleAbs(dst_norm, dst_norm_scaled);
      //iterates over each pixel of the normalized matrix
      for (int i = 0; i < dst_norm.rows; i++) {
        for (int j = 0; j < dst_norm.cols; j++) {
          //value above a threshold of 100, a red circle is drawn on the frame image using the circle function.
          if ((int) dst_norm.at<float>(i, j) > 100) {
            circle(frame, cv::Point(j, i), 1, cv::Scalar(0, 0, 255), 1, 8, 0);
          }
        }
      }
    }
    // m is set to any other value (indicating SIFT corner detection) 
    else {
      // the SIFT feature detector is created using cv::SiftFeatureDetector::create().
      cv::Ptr<cv::SiftFeatureDetector> detector = cv::SiftFeatureDetector::create();

      // SIFT keypoints are then detected in the grayscale image using the detect method of the detector object.
      //The keypoints are stored in a std::vector<cv::KeyPoint>. 
      std::vector<cv::KeyPoint> key_points;
      detector->detect(grey_scale, key_points);

      // The detected keypoints are visualized by drawing them on the grayscale image grey_scale using drawKeypoints. 
      // The resulting image is copied back to the frame variable.
      cv::Mat output;
      cv::drawKeypoints(grey_scale, key_points, output, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
      output.copyTo(frame);
    }



    // see if there is a waiting keystroke
    int key = cv::waitKey(10);
    // if user types 'q', quit the program
    if (key == 'q') {
      break;
    }
    if (key == 's') {
      cv::imwrite("../corners.jpg", frame);
    }
    // if user types 'h', show the harris corners
    else if (key == 'h') {
      m = HARRIS;
    }
      // if user types 'f', show the sift corners with orientations
    else if (key == 'f') {
      m = SIFT;
    }

    imshow("Video", frame);
  }
  delete capdev;
  return (0);
}