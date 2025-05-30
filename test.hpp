#ifndef __TESTCODE_HPP
#define __TESTCODE_HPP
#include <stdint.h>
#include <array>

// ����
typedef uint8_t     uint8;  // 8λ�޷�������
typedef uint16_t    uint16; // 16λ�޷�������
typedef uint32_t    uint32; // 32λ�޷�������
typedef uint64_t    uint64; // 64λ�޷�������
typedef int8_t      int8;   // 8λ�з�������
typedef int16_t     int16;  // 16λ�з�������
typedef int32_t     int32;  // 32λ�з�������
typedef int64_t     int64;  // 64λ�з�������


// ���������һ��Ϊ0����ʱ��Ϊ1�������ʱ������򲻿������������һ��
#define CAM_INDEX   1

// ʵ�ʲ��Ե�ʱ�򣬷������Ⱥ͸߶�ֻ������Ϊ(640*480)��(1920*1080)����֪����ô����
#define CAM_WIDTH   640
#define CAM_HEIGHT  480

#define RESIZED_WIDTH  128  // ���ź�Ŀ��
#define RESIZED_HEIGHT 96   // ���ź�ĸ߶�
#define ROW_UP         55   // ����·��ʱ���ӵڼ��п�ʼ
#define ROW_DOWN       72   // ����·��ʱ�����ڼ��н���
// #define BORDER_MAX 1 // ��ֵ��ͼ������߿���

#define GRAY2BIN_THRESH  119    // �Ҷ�ͼ��ת�ڰ�ͼ��ĻҶ���ֵ
#define MIN_AREA_THRESH  90     // �ڰ�ͼ���У��Ƴ������С�Ĺµ�ɫ��������ֵ

#define BIN_WHITE   255 // ��ֵͼ�񣺰�ɫ
#define BIN_BLACK   0   // ��ֵͼ�񣺺�ɫ

#define BGR_WHITE   255,255,255 // ��ɫͼ�񣺰�ɫ
#define BGR_BLACK   0,0,0       // ��ɫͼ�񣺺�ɫ
#define BGR_BLUE    255,0,0     // ��ɫͼ����ɫ
#define BGR_GREEN   0,255,0     // ��ɫͼ����ɫ
#define BGR_RED     0,0,255     // ��ɫͼ�񣺺�ɫ
#define BGR_PURPLE  128,0,128   // ��ɫͼ����ɫ
#define BGR_ICEBLUE 255,255,0   // ��ɫͼ�񣺱���ɫ

/*һЩ��������*/
void clearScreen(void); // ����նˣ���������ƶ�����ʼλ�ã��������ն�ˢ����ʾ
void findEdgePix(const cv::Mat binImg, std::vector<cv::Point>& leftEdgePoints, std::vector<cv::Point>& rightEdgePoints); // ����һ�Ŷ�ֵ��ͼ�񣬲������ҵı߽��ѹ�뵽����������
void drawLineOnRow(cv::Mat& img, int rowIndex, cv::Scalar color, int thickness); // ����һ��ͼ����ָ�����ϻ���ָ����ɫ��ȵ�ֱ��
cv::Point calMidPoint(const cv::Point pt1, const cv::Point& pt2); // ������������֮����е�����
double myCalAverage(const std::vector<double> vdata); // ����һ���������������ݵ�ƽ��ֵ���Լ�д�ģ����Է�ֹԪ���ܺͳ����������ͱ�ʾ��Χ
template <typename T> T calAverage(const std::vector<T> vdata); // ����һ���������������ݵ�ƽ��ֵ���������V����AI��
double bgr2bin(const cv::Mat& bgrImg, cv::Mat& resizedImg, cv::Mat& grayImg, cv::Mat& binImg,
                const cv::Size& imgSize, double thresh, double maxval);// ָ����ֵ������ɫͼ��תΪָ�����صĶ�ֵͼ��
double bgr2bin(const cv::Mat& bgrImg, cv::Mat& resizedImg, cv::Mat& grayImg, cv::Mat& binImg,
                const cv::Size& imgSize);// ʹ���Զ���ֵ������ɫͼ��תΪָ�����صĶ�ֵͼ�񣬷���ֵΪת��Ϊ��ֵ��ͼ��ʱ����ֵ
