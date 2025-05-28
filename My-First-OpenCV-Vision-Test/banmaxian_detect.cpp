#include <opencv2/opencv.hpp>
#include <iostream>

int test_banmaxian_detect(void) {
    // ��ȡͼ��
    cv::Mat img = cv::imread("./saideao_1/19.jpg"); // �滻Ϊ������ͼ��·��
    if (img.empty()) {
        std::cerr << "Error: Image not found!" << std::endl;
        return -1;
    }

    // �ҶȻ�
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    // ��˹�˲�
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);

    // Canny ��Ե���
    cv::Mat edges;
    cv::Canny(blurred, edges, 50, 150);

    // ����任���ֱ��
    std::vector<cv::Vec2f> lines;
    cv::HoughLines(edges, lines, 1, CV_PI / 180, 50, 0, 0);

    // ���Ƽ�⵽��ֱ��
    for (size_t i = 0; i < lines.size(); i++) {
        float rho = lines[i][0];
        float theta = lines[i][1];
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        cv::Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * (a)));
        cv::Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * (a)));
        cv::line(img, pt1, pt2, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    }

    // ��ʾ���
    cv::imshow("Detected Lines", img);
    cv::waitKey(0);
    return 0;
}
