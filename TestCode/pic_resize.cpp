// ��ͼ���������
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string> // ����ͼ���ϱ�ע�ַ���
#include <chrono>  // ����֡�ʼ����ʱ�����ȡ
#include <vector> // ����������ͼ������ʾ�������ַ���
#include "test.hpp"

int test_pic_resize(void) {
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
    cv::namedWindow("Resized Picture", cv::WINDOW_AUTOSIZE); // ����һ������

    /*��ʼ��֡����ʱ���*/
    int frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();
    double fps = 0;

    while (true) { // ��ʼѭ��
        /*֡�ʼ���*/
        frameCount++; // ����֡��
        auto currentTime = std::chrono::steady_clock::now(); // ��ȡ��ǰʱ��
        double elapsedTime = std::chrono::duration<double>(currentTime - startTime).count(); // ����ʱ���
        if (elapsedTime >= 1.0) { // ÿ�����һ��֡��
            fps = frameCount / elapsedTime; // ����֡��
            frameCount = 0; startTime = currentTime; // ����֡����ʱ���
        }
        std::stringstream ss; // ����һ���ַ�����
        ss << "FPS:" << std::fixed << std::setprecision(2) << fps; // FPS:xx.xx

        /*ͼ�������*/
        cv::Mat frame;
        cv::Mat resizedFrame;

        /*ͼ���ȡ*/
        bool ret = cam.read(frame); // �������ȡ�µ�һ֡
        if (!ret) { // ���û��ͼ��
            std::cout << "ERROR: �޷���ȡ����ͷ���棡" << std::endl;
            break;
        }

        /*ͼ�����*/
        cv::resize(frame, resizedFrame, cv::Size(160, 120)); // ������160x120
        std::vector<std::string> pix;
        pix.push_back(std::to_string(frame.rows) + "*" + std::to_string(frame.cols));
        pix.push_back(std::to_string(resizedFrame.rows) + "*" + std::to_string(resizedFrame.cols));

        /*��ʾͼ��*/
        cv::imshow("Original Picture", frame); // �ڶ�Ӧ��������ʾͼ��
        cv::imshow("Resized Picture", resizedFrame); // �ڶ�Ӧ��������ʾͼ��

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
