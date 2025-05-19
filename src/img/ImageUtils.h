#pragma once

#include <opencv2/opencv.hpp>
#include <string>

namespace ImageUtils {

    void printImageInfo(const cv::Mat& img, const std::string& name = "");

    void printHistogram(const cv::Mat& img,
                        int bins,
                        int chartHeight);
}
