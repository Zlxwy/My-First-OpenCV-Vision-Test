// camera and display 获取相机的图像并显示
// 像照相机那样，获取相机图像显示在一个名为cam的窗口中，按下Esc键退出程序
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string> // 用于图像上标注字符串
#include <chrono>  // 用于帧率计算的时间戳获取
#include <iomanip> // 包含 setprecision 所需的头文件
#include "test.hpp"

int test_cam_disp(void) {
    /*打开相机并设置分辨率*/
    cv::VideoCapture cam(CAM_INDEX); // 打开相机
    if ( !cam.isOpened() ) { // 如果无法打开相机
        std::cerr << "ERROR: 无法打开摄像头！" << std::endl;
        return -1;
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH); // 设置宽度
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT); // 设置高度
    std::cout << cam.get(cv::CAP_PROP_FRAME_WIDTH) << "*" << cam.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    /*创建窗口用于图像显示*/
    cv::namedWindow("CamDisp", cv::WINDOW_AUTOSIZE); // 创建一个窗口

    /*初始化帧数和时间戳*/
    int frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();
    double fps = 0;

    /*图像矩阵定义*/
    cv::Mat frame;

    while (true) { // 开始循环
        /*帧率计算*/
        frameCount++; // 更新帧数
        auto currentTime = std::chrono::steady_clock::now(); // 获取当前时间
        double elapsedTime = std::chrono::duration<double>(currentTime - startTime).count(); // 计算时间差
        if (elapsedTime >= 1.0) { // 每秒更新一次帧率
            fps = frameCount / elapsedTime; // 计算帧率
            frameCount = 0; startTime = currentTime; // 重置帧数和时间戳
        }
        std::stringstream ss_fps; // 创建一个字符串流
        ss_fps << "FPS:" << std::fixed << std::setprecision(2) << fps; // FPS:xx.xx

        /*图像获取*/
        bool ret = cam.read(frame); // 从相机获取新的一帧
        if (!ret) { // 如果没有图像
            std::cout << "ERROR: 无法获取摄像头画面！" << std::endl;
            break;
        }

        /*图像操作*/
        //cv::putText(frame, ss_fps.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2); // 写上帧率值

        /*显示图像*/
        cv::imshow("CamDisp", frame); // 在对应窗口中显示图像

        /*检测按键*/
        int KeyGet = cv::waitKey(1); // 等待1ms获取按键值
        if (KeyGet == KEY_ESC) break; // 如果按下Esc的话，退出循环
        else if (KeyGet == KEY_s || KeyGet == KEY_S) { // 按'S'键保存当前图像
            static int jpg_save_index = 0;
            std::string jpg_name = std::to_string(jpg_save_index++) + ".jpg";
            std::vector<int> compression_params;
            compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
            compression_params.push_back(95); // 这里设置质量为95
            cv::imwrite(jpg_name, frame, compression_params);
            std::cout << "已保存 " << jpg_name << std::endl;
        }
    }

    cam.release(); // 释放相机资源
    std::cout << "相机资源已释放" << std::endl; // 添加日志来跟踪资源释放
    cv::destroyAllWindows(); // 关闭所有OpenCV创建的窗口
    std::cout << "所有窗口已关闭" << std::endl; // 添加日志来跟踪窗口关闭

    return 0;
}
