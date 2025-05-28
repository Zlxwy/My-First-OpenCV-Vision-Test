// calculate steering direction ����ת����
// calculate steering angle ����ת��Ƕ�
// �ڵõ�ѭ���ߺ󣬼���ת��ķ����ڳ���track_rcgn_animated�Ļ����ϼ������������ȡ��ͼ��
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <numeric> // ���� std::accumulate ����������Ԫ���ܺ�
#include "test.hpp"

#define UP_LINE_INDEX  50
#define DOWN_LINE_INDEX 70

int test_cal_steering_dir(void) {
    /*����������÷ֱ���*/
    cv::VideoCapture cam(CAM_INDEX); // �����
    if (!cam.isOpened()) { // ����޷������
        std::cerr << "ERROR: �޷�������ͷ��" << std::endl;
        return -1;
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH); // ���ÿ��
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT); // ���ø߶�
    std::cout << cam.get(cv::CAP_PROP_FRAME_WIDTH) << "*" << cam.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    /*������������ͼ����ʾ*/
    cv::namedWindow("RGB Picture", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("BIN Picture", cv::WINDOW_NORMAL); // ����һ������

    /*��ʼ��֡����ʱ���*/
    int frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();
    double fps = 0;

    /*ͼ�������*/
    cv::Mat frame;
    cv::Mat resizedFrame;
    cv::Mat grayFrame;
    cv::Mat binFrame;

    while (true) { // ��ʼѭ��
        /*֡�ʼ���*/
        frameCount++; // ����֡��
        auto currentTime = std::chrono::steady_clock::now(); // ��ȡ��ǰʱ��
        double elapsedTime = std::chrono::duration<double>(currentTime - startTime).count(); // ����ʱ���
        if (elapsedTime >= 1.0) { // ÿ�����һ��֡��
            fps = frameCount / elapsedTime; // ����֡��
            frameCount = 0; startTime = currentTime; // ����֡����ʱ���
        }
        std::stringstream ss_fps; // ����һ���ַ�����
        ss_fps << "FPS:" << std::fixed << std::setprecision(2) << fps; // FPS:xx.xx


        /*ͼ���ȡ*/
        bool ret = cam.read(frame); // �������ȡ�µ�һ֡
        if (!ret) { // ���û��ͼ��
            std::cout << "ERROR: �޷���ȡ����ͷ���棡" << std::endl;
            break;
        }


        /*ͼ��ת��*/
        cv::resize(frame, resizedFrame, cv::Size(RESIZED_WIDTH, RESIZED_HEIGHT));
        cv::cvtColor(resizedFrame, grayFrame, cv::COLOR_BGR2GRAY); // ��ԭʼͼ��ת��Ϊ�Ҷ�ͼ
        cv::threshold(grayFrame, binFrame, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);


        /*ͼ����1*/
        std::vector<cv::Point> leftEdgePoints, rightEdgePoints;  // ���������������洢���ұ�Ե��
        findEdgePix(binFrame, leftEdgePoints, rightEdgePoints); // ���Ҷ�ֵͼ������ұ߽����ص����꣬��������������


        /*ͼ�����*/
        size_t min_dop_num = std::min<size_t>(leftEdgePoints.size(), rightEdgePoints.size()); // ��ȡ������������Ԫ��������Сֵ
        std::vector<cv::Point> midPoint; // �����ռ���������֮����е�����ֵ
        for (size_t i = 0; i < min_dop_num; i++) {
            midPoint.push_back( calMidPoint(leftEdgePoints.at(i),rightEdgePoints.at(i)) ); // ��������е�
            resizedFrame.at<cv::Vec3b>(leftEdgePoints.at(i)) = cv::Vec3b(255, 0, 0); // �������
            resizedFrame.at<cv::Vec3b>(midPoint.back()) = cv::Vec3b(0, 255, 0); // �����ұ���֮����е��ߣ���ѭ����
            resizedFrame.at<cv::Vec3b>(rightEdgePoints.at(i)) = cv::Vec3b(0, 0, 255); // ���ұ���
        }
        cv::putText(resizedFrame, ss_fps.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2); // д��֡��ֵ

        /*ͼ����2*/
        clearScreen();
        double xl = calAverSlopeFromRowToRow(resizedFrame, midPoint, UP_LINE_INDEX, DOWN_LINE_INDEX);
        std::cout << "б�ʣ�" << xl << std::endl;
        // �������ת����ʾL����ת����ʾR��ֱ������ʾS
        std::stringstream LRS_ss;
        cv::Scalar color;
        if (xl <= -0.2f) LRS_ss << "R:" << std::fixed << std::setprecision(2) << std::abs(xl), color = cv::Scalar(0, 0, 255); // R:x.xx
        else if (xl >= +0.2f) LRS_ss << "L:" << std::fixed << std::setprecision(2) << xl, color = cv::Scalar(255, 0, 0); // L:x.xx
        else LRS_ss << "Go", color = cv::Scalar(0,255,0); // Go
        cv::putText(resizedFrame, LRS_ss.str(), cv::Point(0, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, color, 2); // ��ʾ����



        /*ͼ����ʾ*/
        cv::imshow("RGB Picture", resizedFrame); // �ڶ�Ӧ��������ʾͼ��
        cv::imshow("BIN Picture", binFrame); // �ڶ�Ӧ��������ʾͼ��



        /*��ⰴ��*/
        int KeyGet = cv::waitKey(1); // �ȴ�1ms��ȡ����ֵ
        if (KeyGet == KEY_ESC) break; // �������Esc�Ļ����˳�ѭ��
    }

    cam.release(); // �ͷ������Դ
    std::cout << "�����Դ���ͷ�" << std::endl; // �����־��������Դ�ͷ�
    cv::destroyAllWindows(); // �ر�����OpenCV�����Ĵ���
    std::cout << "���д����ѹر�" << std::endl; // �����־�����ٴ��ڹر�
    return 0;
}

/*��ͼ���ϵ�ָ���л�һ��ֱ��*/
void drawLineOnRow(cv::Mat& img, int rowIndex, cv::Scalar color, int thickness) {
    cv::line(img, cv::Point(0, rowIndex), cv::Point(img.cols - 1, rowIndex), color, thickness);
}

/*����һ��double����������ƽ��ֵ*/
// �������
// 8 2 7
// (0 * 0 + 8) / 1 = 8
// (8 * 1 + 2) / 2 = 5
// (5 * 2 + 7) / 3 �� 5.67
// �仯��
// 0 / 1 * 0 + 8 / 1 = 8
// 8 / 2 * 1 + 2 / 2 = 5
// 5 / 3 * 2 + 7 / 3 �� 5.67
// �������Է�ֹ�ӵ��ܺ͹��󣬵������ݳ������ͱ�ʾ��Χ��
double myCalAverage(const std::vector<double> vdata) {
    if ( vdata.empty() ) return 0.0f; // ����Ĭ�Ϲ����ֵ�����ڸ������� 0.0����������Ҳ�� 0
    else {
        double cnt=0, numMark=0;
        for (auto& v : vdata) {
            numMark = numMark / (cnt + 1) * cnt + v / (cnt + 1); // �ȳ������Ϊ�˷�ֹ�������
            cnt += 1;
        }
        return numMark;
    }
}

template <typename T>
T calAverage(const std::vector<T> vdata) {
    if ( vdata.empty() ) return T(); // ���vector�����ǿյģ�����0
    else return ( std::accumulate(vdata.begin(), vdata.end(), T()) / (T)vdata.size() );
}

// Calculate average slope from row to row
// ��������֮�����ߵ��ƽ��б�ʣ���ȷ��������һ��ת��
double calAverSlopeFromRowToRow(cv::Mat& img, std::vector<cv::Point> midPoint, int row_up, int row_down) {
    drawLineOnRow(img, row_up, cv::Scalar(BGR_ICEBLUE), 1); // �ڵ�row_up�л�һ�����Ϊ1��ֱ��
    drawLineOnRow(img, row_down, cv::Scalar(BGR_ICEBLUE), 1); // �ڵ�row_down�л�һ�����Ϊ1��ֱ��
    cv::Point pointMark(0, 0); // ���ڼ�¼ǰһ�ε�����ֵ���뵱ǰ����ֵ���м���ó�б��
    bool isFirst = true; // �����ڵ�һ�ν����ʱ��ֻ��¼ֵ��������
    std::vector<double> slope; // ���ڼ�¼б��
    for (auto& pt : midPoint) { // ����ѭ���ߵ����ص�
        if (pt.y > row_up && pt.y < row_down) { // �����row_up�е�row_down��֮��
            if (isFirst) { // ����ǵ�һ�ν���
                pointMark.x = pt.x; // ��¼һ��x����
                pointMark.y = pt.y; // ��¼һ��y����
                isFirst = false; // ֮��Ͳ��ǵ�һ�ν�����
            }
            else { // ������ǵ�һ�ν�����
                slope.push_back((double)(pt.x - pointMark.x) / (double)(pt.y - pointMark.y)); // �����б�ʣ�����������
                pointMark.x = pt.x; // ���¼�¼x����
                pointMark.y = pt.y; // ���¼�¼y����
            }
            img.at<cv::Vec3b>(pt) = cv::Vec3b(BGR_BLACK); // ����������ص���
        }
        else continue; // ���������֮�䣬��ѭ������һ���㣨��ʵ�������Ӳ��Ӷ�û��ϵ��������Ҫ��һ��ѭ���ˣ�
    }
    return calAverage(slope);
}

// Calculate average x coordinate from row to row
// ��������֮�����ߵ��x����ƽ��ֵ��Ҳ��Ϊ������һ��ת���һ��Ӱ������
double calAverXCoordFromRowToRow(cv::Mat& img, std::vector<cv::Point> midPoint, int row_up, int row_down) {
    drawLineOnRow(img, row_up, cv::Scalar(BGR_ICEBLUE), 1); // �ڵ�row_up�л�һ�����Ϊ1��ֱ��
    drawLineOnRow(img, row_down, cv::Scalar(BGR_ICEBLUE), 1); // �ڵ�row_down�л�һ�����Ϊ1��ֱ��
    cv::line(img, cv::Point((img.cols-1)/2, row_up), cv::Point((img.cols - 1) / 2, row_up-10), cv::Scalar(BGR_ICEBLUE), 1); // ���м仭һ����
    cv::line(img, cv::Point((img.cols-1)/2, row_down), cv::Point((img.cols - 1) / 2, row_down+10), cv::Scalar(BGR_ICEBLUE), 1); // ���м仭һ����
    std::vector<double> xCoord; // ���ڼ�¼x����
    for (auto& pt: midPoint) {
        if (pt.y > row_up && pt.y < row_down) {
            xCoord.push_back(pt.x); // ��¼x����
        }
        else continue; // ���������֮�䣬��ѭ������һ���㣨��ʵ�������Ӳ��Ӷ�û��ϵ��������Ҫ��һ��ѭ���ˣ�
    }
    return calAverage(xCoord);
}