#ifndef __TESTCODE_HPP
#define __TESTCODE_HPP
#include <stdint.h>
#include <array>

// 类型
typedef uint8_t     uint8;  // 8位无符号整数
typedef uint16_t    uint16; // 16位无符号整数
typedef uint32_t    uint32; // 32位无符号整数
typedef uint64_t    uint64; // 64位无符号整数
typedef int8_t      int8;   // 8位有符号整数
typedef int16_t     int16;  // 16位有符号整数
typedef int32_t     int32;  // 32位有符号整数
typedef int64_t     int64;  // 64位有符号整数


// 相机索引，一般为0，有时候为1，如果有时候相机打不开，在这里更改一下
#define CAM_INDEX   1

// 实际测试的时候，发现这宽度和高度只能设置为(640*480)和(1920*1080)，不知道怎么回事
#define CAM_WIDTH   640
#define CAM_HEIGHT  480

#define RESIZED_WIDTH  128  // 缩放后的宽度
#define RESIZED_HEIGHT 96   // 缩放后的高度
#define ROW_UP         55   // 计算路径时，从第几行开始
#define ROW_DOWN       72   // 计算路径时，到第几行结束
// #define BORDER_MAX 1 // 二值化图像的最大边框宽度

#define GRAY2BIN_THRESH  119    // 灰度图像转黑白图像的灰度阈值
#define MIN_AREA_THRESH  90     // 黑白图像中，移除面积较小的孤岛色块的面积阈值

#define BIN_WHITE   255 // 二值图像：白色
#define BIN_BLACK   0   // 二值图像：黑色

#define BGR_WHITE   255,255,255 // 彩色图像：白色
#define BGR_BLACK   0,0,0       // 彩色图像：黑色
#define BGR_BLUE    255,0,0     // 彩色图像：蓝色
#define BGR_GREEN   0,255,0     // 彩色图像：绿色
#define BGR_RED     0,0,255     // 彩色图像：红色
#define BGR_PURPLE  128,0,128   // 彩色图像：紫色
#define BGR_ICEBLUE 255,255,0   // 彩色图像：冰蓝色

/*一些辅助函数*/
void clearScreen(void); // 清空终端，并将光标移动到起始位置，可用于终端刷新显示
void findEdgePix(const cv::Mat binImg, std::vector<cv::Point>& leftEdgePoints, std::vector<cv::Point>& rightEdgePoints); // 传入一张二值化图像，并将左右的边界点压入到两个容器中
void drawLineOnRow(cv::Mat& img, int rowIndex, cv::Scalar color, int thickness); // 传入一张图像，在指定行上绘制指定颜色厚度的直线
cv::Point calMidPoint(const cv::Point pt1, const cv::Point& pt2); // 计算两个坐标之间的中点坐标
double myCalAverage(const std::vector<double> vdata); // 计算一个容器中所有数据的平均值，自己写的，可以防止元素总和超出数据类型表示范围
template <typename T> T calAverage(const std::vector<T> vdata); // 计算一个容器中所有数据的平均值（来自智譜清言AI）
double bgr2bin(const cv::Mat& bgrImg, cv::Mat& resizedImg, cv::Mat& grayImg, cv::Mat& binImg,
                const cv::Size& imgSize, double thresh, double maxval);// 指定阈值，将彩色图像转为指定像素的二值图像
double bgr2bin(const cv::Mat& bgrImg, cv::Mat& resizedImg, cv::Mat& grayImg, cv::Mat& binImg,
                const cv::Size& imgSize);// 使用自动阈值，将彩色图像转为指定像素的二值图像，返回值为转换为二值化图像时的阈值
