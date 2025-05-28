// track recognition for animated images 动态图像的轨迹识别
// 动态的赛道图像识别，在程序track_rcgn_static的计算基础上去解析相机获取的图像
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "test.hpp"

int test_track_rcgn_animated(void) {
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
    cv::namedWindow("缩放后的彩色图", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("原二值化图像", cv::WINDOW_NORMAL); // 创建一个窗口
    cv::namedWindow("处理后的二值化图像", cv::WINDOW_NORMAL); // 创建一个窗口

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
            resizedFrame.at<cv::Vec3b>(midPoint.back()) = cv::Vec3b(BGR_GREEN); // 画左右边线之间的中点线，即循迹线
            resizedFrame.at<cv::Vec3b>(rightEdgePoints.at(i)) = cv::Vec3b(BGR_RED); // 画右边线
        }
        double xl = calAverSlopeFromRowToRow(resizedFrame, midPoint, ROW_UP, ROW_DOWN); // 计算中线的偏斜
        double py = calAverXCoordFromRowToRow(resizedFrame, midPoint, ROW_UP, ROW_DOWN); // 计算X坐标平均值

        cv::imshow("缩放后的彩色图", resizedFrame); // 在对应窗口中显示图像
        cv::imshow("处理后的二值化图像", binFrame); // 在对应窗口中显示图像

        clearScreen();
        std::cout << "帧率：" << ss_fps.str() << std::endl;
        std::cout << "阈值：" << th << std::endl;
        std::cout << "偏斜：" << xl << std::endl;
        std::cout << "偏移：" << py << std::endl;
        std::cout << "转向：" << ((xl < -0.2) ? "向右" : ((xl > 0.2) ? "向左" : "直走")) << std::endl;



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

/**
 * @brief 传入一张二值化图像，并将左右的边界点压入到两个容器中
 * @note  搜寻方向是：从左到右、从上到下（都是0索引起始）
 * @param binImg 二值化图像
 * @param LeftEdgePoints 收集左边界点的容器引用
 * @param rightEdgePoints 收集右边界点的容器引用
 */
// 使用哪一种方法来查找边线
#define use_findEdgePix  5 // 使用第几种方法
void findEdgePix(const cv::Mat binImg, std::vector<cv::Point>& leftEdgePoints, std::vector<cv::Point>& rightEdgePoints) {
#if use_findEdgePix == 1
    // 1、简单的边线识别：从左到右依此寻找 黑转白 和 白转黑 的点，分别用作左右边界点===============================================
    bool findLeftEdge = false, findRightEdge = false;
    for (int r = 0; r < binImg.rows; r++) {
       findLeftEdge = true; findRightEdge = false;
       for (int c = 0; c < binImg.cols; c++) {
           uint8_t pix = binImg.at<uint8_t>(r, c);
           if (findLeftEdge && pix == BIN_WHITE) {
               leftEdgePoints.push_back(cv::Point(c, r));
               findLeftEdge = false;
               findRightEdge = true;
           }
           else if (findRightEdge && pix == BIN_BLACK) {
               rightEdgePoints.push_back(cv::Point(c, r));
               findLeftEdge = true;
               findLeftEdge = false;
               break;
           }
       }
    }

#elif use_findEdgePix == 2
    // 2、带有怀疑的边线识别：比如在同一行中，由黑转白，要连续识别到多个白色像素，才能认定为左边线===============================================
    bool findLeftEdge = false, findRightEdge = false; // 是否在查找左边线，是否在查找右边线
    bool leftEdge_huaiyi = false, rightEdge_huaiyi = false; // 怀疑是否为边线了
    uint8_t leftEdge_huaiyi_cnt = 0, rightEdge_huaiyi_cnt = 0; // 怀疑计数
    constexpr auto leftEdge_huaiyi_cnt_MAX = 10, rightEdge_huaiyi_cnt_MAX = 5; // 要连续怀疑了这么多次·，才会被认定为边线
    cv::Point leftEdge_huaiyi_mark = cv::Point(0, 0), rightEdge_huaiyi_mark = cv::Point(0, 0); // 怀疑是否为边线的标记点
    for (int r = 0; r < binImg.rows; r++) {
        /*在每一行起始时*/
        findLeftEdge = true; findRightEdge = false;
        leftEdge_huaiyi = false; rightEdge_huaiyi = false;
        leftEdge_huaiyi_cnt = 0; rightEdge_huaiyi_cnt = 0;
        leftEdge_huaiyi_mark = cv::Point(0, 0); rightEdge_huaiyi_mark = cv::Point(0, 0);
        for (int c = 0; c < binImg.cols; c++) {
            uint8_t pix = binImg.at<uint8_t>(r, c);
            if (findLeftEdge) {
                /*没有怀疑时，发现白色块了，置怀疑，并记录当前白色块坐标*/
                if (!leftEdge_huaiyi && pix == BIN_WHITE) {
                    leftEdge_huaiyi = true; // 置怀疑
                    leftEdge_huaiyi_cnt++; // 怀疑度+1
                    leftEdge_huaiyi_mark = cv::Point(c, r); // 记录当前怀疑坐标
                }
                /*还没怀疑到极限，发现了白色块，怀疑加深*/
                else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt < leftEdge_huaiyi_cnt_MAX && pix == BIN_WHITE) {
                    leftEdge_huaiyi_cnt++; // 怀疑度+1
                }
                /*还没怀疑到极限，发现黑色块了，消除怀疑，怀疑度清零*/
                else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt < leftEdge_huaiyi_cnt_MAX && pix == BIN_BLACK) {
                    leftEdge_huaiyi = false; // 消除怀疑
                    leftEdge_huaiyi_cnt = 0; // 怀疑度清零
                }
                /*怀疑到极限了，确定无误是左边界了*/
                else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt >= leftEdge_huaiyi_cnt_MAX) {
                    leftEdgePoints.push_back(leftEdge_huaiyi_mark); // 记录这个左边界点
                    findLeftEdge = false; findRightEdge = true; // 开始搜寻右边界
                }
            }
            else if (findRightEdge) {
                /*如果一直没找到右边线*/
                if (c == binImg.cols - 1) {
                    // 还没有怀疑，则直接把这行最后一个点加进去
                    if (!rightEdge_huaiyi) rightEdgePoints.push_back(cv::Point(c, r));
                    // 有怀疑了，但还没有怀疑到极限，则把怀疑的点加进去
                    else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt <= rightEdge_huaiyi_cnt_MAX) {
                        rightEdgePoints.push_back(rightEdge_huaiyi_mark);
                    }
                }
                /*没有怀疑时，发现黑色块了，置怀疑，并记录当前黑色块坐标*/
                else if (!rightEdge_huaiyi && pix == BIN_BLACK) {
                    rightEdge_huaiyi = true; // 置怀疑
                    rightEdge_huaiyi_cnt++; // 怀疑度+1
                    rightEdge_huaiyi_mark = cv::Point(c, r); // 记录当前怀疑坐标
                }
                /*还没怀疑到极限，发现了黑色块，怀疑加深*/
                else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt < rightEdge_huaiyi_cnt_MAX && pix == BIN_BLACK) {
                    rightEdge_huaiyi_cnt++; // 怀疑度+1
                }
                /*还没怀疑到极限，发现白色块了，消除怀疑，怀疑度清零*/
                else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt < rightEdge_huaiyi_cnt_MAX && pix == BIN_WHITE) {
                    rightEdge_huaiyi = false; // 消除怀疑
                    rightEdge_huaiyi_cnt = 0; // 怀疑度清零
                }
                /*怀疑到极限了，确认无误是右边界了*/
                else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt >= rightEdge_huaiyi_cnt_MAX) {
                    rightEdgePoints.push_back(rightEdge_huaiyi_mark); // 记录这个右边界点
                    findLeftEdge = true; findRightEdge = false; // 开始搜寻左边界
                    break;
                }
            }
        }
    }

