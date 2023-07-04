#include <iostream>
#include <bits/stdc++.h>
#include "utils.h"

/*
* 1. 
* Command line arguments :  The program expects 2 command line arguments <target> and <obj_name>
*   - <target> should be either "chessboard" or "circlegrid", indicating the type of target pattern
*   - <obj_name> refers to the name of the 3D object file located in the "obj" folder
*/
int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Usage: ./ar.exe <target> <obj_name>" << std::endl;
    std::cout << "<target> should be chessboard or circlesgrid, <obj_name> should be one in objs folder" << std::endl;
    exit(-1);
  }

/*
* 2.
* Blob Detector Initialization : The program initializes a * **cv::SimpleBlobDetector** using predefined parameters (params)
* This detector is usde for detecting circular grid corners in case of "circlesgrid" target
*/
  //an instance of cv::SimpleBlobDetector::Params is created, it holds the parameters for the SimpleBlobDetector algorithm
  cv::SimpleBlobDetector::Params params;
  params.maxArea = 10e4; //maximum area of a blob
  params.minArea = 10;  //minimum area of a blob
  params.minDistBetweenBlobs = 5; //minimum distance between two blobs
  // Initialize the simple blob detector with the parameters above for the findCirclesGrid() function to find the circle grids
  // a pointer to cv::SimpleBlobDetector is created using the create function and the parameters.
  cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

  /*
  * 3.
  * Target Setup :  Based on the <target> value, the program sets the dimensions of the target pattern (points_per_row and points_per_column).
  * For the "chessboard" target, it sets points_per_row to 9 and points_per_column to 6.
  * For the "circlesgrid" target, it sets points_per_row to 4 and points_per_column to 11.
  */
  //declared to store the number of points in the chessboard or circles grid pattern.
  int points_per_row, points_per_column;
  //value of target is assigned from the second command-line argument (argv[1]).
  std::string target = std::string(argv[1]);
  //If target is "chessboard," points_per_row is set to 9 and points_per_column is set to 6.
  if (target == "chessboard") {
    points_per_row = 9;
    points_per_column = 6;
  }
  //If target is "circlesgrid," points_per_row is set to 4 and points_per_column is set to 11.
  else if (target == "circlesgrid") {
    points_per_row = 4;
    points_per_column = 11;
  } 
  else {
    std::cout << "Usage: ./ar.exe <camera_index> <target> <obj_name>" << std::endl;
    std::cout << "<camera_index> could be any integer, <target> should be chessboard or circlesgrid" << std::endl;
    exit(-1);
  }

  //A cv::Size object called pattern_size is created with the dimensions specified by points_per_row and points_per_column
  cv::Size pattern_size = cv::Size(points_per_row, points_per_column);

  //Two empty vectors are declared
  std::vector<cv::Point2f> corner_set;
  std::vector<cv::Vec3f> point_set;

  //termcrit is a cv::TermCriteria object that specifies the termination criteria for corner refinement.
  cv::TermCriteria termcrit(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 20, 0.03);
  //objects to store camera intrinsic parameters

  cv::Mat camera_matrix;
  cv::Mat distortion_coefficients;
  cv::Mat rvec, tvec;
  //boolean flag to determine whether to show the 3D object
  bool show_obj = false;
  //count is an integer to keep track of the captured frames.
  int count = 0;

  //store the 3D coordinates of the object vertices
  std::vector<cv::Point3f> vertices;
  //store the indices of the object's faces
  std::vector<std::vector<int>> faces;
  /*
  * 4.
  * Object Loading : The program reads the 3D object file (<obj_name>.obj) corresponding to the target pattern. 
  * It populates the vertices and faces vectors with the object's vertex coordinates and face indices.
  */
  //Depending on the value of target
  //the read_obj() is called with different arguments to read the corresponding object file and populate the vertices and faces vectors
  if (target == "chessboard") {
    read_obj("../objs/" + std::string(argv[2]) + ".obj", vertices, faces, 4.f, -2.5f);
  } else {
    read_obj("../objs/" + std::string(argv[2]) + ".obj", vertices, faces, 3.5f, -5.f);
  }

  /*
  * 5.
  * Video Capture Setup : The program initializes a video capture device (capdev) to read frames from the camera.
  * It also obtains the width and height of the video frames (refS) for further processing.
  */

  cv::VideoCapture *capdev;
  // open the video device
  capdev = new cv::VideoCapture(0);
  if (!capdev->isOpened()) {
    printf("Unable to open video device\n");
    return (-1);
  }
  // refS.width and refS.height, are assigned the width and height of the video capture frame
  cv::Size refS((int) capdev->get(cv::CAP_PROP_FRAME_WIDTH),
                (int) capdev->get(cv::CAP_PROP_FRAME_HEIGHT));
  printf("Expected size: %d %d\n", refS.width, refS.height);
  cv::namedWindow("Video", cv::WindowFlags::WINDOW_NORMAL); // identifies a window
  cv::Mat frame;
  cv::Mat grey_scale;

  /*
  * 6.
  * Camera Calibration Parameters : The program reads camera intrinsic parameters (camera_matrix and distortion_coefficients) from a CSV file based on the target value.
  * These parameters are used in the camera calibration process.
  */
  // read the camera intrinsic parameters and distortion coefficients from a CSV file based on the target value
  read_intrinsic_paras("camera_intrinsic_paras_" + target + ".csv", camera_matrix, distortion_coefficients);

  /*
  * 7.
  * Frame Processing Loop: The program enters a loop to process each frame from the video stream.
  */
  for (;;) {
    *capdev >> frame; // get a new frame from the camera, treat as a stream
    if (frame.empty()) {
      printf("frame is empty\n");
      break;
    }
    /*
    * 7(a).
    * Frame Preprocessing: The frame is converted to grayscale for corner detection.
    */
    cv::cvtColor(frame, grey_scale, cv::COLOR_BGR2GRAY);

    //corner_set and point_set vectors are cleared to store 
    //the new corner and 3D point data for the current frame
    corner_set.clear();
    point_set.clear();

    /*
    * 7(b).
    * Corner Detection: Using OpenCV's findChessboardCorners or findCirclesGrid functions, corners are detected in the target pattern based on the selected target type. Detected corners are refined using cornerSubPix for better accuracy.
    */
    bool corner_find_flag;
    //If the target is "chessboard," cv::findChessboardCorners is called to detect the chessboard corners in the frame using the specified pattern_size
    if (target == "chessboard") {
      corner_find_flag = cv::findChessboardCorners(frame, pattern_size, corner_set);
    }
    //If the target is "circlesgrid," cv::findCirclesGrid is called to detect the circular grid corners using the specified pattern_size and the detector object.

    //The detected corners are stored in the corner_set, and the function returns true if corners are found 
    else {
      corner_find_flag = cv::findCirclesGrid(frame, pattern_size, corner_set, cv::CALIB_CB_ASYMMETRIC_GRID, detector);
    }

    //If corners are found (corner_find_flag is true), the corners' positions are refined using cv::cornerSubPix for more accurate results.
    if (corner_find_flag) {
      cv::cornerSubPix(grey_scale, corner_set, cv::Size(5, 5), cv::Size(-1, -1), termcrit);
      //The refined corners are drawn on the frame using cv::drawChessboardCorners
      cv::drawChessboardCorners(frame, pattern_size, corner_set, corner_find_flag);

      /*
      * 7(c).
      * 3D Point Generation : depending on the target type, a set of 3D points (point_set) is generated to represent the target pattern's corner.
      * The z-coordinates is set to 0 since the pattern is assumed to lie on the flat surface.
      */
      //If the target is "chessboard," a set of 3D points is constructed by iterating over the rows and columns of the pattern, with the z-coordinate set to 0
      if (target == "chessboard") {
        for (int i = 0; i < points_per_column; i++) {
          for (int j = 0; j < points_per_row; j++) {
            point_set.push_back(cv::Point3f((float) j, (float) (-i), 0));
          }
        }
      } 
      //If the target is "circlesgrid," a set of 3D points is constructed by iterating over the rows and columns of the pattern, with alternate columns skipped, and the z-coordinate set to 0.
      else {
        for (int i = 0; i < points_per_column; i++) {
          int j = (i % 2) == 0 ? 0 : 1;
          for (; j < points_per_row * 2; j = j + 2) {
            point_set.push_back(cv::Point3f((float) j, (float) (-i), 0.f));
          }
        }
      }
      /*
      * 7(d).
      * Pose Estimation : The program calls solvePnP to estimate the pose (rotation and translation) of the object in the scene using 
      * the 2D-3D correspondences (corner positions and 3D points). Camera intrinsic parameters and distortion coefficients are provided as inputs.
      */
      //cv::solvePnP is called to estimate the pose (rotation and translation) of the object in the scene based on the 2D-3D correspondences (point_set and corner_set)
      //The camera intrinsic parameters (camera_matrix) and distortion coefficients are provided as inputs.
      //The resulting rotation and translation vectors are stored in rvec and tvec, respectively.
      cv::solvePnP(point_set, corner_set, camera_matrix, distortion_coefficients, rvec, tvec);
      print_matrix("rotation matrix", rvec);
      print_matrix("translation matrix", tvec);
      /*
      * 7(e).
      * Visualization: The program visualizes the detected corners on the frame and draws coordinate axes representing the object's pose
      * using the drawChessboardCorners and draw_axes functions.
      */
      //The draw_axes function is called to draw coordinate axes on the frame to represent the object's pose.
      draw_axes(rvec, tvec, camera_matrix, distortion_coefficients, frame, corner_set[0]);
    }

    /*
    * User Interaction : The program waits for keystrokes from the user. Pressing 'q' quits the program, 'a' shows only the axes, 
    * 'w' shows both axes and the 3D object. Pressing 's' saves the current frame as an image.
    */
    // see if there is a waiting keystroke
    int key = cv::waitKey(10);
    // if user types 'q', quit the program
    if (key == 'q') {
      break;
    }
    // if user types 'a', only shows the axes
    if (key == 'a') {
      show_obj = false;
    }
      // if user types 'w', keep showing the axes and 3d object
    else if (key == 'w' || show_obj) {
      if (!corner_find_flag) {
        std::cout << "No Corners found!" << std::endl;
      } else {
        show_obj = true;
        draw_object(rvec, tvec, camera_matrix, distortion_coefficients, vertices, faces, frame);
      }
    }
    if (key == 's') {
      cv::imwrite("../ar_captured_" + std::to_string(count++) + ".jpg", frame);
    }
    cv::imshow("Video", frame);
  }
  delete capdev;
  return (0);
}