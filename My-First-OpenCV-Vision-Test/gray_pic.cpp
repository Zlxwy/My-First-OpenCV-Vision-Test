// gray picture display �Ҷ�ͼ����ʾ
// ����һ����������ʾԭͼ��һ��������ʾ�Ҷ�ͼ��һ��������ʾ�ڰ�ͼ��
#include <opencv2/opencv.hpp>
#include <iostream>
#include <Windows.h> // ������ʱ����������Windowns����ϵͳ
#include <iomanip> // ���ڸ�ʽ�����
#include "test.hpp"

int test_gray_pic(void) {
    cv::VideoCapture cam(CAM_INDEX); // �����
    if (!cam.isOpened()) { // ����޷������
        std::cerr << "ERROR: �޷�������ͷ��" << std::endl;
        return -1;
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH); // ���ÿ��
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT); // ���ø߶�
    std::cout << cam.get(cv::CAP_PROP_FRAME_WIDTH) << "*" << cam.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    /*������������ͼ����ʾ*/
    cv::namedWindow("Original Picture", cv::WINDOW_AUTOSIZE); // ����һ������
    cv::namedWindow("Gray Picture", cv::WINDOW_AUTOSIZE); // ����һ������
    cv::namedWindow("Binary Picture", cv::WINDOW_AUTOSIZE); // ����һ������

    /*��ʼ��֡����ʱ���*/
    int frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();
    double fps = 0;

    while (true) { // ��ʼѭ��
        /*֡�ʼ���*/
        frameCount++; // ����֡��
        auto currentTime = std::chrono::steady_clock::now(); // ��ȡ��ǰʱ��
        double elapsedTime = std::chrono::duration<double>(currentTime - startTime).count(); // ����ʱ���
        // ÿ�����һ��֡��
        if (elapsedTime >= 1.0) {
            fps = frameCount / elapsedTime; // ����֡��
            frameCount = 0; startTime = currentTime; // ����֡����ʱ���
        }
        std::stringstream ss; // ����һ���ַ�����
        ss << "FPS:" << std::fixed << std::setprecision(2) << fps; // FPS:xx.xx

        /*ͼ�������*/
        cv::Mat frame;
        cv::Mat grayFrame;
        cv::Mat binaryFrame;

        /*ͼ���ȡ*/
        bool ret = cam.read(frame); // �������ȡ�µ�һ֡
        if (!ret) { // ���û��ͼ��
            std::cout << "ERROR: �޷���ȡ����ͷ���棡" << std::endl;
            break;
        }
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::threshold(grayFrame, binaryFrame, 128, 255, cv::THRESH_BINARY);

        /*ͼ�����*/
        cv::putText(frame, ss.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2); // д��֡��ֵ
        
        ///*������ʾͼ��*/
        cv::imshow("Original Picture", frame); // ��ʾͼ��
        cv::imshow("Gray Picture", grayFrame); // ��ʾͼ��
        cv::imshow("Binary Picture", binaryFrame); // ��ʾͼ��

        /*�������*/
        int KeyGet = cv::waitKey(1); // �ȴ�1ms��ȡ����ֵ
        if (KeyGet == KEY_ESC) break; // �������Esc�Ļ����˳�ѭ��
    }

    cam.release(); // �ͷ������Դ
    std::cout << "�����Դ���ͷ�" << std::endl; // �����־��������Դ�ͷ�
    cv::destroyAllWindows(); // �ر�����OpenCV�����Ĵ���
    std::cout << "���д����ѹر�" << std::endl; // �����־�����ٴ��ڹر�

    return 0;
}