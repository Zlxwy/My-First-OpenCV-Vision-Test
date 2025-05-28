// 将图像进行拉伸变换，分别显示在两个窗口上
#include <opencv2/opencv.hpp>
#include <iostream>
#include "test.hpp"

int test_perspective_transform(void) {
    cv::VideoCapture cam(CAM_INDEX); // 打开相机
    if (!cam.isOpened()) { // 如果无法打开相机
        std::cerr << "ERROR: 无法打开摄像头！" << std::endl;
        return -1;
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH); // 设置宽度
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT); // 设置高度
    std::cout << cam.get(cv::CAP_PROP_FRAME_WIDTH) << "*" << cam.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    /*创建窗口用于图像显示*/
    cv::namedWindow("windows1", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("windows2", cv::WINDOW_NORMAL); // 创建一个窗口

    /*初始化帧数和时间戳*/
    int frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();
    double fps = 0;

    while (true) { // 开始循环
        frameCount++; // 更新帧数
        auto currentTime = std::chrono::steady_clock::now(); // 获取当前时间
        double elapsedTime = std::chrono::duration<double>(currentTime - startTime).count(); // 计算时间差
        // 每秒更新一次帧率
        if (elapsedTime >= 1.0) {
            fps = frameCount / elapsedTime; // 计算帧率
            frameCount = 0; startTime = currentTime; // 重置帧数和时间戳
        }
        std::stringstream ss; // 创建一个字符串流
        ss << "FPS:" << std::fixed << std::setprecision(2) << fps; // FPS:xx.xx

        cv::Mat frame; // 获取原图像
        cv::Mat result; // 接收变换后的图像
        bool ret = cam.read(frame); // 从相机获取新的一帧
        if (!ret) { // 如果没有图像
            std::cout << "ERROR: 无法获取摄像头画面！" << std::endl;
            break;
        }
        int sui = 200; // 拉伸的位置
        cv::Point2f pts1[4] = {
            cv::Point2f(sui, 0),
            cv::Point2f(CAM_WIDTH-1-sui, 0),
            cv::Point2f(0, CAM_HEIGHT-1),
            cv::Point2f(CAM_WIDTH-1, CAM_HEIGHT-1)
        };
        cv::Point2f pts2[4] = {
            cv::Point2f(sui, 0),
            cv::Point2f(CAM_WIDTH-1-sui, 0),
            cv::Point2f(sui, CAM_HEIGHT-1),
            cv::Point2f(CAM_WIDTH-1-sui, CAM_HEIGHT-1)
        };
        cv::Mat matrix = cv::getPerspectiveTransform(pts1, pts2); // 计算透视变换矩阵
        cv::warpPerspective(frame, result, matrix, cv::Size(CAM_WIDTH, CAM_HEIGHT)); // 应用透视变换

        /*绘制源图像中、目标图像中的四个点*/
        for (auto &pt : pts1) cv::circle(frame, pt, 10, cv::Scalar(0, 255, 0), -1);
        for (auto &pt : pts2) cv::circle(result, pt, 10, cv::Scalar(0, 255, 0), -1);
        /*在图像上写上帧率值*/
        cv::putText(frame, ss.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
        cv::putText(result, ss.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        cv::imshow("windows1", frame); // 在窗口1中显示原图像
        cv::imshow("windows2", result); // 在窗口2中显示拉伸后的图像

        int KeyGet = cv::waitKey(1); // 等待1ms获取按键值
        if (KeyGet == KEY_ESC) break; // 如果按下Esc的话，退出循环

    }

    cam.release(); // 释放相机资源
    std::cout << "相机资源已释放" << std::endl; // 添加日志来跟踪资源释放
    cv::destroyAllWindows(); // 关闭所有OpenCV创建的窗口
    std::cout << "所有窗口已关闭" << std::endl; // 添加日志来跟踪窗口关闭

    return 0;
}
