/*author: RISHABH SINGH, HRITHIK KANOJE
Class: CS5330 Pattern Recog & Computer Vision
Prof: Bruce Maxwell
Project 4: Calibration and Augmented Reality
*/

#include <iostream>
#include "utils.h"
using namespace std;
using namespace cv;

#if 0
int main() {
	std::string argv0 = "a.out";
	std::string argv1 = "chessboard";

	int argc = 2;
	char* argv[2];
	argv[0] = _strdup(argv0.c_str());
	argv[1] = _strdup(argv1.c_str());

	if (argc != 2) {
		std::cout << "Usage: ./main.exe <target>" << std::endl;
		std::cout << "<target> is chessboard " << std::endl;
		exit(-1);
	}

	// declare vectors
	std::vector<cv::Point2f> corner_set;        //coordinates of detected corners
	std::vector<cv::Vec3f> point_set;           //3D world coordinates of detected corners
	std::vector<std::vector<cv::Vec3f> > point_list;    //list of 3D world coordinates of detected corners
	std::vector<std::vector<cv::Point2f> > corner_list; //list of 2D points of detected corners

	cv::TermCriteria term_criteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 20, 0.03); //terminate when maximum number of iteration is reached, max_itr = 20, accuracy = 0.03
	cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64FC1);   //3x3 matrix with 64-bit floating point
	cv::Mat distort_coeff;

	int row_pts, cols_pts;
	std::string target = std::string(argv[1]);  //initialize variable name target to the first command line argument
	// if target name is chessboard no. of rows & cols is 9 and 6 respectively else exit the function with -1
	if (target == "chessboard") {
		row_pts = 9;
		cols_pts = 6;
	}
	else {
		std::cout << "Usage: ./main.exe <camera_index> <target>" << std::endl;
		std::cout << "<camera_index> 0 to 4, <target> is chessboard " << std::endl;
		exit(-1);
	}
	cv::Size pattern_size = cv::Size(row_pts, cols_pts); //initalize object "pattern_size" size of calibration pattern no of rows & cols 

	cv::VideoCapture* capdev;
	// open the video device 
	capdev = new cv::VideoCapture(0);
	if (!capdev->isOpened()) {
		printf("Unable to open video device\n");
		return (-1);
	}
	// setup for video processing get some properties of the image
	cv::Size refS((int)capdev->get(cv::CAP_PROP_FRAME_WIDTH),
		(int)capdev->get(cv::CAP_PROP_FRAME_HEIGHT));
	printf("Expected size: %d %d\n", refS.width, refS.height);
	cv::namedWindow("Video", 1); // identifies a display window
	cv::Mat frame;
	cv::Mat grey_scale;
	int count = 0;
	std::string delimiter(50, '-');

	for (;;) {
		*capdev >> frame; // getting new frame from the camera, treating it as stream
		if (frame.empty()) {
			printf("frame is empty\n");
			break;
		}
		corner_set.clear();
		point_set.clear();

		camera_matrix.at<double>(0, 0) = 1;
		camera_matrix.at<double>(1, 1) = 1;
		camera_matrix.at<double>(0, 2) = frame.cols / 2;
		camera_matrix.at<double>(1, 2) = frame.rows / 2;

		cv::cvtColor(frame, grey_scale, cv::COLOR_BGR2GRAY);

		// call the find chess board corners function to find the corners on a chess board and get the corner set
		bool found_corner;
		if (target == "chessboard") {
			found_corner = cv::findChessboardCorners(frame, pattern_size, corner_set);
		}

		if (found_corner) {
			// if found, find more exact corner positions and draw the corners
			cv::cornerSubPix(grey_scale, corner_set, cv::Size(5, 5), cv::Size(-1, -1), term_criteria);
			cv::drawChessboardCorners(frame, pattern_size, corner_set, found_corner);
		}

		// see if there is a waiting keystroke
		int key = cv::waitKey(10);
		// type 'q', to quit the program
		if (key == 'q') {
			break;
		}
		// type 's' corners are found and saved the frames
		if (key == 's') {
			if (!found_corner) {
				std::cout << "No Corners found!" << std::endl;
			}
			//save the image points and corner points for further calibration
			else {
				std::cout << delimiter << std::endl;
				std::cout << "calibration frame " + std::to_string(count) + " saved." << std::endl;

				// generate the point set
				if (target == "chessboard") {
					int i = 0;
					while (i < cols_pts) {
						int j = 0;
						while (j < row_pts) {
							point_set.push_back(cv::Point3f((float)j, (float)(-i), 0));
							j++;
						}
						i++;
					}
				}
				corner_list.push_back(std::vector<cv::Point2f>(corner_set));
				point_list.push_back(std::vector<cv::Vec3f>(point_set));

				print_set("adding corner set" + std::to_string(count) + " to the corner list:", point_set);
				print_set("", corner_set);

				cv::imwrite("../img/" + target + "/screenshot_" + std::to_string(count++) + ".jpg", frame);
			}
		}
		// type 'c' it will check if calibration images are more than five then it will perform camera calibration  
		else if (key == 'c') {
			if (point_list.size() >= 5) {
				std::cout << delimiter << std::endl;
				std::cout << std::to_string(point_list.size()) + " frame collected, running a calibration:" << std::endl;
				print_matrix("initial camera matrix: ", camera_matrix);

				std::vector<cv::Mat> rvecs, tvecs;
				double reprojection_error = cv::calibrateCamera(point_list,
					corner_list,
					frame.size(),
					camera_matrix,
					distort_coeff,
					rvecs,
					tvecs,
					cv::CALIB_FIX_ASPECT_RATIO,
					term_criteria);

				print_matrix("camera_matrix: ", camera_matrix);
				print_matrix("distortion_coefficients:", distort_coeff);
				std::cout << "reprojection error: " << reprojection_error << std::endl;
				std::cout << "save the camera matrix & distort_coeff to csv file." << std::endl;
				std::cout << delimiter << std::endl;

				// write to csv file
				save_intrinsic_parameters("camera_intrinsic_paras_" + target + ".csv",
					camera_matrix,
					distort_coeff);
			}
			else {
				std::cout << "calibration frames are not enough: require 5, current " + std::to_string(point_list.size())
					<< std::endl;
			}
		}
		cv::imshow("Video", frame);
	}
	delete capdev;
	return (0);
}
#endif