#include <bits/stdc++.h>
#include <iostream>
#include "utils.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[]){
	if(argc != 2){
		cout<<"Usage: ./main.exe <target>"<<endl;
		cout<<"<target> should be chessboard or circlesgrid"<<endl;
		exit(-1);
	}
	// Circle grid configuration
	cv::SimpleBlobDetector::Params params;
  	params.maxArea = 10e4;
  	params.minArea = 10;
  	params.minDistBetweenBlobs = 5;
  	//// Initialize the simple blob detector with the parameters above for the findCirclesGrid() function to find the circle grids
  	cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

  	vector<cv::Point2f> corner_set;
  	vector<cv::Vec3f> point_set;
  	vector<vector<cv::Vec3f>> point_list;
  	vector<vector<cv::Point2f>> corner_list;

  	cv::TermCriteria termcrit(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 20, 0.03);
  	cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64FC1);
  	cv::Mat distortion_coefficients;

  	int points_per_row, points_per_column;
  	string target = std::string(argv[1]);
  	if(target == "chessboard"){
  		points_per_row = 9;
  		points_per_column = 6;
  	}
  	else if(target == "circlesgrid"){
  		points_per_row = 4;
  		points_per_column = 11;
  	}
  	else{
  		cout<<"Usage: ./main.exe<camera_index> <target>"<<endl;
  		cout<<"<camera_index> could be any integer, <target> should be chessboard or circlesgrid"<<endl;
  		exit(-1);
  	}
  	cv::Size pattern_size = cv::Size(points_per_row, points_per_column);
  	cv::VideoCapture *capdev;
  	capdev = new cv::VideoCapture(0);
  	if(!capdev->isOpened()){
  		printf("Unable to open video device\n");
  		return(-1);
  	}
  	// get some properties of the image
  	cv::Size refS((int) capdev->get(cv::CAP_PROP_FRAME_WIDTH),
  				(int) capdev->get(cv::CAP_PROP_FRAME_HEIGHT));
  	printf("Expected size: %d %d\n", refS.width, refS.height);
  	cv::namedWindow("Video", cv::WindowFlags::WINDOW_NORMAL); //identifies a window
  	cv::Mat frame;
  	cv::Mat grey_scale;
  	int count = 0;
  	std::string delimiter(50, '-');

  	for(;;){
  		*capdev >> frame; //get a new framee from the camera, treat as a stream
  		if(frame.empty()){
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

  		// Call the find chess board corners function to find the corners on a chess board and get the corner set
  		bool corner_find_flag;
  		if(target == "chessboard"){
  			corner_find_flag = cv::findChessboardCorners(frame, pattern_size, corner_set);
  		}
  		else{
  			corner_find_flag = cv::findCirclesGrid(frame, pattern_size, corner_set, cv::CALIB_CB_ASYMMETRIC_GRID, detector);
  		}
  	
	  	if (corner_find_flag) {
	      // if found, find more exact corner positions and draw the corners
	      cv::cornerSubPix(grey_scale, corner_set, cv::Size(5, 5), cv::Size(-1, -1), termcrit);
	      cv::drawChessboardCorners(frame, pattern_size, corner_set, corner_find_flag);
	    }

	    // see if there is a waiting keystroke
	    int key = cv::waitKey(10);
	    // if user types 'q', quit the program
	    if (key == 'q') {
	      break;
	    }
	    if (key == 's') {
	    	if (!corner_find_flag) {
	        	std::cout << "No Corners found!" << std::endl;
	    	}
	        // if user types 's' and corners are found, save the frames. Also, save the image points and corner points for further calibration
	      	else{
		        std::cout << delimiter << std::endl;
		        std::cout << "calibration frame " + std::to_string(count) + " saved." << std::endl;
		        // generate the point set
		        if (target == "chessboard") {
		          for (int i = 0; i < points_per_column; i++) {
		            	for (int j = 0; j < points_per_row; j++) {
		              		point_set.push_back(cv::Point3f((float) j, (float) (-i), 0));
		            	}
		          	}
		        }
		        else {
		        	for (int i = 0; i < points_per_column; i++) {
		            	int j = (i % 2) == 0 ? 0 : 1;
		            	for (; j < points_per_row * 2; j = j + 2) {
		              		point_set.push_back(cv::Point3f((float) j, (float) (-i), 0.f));
		            	}
		          	}
		        }

		        corner_list.push_back(std::vector<cv::Point2f>(corner_set));
		        point_list.push_back(std::vector<cv::Vec3f>(point_set));

		        print_set("adding corner set" + std::to_string(count) + " to the corner list:", point_set);
		        print_set("", corner_set);

		        cv::imwrite("../img/" + target + "/screenshot_" + std::to_string(count++) + ".jpg", frame);
	      	}
	    } 
	    else if (key == 'c') {
	      // if user types 'c' and there are enough frames to be calibrated. Do the calibration to calculate the intrinsic matrix and distortion matrix
	    	if (point_list.size() >= 5) {
		        std::cout << delimiter << std::endl;
		        std::cout << std::to_string(point_list.size()) + " frame collected, running a calibration:" << std::endl;
		        print_matrix("initial camera matrix: ", camera_matrix);

		        std::vector<cv::Mat> rvecs, tvecs;
		        double re_projection_error = cv::calibrateCamera(point_list,
		                                                         corner_list,
		                                                         frame.size(),
		                                                         camera_matrix,
		                                                         distortion_coefficients,
		                                                         rvecs,
		                                                         tvecs,
		                                                         cv::CALIB_FIX_ASPECT_RATIO,
		                                                         termcrit);

		        print_matrix("camera_matrix: ", camera_matrix);
		        print_matrix("distortion_coefficients:", distortion_coefficients);
		        std::cout << "re-projection error: " << re_projection_error << std::endl;
		        std::cout << "save camera matrix and distortion coefficients to csv file." << std::endl;
		        std::cout << delimiter << std::endl;

		        // write to file
		        write_intrinsic_paras("camera_intrinsic_paras_" + target + ".csv",
	                              camera_matrix,
	                              distortion_coefficients);
	      	} 
	      	else {
	        std::cout << "calibration frames not enough: require 5, current " + std::to_string(point_list.size())
	                  << std::endl;
	      	}
	    }
	    cv::imshow("Video", frame);
  }
  delete capdev;
  return (0);
}