#elif use_findEdgePix == 3
    // 3、带有怀疑，从中点往两边找边线，要连续识别到多个黑色像素，才能认定为边线===============================================
    bool huaiyi = false; // 是否怀疑
    uint8_t huaiyi_cnt = 0; // 怀疑计数
    constexpr int huaiyi_cnt_MAX = 5; // 怀疑计数最大值
    cv::Point leftEdge_huaiyi_mark = cv::Point(0, 0), rightEdge_huaiyi_mark = cv::Point(0, 0); // 怀疑是否为边线的标记点
    for (int r=0; r<binImg.rows; r++) {
        /*每一行起始*/
        leftEdge_huaiyi_mark = cv::Point(0, 0); rightEdge_huaiyi_mark = cv::Point(0, 0);
        /*搜寻左边界*/
        huaiyi = false;
        huaiyi_cnt = 0;
        for (int c=binImg.cols/2; c>=0; c--) {
            /*从中点往左边找*/
            uint8_t pix = binImg.at<uint8_t>(r, c);
            /*如果一直没找到左边线*/
            if(c == 0) {
                // 还没有怀疑，则直接把搜寻的最后一个点加进去
                if(!huaiyi) leftEdgePoints.push_back(cv::Point(c, r));
                // 有怀疑了，但还没有怀疑到极限，则把怀疑的点加进去
                else if (huaiyi && huaiyi_cnt <= huaiyi_cnt_MAX) {
                    leftEdgePoints.push_back(leftEdge_huaiyi_mark);
                }
            }
            /*没有怀疑时，发现黑色块了，置怀疑，并记录当前黑色块坐标*/
            else if(!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // 置怀疑
                huaiyi_cnt++; // 怀疑度+1
                leftEdge_huaiyi_mark = cv::Point(c, r); // 记录当前怀疑坐标
            }
            /*还没怀疑到极限，发现了黑色块，怀疑加深*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_BLACK) {
                huaiyi_cnt++; // 怀疑度+1
            }
            /*还没怀疑到极限，发现白色块了，消除怀疑，怀疑度清零*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_WHITE) {
                huaiyi = false; // 消除怀疑
                huaiyi_cnt = 0; // 怀疑度清零
            }
            /*怀疑到极限了，确认无误是左边界了*/
            else if(huaiyi && huaiyi_cnt >= huaiyi_cnt_MAX) {
                leftEdgePoints.push_back(leftEdge_huaiyi_mark); // 记录这个左边界点
                huaiyi = false; // 不怀疑了，确定了
                huaiyi_cnt = 0; // 怀疑度清零
                break; // 退出循环，开始搜寻右边界
            }
        }
        /*搜寻右边界*/
        huaiyi = false;
        huaiyi_cnt = 0;
        for (int c=binImg.cols/2; c<binImg.cols; c++) {
            /*从中点往右边找*/
            uint8_t pix = binImg.at<uint8_t>(r, c);
            /*如果一直没找到右边线*/
            if(c == binImg.cols - 1) {
                // 还没有怀疑，则直接把搜寻的最后一个点加进去
                if(!huaiyi) rightEdgePoints.push_back(cv::Point(c, r));
                // 有怀疑了，但还没有怀疑到极限，则把怀疑的点加进去
                else if (huaiyi && huaiyi_cnt <= huaiyi_cnt_MAX) {
                    rightEdgePoints.push_back(rightEdge_huaiyi_mark);
                }
            }
            /*没有怀疑时，发现黑色块了，置怀疑，并记录当前黑色块坐标*/
            else if(!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // 置怀疑
                huaiyi_cnt++; // 怀疑度+1
                rightEdge_huaiyi_mark = cv::Point(c, r); // 记录当前怀疑坐标
            }
            /*还没怀疑到极限，发现了黑色块，怀疑加深*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_BLACK) {
                huaiyi_cnt++; // 怀疑度+1
            }
            /*还没怀疑到极限，发现白色块了，消除怀疑，怀疑度清零*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_WHITE) {
                huaiyi = false; // 消除怀疑
                huaiyi_cnt = 0; // 怀疑度清零
            }
            /*怀疑到极限了，确认无误是右边界了*/
            else if(huaiyi && huaiyi_cnt >= huaiyi_cnt_MAX) {
                rightEdgePoints.push_back(rightEdge_huaiyi_mark); // 记录这个右边界点
                huaiyi = false; // 不怀疑了，确定了
                huaiyi_cnt = 0; // 怀疑度清零
                break; // 退出循环
            }
        }
    }
