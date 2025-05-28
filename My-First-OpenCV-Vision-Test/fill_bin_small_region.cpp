// fill the small region on binary image ����ֵͼ���е�С�������
// ��Ӣ�Ķ��ż��л���һ��ͼ�񣬰�Ӣ�ľ�ż��л���һ��ͼ�񣬰�Esc���˳�����
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "test.hpp"


#define TRACK_PIC_NUM  25 // ����ͼƬ
int test_fill_bin_small_region(void) {
    cv::namedWindow("���ź�Ĳ�ɫͼ", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("ԭ��ֵ��ͼ��", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("�����Ķ�ֵ��ͼ��", cv::WINDOW_NORMAL); // ����һ������
    int pic_index = 0;
    while (true) {
        std::string filename = "./saideao/" + std::to_string(pic_index) + ".jpg";
        cv::Mat frame = cv::imread(filename); // ��ȡһ������ͼƬ

        if (frame.empty()) { // ���û��ȡ��
            std::cout << "Could not read the frame: " << filename << std::endl; // ��ӡδ��ȡ��Ϣ
            return -1;
        }

        cv::Mat resizedFrame; // ԭͼ > ����ͼ��
        cv::Mat grayFrame; // ����ͼ�� > �Ҷ�ͼ��
        cv::Mat binFrame; // �Ҷ�ͼ�� > ��ֵͼ��
        //double th = bgr2bin(frame, resizedFrame, grayFrame, binFrame, cv::Size(RESIZED_WIDTH, RESIZED_HEIGHT)); // OTSU�Զ���ֵ
        double th = bgr2bin(frame, resizedFrame, grayFrame, binFrame, cv::Size(RESIZED_WIDTH, RESIZED_HEIGHT), GRAY2BIN_THRESH, 255); // �Զ�����ֵ
        cv::imshow("ԭ��ֵ��ͼ��", binFrame); // �ڶ�Ӧ��������ʾͼ��

        // ��תС����ڰ����򷽷�1
        // invertSmallRegion_1(binFrame, MIN_AREA_THRESH); // ��ת���С��50������

        // ��תС����ڰ����򷽷�2
        invertSmallRegion_2(binFrame, MIN_AREA_THRESH); // ��ת���С��50������

        cv::imshow("���ź�Ĳ�ɫͼ", resizedFrame); // �ڶ�Ӧ��������ʾͼ��
        cv::imshow("�����Ķ�ֵ��ͼ��", binFrame); // �ڶ�Ӧ��������ʾͼ��

        clearScreen();
        std::cout << "��ֵ��" << th << std::endl;
        std::cout << "ͼƬ��" << pic_index << ".jpg" << std::endl;
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

/*����һ���ں�ɫ�����Ѱ�Ұ�ɫС��ĺ������Ὣ�õ��İ�ɫС��������������뵽vector����centers��*/
void getWhiteCentersOfSmallRegions(const cv::Mat& binImg, double minArea, std::vector<cv::Point>& centers) {
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binImg, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // ��������
    for (auto& contour : contours) { // ������������
        double area = cv::contourArea(contour); // �������������
        if (area < minArea) {  // ������С����С�����ֵ����������ĵ�
            cv::Moments m = cv::moments(contour);
            int cx = static_cast<int>(m.m10 / m.m00);
            int cy = static_cast<int>(m.m01 / m.m00);
            centers.push_back(cv::Point(cx, cy));
        }
    }
}

/*����һ���ڰ�ɫ�����Ѱ�Һ�ɫС��ĺ������Ὣ�õ��ĺ�ɫС��������������뵽vector����centers��*/
void getBlackCentersOfSmallRegions(const cv::Mat& binImg, double minArea, std::vector<cv::Point>& centers) {
    cv::Mat binInvImg; // ԭ��ֵͼ�� > ��ֵͼ��תͼ��
    cv::bitwise_not(binImg, binInvImg); // ��ֵͼ��ת
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binInvImg, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); // ��������
    for (auto& contour : contours) { // ������������
        double area = cv::contourArea(contour); // �������������
        if (area < minArea) {  // ������С����С�����ֵ����������ĵ�
            cv::Moments m = cv::moments(contour);
            int cx = static_cast<int>(m.m10 / m.m00);
            int cy = static_cast<int>(m.m01 / m.m00);
            centers.push_back(cv::Point(cx, cy));
        }
    }
}

/*��ˮ����ֵͼ��ĳһ�������ɫ������Windows��ͼ����ĵ�īˮ*/
void fillBlobRecursive(cv::Mat& binImg, cv::Point pt, uint8_t newColor) {
    if (pt.x < 0 || pt.x >= binImg.cols || pt.y < 0 || pt.y >= binImg.rows) return; // ��������Ƿ񳬳�ͼ��߽�
    uint8_t currentColor = binImg.at<uint8_t>(pt); // ��ȡ��ǰ���ص�ĻҶ�ֵ
    if (currentColor == newColor) return; // �����ǰ���ص��Ѿ�������ɫ�����߲��Ǿ���ɫ���򷵻�
    binImg.at<uint8_t>(pt) = newColor; // ����ǰ���ص����Ϊָ��������ɫ

    /*�ݹ���ã�������������ĸ���������ص�*/
    fillBlobRecursive(binImg, cv::Point(pt.x + 1, pt.y), newColor);
    fillBlobRecursive(binImg, cv::Point(pt.x - 1, pt.y), newColor);
    fillBlobRecursive(binImg, cv::Point(pt.x, pt.y + 1), newColor);
    fillBlobRecursive(binImg, cv::Point(pt.x, pt.y - 1), newColor);
}

/*��ˮ����ɫͼĳһ�������ɫ������Windows��ͼ����ĵ�īˮ*/
// ע������jpg��ʽ����ѹ��������ʱ���ʹһ�������ص�RGBͨ���仯�����º�ˮ��䲢����ȡ�ýϺõ�Ԥ��Ч���������ɫ����ٵ�ʹ�á�
void fillBlobRecursive(cv::Mat& bgrImg, cv::Point pt, cv::Vec3b newColor, cv::Vec3b oldColor) {
    if (pt.x < 0 || pt.x >= bgrImg.cols || pt.y < 0 || pt.y >= bgrImg.rows) return;
    cv::Vec3b currentColor = bgrImg.at<cv::Vec3b>(pt);
    if (currentColor == newColor || currentColor != oldColor) return;
    bgrImg.at<cv::Vec3b>(pt) = newColor;
    /*�ݹ���ã�������������ĸ���������ص�*/
    fillBlobRecursive(bgrImg, cv::Point(pt.x + 1, pt.y), newColor, oldColor);
    fillBlobRecursive(bgrImg, cv::Point(pt.x - 1, pt.y), newColor, oldColor);
    fillBlobRecursive(bgrImg, cv::Point(pt.x, pt.y + 1), newColor, oldColor);
    fillBlobRecursive(bgrImg, cv::Point(pt.x, pt.y - 1), newColor, oldColor);
}

/*��ת��ֵͼ�������С��ָ��ֵ�Ķ�������*/
// ����һ������ֵͼ�������С��ָ��ֵ�Ķ���������������꣬Ȼ��ʹ�ú�ˮ���ķ������������ɫ��ת
void invertSmallRegion_1(cv::Mat& binImg, double minArea) {
    std::vector<cv::Point> centerWhite; // �����洢���С��ĳһ��ֵ�İ�ɫ�������������ֵ
    std::vector<cv::Point> centerBlack; // �����洢���С��ĳһ��ֵ�ĺ�ɫ�������������ֵ
    getWhiteCentersOfSmallRegions(binImg, minArea, centerWhite); // ��ȡ���С��50�İ�ɫ�������������ֵ
    getBlackCentersOfSmallRegions(binImg, minArea, centerBlack); // ��ȡ���С��50�ĺ�ɫ�������������ֵ
    for (auto& cenW : centerWhite)  fillBlobRecursive(binImg, cenW, BIN_BLACK); // ʹ����Щ��������ֵ����ˮ�����Щ��ɫ����
    for (auto& cenB : centerBlack)  fillBlobRecursive(binImg, cenB, BIN_WHITE); // ʹ����Щ��������ֵ����ˮ�����Щ��ɫ����
}

/*��ת��ֵͼ�������С��ָ��ֵ�Ķ�������*/
// ��������ֱ��ʹ��OpenCV��findContours����������������Ȼ��ʹ��drawContours��������ת��ɫ
void invertSmallRegion_2(cv::Mat& binImg, double minArea) {
    /*����AI Copilot GPT-4o*/
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(binImg, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    for (size_t i = 0; i < contours.size(); i++) { // ����ÿ������
        double area = cv::contourArea(contours[i]);
        if (area < minArea) { // ������С����ֵ����ת��ɫ
            cv::drawContours(binImg, contours, static_cast<int>(i), cv::Scalar(255-binImg.at<uint8_t>(contours[i][0])), cv::FILLED);
        }
    }
}