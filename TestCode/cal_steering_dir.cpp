// calculate steering direction 计算转向方向
// calculate steering angle 计算转向角度
// 在得到循迹线后，计算转向的方向，在程序track_rcgn_animated的基础上继续解析相机获取的图像
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <numeric> // 用于 std::accumulate ，容器计算元素总和
#include "test.hpp"

#define UP_LINE_INDEX  50
#define DOWN_LINE_INDEX 70

int test_cal_steering_dir(void) {
    /*打开相机并设置分辨率*/
    cv::VideoCapture cam(CAM_INDEX); // 打开相机
    if (!cam.isOpened()) { // 如果无法打开相机
        std::cerr << "ERROR: 无法打开摄像头！" << std::endl;
        return -1;
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH); // 设置宽度
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT); // 设置高度
    std::cout << cam.get(cv::CAP_PROP_FRAME_WIDTH) << "*" << cam.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    /*创建窗口用于图像显示*/
    cv::namedWindow("RGB Picture", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("BIN Picture", cv::WINDOW_NORMAL); // 创建一个窗口

    /*初始化帧数和时间戳*/
    int frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();
    double fps = 0;

    /*图像矩阵定义*/
    cv::Mat frame;
    cv::Mat resizedFrame;
    cv::Mat grayFrame;
    cv::Mat binFrame;

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


        /*图像转换*/
        cv::resize(frame, resizedFrame, cv::Size(RESIZED_WIDTH, RESIZED_HEIGHT));
        cv::cvtColor(resizedFrame, grayFrame, cv::COLOR_BGR2GRAY); // 将原始图像转换为灰度图
        cv::threshold(grayFrame, binFrame, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);


        /*图像检测1*/
        std::vector<cv::Point> leftEdgePoints, rightEdgePoints;  // 定义两个容器来存储左右边缘点
        findEdgePix(binFrame, leftEdgePoints, rightEdgePoints); // 查找二值图像的左右边界像素点坐标，存入两个容器中


        /*图像操作*/
        size_t min_dop_num = std::min<size_t>(leftEdgePoints.size(), rightEdgePoints.size()); // 获取这两个容器的元素数量最小值
        std::vector<cv::Point> midPoint; // 用于收集两条边线之间的中点坐标值
        for (size_t i = 0; i < min_dop_num; i++) {
            midPoint.push_back( calMidPoint(leftEdgePoints.at(i),rightEdgePoints.at(i)) ); // 加入这个中点
            resizedFrame.at<cv::Vec3b>(leftEdgePoints.at(i)) = cv::Vec3b(255, 0, 0); // 画左边线
            resizedFrame.at<cv::Vec3b>(midPoint.back()) = cv::Vec3b(0, 255, 0); // 画左右边线之间的中点线，即循迹线
            resizedFrame.at<cv::Vec3b>(rightEdgePoints.at(i)) = cv::Vec3b(0, 0, 255); // 画右边线
        }
        cv::putText(resizedFrame, ss_fps.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2); // 写上帧率值

        /*图像检测2*/
        clearScreen();
        double xl = calAverSlopeFromRowToRow(resizedFrame, midPoint, UP_LINE_INDEX, DOWN_LINE_INDEX);
        std::cout << "斜率：" << xl << std::endl;
        // 如果是左转则显示L，右转则显示R，直行则显示S
        std::stringstream LRS_ss;
        cv::Scalar color;
        if (xl <= -0.2f) LRS_ss << "R:" << std::fixed << std::setprecision(2) << std::abs(xl), color = cv::Scalar(0, 0, 255); // R:x.xx
        else if (xl >= +0.2f) LRS_ss << "L:" << std::fixed << std::setprecision(2) << xl, color = cv::Scalar(255, 0, 0); // L:x.xx
        else LRS_ss << "Go", color = cv::Scalar(0,255,0); // Go
        cv::putText(resizedFrame, LRS_ss.str(), cv::Point(0, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, color, 2); // 显示方向



        /*图像显示*/
        cv::imshow("RGB Picture", resizedFrame); // 在对应窗口中显示图像
        cv::imshow("BIN Picture", binFrame); // 在对应窗口中显示图像



        /*检测按键*/
        int KeyGet = cv::waitKey(1); // 等待1ms获取按键值
        if (KeyGet == KEY_ESC) break; // 如果按下Esc的话，退出循环
    }

    cam.release(); // 释放相机资源
    std::cout << "相机资源已释放" << std::endl; // 添加日志来跟踪资源释放
    cv::destroyAllWindows(); // 关闭所有OpenCV创建的窗口
    std::cout << "所有窗口已关闭" << std::endl; // 添加日志来跟踪窗口关闭
    return 0;
}

