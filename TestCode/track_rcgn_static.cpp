// track recognition for static images ��̬ͼ��Ĺ켣ʶ��
// ��Ӣ�Ķ��ż��л���һ�ţ���Ӣ�ľ�ż��л���һ�ţ���Ӣ��б�˼��л���һ����Ƭ
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include "test.hpp"

#define PIC_FILE_NUM 2 // ����ͼƬ�ļ��е�����
#define TRACK_PIC_NUM  20 // ����ͼƬ
int test_track_rcgn_static(void) {
    cv::namedWindow("���ź�Ĳ�ɫͼ", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("ԭ��ֵ��ͼ��", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("�����Ķ�ֵ��ͼ��", cv::WINDOW_NORMAL); // ����һ������
    int pic_file=0, pic_index=0;
    while (true) {
        std::string filename = "./images/saideao_" + std::to_string(pic_file) + "/" + std::to_string(pic_index) + ".jpg";
        cv::Mat frame = cv::imread(filename); // ��ȡһ������ͼƬ

        if (frame.empty()) { // ���û��ȡ��
            std::cout << "Could not read the frame: " << filename << std::endl; // ��ӡδ��ȡ��Ϣ
            return -1;
        }

        cv::Mat resizedFrame; // ԭͼ > ����ͼ��
        cv::Mat grayFrame; // ����ͼ�� > �Ҷ�ͼ��
        cv::Mat binFrame; // �Ҷ�ͼ�� > ��ֵͼ��
        double th = bgr2bin(frame, resizedFrame, grayFrame, binFrame, cv::Size(RESIZED_WIDTH, RESIZED_HEIGHT), GRAY2BIN_THRESH, 255); // �Զ�����ֵ
        cv::imshow("ԭ��ֵ��ͼ��", binFrame); // �ڶ�Ӧ��������ʾͼ��
        // drawLeftRightBlackBorder(binFrame); // �ڶ�ֵͼ������ܻ�����

        // ��תС����ڰ����򷽷�1
        // invertSmallRegion_1(binFrame, MIN_AREA_THRESH); // ��ת���С��MIN_AREA_THRESH������

        // ��תС����ڰ����򷽷�2
        // invertSmallRegion_2(binFrame, MIN_AREA_THRESH); // ��ת���С��MIN_AREA_THRESH������

        // ʹ��һ���˲��㷨
        binImgFilter(binFrame);

        // ����ʶ����ͬһ���У�Ҫ����ʶ�𵽶���෴���أ������϶�Ϊ����===============================================
        std::vector<cv::Point> leftEdgePoints, rightEdgePoints;  // ���������������洢���ұ�Ե��
        findEdgePix(binFrame, leftEdgePoints, rightEdgePoints); // ���Ҷ�ֵͼ������ұ߽����ص����꣬��������������

        size_t min_dop_num = std::min<size_t>(leftEdgePoints.size(), rightEdgePoints.size()); // ��ȡ������������Ԫ��������Сֵ
        std::vector<cv::Point> midPoint; // �����ռ���������֮����е�����ֵ
        for (size_t i = 0; i < min_dop_num; i++) {
            midPoint.push_back(calMidPoint(leftEdgePoints.at(i), rightEdgePoints.at(i))); // ��������е�
            resizedFrame.at<cv::Vec3b>(leftEdgePoints.at(i)) = cv::Vec3b(BGR_BLUE); // �������
            // resizedFrame.at<cv::Vec3b>(midPoint.back()) = cv::Vec3b(BGR_GREEN); // �����ұ���֮����е��ߣ���ѭ����
            resizedFrame.at<cv::Vec3b>(rightEdgePoints.at(i)) = cv::Vec3b(BGR_RED); // ���ұ���
        }
        std::vector<cv::Point> midPointFiltered;
        filterXCoord(midPoint, midPointFiltered, 7); // ���е�����˲�
        for (auto& pt : midPointFiltered) { // �����е�
            resizedFrame.at<cv::Vec3b>(pt) = cv::Vec3b(BGR_PURPLE); // �����ұ���֮����˲�����е��ߣ���ѭ����
        }
        double xl = calAverSlopeFromRowToRow(resizedFrame, midPointFiltered, ROW_UP, ROW_DOWN); // �������ߵ�ƫб
        double py = calAverXCoordFromRowToRow(resizedFrame, midPoint, ROW_UP, ROW_DOWN); // ����X����ƽ��ֵ

        cv::imshow("���ź�Ĳ�ɫͼ", resizedFrame); // �ڶ�Ӧ��������ʾͼ��
        cv::imshow("�����Ķ�ֵ��ͼ��", binFrame); // �ڶ�Ӧ��������ʾͼ��

        clearScreen();
        std::cout << "ͼƬ��saideao" <<pic_file << "/" << pic_index << ".jpg" << std::endl;
        std::cout << "��ֵ��" << th << std::endl;
        std::cout << "ƫб��" << xl << std::endl;
        std::cout << "ƫ�ƣ�" << (resizedFrame.cols-1)/2-py << std::endl;
        std::cout << "ת��" << ((xl < -0.2) ? "����" : ((xl > 0.2) ? "����" : "ֱ��")) << std::endl;




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
 * @brief �ڶ�ֵͼ�����ܻ���һ�����Ϊ1�ĺ�ɫ���α߿�
 * @param binImg ��ֵͼ������ΪCV_8UC1
 */
void drawBlackRectBorder(cv::Mat& binImg) {
    if (binImg.type() != CV_8UC1) { // ���ͼ���Ƿ�Ϊ��ֵͼ��
        std::cerr << "Image is not a binary image!" << std::endl;
        return;
    }
    // ��ȡͼ��ĳߴ�
    int rows = binImg.rows;
    int cols = binImg.cols;
    for (int j=0; j<cols; ++j) { // ��ͼ��Ķ����͵ײ�������
        binImg.at<uint8>(0,j) = 0;             // ����
        binImg.at<uint8>(rows-1,j) = 0;     // �ײ�
    }
    for (int i=0; i<rows; ++i) { // ��ͼ��������Ҳ໭����
        binImg.at<uint8>(i,0) = 0;             // ���
        binImg.at<uint8>(i,cols-1) = 0;     // �Ҳ�
    }
}

/**
 * @brief �Ե������е�Ԫ�ص�x��������˲�
 * @note �˲��㷨���ڴ����ڣ���x������ƽ��
 * @param points ����ĵ�����
 * @param filteredPoints ������˲���ĵ�����
 * @param windowSize �˲����ڴ�С�����������
 */
void filterXCoord(std::vector<cv::Point>& points, std::vector<cv::Point>& filteredPoints, int windowSize) {
    if (points.empty()) return; // ���������Ϊ�գ�ֱ�ӷ���
    filteredPoints.clear(); // �������ĵ�����
    int halfWindow = windowSize / 2; // ���ڵ�һ���С
    size_t pointsSize = points.size(); // �������Ĵ�С
    for (size_t i=0; i<pointsSize; i++) {
        int start = std::max(0, (int)i-halfWindow); // ������ʼλ��
        int end = std::min((int)pointsSize-1, (int)i+halfWindow); // ���ڽ���λ��
        int xFiltered = std::accumulate(
            points.begin() + start,
            points.begin() + end,
            0, [](int sum, const cv::Point& pt) { return (sum + pt.x); }
        ) / (end-start); // ����ƽ��ֵ
        filteredPoints.push_back(cv::Point(xFiltered, points[i].y)); // ��ƽ��ֵ��Ϊ�µĵ�����������
    }
}