#elif use_findEdgePix == 4
    // 4、先从底部开始，以第2种方法找到一组边界点，如果没找到，则继续往上找，
    //    在找到一组边界点后，就开始往上找边线，每轮到上一行，只在在前一个边界点横坐标附近找边界点
    bool hasFoundBasis = false; // 是否已经找到基础识别点
    constexpr int FIND_ZONE = 15; // 查找区域
    bool findLeftEdge = false, findRightEdge = false; // 是否在查找左边线，是否在查找右边线
    bool leftEdge_huaiyi = false, rightEdge_huaiyi = false; // 怀疑是否为边线了
    uint8_t leftEdge_huaiyi_cnt = 0, rightEdge_huaiyi_cnt = 0; // 怀疑计数
    constexpr auto leftEdge_huaiyi_cnt_MAX = 10, rightEdge_huaiyi_cnt_MAX = 20; // 要连续怀疑了这么多次·，才会被认定为边线
    cv::Point leftEdge_huaiyi_mark = cv::Point(0, 0), rightEdge_huaiyi_mark = cv::Point(0, 0); // 怀疑是否为边线的标记点
    for (int r=binImg.rows-1; r>=0; r--) {
        /*从最低行开始，先查找一个基础识别点，再往上推进（用的是第2种查找边线的方法）*/
        if (!hasFoundBasis) { // 如果还没有找到基础识别点
            /*在每一行起始时*/
            findLeftEdge = true; findRightEdge = false;
            leftEdge_huaiyi = false; rightEdge_huaiyi = false;
            leftEdge_huaiyi_cnt = 0; rightEdge_huaiyi_cnt = 0;
            leftEdge_huaiyi_mark = cv::Point(0, 0); rightEdge_huaiyi_mark = cv::Point(0, 0);
            for (int c = 0; c < binImg.cols; c++) {
                uint8_t pix = binImg.at<uint8_t>(r, c);
                if (findLeftEdge) {
                    /*没有怀疑时，发现白色块了，置怀疑，并记录当前白色块坐标*/
                    if (!leftEdge_huaiyi && pix == BIN_WHITE) {
                        leftEdge_huaiyi = true; // 置怀疑
                        leftEdge_huaiyi_cnt++; // 怀疑度+1
                        leftEdge_huaiyi_mark = cv::Point(c, r); // 记录当前怀疑坐标
                    }
                    /*还没怀疑到极限，发现了白色块，怀疑加深*/
                    else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt < leftEdge_huaiyi_cnt_MAX && pix == BIN_WHITE) {
                        leftEdge_huaiyi_cnt++; // 怀疑度+1
                    }
                    /*还没怀疑到极限，发现黑色块了，消除怀疑，怀疑度清零*/
                    else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt < leftEdge_huaiyi_cnt_MAX && pix == BIN_BLACK) {
                        leftEdge_huaiyi = false; // 消除怀疑
                        leftEdge_huaiyi_cnt = 0; // 怀疑度清零
                    }
                    /*怀疑到极限了，确定无误是左边界了*/
                    else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt >= leftEdge_huaiyi_cnt_MAX) {
                        leftEdgePoints.push_back(leftEdge_huaiyi_mark); // 记录这个左边界点
                        findLeftEdge = false; findRightEdge = true; // 开始搜寻右边界
                    }
                }
                else if (findRightEdge) {
                    /*如果一直没找到右边线*/
                    if (c == binImg.cols - 1) {
                        // 还没有怀疑，则直接把这行最后一个点加进去
                        if (!rightEdge_huaiyi) rightEdgePoints.push_back(cv::Point(c, r));
                        // 有怀疑了，但还没有怀疑到极限，则把怀疑的点加进去
                        else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt <= rightEdge_huaiyi_cnt_MAX) {
                            rightEdgePoints.push_back(rightEdge_huaiyi_mark);
                        }
                    }
                    /*没有怀疑时，发现黑色块了，置怀疑，并记录当前黑色块坐标*/
                    else if (!rightEdge_huaiyi && pix == BIN_BLACK) {
                        rightEdge_huaiyi = true; // 置怀疑
                        rightEdge_huaiyi_cnt++; // 怀疑度+1
                        rightEdge_huaiyi_mark = cv::Point(c, r); // 记录当前怀疑坐标
                    }
                    /*还没怀疑到极限，发现了黑色块，怀疑加深*/
                    else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt < rightEdge_huaiyi_cnt_MAX && pix == BIN_BLACK) {
                        rightEdge_huaiyi_cnt++; // 怀疑度+1
                    }
                    /*还没怀疑到极限，发现白色块了，消除怀疑，怀疑度清零*/
                    else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt < rightEdge_huaiyi_cnt_MAX && pix == BIN_WHITE) {
                        rightEdge_huaiyi = false; // 消除怀疑
                        rightEdge_huaiyi_cnt = 0; // 怀疑度清零
                    }
                    /*怀疑到极限了，确认无误是右边界了*/
                    else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt >= rightEdge_huaiyi_cnt_MAX) {
                        rightEdgePoints.push_back(rightEdge_huaiyi_mark); // 记录这个右边界点
                        findLeftEdge = true; findRightEdge = false; // 开始搜寻左边界
                        break;
                    }
                }
            }
            if (leftEdgePoints.empty() || rightEdgePoints.empty()) { // 如果有一边还没有找到基础识别点（边界点）
                hasFoundBasis = false; // 保持为false，表示还没有找到基础识别点
                leftEdgePoints.erase(leftEdgePoints.begin(), leftEdgePoints.end()); // 清空左边线点
                rightEdgePoints.erase(rightEdgePoints.begin(), rightEdgePoints.end()); // 清空右边线点
                // 清空是为了能保持寻找到的点能纵坐标（行号）对齐
            }
            else { // 如果找到了边线
                hasFoundBasis = true; // 置为true，表示已经找到基础识别点了
            }
        }
        else { // 好的，已经找到基础识别点了，开始不断向上查找
            cv::Point leftLastMark(leftEdgePoints.back().x, leftEdgePoints.back().y); // 左边线最后一个点
            for (int cl=leftLastMark.x+FIND_ZONE; cl>=leftLastMark.x-FIND_ZONE; cl--) { // 左边线，从右往左找边界
                if (isBinImgRectRoiAllColor(
                        binImg,
                        cv::Rect(
                            (leftLastMark.x-FIND_ZONE<0) ? 0 : leftLastMark.x-FIND_ZONE, // 这是起始横坐标值，如果小于0的话，则直接为0，否则为原数
                            r, // 这是起始纵坐标值
                            (leftLastMark.x+FIND_ZONE>binImg.cols) ? binImg.cols-(leftLastMark.x-FIND_ZONE) : FIND_ZONE*2+1, // 这是宽度值，如果大于图像宽度的话，则直接为图像宽度减去起始横坐标值，否则为FIND_ZONE*2+1
                            1 // 这是高度值
                        ),
                        BIN_BLACK)// 如果这个区域都是黑色的
                ) {
                    leftEdgePoints.push_back(cv::Point(leftLastMark.x, r)); // 保持上一次记录的横坐标值
                    break; // 结束循环
                }
                else if(isBinImgRectRoiAllColor(
                        binImg,
                        cv::Rect(
                            (leftLastMark.x-FIND_ZONE<0) ? 0 : leftLastMark.x-FIND_ZONE, // 这是起始横坐标值，如果小于0的话，则直接为0，否则为原数
                            r, // 这是起始纵坐标值
                            (leftLastMark.x+FIND_ZONE>binImg.cols) ? binImg.cols-(leftLastMark.x-FIND_ZONE) : FIND_ZONE*2+1, // 这是宽度值，如果大于图像宽度的话，则直接为图像宽度减去起始横坐标值，否则为FIND_ZONE*2+1
                            1 // 这是高度值
                        ),
                        BIN_WHITE) // 如果这个区域都是白色的
                ) {
                    leftEdgePoints.push_back(cv::Point(leftLastMark.x, r)); // 保持上一次记录的横坐标值
                    break; // 结束循环
                }
                else if (cl >= binImg.cols) continue; // 越界了，直接下一次循环
                else if (cl < 0) {
                    leftEdgePoints.push_back(cv::Point(leftLastMark.x, r)); // 保持上一次记录的横坐标值
                    break; // 结束循环
                }
                else if (binImg.at<uint8_t>(r,cl) == BIN_BLACK) {
                    leftEdgePoints.push_back(cv::Point(cl, r)); // 记录这个边界点
                    break; // 结束循环
                }
            }
            cv::Point rightLastMark(rightEdgePoints.back().x, rightEdgePoints.back().y); // 右边线最后一个点
            for (int cr=rightLastMark.x-FIND_ZONE; cr<=rightLastMark.x+FIND_ZONE; cr++) { // 右边线，从左往右找边界
                if (isBinImgRectRoiAllColor(
                        binImg,
                        cv::Rect(
                            (rightLastMark.x-FIND_ZONE<0) ? 0 : rightLastMark.x-FIND_ZONE, // 这是起始横坐标值，如果小于0的话，则直接为0，否则为原数
                            r, // 这是起始纵坐标值
                            (rightLastMark.x+FIND_ZONE>binImg.cols)?binImg.cols-(rightLastMark.x-FIND_ZONE):FIND_ZONE*2+1, // 这是宽度值，如果大于图像宽度的话，则直接为图像宽度减去起始横坐标值，否则为FIND_ZONE*2+1
                            1 // 这是高度值
                        ),
                        BIN_BLACK) // 如果这个区域都是黑色的
                ) {
                    rightEdgePoints.push_back(cv::Point(rightLastMark.x, r)); // 保持上一次记录的横坐标值
                    break; // 结束循环
                }
                else if(isBinImgRectRoiAllColor(
                        binImg,
                        cv::Rect(
                            (rightLastMark.x-FIND_ZONE<0) ? 0 : rightLastMark.x-FIND_ZONE, // 这是起始横坐标值，如果小于0的话，则直接为0，否则为原数
                            r, // 这是起始纵坐标值
                            (rightLastMark.x+FIND_ZONE>binImg.cols)?binImg.cols-(rightLastMark.x-FIND_ZONE):FIND_ZONE*2+1, // 这是宽度值，如果大于图像宽度的话，则直接为图像宽度减去起始横坐标值，否则为FIND_ZONE*2+1
                            1 // 这是高度值
                        ),
                        BIN_WHITE) // 如果这个区域都是白色的
                ) {
                    rightEdgePoints.push_back(cv::Point(rightLastMark.x, r)); // 保持上一次记录的横坐标值
                    break; // 结束循环
                }
                else if (cr < 0) continue; // 越界了，直接下一次循环
                else if (cr >= binImg.cols) {
                    rightEdgePoints.push_back(cv::Point(rightLastMark.x, r)); // 保持上一次记录的横坐标值
                    break; // 结束循环
                }
                else if (binImg.at<uint8_t>(r, cr) == BIN_BLACK) {
                    rightEdgePoints.push_back(cv::Point(cr, r)); // 记录这个边界点
                    break; // 结束循环
                }
            }
        }
    }
