#include "ImageUtils.h"
#include <iostream>
#include <vector>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>

namespace ImageUtils {

    void printImageInfo(const cv::Mat& img, const std::string& name) {
        std::string prefix = name.empty() ? "" : name + ": ";
        std::cout << prefix
                  << "size=" << img.cols << "x" << img.rows
                  << ", channels=" << img.channels()
                  << ", depth=" << img.depth()
                  << ", total pixels=" << img.total()
                  << "\n";
    }


    void printHistogram(const cv::Mat& img,
                        const int bins,
                        const int chartHeight)
    {
        CV_Assert(img.channels() <= 4);

        std::vector<std::string> chanNames;
        switch (img.channels()) {
            case 1: chanNames = {"Gray"}; break;
            case 3: chanNames = {"Blue","Green","Red"}; break;
            case 4: chanNames = {"Blue","Green","Red","Alpha"}; break;
            default: throw std::runtime_error("Unsupported number of channels.");
        }

        std::vector<cv::Mat> channels;
        cv::split(img, channels);

        for (size_t c = 0; c < channels.size(); ++c) {
            cv::Mat hist;
            float range[] = {0, 256};
            const float* histRange = {range};
            cv::calcHist(&channels[c], 1, 0, cv::Mat(), hist, 1, &bins,
                         &histRange, true, false);

            double maxVal = 0;
            cv::minMaxLoc(hist, nullptr, &maxVal);
            int peakBin = 0;
            for (int b = 1; b < bins; ++b) {
                if (hist.at<float>(b) > hist.at<float>(peakBin)) peakBin = b;
            }
            int peakVal = static_cast<int>(hist.at<float>(peakBin) + 0.5);

            int labelWidth = std::max(
                static_cast<int>(std::to_string(static_cast<int>(maxVal)).size()),
                static_cast<int>(std::to_string(peakVal).size())
            );

            std::cout << "\n=== Channel " << chanNames[c]
                      << " (bins=" << bins
                      << ", height=" << chartHeight << ") ===\n";
            std::cout << std::string(labelWidth + 2, ' ') << "Count\n";

            std::vector grid(chartHeight, std::string(bins, ' '));
            for (int b = 0; b < bins; ++b) {
                int h = static_cast<int>((hist.at<float>(b) / maxVal) * chartHeight + 0.5);
                for (int row = chartHeight - 1; row >= chartHeight - h; --row) {
                    grid[row][b] = '#';
                }
            }

            for (int row = 0; row < chartHeight; ++row) {
                double value = maxVal * (chartHeight - row) / chartHeight;
                int ivalue = static_cast<int>(value + 0.5);
                std::cout << std::setw(labelWidth) << ivalue << " |";
                std::cout.write(grid[row].data(), bins);
                std::cout << "\n";
            }

            std::cout << std::string(labelWidth, ' ') << " +"
                      << std::string(bins, '-') << "\n";

            int bin0   = 0;
            int binPeak= peakBin;
            int binMid = bins / 2;
            int binMax = bins - 1;
            auto binToVal = [&](int b){ return static_cast<int>((256.0 * b / bins) + 0.5); };
            struct Tick { int pos; int val; };
            std::vector<Tick> ticks = {{bin0, binToVal(bin0)},
                                       {binPeak, binToVal(binPeak)},
                                       {binMid, binToVal(binMid)},
                                       {binMax, binToVal(binMax)}};

            std::string lbl(bins, ' ');
            for (auto &t: ticks) {
                std::string vs = std::to_string(t.val);
                int start = std::max(0, std::min(bins - (int)vs.size(), t.pos - (int)vs.size()/2));
                for (size_t i = 0; i < vs.size(); ++i)
                    lbl[start + i] = vs[i];
            }

            std::cout << std::string(labelWidth + 1, ' ') << lbl << "\n";

            std::cout << std::string(labelWidth + 1, ' ') << " Intensity →\n";
        }
    }

} // namespace ImageUtils