double calAverSlopeFromRowToRow(cv::Mat& img, std::vector<cv::Point> midPoint, int row_up, int row_down); // 计算行到行之间点的斜率
double calAverXCoordFromRowToRow(cv::Mat& img, std::vector<cv::Point> midPoint, int row_up, int row_down); // 计算行到行之间点的X坐标平均值
/*对于以下两个寻找小块函数，需要说明的是，在嵌套内的小块的坐标并不会被加入容器*/
void getWhiteCentersOfSmallRegions(const cv::Mat& binImg, double minArea, std::vector<cv::Point>& centers); // 获取二值图像中面积小于指定值的独立白色区域
void getBlackCentersOfSmallRegions(const cv::Mat& binImg, double minArea, std::vector<cv::Point>& centers); // 获取二值图像中面积小于指定值的独立黑色区域
void fillBlobRecursive(cv::Mat& binImg, cv::Point pt, uint8_t newColor); // 洪水填充二值图像某一区域的颜色，就像Windows画图软件的倒墨水
void fillBlobRecursive(cv::Mat& bgrImg, cv::Point pt, cv::Vec3b newColor, cv::Vec3b oldColor); // 洪水填充彩色图某一区域的颜色，就像Windows画图软件的倒墨水
void invertSmallRegion_1(cv::Mat& binImg, double minArea); // 反转二值图像中面积小于指定值的独立区域，方法1
void invertSmallRegion_2(cv::Mat& binImg, double minArea); // 反转二值图像中面积小于指定值的独立区域，方法2
bool isBinImgRectRoiAllColor(const cv::Mat& binImg, const cv::Rect& rect, uint8_t color); // 二值化图像的某个矩形区域是否都为color颜色
void drawBlackRectBorder(cv::Mat& binImg); // 在二值图像四周绘制一个厚度为1的黑色矩形边框
void drawLeftRightBlackBorder(cv::Mat& binImg); // 在二值图像的左右画黑线
void drawTopBottomBlackBorder(cv::Mat& binImg); // 在二值图像的上下画黑线
void filterXCoord(std::vector<cv::Point>& points, std::vector<cv::Point>& filteredPoints, int windowSize);
// 参考自B站up主“村东头无敌的小瞎子”(uid:385282905)的函数===========================================================
void blyfindEdgePoint(cv::Mat& binImg,
                       std::vector<cv::Point>& leftEdgePoints, std::vector<cv::Point>& rightEdgePoints,
                       std::vector<int>& leftEdgePointDir, std::vector<int>& rightEdgePointDir,
                       int leftEdgePointsNum, int rightEdgePointsNum); // 八领域查找边界点
void binImgFilter(cv::Mat& binImg); // 二值图像滤波
void binImgDrawRect(cv::Mat& binImg); // 在二值图像的四周画一个厚度为2的矩形边框
// ==========================================================================================================

/*用于主函数调用的测试程序*/
int test_cam_disp(void); // 获取图像并显示
int test_perspective_transform(void); // 图像拉伸
int test_gray_pic(void); // #include <windows.h> 灰度图、黑白图显示
int test_mat_refresh_disp(void); // #include <windows.h> 在终端中进行矩阵刷新显示
int test_pic_resize(void); // 图像缩放
int test_pic_roi(void); // 图像截取
int test_terminal_pic_disp(void); // 终端显示
int track_detect_ilikara(void); // 来自ilikara的一个循迹识别程序
int test_track_rcgn_static(void); // 简单的循迹测试（静态图）
int test_track_rcgn_animated(void); // 循迹测试程序（动态识别）
int test_cal_steering_dir(void); // 计算转向方向
int test_multi_threads_cam_disp(void); // 多线程显示图像
int test_fill_bin_small_region(void); // 测试对二值图像小色块的滤波
int test_track_bly_static(void);
int test_banmaxian_detect(void);
int test_avoid_obstacles(void);

/*一些按键的ASCII码值，用于检测按键退出程序*/
constexpr auto KEY_ESC = 27;
constexpr auto KEY_SPACE = 32;
constexpr auto KEY_W = 87;
constexpr auto KEY_S = 83;
constexpr auto KEY_A = 65;
constexpr auto KEY_D = 68;
constexpr auto KEY_w = 119;
constexpr auto KEY_s = 115;
constexpr auto KEY_a = 97;
constexpr auto KEY_d = 100;
constexpr auto KEY_0 = 48;
constexpr auto KEY_1 = 49;
constexpr auto KEY_2 = 50;
constexpr auto KEY_3 = 51;
constexpr auto KEY_4 = 52;
constexpr auto KEY_5 = 53;
constexpr auto KEY_6 = 54;
constexpr auto KEY_7 = 55;
constexpr auto KEY_8 = 56;
constexpr auto KEY_9 = 57;
constexpr auto KEY_COMMA = 44; // 逗号键
constexpr auto KEY_DOT   = 46; // 点号键
constexpr auto KEY_SLASH = 47; // 正斜杠键
constexpr auto KEY_BACKSLASH = 92; // 反斜杠键

#endif // #ifndef __TESTCODE_HPP
