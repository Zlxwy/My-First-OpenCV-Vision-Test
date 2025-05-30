// track recognition for static images ��̬ͼ��Ĺ켣ʶ��
// ��Ӣ�Ķ��ż��л���һ�ţ���Ӣ�ľ�ż��л���һ�ţ���Ӣ��б�˼��л���һ����Ƭ
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include "test.hpp"

#define PIC_FILE_NUM 2 // ����ͼƬ�ļ��е�����
#define OBSTACLES_PIC_NUM  20 // ����ͼƬ
int test_avoid_obstacles(void) {
    cv::namedWindow("���ź�Ĳ�ɫͼ", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("ԭ��ֵ��ͼ��", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("�����Ķ�ֵ��ͼ��", cv::WINDOW_NORMAL); // ����һ������
    int pic_file = 0, pic_index = 0;
    while (true) {
        std::string filename = "./images/flag_zhangai/" + std::to_string(pic_index) + ".jpg";
        //std::string filename = "./saideao_0/" + std::to_string(pic_index) + ".jpg";
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

        // ʹ��һ���˲��㷨�����ֵ��ͼ��
        binImgFilter(binFrame);

        // ����ʶ��=================================================================================================
        std::vector<cv::Point> leftEdgePoints, rightEdgePoints;  // ���������������洢Ѱ�ҵ������ұ�Ե�������ֵ
        std::vector<cv::Point> leftEdgePointsFiltered, rightEdgePointsFiltered; // ���������������洢Ѱ�ҵ������ұ�Ե���x����ֵ�˲��������ֵ
        findEdgePix(binFrame, leftEdgePoints, rightEdgePoints); // ���Ҷ�ֵͼ������ұ߽����ص����꣬��������������
        filterXCoord(leftEdgePoints, leftEdgePointsFiltered, 25); // ����߽���x����ֵ�����˲�����
        filterXCoord(rightEdgePoints, rightEdgePointsFiltered, 25); // ���ұ߽���x����ֵ�����˲�����

        size_t min_dop_num = std::min<size_t>(leftEdgePointsFiltered.size(), rightEdgePointsFiltered.size()); // ��ȡ������������Ԫ��������Сֵ
        std::vector<cv::Point> midPoint; // �����ռ���������֮����е�����ֵ
        for (size_t i = 0; i < min_dop_num; i++) {
            midPoint.push_back(calMidPoint(leftEdgePointsFiltered.at(i), rightEdgePointsFiltered.at(i))); // ��������е�
            resizedFrame.at<cv::Vec3b>(leftEdgePointsFiltered.at(i)) = cv::Vec3b(BGR_BLUE); // �������
            resizedFrame.at<cv::Vec3b>(rightEdgePointsFiltered.at(i)) = cv::Vec3b(BGR_RED); // ���ұ���
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
        std::cout << "ͼƬ��saideao" << pic_file << "/" << pic_index << ".jpg" << std::endl;
        std::cout << "��ֵ��" << th << std::endl;
        std::cout << "ƫб��" << xl << std::endl;
        std::cout << "ƫ�ƣ�" << (resizedFrame.cols - 1) / 2 - py << std::endl;
        std::cout << "ת��" << ((xl < -0.2) ? "����" : ((xl > 0.2) ? "����" : "ֱ��")) << std::endl;




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