#elif use_findEdgePix == 5
    // 5、带有怀疑，从底部向上，从中点往两边找边线，在找到一组边线后，以找到的边线计算出的点作为新的中点，
    //    轮到上一行，就以这个新的中点往两边搜线，要连续识别到多个黑色像素，才能认定为边线
    bool huaiyi = false; // 是否怀疑
    uint8_t huaiyi_cnt = 0; // 怀疑计数
    constexpr int huaiyi_cnt_MAX = 5; // 怀疑计数最大值
    cv::Point midPointMark = cv::Point(binImg.cols/2, binImg.rows-1); // 中点标记，初始为图像底部中点
    cv::Point leftEdge_huaiyi_mark = cv::Point(0, 0), rightEdge_huaiyi_mark = cv::Point(0, 0); // 怀疑是否为边线的标记点
    for (int r=binImg.rows-1; r>=0; r--) { // 从底部向上
        /*每一行起始*/
        leftEdge_huaiyi_mark = cv::Point(0, 0); rightEdge_huaiyi_mark = cv::Point(0, 0);
        /*搜寻左边界*/
        huaiyi = false;
        huaiyi_cnt = 0;
        for (int c=midPointMark.x; c>=0; c--) {
            /*从中点往左边找*/
            uint8_t pix = binImg.at<uint8_t>(r, c);
            /*如果一直没找到左边线*/
            if(c == 0) {
                // 还没有怀疑，则直接把搜寻的最后一个点加进去
                if(!huaiyi) leftEdgePoints.push_back(cv::Point(c, r));
                // 有怀疑了，但还没有怀疑到极限，则把怀疑的点加进去
                else if (huaiyi && huaiyi_cnt <= huaiyi_cnt_MAX) {
                    leftEdgePoints.push_back(leftEdge_huaiyi_mark);
                }
            }
            /*没有怀疑时，发现黑色块了，置怀疑，并记录当前黑色块坐标*/
            else if(!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // 置怀疑
                huaiyi_cnt++; // 怀疑度+1
                leftEdge_huaiyi_mark = cv::Point(c, r); // 记录当前怀疑坐标
            }
            /*还没怀疑到极限，发现了黑色块，怀疑加深*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_BLACK) {
                huaiyi_cnt++; // 怀疑度+1
            }
            /*还没怀疑到极限，发现白色块了，消除怀疑，怀疑度清零*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_WHITE) {
                huaiyi = false; // 消除怀疑
                huaiyi_cnt = 0; // 怀疑度清零
            }
            /*怀疑到极限了，确认无误是左边界了*/
            else if(huaiyi && huaiyi_cnt >= huaiyi_cnt_MAX) {
                leftEdgePoints.push_back(leftEdge_huaiyi_mark); // 记录这个左边界点
                huaiyi = false; // 不怀疑了，确定了
                huaiyi_cnt = 0; // 怀疑度清零
                break; // 退出循环，开始搜寻右边界
            }
        }
        /*搜寻右边界*/
        huaiyi = false;
        huaiyi_cnt = 0;
        for (int c=midPointMark.x; c<binImg.cols; c++) {
            /*从中点往右边找*/
            uint8_t pix = binImg.at<uint8_t>(r, c);
            /*如果一直没找到右边线*/
            if(c == binImg.cols - 1) {
                // 还没有怀疑，则直接把搜寻的最后一个点加进去
                if(!huaiyi) rightEdgePoints.push_back(cv::Point(c, r));
                // 有怀疑了，但还没有怀疑到极限，则把怀疑的点加进去
                else if (huaiyi && huaiyi_cnt <= huaiyi_cnt_MAX) {
                    rightEdgePoints.push_back(rightEdge_huaiyi_mark);
                }
            }
            /*没有怀疑时，发现黑色块了，置怀疑，并记录当前黑色块坐标*/
            else if(!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // 置怀疑
                huaiyi_cnt++; // 怀疑度+1
                rightEdge_huaiyi_mark = cv::Point(c, r); // 记录当前怀疑坐标
            }
            /*还没怀疑到极限，发现了黑色块，怀疑加深*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_BLACK) {
                huaiyi_cnt++; // 怀疑度+1
            }
            /*还没怀疑到极限，发现白色块了，消除怀疑，怀疑度清零*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_WHITE) {
                huaiyi = false; // 消除怀疑
                huaiyi_cnt = 0; // 怀疑度清零
            }
            /*怀疑到极限了，确认无误是右边界了*/
            else if(huaiyi && huaiyi_cnt >= huaiyi_cnt_MAX) {
                rightEdgePoints.push_back(rightEdge_huaiyi_mark); // 记录这个右边界点
                huaiyi = false; // 不怀疑了，确定了
                huaiyi_cnt = 0; // 怀疑度清零
                break; // 退出循环
            }
        }
        if (leftEdgePoints.empty() || rightEdgePoints.empty()) continue;
        else midPointMark = calMidPoint(leftEdgePoints.back(), rightEdgePoints.back()); // 计算新的中点
    }
