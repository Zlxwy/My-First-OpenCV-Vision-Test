// gray picture display 灰度图像显示
// 在在一个窗口上显示原图像、一个窗口显示灰度图像、一个窗口显示黑白图像
#include <opencv2/opencv.hpp>
#include <iostream>
#include <Windows.h> // 用于延时，仅适用于Windowns操作系统
#include <iomanip> // 用于格式化输出
#include "test.hpp"

int test_gray_pic(void) {
    cv::VideoCapture cam(CAM_INDEX); // 打开相机
    if (!cam.isOpened()) { // 如果无法打开相机
        std::cerr << "ERROR: 无法打开摄像头！" << std::endl;
        return -1;
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH); // 设置宽度
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT); // 设置高度
    std::cout << cam.get(cv::CAP_PROP_FRAME_WIDTH) << "*" << cam.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    /*创建窗口用于图像显示*/
    cv::namedWindow("Original Picture", cv::WINDOW_AUTOSIZE); // 创建一个窗口
    cv::namedWindow("Gray Picture", cv::WINDOW_AUTOSIZE); // 创建一个窗口
    cv::namedWindow("Binary Picture", cv::WINDOW_AUTOSIZE); // 创建一个窗口

    /*初始化帧数和时间戳*/
    int frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();
    double fps = 0;

    while (true) { // 开始循环
        /*帧率计算*/
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

        /*图像矩阵定义*/
        cv::Mat frame;
        cv::Mat grayFrame;
        cv::Mat binaryFrame;

        /*图像获取*/
        bool ret = cam.read(frame); // 从相机获取新的一帧
        if (!ret) { // 如果没有图像
            std::cout << "ERROR: 无法获取摄像头画面！" << std::endl;
            break;
        }
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::threshold(grayFrame, binaryFrame, 128, 255, cv::THRESH_BINARY);

        /*图像操作*/
        cv::putText(frame, ss.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2); // 写上帧率值
        
        ///*窗口显示图像*/
        cv::imshow("Original Picture", frame); // 显示图像
        cv::imshow("Gray Picture", grayFrame); // 显示图像
        cv::imshow("Binary Picture", binaryFrame); // 显示图像

        /*按键检测*/
        int KeyGet = cv::waitKey(1); // 等待1ms获取按键值
        if (KeyGet == KEY_ESC) break; // 如果按下Esc的话，退出循环
    }

    cam.release(); // 释放相机资源
    std::cout << "相机资源已释放" << std::endl; // 添加日志来跟踪资源释放
    cv::destroyAllWindows(); // 关闭所有OpenCV创建的窗口
    std::cout << "所有窗口已关闭" << std::endl; // 添加日志来跟踪窗口关闭

    return 0;
}