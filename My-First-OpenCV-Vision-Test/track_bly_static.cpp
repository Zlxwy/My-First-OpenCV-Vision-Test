// track 八领域 for static images 使用八领域算法对静态图像进行轨迹识别（感觉效果不太好）
// 参考自B站up主“村东头无敌的小瞎子”(uid:385282905)的程序
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include "test.hpp"

#define PIC_FILE_NUM 2 // 赛道图片文件夹的数量
#define TRACK_PIC_NUM  20 // 几张图片
int blue=0, green=0, red=0;
int test_track_bly_static(void) {
    cv::namedWindow("缩放后的彩色图", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("原二值化图像", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("处理后的二值化图像", cv::WINDOW_NORMAL); // 创建一个窗口
    int pic_file = 0, pic_index = 0;
    while (true) {
        std::string filename = "./saideao_" + std::to_string(pic_file) + "/" + std::to_string(pic_index) + ".jpg";
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

        binImgFilter(binFrame); // 对二值图像进行滤波
        binImgDrawRect(binFrame); // 在二值图像画一个边框

        // 反转小面积黑白区域方法1
        // invertSmallRegion_1(binFrame, MIN_AREA_THRESH); // 反转面积小于MIN_AREA_THRESH的轮廓

        // 反转小面积黑白区域方法2
        // invertSmallRegion_2(binFrame, MIN_AREA_THRESH); // 反转面积小于MIN_AREA_THRESH的轮廓

        // 边线识别
        std::vector<cv::Point> leftEdgePoints, rightEdgePoints; // 存储边线点坐标的容器
        std::vector<int> leftEdgePointDir, rightEdgePointDir; // 存储边线点坐标生长方向的容器
        blyfindEdgePoint(binFrame, leftEdgePoints, rightEdgePoints, leftEdgePointDir, rightEdgePointDir, binFrame.rows, binFrame.rows);

        // 画轨迹
        for (auto& pt: leftEdgePoints) blue = blue==255?0:blue+1,resizedFrame.at<cv::Vec3b>(pt) = cv::Vec3b(blue,green,red);
        for (auto& pt: rightEdgePoints) resizedFrame.at<cv::Vec3b>(pt) = cv::Vec3b(BGR_RED);



        cv::imshow("缩放后的彩色图", resizedFrame); // 在对应窗口中显示图像
        cv::imshow("处理后的二值化图像", binFrame); // 在对应窗口中显示图像

        clearScreen();
        std::cout << "阈值：" << th << std::endl;
        std::cout << "图片：saideao" << pic_file << "/" << pic_index << ".jpg" << std::endl;
        //====================================================================================================|




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
 * @brief 八领域查找
 * @param binImg 输入二值化图像
 * @param leftEdgePoints 输出左边界点坐标
 * @param rightEdgePoints 输出右边界点坐标
 * @param leftEdgePointDir 输出左边界点的生长方向，其下标和leftEdgePoints是对应的，比如leftEdgePoints[0]的生长方向就是leftEdgePointDir[0]，
 *                         但在最后一个点的时候，已经没有下一个点了，也就没有生长方向了，所以leftEdgePointDir的最后一个元素是无效的。
 * @param rightEdgePointDir 输出右边界点的生长方向，其下标和rightEdgePoints是对应的，比如rightEdgePoints[0]的生长方向就是rightEdgePointDir[0]
 *                          但在最后一个点的时候，已经没有下一个点了，也就没有生长方向了，所以rightEdgePointDir的最后一个元素是无效的。
 * @param leftEdgePointsNum 需要查找左边界点的个数
 * @param rightEdgePointsNum 需要查找右边界点的个数
 */
void blyfindEdgePoint(cv::Mat& binImg,
                       std::vector<cv::Point>& leftEdgePoints, std::vector<cv::Point>& rightEdgePoints,
                       std::vector<int>& leftEdgePointDir, std::vector<int>& rightEdgePointDir,
                       int leftEdgePointsNum, int rightEdgePointsNum) {
    /*清空所有容器*/
    leftEdgePoints.erase(leftEdgePoints.begin(), leftEdgePoints.end());
    rightEdgePoints.erase(rightEdgePoints.begin(), rightEdgePoints.end());
    leftEdgePointDir.erase(leftEdgePointDir.begin(), leftEdgePointDir.end());
    rightEdgePointDir.erase(rightEdgePointDir.begin(), rightEdgePointDir.end());

    /*在二值图像外围画一个厚度为2的矩形边框，这是为了在八领域识别时不会超出图像边界*/
    // cv::rectangle(binImg, cv::Rect(0,0,binImg.cols,binImg.rows),cv::Scalar(BIN_BLACK), 1);
    // cv::rectangle(binImg, cv::Rect(1,1,binImg.cols-2,binImg.rows-2),cv::Scalar(BIN_BLACK), 1);
    
    /*先查找一个基础起始点，从中间往两边查找的，带有怀疑：必须要连续查找到多个黑点才能认定为边界*/
    bool huaiyi = false; // 是否怀疑
    uint8 huaiyiCnt = 0; // 怀疑计数器
    constexpr int huaiyiCntMax = 5; // 怀疑计数最大值
    cv::Point leftEdge_huaiyi_mark = cv::Point(0, 0), rightEdge_huaiyi_mark = cv::Point(0, 0); // 怀疑是否为边线的标记点
    for (int r=binImg.rows-3; r>=0; r--) { // 为什么从binImg.rows-3开始，因为前边会对二值图像外围画一个宽度画一个厚度为2的黑色边框
        /*搜寻左边界*/
        huaiyi = false; // 每一行起始时，清除怀疑
        huaiyiCnt = 0; // 怀疑计数器清零
        for (int c=binImg.cols/2; c>=0; c--) {
            /*从中间往左边找*/
            uint8 pix = binImg.at<uint8>(r,c);
            /*如果一直没找到左边线*/
            if (c == 0) {
                // 还没有怀疑，则直接把搜寻的最后一个点加进去
                if (!huaiyi) leftEdgePoints.push_back(cv::Point(c,r));
                // 有怀疑了，但还没有怀疑到极限，则把怀疑的点加进去
                else if (huaiyi && huaiyiCnt < huaiyiCntMax) {
                    leftEdgePoints.push_back(leftEdge_huaiyi_mark);
                }
            }
            /*没有怀疑时，发现黑色块了，置怀疑，并记录当前黑色块坐标*/
            else if (!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // 置怀疑
                huaiyiCnt++; // 怀疑度+1
                leftEdge_huaiyi_mark = cv::Point(c,r); // 记录当前怀疑坐标
            }
            /*还没怀疑到极限，发现了黑色块，怀疑加深*/
            else if (huaiyi && huaiyiCnt < huaiyiCntMax && pix == BIN_BLACK) {
                huaiyiCnt++; // 怀疑度+1
            }
            /*还没怀疑到极限，发现白色块了，消除怀疑，怀疑度清零*/
            else if (huaiyi && huaiyiCnt < huaiyiCntMax && pix == BIN_WHITE) {
                huaiyi = false; // 消除怀疑
                huaiyiCnt = 0; // 怀疑度清零
            }
            /*怀疑到极限了，确认无误是左边界了*/
            else if (huaiyi && huaiyiCnt >= huaiyiCntMax) {
                leftEdgePoints.push_back(leftEdge_huaiyi_mark); // 记录这个左边界点
                huaiyi = false; // 不怀疑了，确定了
                huaiyiCnt = 0; // 怀疑度清零
                break; // 退出循环
            }
        }
        /*搜寻右边界*/
        huaiyi = false; // 每一行起始时，清除怀疑
        huaiyiCnt = 0; // 怀疑计数器清零
        for (int c=binImg.cols/2; c<binImg.cols; c++) {
            /*从中间往右边找*/
            uint8 pix = binImg.at<uint8>(r,c);
            /*如果一直没找到右边线*/
            if (c == binImg.cols - 1) {
                // 还没有怀疑，则直接把搜寻的最后一个点加进去
                if (!huaiyi) rightEdgePoints.push_back(cv::Point(c,r));
                // 有怀疑了，但还没有怀疑到极限，则把怀疑的点加进去
                else if (huaiyi && huaiyiCnt < huaiyiCntMax) {
                    rightEdgePoints.push_back(rightEdge_huaiyi_mark);
                }
            }
            /*没有怀疑时，发现黑色块了，置怀疑，并记录当前黑色块坐标*/
            else if (!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // 置怀疑
                huaiyiCnt++; // 怀疑度+1
                rightEdge_huaiyi_mark = cv::Point(c,r); // 记录当前怀疑坐标
            }
            /*还没怀疑到极限，发现了黑色块，怀疑加深*/
            else if (huaiyi && huaiyiCnt < huaiyiCntMax && pix == BIN_BLACK) {
                huaiyiCnt++; // 怀疑度+1
            }
            /*还没怀疑到极限，发现白色块了，消除怀疑，怀疑度清零*/
            else if (huaiyi && huaiyiCnt < huaiyiCntMax && pix == BIN_WHITE) {
                huaiyi = false; // 消除怀疑
                huaiyiCnt = 0; // 怀疑度清零
            }
            /*怀疑到极限了，确认无误是右边界了*/
            else if (huaiyi && huaiyiCnt >= huaiyiCntMax) {
                rightEdgePoints.push_back(rightEdge_huaiyi_mark); // 记录这个右边界点
                huaiyi = false; // 不怀疑了，确定了
                huaiyiCnt = 0; // 怀疑度清零
                break; // 退出循环
            }
        }
        if (leftEdgePoints.empty() || rightEdgePoints.empty()) { // 如果有一边还没有找到基础识别点（边界点）
            leftEdgePoints.erase(leftEdgePoints.begin(), leftEdgePoints.end()); // 清空左边界点
            rightEdgePoints.erase(rightEdgePoints.begin(), rightEdgePoints.end()); // 清空右边界点
            // 清空是为了能保持寻找到的基础识别点能纵坐标对齐
            continue; // 继续查找下一行
        }
        else { // 如果两边都找到了基础识别点（边界点）
            leftEdgePointsNum--; // 查找到一个左边界点了，leftEdgePointsNum减1
            rightEdgePointsNum--; // 查找到一个右边界点了，rightEdgePointsNum减1
            break; // 退出查找基础识别点的for循环
        }
    }

    cv::Point leftCenPoint; // 八领域查找时的左边的中心点，初始化为基础识别点
    cv::Point rightCenPoint; // 八领域查找时的左右边的中心点，初始化为基础识别点
    std::array<cv::Point,8> leftFindField, rightFindField; // 用于存放查找的八个领域的坐标

    // 八领域查找时的一些常量
    std::array<cv::Point,8> leftSeeds{
        cv::Point(0,1), cv::Point(-1,1), cv::Point(-1,0), cv::Point(-1,-1),
        cv::Point(0,-1), cv::Point(1,-1), cv::Point(1,0), cv::Point(1,1)
    };
    std::array<cv::Point,8> rightSeeds{
        cv::Point(0,1), cv::Point(1,1), cv::Point(1,0), cv::Point(1,-1),
        cv::Point(0,-1), cv::Point(-1,-1), cv::Point(-1,0), cv::Point(-1,1)
    };
    // 对于左边线是顺时针                     // 对于右边线是逆时针
    // {-1,-1}, {0,-1}, {+1,-1},           // {-1,-1}, {0,-1}, {+1,-1},
    // {-1, 0},         {+1, 0},           // {-1, 0},         {+1, 0},
    // {-1,+1}, {0,+1}, {+1,+1},           // {-1,+1}, {0,+1}, {+1,+1},

    /*搜寻左边界点*/
    while (leftEdgePointsNum--) {
        leftCenPoint = leftEdgePoints.back(); // 更新左中心点
        for (int i=0; i<8; i++) leftFindField[i] = leftCenPoint + leftSeeds[i]; // 计算出左中心点八个领域的坐标存放到leftFindField中
        for (int i=0; i<8; i++) {
            if (binImg.at<uint8>(leftFindField[i])==BIN_BLACK && binImg.at<uint8>(leftFindField[(i+1)%8])==BIN_WHITE) {
                leftEdgePoints.push_back(leftFindField[i]);
                leftEdgePointDir.push_back(i);
                break;
            }
        }
    }
    /*搜寻右边界点*/
    while (rightEdgePointsNum--) {
        rightCenPoint = rightEdgePoints.back(); // 更新右中心点
        for (int i=0; i<8; i++) rightFindField[i] = rightCenPoint + rightSeeds[i];
        for (int i=0; i<8; i++) {
            if (binImg.at<uint8>(rightFindField[i])==BIN_BLACK && binImg.at<uint8>(rightFindField[(i+1)%8])==BIN_WHITE) {
                rightEdgePoints.push_back(rightFindField[i]);
                rightEdgePointDir.push_back(i);
                break;
            }
        }
    }
}

/**
 * @brief 对二值图像进行滤波处理，该函数会对传入的二值图像进行滤波处理，根据周围像素值的统计结果，修改中心像素的值。
 *
 * @param binImg 需要进行滤波处理的二值图像
 */
// 定义膨胀和腐蚀的阈值区间
#define threshold_max  255*5 //此参数可根据自己的需求调节
#define threshold_min  255*2 //此参数可根据自己的需求调节
void binImgFilter(cv::Mat& binImg) {
    uint32 num = 0;
    for (int i=1; i<RESIZED_HEIGHT-1; i++) {
        for (int j=1; j<RESIZED_WIDTH-1; j++) {
            //统计八个方向的像素值
            num = binImg.at<uint8>(i-1,j-1) + binImg.at<uint8>(i-1,j) + binImg.at<uint8>(i-1,j+1)
                + binImg.at<uint8>(i,j-1) + binImg.at<uint8>(i,j+1)
                + binImg.at<uint8>(i+1,j-1) + binImg.at<uint8>(i+1,j) + binImg.at<uint8>(i+1,j+1);
            if (num >= threshold_max && binImg.at<uint8>(i,j) == 0) {
                binImg.at<uint8>(i,j) = BIN_WHITE;//白  可以搞成宏定义，方便更改
            }
            if (num <= threshold_min && binImg.at<uint8>(i,j) == BIN_WHITE) {
                binImg.at<uint8>(i,j) = BIN_BLACK;//黑
            }
        }
    }
}

/*在二值图像的四周画一个厚度为2的矩形边框*/
void binImgDrawRect(cv::Mat& binImg) {
    for (int i=0; i<RESIZED_HEIGHT; i++) {
        binImg.at<uint8>(i,0) = 0;
        binImg.at<uint8>(i,1) = 0;
        binImg.at<uint8>(i,RESIZED_WIDTH - 1) = 0;
        binImg.at<uint8>(i,RESIZED_WIDTH - 2) = 0;
    }
    for (int i=0; i<RESIZED_WIDTH; i++) {
        binImg.at<uint8>(0,i) = 0;
        binImg.at<uint8>(1,i) = 0;
    }
}

// /**
//  * @brief 这是一个用于八领域找边界点的前备函数，用此函数先在底部查找到一个起始的左右边界点，然后以此为基础不断向上查找
//  * @param binImg 输入二值化图像
//  * @param start_row 输入起始行坐标
//  * @param leftStartPoint 输出左边界起始点坐标
//  * @param rightStartPoint 输出右边界起始点坐标
//  */
// bool blyfindStartEdgePoint(cv::Mat& binImg, uint16 start_row, cv::Point& leftStartPoint, cv::Point& rightStartPoint) {
//     bool hasLeftEdgeFound=false, hasRightEdgeFound=false;
//     /*将边界起始坐标值清零，下标0是x坐标值，下标1是y坐标值*/
//     leftStartPoint.x = 0; leftStartPoint.y = 0;
//     rightStartPoint.x = 0; rightStartPoint.y = 0;

//     /*从中间往左边*/
//     for (int i=RESIZED_WIDTH/2; i>BORDER_MIN; i--) {
//         leftStartPoint.x = i;
//         leftStartPoint.y = start_row;
//         if (binImg.at<uint8>(leftStartPoint.y, leftStartPoint.x) == 255 &&
//              binImg.at<uint8>(leftStartPoint.y, leftStartPoint.x-1) == 0) {
//             hasLeftEdgeFound = true;
//             break;
//         }
//     }
//     for (int i=RESIZED_WIDTH/2; i<BORDER_MAX; i++) {
//         start_point_r[0] = i;
//         start_point_r[1] = start_row;
//         if (binImg.at<uint8>(rightStartPoint.y, rightStartPoint.x) == 255 &&
//              binImg.at<uint8>(rightStartPoint.y, rightStartPoint.x+1) == 0) {
//             hasRightEdgeFound = true;
//             break;
//         }
//     }

//     if(hasLeftEdgeFound && hasRightEdgeFound) return true; // 如果都找到了
//     else return false; // 如果没有找到
// }

// #define USE_NUM	 RESIZED_HEIGHT*3	//定义找点的数组成员个数按理说300个点能放下，但是有些特殊情况确实难顶，多定义了一点
// std::array<cv::Point, USE_NUM> leftPoints; // 定义左边线点的数组
// std::array<cv::Point, USE_NUM> rightPoints; // 定义右边线点的数组
// std::array<int, USE_NUM> leftDir; // 定义左边线的生长方向数组
// std::array<int, USE_NUM> rightDir; // 定义右边线的生长方向数组
// int data_stastics_l = 0; //统计左边找到点的个数
// int data_stastics_r = 0; //统计右边找到点的个数
// uint8 hightest = 0; //最高点
// /**
//  * @brief 开始八领域查找边界点
//  * @param binImg 输入二值化图像
//  * @param leftStartPoint 搜寻左边界起始点坐标，由函数blyfindStartEdgePoint()得到
//  * @param rightStartPoint 搜寻右边界起始点坐标，由函数blyfindStartEdgePoint()得到
//  * @param leftPointsNum 找到的左边界点的个数
//  * @param rightPointsNum 找到的右边界点的个数
//  * @param findTimes 查找的次数
//  * @return 无
//  */
// void blyfindAllEdgePoint(cv::Mat& binImg, cv::Point& leftStartPoint, cv::Point& rightStartPoint
//                          int& leftPointsNum, int& rightPointsNum
//                          int findTimes) {
//     // 关于左边线的变量
//     cv::Point leftCenPoint; // 此刻搜索左边线的中心点
//     std::array<cv::Point, 8> leftSearchField; // 定义查找左边线时某个点的搜索域
//     int leftIndex = 0;
//     std::array<cv::Point, 8> tempL;
//     std::array<cv::Point, 8> leftSeeds{
//         cv::Point(0,1), cv::Point(-1,1), cv::Point(-1,0), cv::Point(-1,-1),
//         cv::Point(0,-1), cv::Point(1,-1), cv::Point(1,0), cv::Point(1,1)
//     }; // 这个是顺时针
//     // {-1,-1}, {0,-1}, {+1,-1},
//     // {-1, 0},         {+1, 0},
//     // {-1,+1}, {0,+1}, {+1,+1},

//     // 关于右边线的变量
//     cv::Point rightCenPoint; // 此刻搜索右边线的中心点
//     std::array<cv::Point, 8> rightSearchField; // 定义查找右边线时某个点的搜索域
//     int rightIndex = 0;
//     std::array<cv::Point, 8> tempR;
//     std::array<cv::Point, 8> rightSeeds{
//         cv::Point(0,1), cv::Point(1,1), cv::Point(1,0), cv::Point(1,-1),
//         cv::Point(0,-1), cv::Point(-1,-1), cv::Point(-1,0), cv::Point(-1,1)
//     }; // 这个是逆时针
//     // {-1,-1}, {0,-1}, {+1,-1},
//     // {-1, 0},         {+1, 0},
//     // {-1,+1}, {0,+1}, {+1,+1},

//     leftPointsNum = 0; // 初始化左边线点的个数
//     rightPointsNum = 0; // 初始化右边线点的个数

//     leftCenPoint = leftStartPoint; // 初始化左边线的中心点
//     rightCenPoint = rightStartPoint; // 初始化右边线的中心点

//     while (findTimes--) {
//         for (int i=0; i<8; i++) {
//             leftSearchField[i] = leftCenPoint + leftSeeds[i];
//         }
//         leftPoints[leftPointsNum] = leftCenPoint; // 将当前的中心点加入到左边线点的数组中
//         leftPointsNum ++;

//         for (int i=0; i<8; i++) {
//             rightSearchField[i] = rightCenPoint + rightSeeds[i];
//         }
//         rightPoints[rightPointsNum] = rightCenPoint; // 将当前的中心点加入到右边线点的数组中

//         leftIndex = 0; // 先清零，后使用
//         for (int i=0; i<8; i++) {
//             tempL[i] = cv::Point(0,0); // 清零
//         }

//         /*正式搜寻左边线*/
//         for (int i=0; i<8; i++) {
//             if (binImg.at<uint8>(leftSearchField[i]) == BIN_BLACK && binImg.at<uint8>(leftSearchField[(i+1)%8]) == BIN_WHITE) {
//                 tempL[leftIndex] = leftSearchField[i]; // 这个是找到的点
//                 leftIndex++;
//                 leftDir[leftPointsNum-1] = i; // 记录找到的这个点的生长方向
//                 if (leftIndex) { // 如果找到了由黑转白的点
//                     leftCenPoint = tempL[0];
//                     for (int j=0; j<leftIndex; j++) {
//                         if (leftCenPoint.y > tempL[j].y) { // 找到最上面的点
//                             leftCenPoint = tempL[j];
//                         }
//                     }
//                 }
//             }
//         }
//         if ((points_r[r_data_statics][0]== points_r[r_data_statics-1][0]&& points_r[r_data_statics][0] == points_r[r_data_statics - 2][0]
//            && points_r[r_data_statics][1] == points_r[r_data_statics - 1][1] && points_r[r_data_statics][1] == points_r[r_data_statics - 2][1])
//            ||(points_l[l_data_statics-1][0] == points_l[l_data_statics - 2][0] && points_l[l_data_statics-1][0] == points_l[l_data_statics - 3][0]
//                 && points_l[l_data_statics-1][1] == points_l[l_data_statics - 2][1] && points_l[l_data_statics-1][1] == points_l[l_data_statics - 3][1]))
//         {
//             //printf("三次进入同一个点，退出\n");
//             break;
//         }
//     }
// }