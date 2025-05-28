// 赛道识别，来自ilikara的程序
#include <opencv2/opencv.hpp>
#include <vector>
#include <numeric>
#include <algorithm>
#include <iostream>

int track_detect_ilikara(void) {
    for (int n=0; n<20; ++n) {
        std::string filename = "./saideao_0/" + std::to_string(n) + ".jpg";
        cv::Mat image = cv::imread(filename); // 读取一张赛道图片

        if (image.empty()) { // 如果没读取到
            std::cout << "Could not read the image: " << filename << std::endl; // 打印未读取信息
            return -1;
        }

        // 转换为灰度图像
        cv::Mat gray;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY); // 将原始图像转换为灰度图

        // 使用threshold函数进行OTSU自动阈值二值化
        cv::Mat binary;
        cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

        // 检测边缘
        cv::Mat edges;
        cv::Canny(binary, edges, 50, 150);
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(edges, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

        // 过滤掉闭合边
        contours.erase(std::remove_if(contours.begin(), contours.end(),
            [](const std::vector<cv::Point>& cnt) {
                return cv::isContourConvex(cnt);
            }), contours.end());

        // 过滤掉长度太短的边
        const double min_length = 200;
        contours.erase(std::remove_if(contours.begin(), contours.end(),
            [min_length](const std::vector<cv::Point>& cnt) {
                return cv::arcLength(cnt, false) <= min_length;
            }), contours.end());

        // 绘制轮廓
        cv::Mat result = cv::Mat::zeros(image.size(), image.type());
        cv::drawContours(result, contours, -1, cv::Scalar(0, 255, 0), 2);

        // 计算中心线
        std::vector<cv::Point> center_points;
        for (int y = 0; y < result.rows; ++y) {
            std::vector<int> x_coords;
            for (const auto& contour : contours) {
                for (const auto& point : contour) {
                    if (point.y == y) {
                        x_coords.push_back(point.x);
                    }
                }
            }
            if (!x_coords.empty()) {
                std::vector<int> filtered_x_coords = { x_coords[0] };
                for (size_t i = 1; i < x_coords.size(); ++i) {
                    if (x_coords[i] - filtered_x_coords.back() > 30) {
                        filtered_x_coords.push_back(x_coords[i]);
                    }
                }
                if (filtered_x_coords.size() > 1) {
                    int avg_x = std::accumulate(filtered_x_coords.begin(), filtered_x_coords.end(), 0) / filtered_x_coords.size();
                    center_points.emplace_back(avg_x, y);
                }
            }
        }

        for (size_t i = 0; i < center_points.size() - 1; ++i) {
            cv::line(result, center_points[i], center_points[i + 1], cv::Scalar(0, 0, 255), 2);
        }

        cv::imshow("Source", image);
        cv::imshow("Result", result);
        cv::waitKey(0);
    }

    return 0;
}
