// camera and display ��ȡ�����ͼ����ʾ
// ���������������ȡ���ͼ����ʾ��һ����Ϊcam�Ĵ����У�����Esc���˳�����
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string> // ����ͼ���ϱ�ע�ַ���
#include <chrono>  // ����֡�ʼ����ʱ�����ȡ
#include <iomanip> // ���� setprecision �����ͷ�ļ�

int main(void) {
    /*����������÷ֱ���*/
    cv::VideoCapture cam(0); // �����
    if (!cam.isOpened()) { // ����޷������
        std::cerr << "ERROR: �޷�������ͷ��" << std::endl;
        return -1;
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, 1280); // ���ÿ��
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, 720); // ���ø߶�
    std::cout << cam.get(cv::CAP_PROP_FRAME_WIDTH) << "*" << cam.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    /*������������ͼ����ʾ*/
    cv::namedWindow("CamDisp", cv::WINDOW_AUTOSIZE); // ����һ������

    /*��ʼ��֡����ʱ���*/
    int frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();
    double fps = 0;

    /*ͼ�������*/
    cv::Mat frame;

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

        /*ͼ�����*/
        cv::putText(frame, ss_fps.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2); // д��֡��ֵ

        /*��ʾͼ��*/
        cv::imshow("CamDisp", frame); // �ڶ�Ӧ��������ʾͼ��

        /*��ⰴ��*/
        int KeyGet = cv::waitKey(1); // �ȴ�1ms��ȡ����ֵ
        if (KeyGet == 27) break; // �������Esc�Ļ����˳�ѭ��
    }

    cam.release(); // �ͷ������Դ
    std::cout << "�����Դ���ͷ�" << std::endl; // �����־��������Դ�ͷ�
    cv::destroyAllWindows(); // �ر�����OpenCV�����Ĵ���
    std::cout << "���д����ѹر�" << std::endl; // �����־�����ٴ��ڹر�

    return 0;
}
