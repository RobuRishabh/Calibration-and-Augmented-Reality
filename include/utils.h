
/*author: RISHABH SINGH, HRITHIK KANOJE
Class: CS5330 Pattern Recog & Computer Vision
Prof: Bruce Maxwell
Project 4: Calibration and Augmented Reality
*/

#pragma once

#include <opencv2/opencv.hpp>
#ifndef UTIL_H
#define UTIL_H

//function to print set along with comment
template<typename T>
void print_set(const std::string& comment, std::vector<T> set) {
    std::cout << comment << std::endl;
    int index = 0;
    for (T point : set) {
        std::cout << point << " ";
        index++;
        if (index % 9 == 0) {
            std::cout << std::endl;
        }
    }
}

//function to write matrix
void print_matrix(const std::string& comment, cv::Mat matrix);

//function to write instric parameter to csv file
void save_intrinsic_parameters(const std::string& file_name,
    const cv::Mat& camera_matrix,
    const cv::Mat& distortion_coefficients);

//function to read instric parameter to csv file
void read_intrinsic_parameters(const std::string& file_name, cv::Mat& camera_matrix, cv::Mat& distortion_coefficients);

//function to read object file
void read_object(const std::string& file_path,
    std::vector<cv::Point3f>& vertices,
    std::vector<std::vector<int>>& faces,
    float x_shift,
    float y_shift);

//function to construct axes
void construct_axes(const cv::Mat& rvec,
    const cv::Mat& tvec,
    const cv::Mat& camera_matrix,
    const cv::Mat& distortion_coefficients,
    cv::Mat& frame,
    cv::Point2f origin);

//function to construct object
void construct_object(const cv::Mat& rvec,
    const cv::Mat& tvec,
    const cv::Mat& camera_matrix,
    const cv::Mat& distortion_coefficients,
    const std::vector<cv::Point3f>& vertices,
    std::vector<std::vector<int>>& faces,
    cv::Mat& frame
);

#endif //UTIL

