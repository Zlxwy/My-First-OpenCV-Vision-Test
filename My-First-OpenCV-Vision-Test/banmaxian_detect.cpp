#include <opencv2/opencv.hpp>
#include <iostream>

int test_banmaxian_detect(void) {
    // 读取图像
    cv::Mat img = cv::imread("./saideao_1/19.jpg"); // 替换为斑马线图像路径
    if (img.empty()) {
        std::cerr << "Error: Image not found!" << std::endl;
        return -1;
    }

    // 灰度化
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    // 高斯滤波
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);

    // Canny 边缘检测
    cv::Mat edges;
    cv::Canny(blurred, edges, 50, 150);

    // 霍夫变换检测直线
    std::vector<cv::Vec2f> lines;
    cv::HoughLines(edges, lines, 1, CV_PI / 180, 50, 0, 0);

    // 绘制检测到的直线
    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0];
        float theta = lines[i][1];
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        cv::Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
        cv::Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));
        cv::line(img, pt1, pt2, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    }

    // 显示结果
    cv::imshow("Detected Lines", img);
    cv::waitKey(0);
    return 0;
}
