// Multi Threads for camera display ͼ���ȡ��ʾ�Ķ��̳߳���
// ������һ���̻߳�ȡ�����ͼ�������̲߳�������ʾͼ��
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string> // ����ͼ���ϱ�ע�ַ���
#include <chrono>  // ����֡�ʼ����ʱ�����ȡ
#include <memory> // ��������ָ����ڴ����
#include <atomic> // ����ԭ�ӱ���
#include <iomanip> // ���� setprecision �����ͷ�ļ�
#include <thread> 
#include "test.hpp"

std::atomic<bool> shouldExit(false); // �����߳��˳�
std::shared_ptr< cv::VideoCapture> cam;

/*��ʼ��֡����ʱ���*/
int frameCount = 0;
auto startTime = std::chrono::steady_clock::now();;
double frameFPS = 0;

/*ͼ�������*/
cv::Mat frame(CAM_HEIGHT, CAM_WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));
volatile bool hasGetFrame = false;

/*������������˳������߳�*/
void exitAllThreads(void) { shouldExit.store(true); }

/*���̻߳�ȡͼ�񣨴����frame����������֡�ʣ������frameFPS��*/
void getFrame(void) {
    std::cout << "�ѽ��� getFrame �߳�" << std::endl;
    while ( !shouldExit.load() ) {
        /*֡�ʼ���*/
        frameCount++; // ����֡��
        auto currentTime = std::chrono::steady_clock::now(); // ��ȡ��ǰʱ��
        double elapsedTime = std::chrono::duration<double>(currentTime - startTime).count(); // ����ʱ���
        if (elapsedTime >= 1.0) { // ÿ�����һ��֡��
            frameFPS = frameCount / elapsedTime; // ����֡��
            frameCount = 0; startTime = currentTime; // ����֡����ʱ���
        }

        /*ͼ���ȡ*/
        hasGetFrame = cam->read(frame); // �������ȡ�µ�һ֡
        if (!hasGetFrame) { // ���û��ͼ��
            std::cout << "ERROR: �޷���ȡ����ͷ���棡" << std::endl;
            frame = cv::Mat(CAM_HEIGHT, CAM_WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));
        }
    }
    std::cout << "�߳� getFrame ���˳���" << std::endl;
}

int test_multi_threads_cam_disp(void) {
    /*����������÷ֱ���*/
    cam = std::make_shared<cv::VideoCapture>(CAM_INDEX); // �����
    if ( !cam->isOpened() ) { // ����޷������
        std::cerr << "ERROR: �޷�������ͷ��" << std::endl;
        return -1;
    }
    cam->set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH); // ���ÿ��
    cam->set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT); // ���ø߶�
    std::cout << cam->get(cv::CAP_PROP_FRAME_WIDTH) << "*" << cam->get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    // ���������̣߳��ֱ����ڻ�ȡ֡������֡����ʾ֡
    std::shared_ptr<std::thread> thread_get_frame = std::make_shared<std::thread>(getFrame);

    while (true) { // ��ʼѭ��
        cv::Mat frame_clone = frame.clone();
        std::stringstream ss_text; // ����һ���ַ�����
        if (!hasGetFrame) ss_text << "No Image!";
        else ss_text << "FPS:" << std::fixed << std::setprecision(2) << frameFPS; // FPS:xx.xx
        cv::putText(frame_clone, ss_text.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        /*������������ͼ����ʾ*/
        cv::namedWindow("CamDisp", cv::WINDOW_NORMAL); // ����һ������
        cv::imshow("CamDisp", frame_clone); // �ڶ�Ӧ��������ʾͼ��

        /*��ⰴ��*/
        int KeyGet = cv::waitKey(1); // �ȴ�1ms��ȡ����ֵ
        if (KeyGet == KEY_ESC) {
            exitAllThreads(); // �����߳���Ҫ�˳���
            break; // �������Esc�Ļ����˳�ѭ��
        }
    }

    /*�����ȴ��߳��˳������ͷ�������Դ*/
    if (thread_get_frame->joinable()) thread_get_frame->join();

    cam->release(); // �ͷ������Դ
    std::cout << "�����Դ���ͷ�" << std::endl; // �����־��������Դ�ͷ�
    cv::destroyAllWindows(); // �ر�����OpenCV�����Ĵ���
    std::cout << "���д����ѹر�" << std::endl; // �����־�����ٴ��ڹر�

    std::cout << "���߳��˳�����������ȫ�رա�" << std::endl;

    return 0;
}
