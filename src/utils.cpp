/*author: RISHABH SINGH, HRITHIK KANOJE
Class: CS5330 Pattern Recog & Computer Vision
Prof: Bruce Maxwell
Project 4: Calibration and Augmented Reality
*/

#include <fstream>
#include "utils.h"

using namespace std;

//finction "print_matrix" prints out the elements of input matrix row by row
void print_matrix(const std::string &comment, cv::Mat matrix) {
  std::cout << comment << std::endl;
  //iterate over rows and cols
  for (int i = 0; i < matrix.rows; i++) {
    for (int j = 0; j < matrix.cols; j++) {
        //print the values of matrix element at index (i,j)
      std::cout << matrix.at<double>(i, j) << "\t";
    }
    std::cout << std::endl;
  }
}

void save_intrinsic_parameters(const std::string &file_name,
                           const cv::Mat &cam_mat,
                           const cv::Mat &distort_coeff) {
  std::ofstream intrinsic_file;
  intrinsic_file.open("../" + file_name);
  intrinsic_file.flush();

  //header contains cols name for camera matrix and distortaion coefficient
  std::string header = "cam_00,cam_01,cam_02,cam_10,cam_11,cam_12,cam_20,cam_21,cam_22,";
  //add distortion coefficient header to the header row
  for (int i = 0; i < distort_coeff.rows * distort_coeff.cols; i++) {
    header += "distort_coeff" + std::to_string(i) + ",";
  }
  header += "\n";
  intrinsic_file << header;
  //camera matrix values to the output file
  for (int i = 0; i < cam_mat.rows; i++) {
    for (int j = 0; j < cam_mat.cols; j++) {
      intrinsic_file << cam_mat.at<double>(i, j) << ",";
    }
  }
  //distortion coefficient values to the output file
  for (int i = 0; i < distort_coeff.rows; i++) {
    for (int j = 0; j < distort_coeff.cols; j++) {
      intrinsic_file << distort_coeff.at<double>(i, j) << ",";
    }
  }
  intrinsic_file.close();
}

//read intrinsic parameters from a CSV file then store them in 2 matrices
void read_intrinsic_parameters(const std::string &file_name, cv::Mat &camera_matrix, cv::Mat &distortion_coefficients) {
  std::fstream intrinsic_file;
  intrinsic_file.open("../" + file_name);

  std::vector<double> result;
  std::string line;
  // if there is any content in the file. If not, it outputs an error message and terminates the program.
  if (!std::getline(intrinsic_file, line, '\n')) {
    std::cout << "No content in the csv file" << std::endl;
    exit(-1);
  }
  std::getline(intrinsic_file, line, '\n');

  camera_matrix = cv::Mat::zeros(3, 3, CV_64FC1);       //3x3 matrix filled with zeros, which will hold the camera matrix values.
  distortion_coefficients = cv::Mat(1, 5, CV_64FC1);    //1x5 matrix, which will hold the distortion coefficient values.

  //creates a stringstream object to parse the values from the second line of the file.
  std::stringstream ss(line);
  //reads the values from the stringstream object and stores them in the result vector.
  while (ss.good()) {
    std::string substr;
    std::getline(ss, substr, ',');
    if (!substr.empty())
      result.push_back(std::strtod(substr.c_str(), nullptr));
  }
  //iterate over camera matrix rows and cols
  int index = 0;
  for (int i = 0; i < camera_matrix.rows; i++) {
    for (int j = 0; j < camera_matrix.cols; j++) {
      camera_matrix.at<double>(i, j) = result.at(index++);
    }
  }
  //iterate over distortion coefficient rows and cols
  for (int i = 0; i < distortion_coefficients.rows; i++) {
    for (int j = 0; j < distortion_coefficients.cols; j++) {
      distortion_coefficients.at<double>(i, j) = result.at(index++);
    }
  }
  intrinsic_file.close();
}