#endif
}

// 计算两个坐标之间的中点坐标
cv::Point calMidPoint(const cv::Point pt1, const cv::Point& pt2) {
    return cv::Point((pt1.x + pt2.x) / 2, (pt1.y + pt2.y) / 2);
}

// 使用自动阈值，将彩色图像转为指定像素的二值图像，返回值为转换为二值化图像时的阈值
double bgr2bin(const cv::Mat& bgrImg, cv::Mat& resizedImg, cv::Mat& grayImg, cv::Mat& binImg,
               const cv::Size& imgSize) {
    cv::resize(bgrImg, resizedImg, imgSize);
    cv::cvtColor(resizedImg, grayImg, cv::COLOR_BGR2GRAY); // 将原始图像转换为灰度图
    return cv::threshold(grayImg, binImg, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
}

// 指定阈值，将彩色图像转为指定像素的二值图像
double bgr2bin(const cv::Mat& bgrImg, cv::Mat& resizedImg, cv::Mat& grayImg, cv::Mat& binImg,
             const cv::Size& imgSize, double thresh, double maxval) {
    cv::resize(bgrImg, resizedImg, imgSize);
    cv::cvtColor(resizedImg, grayImg, cv::COLOR_BGR2GRAY); // 将原始图像转换为灰度图
    cv::threshold(grayImg, binImg, thresh, maxval, cv::THRESH_BINARY);
    return thresh;
}

/*判断二值化图像中的某个矩形区域是否都为color颜色，是的话返回true，否的话返回false，如果传入的矩形范围超过图像尺寸也会返回false*/
bool isBinImgRectRoiAllColor(const cv::Mat& binImg, const cv::Rect& rect, uint8_t color) {
    if (rect.x < 0 || rect.y < 0 || rect.width <= 0 || rect.height <= 0) return false; // 检查数据是否合法
    if (rect.x + rect.width > binImg.cols || rect.y + rect.height > binImg.rows) return false; // 检查矩形区域是否在图像范围内
    cv::Mat imgRoi = binImg(rect); // 提取矩形区域
    for (int i = 0; i < imgRoi.rows; ++i) // 遍历矩形区域中的所有像素
        for (int j = 0; j < imgRoi.cols; ++j)
            if (imgRoi.at<uint8_t>(i,j) != color) return false; // 如果发现非黑色像素，返回false
    return true; // 所有像素都是黑色，返回true
}

/**
 * @brief 在二值图像上的左右边界绘制黑色边框
 * @param binImg 输入的二值图像
 */
void drawLeftRightBlackBorder(cv::Mat& binImg) {
    if (binImg.type() != CV_8UC1) { // 检查图像是否为二值图像
        std::cerr << "Image is not a binary image!" << std::endl;
        return;
    }
    int rows = binImg.rows;
    int cols = binImg.cols;
    for (int r=0; r<rows; r++) {
        binImg.at<uint8>(r,0) = BIN_BLACK; // 左边界
        binImg.at<uint8>(r,cols-1) = BIN_BLACK; // 右边界
    }
}

/**
 * @brief 在二值图像顶部和底部绘制黑色边框
 * @param binImg 二值图像
 */
void drawTopBottomBlackBorder(cv::Mat& binImg) {
    if (binImg.type() != CV_8UC1) { // 检查图像是否为二值图像
        std::cerr << "Image is not a binary image!" << std::endl;
        return;
    }
    int rows = binImg.rows;
    int cols = binImg.cols;
    for (int c=0; c<cols; c++) {
        binImg.at<uint8>(0,c) = BIN_BLACK; // 上边界
        binImg.at<uint8>(rows-1,c) = BIN_BLACK; // 下边界
    }
}

