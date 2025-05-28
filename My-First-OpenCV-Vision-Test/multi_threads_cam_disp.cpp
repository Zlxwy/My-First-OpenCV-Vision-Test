// Multi Threads for camera display 图像获取显示的多线程程序
// 单独开一个线程获取相机的图像，在主线程操作并显示图像
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string> // 用于图像上标注字符串
#include <chrono>  // 用于帧率计算的时间戳获取
#include <memory> // 用于智能指针等内存管理
#include <atomic> // 用于原子变量
#include <iomanip> // 包含 setprecision 所需的头文件
#include <thread> 
#include "test.hpp"

std::atomic<bool> shouldExit(false); // 用于线程退出
std::shared_ptr< cv::VideoCapture> cam;

/*初始化帧数和时间戳*/
int frameCount = 0;
auto startTime = std::chrono::steady_clock::now();;
double frameFPS = 0;

/*图像矩阵定义*/
cv::Mat frame(CAM_HEIGHT, CAM_WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));
volatile bool hasGetFrame = false;

/*调用这个函数退出所有线程*/
void exitAllThreads(void) { shouldExit.store(true); }

/*此线程获取图像（存放于frame），并计算帧率（存放于frameFPS）*/
void getFrame(void) {
    std::cout << "已进入 getFrame 线程" << std::endl;
    while ( !shouldExit.load() ) {
        /*帧率计算*/
        frameCount++; // 更新帧数
        auto currentTime = std::chrono::steady_clock::now(); // 获取当前时间
        double elapsedTime = std::chrono::duration<double>(currentTime - startTime).count(); // 计算时间差
        if (elapsedTime >= 1.0) { // 每秒更新一次帧率
            frameFPS = frameCount / elapsedTime; // 计算帧率
            frameCount = 0; startTime = currentTime; // 重置帧数和时间戳
        }

        /*图像获取*/
        hasGetFrame = cam->read(frame); // 从相机获取新的一帧
        if (!hasGetFrame) { // 如果没有图像
            std::cout << "ERROR: 无法获取摄像头画面！" << std::endl;
            frame = cv::Mat(CAM_HEIGHT, CAM_WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));
        }
    }
    std::cout << "线程 getFrame 已退出。" << std::endl;
}

int test_multi_threads_cam_disp(void) {
    /*打开相机并设置分辨率*/
    cam = std::make_shared<cv::VideoCapture>(CAM_INDEX); // 打开相机
    if ( !cam->isOpened() ) { // 如果无法打开相机
        std::cerr << "ERROR: 无法打开摄像头！" << std::endl;
        return -1;
    }
    cam->set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH); // 设置宽度
    cam->set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT); // 设置高度
    std::cout << cam->get(cv::CAP_PROP_FRAME_WIDTH) << "*" << cam->get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    // 创建三个线程，分别用于获取帧、操作帧和显示帧
    std::shared_ptr<std::thread> thread_get_frame = std::make_shared<std::thread>(getFrame);

    while (true) { // 开始循环
        cv::Mat frame_clone = frame.clone();
        std::stringstream ss_text; // 创建一个字符串流
        if (!hasGetFrame) ss_text << "No Image!";
        else ss_text << "FPS:" << std::fixed << std::setprecision(2) << frameFPS; // FPS:xx.xx
        cv::putText(frame_clone, ss_text.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        /*创建窗口用于图像显示*/
        cv::namedWindow("CamDisp", cv::WINDOW_NORMAL); // 创建一个窗口
        cv::imshow("CamDisp", frame_clone); // 在对应窗口中显示图像

        /*检测按键*/
        int KeyGet = cv::waitKey(1); // 等待1ms获取按键值
        if (KeyGet == KEY_ESC) {
            exitAllThreads(); // 告诉线程需要退出了
            break; // 如果按下Esc的话，退出循环
        }
    }

    /*阻塞等待线程退出后，再释放所有资源*/
    if (thread_get_frame->joinable()) thread_get_frame->join();

    cam->release(); // 释放相机资源
    std::cout << "相机资源已释放" << std::endl; // 添加日志来跟踪资源释放
    cv::destroyAllWindows(); // 关闭所有OpenCV创建的窗口
    std::cout << "所有窗口已关闭" << std::endl; // 添加日志来跟踪窗口关闭

    std::cout << "主线程退出，程序已完全关闭。" << std::endl;

    return 0;
}