void read_object(const std::string &file_path,
              std::vector<cv::Point3f> &vertices,
              std::vector<std::vector<int>> &faces,
              float x_shift,
              float y_shift) {
  std::string line;
  std::vector<std::string> values;
  std::ifstream file(file_path);
  //checks if the file can be opened. If it cannot be opened, it displays an error message and exits the program.
  if (!file.good()) {
    std::cout << "No such file" << std::endl;
    std::cout << "Candidate objects: [humanoid, teapot, teddy], Please refer to the objs directory";
    exit(-1);
  }
  while (std::getline(file, line)) {
    values.clear();

    // split the line into separate strings
    std::stringstream ss(line);
    while (ss.good()) {
      std::string substr;
      std::getline(ss, substr, ' ');
      if (!substr.empty())
        values.push_back(substr);
    }
    if (values.empty()) {
      continue;
    }
    //function displays error and exit the program If the line does not contain exactly 4 values.
    if (values.size() != 4) {
      std::cout << "FILE FORMAT ERROR" << std::endl;
      exit(-1);
    }

    if (values.at(0) == "v") {
      //shift the 3d object to the middle of the chessboard
      vertices.push_back(cv::Point3f(std::stof(values.at(1)) + x_shift, //add x_shift to x
                                     std::stof(values.at(2)) + y_shift, //add y_shift to y
                                     std::stof(values.at(3))));
    } else if (values.at(0) == "f") {
      faces.push_back({std::stoi(values.at(1)), std::stoi(values.at(2)), std::stoi(values.at(3))});
    } else {
      std::cout << "FILED NOT SUPPORT NOW!" << std::endl;
      exit(-1);
    }
  }
  std::cout << "vertices number: " + std::to_string(vertices.size())
            << ", faces number: " + std::to_string(faces.size()) << std::endl;
}

void construct_axes(const cv::Mat &rvec,
               const cv::Mat &tvec,
               const cv::Mat &cam_mat,
               const cv::Mat &distort_coeff,
               cv::Mat &frame,
               cv::Point2f origin) {
  std::vector<cv::Point3f> obj_pts;
  obj_pts.emplace_back(3., 0., 0.);
  obj_pts.emplace_back(0., -3., 0.);
  obj_pts.emplace_back(0., 0., 3.);
  std::vector<cv::Point2f> img_pts;
  cv::projectPoints(obj_pts, rvec, tvec, cam_mat, distort_coeff, img_pts);
  //arrowedLine = draws the 2D lines representing the axes on the input image with each axis being different color
  cv::arrowedLine(frame, origin, img_pts[0], cv::Scalar(255, 0, 0), 2);
  cv::arrowedLine(frame, origin, img_pts[1], cv::Scalar(0, 255, 0), 2);
  cv::arrowedLine(frame, origin, img_pts[2], cv::Scalar(0, 0, 255), 2);
}

//draws a 3D object on a 2D image
void construct_object(const cv::Mat &rvec,
                 const cv::Mat &tvec,
                 const cv::Mat &cam_mat,
                 const cv::Mat &distort_coeff,
                 const std::vector<cv::Point3f> &vertices,
                 std::vector<std::vector<int>> &faces,
                 cv::Mat &frame
) {
  // show 3d objects
  std::vector<cv::Point2f> obj_img_pts;
  cv::projectPoints(vertices, rvec, tvec, cam_mat, distort_coeff, obj_img_pts);
  //iterates over the faces of the object,
  for (std::vector<int> &face: faces) {
 //For each face, it draws three lines between the 2D image points corresponding to the vertices of the face.
    cv::line(frame,
             obj_img_pts[face[0] - 1],
             obj_img_pts[face[1] - 1],
             cv::Scalar(0, 255, 0),
             1);
    cv::line(frame,
             obj_img_pts[face[1] - 1],
             obj_img_pts[face[2] - 1],
             cv::Scalar(0, 255, 0),
             1);
    // draw the line between the last vertices and the first vertices in a face
    cv::line(frame,
             obj_img_pts[face[2] - 1],
             obj_img_pts[face[0] - 1],
             cv::Scalar(0, 255, 0),
             1);
  }
}