/*在图像上的指定行画一条直线*/
void drawLineOnRow(cv::Mat& img, int rowIndex, cv::Scalar color, int thickness) {
    cv::line(img, cv::Point(0, rowIndex), cv::Point(img.cols - 1, rowIndex), color, thickness);
}

/*计算一个double类型容器的平均值*/
// 计算过程
// 8 2 7
// (0 * 0 + 8) / 1 = 8
// (8 * 1 + 2) / 2 = 5
// (5 * 2 + 7) / 3 ≈ 5.67
// 变化后
// 0 / 1 * 0 + 8 / 1 = 8
// 8 / 2 * 1 + 2 / 2 = 5
// 5 / 3 * 2 + 7 / 3 ≈ 5.67
// 这样可以防止加的总和过大，导致数据超出类型表示范围。
double myCalAverage(const std::vector<double> vdata) {
    if ( vdata.empty() ) return 0.0f; // 返回默认构造的值，对于浮点数是 0.0，对于整数也是 0
    else {
        double cnt=0, numMark=0;
        for (auto& v : vdata) {
            numMark = numMark / (cnt + 1) * cnt + v / (cnt + 1); // 先除后乘是为了防止数据溢出
            cnt += 1;
        }
        return numMark;
    }
}

template <typename T>
T calAverage(const std::vector<T> vdata) {
    if ( vdata.empty() ) return T(); // 如果vector容器是空的，返回0
    else return ( std::accumulate(vdata.begin(), vdata.end(), T()) / (T)vdata.size() );
}

// Calculate average slope from row to row
// 计算两行之间中线点的平均斜率，以确定车向哪一边转弯
double calAverSlopeFromRowToRow(cv::Mat& img, std::vector<cv::Point> midPoint, int row_up, int row_down) {
    drawLineOnRow(img, row_up, cv::Scalar(BGR_ICEBLUE), 1); // 在第row_up行画一条厚度为1的直线
    drawLineOnRow(img, row_down, cv::Scalar(BGR_ICEBLUE), 1); // 在第row_down行画一条厚度为1的直线
    cv::Point pointMark(0, 0); // 用于记录前一次的坐标值，与当前坐标值进行计算得出斜率
    bool isFirst = true; // 用于在第一次进入的时候，只记录值而不计算
    std::vector<double> slope; // 用于记录斜率
    for (auto& pt : midPoint) { // 遍历循迹线的像素点
        if (pt.y > row_up && pt.y < row_down) { // 如果在row_up行到row_down行之间
            if (isFirst) { // 如果是第一次进来
                pointMark.x = pt.x; // 记录一下x坐标
                pointMark.y = pt.y; // 记录一下y坐标
                isFirst = false; // 之后就不是第一次进来了
            }
            else { // 如果不是第一次进来了
                slope.push_back((double)(pt.x - pointMark.x) / (double)(pt.y - pointMark.y)); // 计算出斜率，并加入容器
                pointMark.x = pt.x; // 更新记录x坐标
                pointMark.y = pt.y; // 更新记录y坐标
            }
            img.at<cv::Vec3b>(pt) = cv::Vec3b(BGR_BLACK); // 将算过的像素点变黑
        }
        else continue; // 如果不在行之间，则循环到下一个点（其实这条语句加不加都没关系，本来就要下一次循环了）
    }
    return calAverage(slope);
}

// Calculate average x coordinate from row to row
// 计算两行之间中线点的x坐标平均值，也作为车向哪一边转弯的一个影响因素
double calAverXCoordFromRowToRow(cv::Mat& img, std::vector<cv::Point> midPoint, int row_up, int row_down) {
    drawLineOnRow(img, row_up, cv::Scalar(BGR_ICEBLUE), 1); // 在第row_up行画一条厚度为1的直线
    drawLineOnRow(img, row_down, cv::Scalar(BGR_ICEBLUE), 1); // 在第row_down行画一条厚度为1的直线
    cv::line(img, cv::Point((img.cols-1)/2, row_up), cv::Point((img.cols - 1) / 2, row_up-10), cv::Scalar(BGR_ICEBLUE), 1); // 在中间画一条线
    cv::line(img, cv::Point((img.cols-1)/2, row_down), cv::Point((img.cols - 1) / 2, row_down+10), cv::Scalar(BGR_ICEBLUE), 1); // 在中间画一条线
    std::vector<double> xCoord; // 用于记录x坐标
    for (auto& pt: midPoint) {
        if (pt.y > row_up && pt.y < row_down) {
            xCoord.push_back(pt.x); // 记录x坐标
        }
        else continue; // 如果不在行之间，则循环到下一个点（其实这条语句加不加都没关系，本来就要下一次循环了）
    }
    return calAverage(xCoord);
}