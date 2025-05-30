// track recognition for static images 静态图像的轨迹识别
// 按英文逗号键切换上一张，按英文句号键切换下一张，按英文斜杆键切换另一组照片
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include "test.hpp"

#define PIC_FILE_NUM 2 // 赛道图片文件夹的数量
#define OBSTACLES_PIC_NUM  20 // 几张图片
int test_avoid_obstacles(void) {
    cv::namedWindow("缩放后的彩色图", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("原二值化图像", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("处理后的二值化图像", cv::WINDOW_NORMAL); // 创建一个窗口
    int pic_file = 0, pic_index = 0;
    while (true) {
        std::string filename = "./images/flag_zhangai/" + std::to_string(pic_index) + ".jpg";
        //std::string filename = "./saideao_0/" + std::to_string(pic_index) + ".jpg";
        cv::Mat frame = cv::imread(filename); // 读取一张赛道图片

        if (frame.empty()) { // 如果没读取到
            std::cout << "Could not read the frame: " << filename << std::endl; // 打印未读取信息
            return -1;
        }

        cv::Mat resizedFrame; // 原图 > 缩放图像
        cv::Mat grayFrame; // 缩放图像 > 灰度图像
        cv::Mat binFrame; // 灰度图像 > 二值图像
        double th = bgr2bin(frame, resizedFrame, grayFrame, binFrame, cv::Size(RESIZED_WIDTH, RESIZED_HEIGHT), GRAY2BIN_THRESH, 255); // 自定义阈值
        cv::imshow("原二值化图像", binFrame); // 在对应窗口中显示图像
        // drawLeftRightBlackBorder(binFrame); // 在二值图像的四周画黑线

        // 反转小面积黑白区域方法1
        // invertSmallRegion_1(binFrame, MIN_AREA_THRESH); // 反转面积小于MIN_AREA_THRESH的轮廓

        // 反转小面积黑白区域方法2
        // invertSmallRegion_2(binFrame, MIN_AREA_THRESH); // 反转面积小于MIN_AREA_THRESH的轮廓

        // 使用一种滤波算法处理二值化图像
        binImgFilter(binFrame);

        // 边线识别=================================================================================================
        std::vector<cv::Point> leftEdgePoints, rightEdgePoints;  // 定义两个容器来存储寻找到的左右边缘点的坐标值
        std::vector<cv::Point> leftEdgePointsFiltered, rightEdgePointsFiltered; // 定义两个容器来存储寻找到的左右边缘点对x坐标值滤波后的坐标值
        findEdgePix(binFrame, leftEdgePoints, rightEdgePoints); // 查找二值图像的左右边界像素点坐标，存入两个容器中
        filterXCoord(leftEdgePoints, leftEdgePointsFiltered, 25); // 对左边界点的x坐标值进行滤波处理
        filterXCoord(rightEdgePoints, rightEdgePointsFiltered, 25); // 对右边界点的x坐标值进行滤波处理

        size_t min_dop_num = std::min<size_t>(leftEdgePointsFiltered.size(), rightEdgePointsFiltered.size()); // 获取这两个容器的元素数量最小值
        std::vector<cv::Point> midPoint; // 用于收集两条边线之间的中点坐标值
        for (size_t i = 0; i < min_dop_num; i++) {
            midPoint.push_back(calMidPoint(leftEdgePointsFiltered.at(i), rightEdgePointsFiltered.at(i))); // 加入这个中点
            resizedFrame.at<cv::Vec3b>(leftEdgePointsFiltered.at(i)) = cv::Vec3b(BGR_BLUE); // 画左边线
            resizedFrame.at<cv::Vec3b>(rightEdgePointsFiltered.at(i)) = cv::Vec3b(BGR_RED); // 画右边线
        }
        std::vector<cv::Point> midPointFiltered;
        filterXCoord(midPoint, midPointFiltered, 7); // 对中点进行滤波
        for (auto& pt : midPointFiltered) { // 遍历中点
            resizedFrame.at<cv::Vec3b>(pt) = cv::Vec3b(BGR_PURPLE); // 画左右边线之间的滤波后的中点线，即循迹线
        }
        double xl = calAverSlopeFromRowToRow(resizedFrame, midPointFiltered, ROW_UP, ROW_DOWN); // 计算中线的偏斜
        double py = calAverXCoordFromRowToRow(resizedFrame, midPoint, ROW_UP, ROW_DOWN); // 计算X坐标平均值

        cv::imshow("缩放后的彩色图", resizedFrame); // 在对应窗口中显示图像
        cv::imshow("处理后的二值化图像", binFrame); // 在对应窗口中显示图像

        clearScreen();
        std::cout << "图片：saideao" << pic_file << "/" << pic_index << ".jpg" << std::endl;
        std::cout << "阈值：" << th << std::endl;
        std::cout << "偏斜：" << xl << std::endl;
        std::cout << "偏移：" << (resizedFrame.cols - 1) / 2 - py << std::endl;
        std::cout << "转向：" << ((xl < -0.2) ? "向右" : ((xl > 0.2) ? "向左" : "直走")) << std::endl;




        int KeyGet = cv::waitKey(0);
        if (KeyGet == KEY_ESC) break;
        else if (KeyGet == KEY_COMMA) {
            if (pic_index == 0) pic_index = pic_index;
            else pic_index--;
        }
        else if (KeyGet == KEY_DOT) {
            if (pic_index == OBSTACLES_PIC_NUM - 1) pic_index = pic_index;
            else pic_index++;
        }
        else continue;
    }

    return 0;
}