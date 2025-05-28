// �ն���ʾͼ��
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string> // ����ͼ���ϱ�ע�ַ���
#include <chrono>  // ����֡�ʼ����ʱ�����ȡ
#include <vector> // ����������ͼ������ʾ�������ַ���
#include <stdlib.h>
#include "test.hpp"

int test_terminal_pic_disp(void) {
    cv::VideoCapture cam(CAM_INDEX);
    if ( !cam.isOpened() ) { // ����޷������
        std::cerr << "ERROR: �޷�������ͷ��" << std::endl;
        return -1;
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH); // ���ÿ��
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT); // ���ø߶�
    std::cout << cam.get(cv::CAP_PROP_FRAME_WIDTH) << "*" << cam.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    /*������������ͼ����ʾ*/
    cv::namedWindow("gray Picture", cv::WINDOW_AUTOSIZE); // ����һ������

    /*��ʼ��֡����ʱ���*/
    int frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();
    double fps = 0;

    /*ͼ�������*/
    cv::Mat frame; // ԭʼͼ��
    cv::Mat grayFrame; // �Ҷ�ͼ
    cv::Mat grayResizedFrame; // �Ҷ�ͼ�������ź��ͼ��

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

        /*ͼ���ȡ*/
        bool ret = cam.read(frame); // �������ȡ�µ�һ֡
        if (!ret) { // ���û��ͼ��
            std::cout << "ERROR: �޷���ȡ����ͷ���棡" << std::endl;
            break;
        }

        /*ͼ�����*/
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY); // ��ԭͼת��Ϊ�Ҷ�ͼ
        cv::resize(grayFrame, grayResizedFrame, cv::Size(80, 60)); // �ѻҶ�ͼ������160x120
        cv::putText(grayFrame, ss.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0,0,0), 2); // д��֡��ֵ

        /*�ն���ʾͼ��*/
        clearScreen(); // ����ն�
        //std::system("cls");
        const char* chars = " .-:+=?#%@"; // �Ӻڵ��ף�ӳ����0~255
        for (int i=0; i<grayResizedFrame.rows; i++) { // ����ÿһ��
            for (int j=0; j<grayResizedFrame.cols; j++) {  // ����һ���ڵ��������أ��У�
                int avg = grayResizedFrame.at<uint8_t>(i, j); // ��ȡ������λ�õĻҶ�ֵ
                int index = avg * 9 / 255;                  // �������Ӧ�ַ�������
                printf("%c%c", chars[index], chars[index]);  // ��ӡ�����ַ����պ��Ǹ�������
            }
            printf("\n"); // ��׼����һ��ʱ��ӡһ�����з�
        }
        printf("\n%s\nOriginal Size: %d*%d\n", ss.str().c_str(), grayResizedFrame.cols, grayResizedFrame.rows); // ��ʾ֡�ʡ�����������

        /*��ʾͼ��*/
        cv::imshow("gray Picture", grayFrame); // �ڶ�Ӧ��������ʾԭ�ߴ�ĻҶ�ͼ��

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