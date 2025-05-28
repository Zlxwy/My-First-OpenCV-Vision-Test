// fill the small region on binary image 填充二值图像中的小面积区域
// 按英文逗号键切换上一张图像，按英文句号键切换下一张图像，按Esc键退出程序
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "test.hpp"


#define TRACK_PIC_NUM  25 // 几张图片
int test_fill_bin_small_region(void) {
    cv::namedWindow("缩放后的彩色图", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("原二值化图像", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("处理后的二值化图像", cv::WINDOW_NORMAL); // 创建一个窗口
    int pic_index = 0;
    while (true) {
        std::string filename = "./saideao/" + std::to_string(pic_index) + ".jpg";
        cv::Mat frame = cv::imread(filename); // 读取一张赛道图片

        if (frame.empty()) { // 如果没读取到
            std::cout << "Could not read the frame: " << filename << std::endl; // 打印未读取信息
            return -1;
        }

        cv::Mat resizedFrame; // 原图 > 缩放图像
        cv::Mat grayFrame; // 缩放图像 > 灰度图像
        cv::Mat binFrame; // 灰度图像 > 二值图像
        //double th = bgr2bin(frame, resizedFrame, grayFrame, binFrame, cv::Size(RESIZED_WIDTH, RESIZED_HEIGHT)); // OTSU自动阈值
        double th = bgr2bin(frame, resizedFrame, grayFrame, binFrame, cv::Size(RESIZED_WIDTH, RESIZED_HEIGHT), GRAY2BIN_THRESH, 255); // 自定义阈值
        cv::imshow("原二值化图像", binFrame); // 在对应窗口中显示图像

        // 反转小面积黑白区域方法1
        // invertSmallRegion_1(binFrame, MIN_AREA_THRESH); // 反转面积小于50的轮廓

        // 反转小面积黑白区域方法2
        invertSmallRegion_2(binFrame, MIN_AREA_THRESH); // 反转面积小于50的轮廓

        cv::imshow("缩放后的彩色图", resizedFrame); // 在对应窗口中显示图像
        cv::imshow("处理后的二值化图像", binFrame); // 在对应窗口中显示图像

        clearScreen();
        std::cout << "阈值：" << th << std::endl;
        std::cout << "图片：" << pic_index << ".jpg" << std::endl;
        //====================================================================================================|

        int KeyGet = cv::waitKey(0);
        if (KeyGet == KEY_ESC) break;
        else if ((KeyGet == 44 && pic_index == 0) || (KeyGet == 46 && pic_index == TRACK_PIC_NUM - 1))pic_index = pic_index;
        else if (KeyGet == 46) pic_index++;
        else if (KeyGet == 44) pic_index--;
        else continue;
    }

    return 0;
}

/*这是一个在黑色大块中寻找白色小块的函数，会将得到的白色小块的中心坐标点存入到vector容器centers中*/
void getWhiteCentersOfSmallRegions(const cv::Mat& binImg, double minArea, std::vector<cv::Point>& centers) {
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binImg, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 查找轮廓
    for (auto& contour : contours) { // 遍历所有轮廓
        double area = cv::contourArea(contour); // 计算轮廓的面积
        if (area < minArea) {  // 如果面积小于最小面积阈值，则计算中心点
            cv::Moments m = cv::moments(contour);
            int cx = static_cast<int>(m.m10 / m.m00);
            int cy = static_cast<int>(m.m01 / m.m00);
            centers.push_back(cv::Point(cx, cy));
        }
    }
}

/*这是一个在白色大块中寻找黑色小块的函数，会将得到的黑色小块的中心坐标点存入到vector容器centers中*/
void getBlackCentersOfSmallRegions(const cv::Mat& binImg, double minArea, std::vector<cv::Point>& centers) {
    cv::Mat binInvImg; // 原二值图像 > 二值图像翻转图像
    cv::bitwise_not(binImg, binInvImg); // 二值图像翻转
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binInvImg, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // 查找轮廓
    for (auto& contour : contours) { // 遍历所有轮廓
        double area = cv::contourArea(contour); // 计算轮廓的面积
        if (area < minArea) {  // 如果面积小于最小面积阈值，则计算中心点
            cv::Moments m = cv::moments(contour);
            int cx = static_cast<int>(m.m10 / m.m00);
            int cy = static_cast<int>(m.m01 / m.m00);
            centers.push_back(cv::Point(cx, cy));
        }
    }
}

/*洪水填充二值图像某一区域的颜色，就像Windows画图软件的倒墨水*/
void fillBlobRecursive(cv::Mat& binImg, cv::Point pt, uint8_t newColor) {
    if (pt.x < 0 || pt.x >= binImg.cols || pt.y < 0 || pt.y >= binImg.rows) return; // 检查坐标是否超出图像边界
    uint8_t currentColor = binImg.at<uint8_t>(pt); // 获取当前像素点的灰度值
    if (currentColor == newColor) return; // 如果当前像素点已经是新颜色，或者不是旧颜色，则返回
    binImg.at<uint8_t>(pt) = newColor; // 将当前像素点更换为指定的新颜色

    /*递归调用，填充上下左右四个方向的像素点*/
    fillBlobRecursive(binImg, cv::Point(pt.x + 1, pt.y), newColor);
    fillBlobRecursive(binImg, cv::Point(pt.x - 1, pt.y), newColor);
    fillBlobRecursive(binImg, cv::Point(pt.x, pt.y + 1), newColor);
    fillBlobRecursive(binImg, cv::Point(pt.x, pt.y - 1), newColor);
}

/*洪水填充彩色图某一区域的颜色，就像Windows画图软件的倒墨水*/
// 注：对于jpg格式，其压缩保存有时候会使一部分像素的RGB通道变化，导致洪水填充并不能取得较好的预期效果，这个彩色填充少点使用。
void fillBlobRecursive(cv::Mat& bgrImg, cv::Point pt, cv::Vec3b newColor, cv::Vec3b oldColor) {
    if (pt.x < 0 || pt.x >= bgrImg.cols || pt.y < 0 || pt.y >= bgrImg.rows) return;
    cv::Vec3b currentColor = bgrImg.at<cv::Vec3b>(pt);
    if (currentColor == newColor || currentColor != oldColor) return;
    bgrImg.at<cv::Vec3b>(pt) = newColor;
    /*递归调用，填充上下左右四个方向的像素点*/
    fillBlobRecursive(bgrImg, cv::Point(pt.x + 1, pt.y), newColor, oldColor);
    fillBlobRecursive(bgrImg, cv::Point(pt.x - 1, pt.y), newColor, oldColor);
    fillBlobRecursive(bgrImg, cv::Point(pt.x, pt.y + 1), newColor, oldColor);
    fillBlobRecursive(bgrImg, cv::Point(pt.x, pt.y - 1), newColor, oldColor);
}

/*反转二值图像中面积小于指定值的独立区域*/
// 方法一：检测二值图像中面积小于指定值的独立区域的中心坐标，然后使用洪水填充的方法将区域的颜色反转
void invertSmallRegion_1(cv::Mat& binImg, double minArea) {
    std::vector<cv::Point> centerWhite; // 用来存储面积小于某一阈值的白色区域的中心坐标值
    std::vector<cv::Point> centerBlack; // 用来存储面积小于某一阈值的黑色区域的中心坐标值
    getWhiteCentersOfSmallRegions(binImg, minArea, centerWhite); // 获取面积小于50的白色区域的中心坐标值
    getBlackCentersOfSmallRegions(binImg, minArea, centerBlack); // 获取面积小于50的黑色区域的中心坐标值
    for (auto& cenW : centerWhite)  fillBlobRecursive(binImg, cenW, BIN_BLACK); // 使用这些中心坐标值，洪水填充这些白色区域
    for (auto& cenB : centerBlack)  fillBlobRecursive(binImg, cenB, BIN_WHITE); // 使用这些中心坐标值，洪水填充这些黑色区域
}

/*反转二值图像中面积小于指定值的独立区域*/
// 方法二：直接使用OpenCV的findContours函数来查找轮廓，然后使用drawContours函数来反转颜色
void invertSmallRegion_2(cv::Mat& binImg, double minArea) {
    /*来自AI Copilot GPT-4o*/
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(binImg, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    for (size_t i = 0; i < contours.size(); i++) { // 遍历每个轮廓
        double area = cv::contourArea(contours[i]);
        if (area < minArea) { // 如果面积小于阈值，反转颜色
            cv::drawContours(binImg, contours, static_cast<int>(i), cv::Scalar(255-binImg.at<uint8_t>(contours[i][0])), cv::FILLED);
        }
    }
}