double calAverSlopeFromRowToRow(cv::Mat& img, std::vector<cv::Point> midPoint, int row_up, int row_down); // �����е���֮����б��
double calAverXCoordFromRowToRow(cv::Mat& img, std::vector<cv::Point> midPoint, int row_up, int row_down); // �����е���֮����X����ƽ��ֵ
/*������������Ѱ��С�麯������Ҫ˵�����ǣ���Ƕ���ڵ�С������겢���ᱻ��������*/
void getWhiteCentersOfSmallRegions(const cv::Mat& binImg, double minArea, std::vector<cv::Point>& centers); // ��ȡ��ֵͼ�������С��ָ��ֵ�Ķ�����ɫ����
void getBlackCentersOfSmallRegions(const cv::Mat& binImg, double minArea, std::vector<cv::Point>& centers); // ��ȡ��ֵͼ�������С��ָ��ֵ�Ķ�����ɫ����
void fillBlobRecursive(cv::Mat& binImg, cv::Point pt, uint8_t newColor); // ��ˮ����ֵͼ��ĳһ�������ɫ������Windows��ͼ����ĵ�īˮ
void fillBlobRecursive(cv::Mat& bgrImg, cv::Point pt, cv::Vec3b newColor, cv::Vec3b oldColor); // ��ˮ����ɫͼĳһ�������ɫ������Windows��ͼ����ĵ�īˮ
void invertSmallRegion_1(cv::Mat& binImg, double minArea); // ��ת��ֵͼ�������С��ָ��ֵ�Ķ������򣬷���1
void invertSmallRegion_2(cv::Mat& binImg, double minArea); // ��ת��ֵͼ�������С��ָ��ֵ�Ķ������򣬷���2
bool isBinImgRectRoiAllColor(const cv::Mat& binImg, const cv::Rect& rect, uint8_t color); // ��ֵ��ͼ���ĳ�����������Ƿ�Ϊcolor��ɫ
void drawBlackRectBorder(cv::Mat& binImg); // �ڶ�ֵͼ�����ܻ���һ�����Ϊ1�ĺ�ɫ���α߿�
void drawLeftRightBlackBorder(cv::Mat& binImg); // �ڶ�ֵͼ������һ�����
void drawTopBottomBlackBorder(cv::Mat& binImg); // �ڶ�ֵͼ������»�����
void filterXCoord(std::vector<cv::Point>& points, std::vector<cv::Point>& filteredPoints, int windowSize);
// �ο���Bվup�����嶫ͷ�޵е�СϹ�ӡ�(uid:385282905)�ĺ���===========================================================
void blyfindEdgePoint(cv::Mat& binImg,
                       std::vector<cv::Point>& leftEdgePoints, std::vector<cv::Point>& rightEdgePoints,
                       std::vector<int>& leftEdgePointDir, std::vector<int>& rightEdgePointDir,
                       int leftEdgePointsNum, int rightEdgePointsNum); // ��������ұ߽��
void binImgFilter(cv::Mat& binImg); // ��ֵͼ���˲�
void binImgDrawRect(cv::Mat& binImg); // �ڶ�ֵͼ������ܻ�һ�����Ϊ2�ľ��α߿�
// ==========================================================================================================

/*�������������õĲ��Գ���*/
int test_cam_disp(void); // ��ȡͼ����ʾ
int test_perspective_transform(void); // ͼ������
int test_gray_pic(void); // #include <windows.h> �Ҷ�ͼ���ڰ�ͼ��ʾ
int test_mat_refresh_disp(void); // #include <windows.h> ���ն��н��о���ˢ����ʾ
int test_pic_resize(void); // ͼ������
int test_pic_roi(void); // ͼ���ȡ
int test_terminal_pic_disp(void); // �ն���ʾ
int track_detect_ilikara(void); // ����ilikara��һ��ѭ��ʶ�����
int test_track_rcgn_static(void); // �򵥵�ѭ�����ԣ���̬ͼ��
int test_track_rcgn_animated(void); // ѭ�����Գ��򣨶�̬ʶ��
int test_cal_steering_dir(void); // ����ת����
int test_multi_threads_cam_disp(void); // ���߳���ʾͼ��
int test_fill_bin_small_region(void); // ���ԶԶ�ֵͼ��Сɫ����˲�
int test_track_bly_static(void);
int test_banmaxian_detect(void);
int test_avoid_obstacles(void);

/*һЩ������ASCII��ֵ�����ڼ�ⰴ���˳�����*/
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
constexpr auto KEY_COMMA = 44; // ���ż�
constexpr auto KEY_DOT   = 46; // ��ż�
constexpr auto KEY_SLASH = 47; // ��б�ܼ�
constexpr auto KEY_BACKSLASH = 92; // ��б�ܼ�

#endif // #ifndef __TESTCODE_HPP
