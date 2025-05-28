// track recognition for static images 静态图像的轨迹识别
// 按英文逗号键切换上一张，按英文句号键切换下一张，按英文斜杆键切换另一组照片
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include "test.hpp"

#define PIC_FILE_NUM 2 // 赛道图片文件夹的数量
#define TRACK_PIC_NUM  20 // 几张图片
int test_track_rcgn_static(void) {
    cv::namedWindow("缩放后的彩色图", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("原二值化图像", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("处理后的二值化图像", cv::WINDOW_NORMAL); // 创建一个窗口
    int pic_file=0, pic_index=0;
    while (true) {
        std::string filename = "./images/saideao_" + std::to_string(pic_file) + "/" + std::to_string(pic_index) + ".jpg";
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

        // 使用一种滤波算法
        binImgFilter(binFrame);

        // 边线识别：在同一行中，要连续识别到多个相反像素，才能认定为边线===============================================
        std::vector<cv::Point> leftEdgePoints, rightEdgePoints;  // 定义两个容器来存储左右边缘点
        findEdgePix(binFrame, leftEdgePoints, rightEdgePoints); // 查找二值图像的左右边界像素点坐标，存入两个容器中

        size_t min_dop_num = std::min<size_t>(leftEdgePoints.size(), rightEdgePoints.size()); // 获取这两个容器的元素数量最小值
        std::vector<cv::Point> midPoint; // 用于收集两条边线之间的中点坐标值
        for (size_t i = 0; i < min_dop_num; i++) {
            midPoint.push_back(calMidPoint(leftEdgePoints.at(i), rightEdgePoints.at(i))); // 加入这个中点
            resizedFrame.at<cv::Vec3b>(leftEdgePoints.at(i)) = cv::Vec3b(BGR_BLUE); // 画左边线
            // resizedFrame.at<cv::Vec3b>(midPoint.back()) = cv::Vec3b(BGR_GREEN); // 画左右边线之间的中点线，即循迹线
            resizedFrame.at<cv::Vec3b>(rightEdgePoints.at(i)) = cv::Vec3b(BGR_RED); // 画右边线
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
        std::cout << "图片：saideao" <<pic_file << "/" << pic_index << ".jpg" << std::endl;
        std::cout << "阈值：" << th << std::endl;
        std::cout << "偏斜：" << xl << std::endl;
        std::cout << "偏移：" << (resizedFrame.cols-1)/2-py << std::endl;
        std::cout << "转向：" << ((xl < -0.2) ? "向右" : ((xl > 0.2) ? "向左" : "直走")) << std::endl;




        int KeyGet = cv::waitKey(0);
        if (KeyGet == KEY_ESC) break;
        else if ((KeyGet == 47) && (pic_file == PIC_FILE_NUM - 1)) pic_file = 0;
        else if ((KeyGet == 44 && pic_index == 0) || (KeyGet == 46 && pic_index == TRACK_PIC_NUM - 1))pic_index = pic_index;
        else if (KeyGet == 47) pic_file++;
        else if (KeyGet == 46) pic_index++;
        else if (KeyGet == 44) pic_index--;
        else continue;
    }

    return 0;
}

/**
 * @brief 在二值图像四周绘制一个厚度为1的黑色矩形边框
 * @param binImg 二值图像，类型为CV_8UC1
 */
void drawBlackRectBorder(cv::Mat& binImg) {
    if (binImg.type() != CV_8UC1) { // 检查图像是否为二值图像
        std::cerr << "Image is not a binary image!" << std::endl;
        return;
    }
    // 获取图像的尺寸
    int rows = binImg.rows;
    int cols = binImg.cols;
    for (int j=0; j<cols; ++j) { // 在图像的顶部和底部画黑线
        binImg.at<uint8>(0,j) = 0;             // 顶部
        binImg.at<uint8>(rows-1,j) = 0;     // 底部
    }
    for (int i=0; i<rows; ++i) { // 在图像的左侧和右侧画黑线
        binImg.at<uint8>(i,0) = 0;             // 左侧
        binImg.at<uint8>(i,cols-1) = 0;     // 右侧
    }
}

/**
 * @brief 对点容器中的元素的x坐标进行滤波
 * @note 滤波算法：在窗口内，对x坐标求平均
 * @param points 输入的点容器
 * @param filteredPoints 输出的滤波后的点容器
 * @param windowSize 滤波窗口大小，最好是奇数
 */
void filterXCoord(std::vector<cv::Point>& points, std::vector<cv::Point>& filteredPoints, int windowSize) {
    if (points.empty()) return; // 如果点容器为空，直接返回
    filteredPoints.clear(); // 清空输出的点容器
    int halfWindow = windowSize / 2; // 窗口的一半大小
    size_t pointsSize = points.size(); // 点容器的大小
    for (size_t i=0; i<pointsSize; i++) {
        int start = std::max(0, (int)i-halfWindow); // 窗口起始位置
        int end = std::min((int)pointsSize-1, (int)i+halfWindow); // 窗口结束位置
        int xFiltered = std::accumulate(
            points.begin() + start,
            points.begin() + end,
            0, [](int sum, const cv::Point& pt) { return (sum + pt.x); }
        ) / (end-start); // 计算平均值
        filteredPoints.push_back(cv::Point(xFiltered, points[i].y)); // 将平均值作为新的点加入输出容器
    }
}