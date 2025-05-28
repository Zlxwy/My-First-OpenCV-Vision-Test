// ��ͼ���������任���ֱ���ʾ������������
#include <opencv2/opencv.hpp>
#include <iostream>
#include "test.hpp"

int test_perspective_transform(void) {
    cv::VideoCapture cam(CAM_INDEX); // �����
    if (!cam.isOpened()) { // ����޷������
        std::cerr << "ERROR: �޷�������ͷ��" << std::endl;
        return -1;
    }
    cam.set(cv::CAP_PROP_FRAME_WIDTH, CAM_WIDTH); // ���ÿ��
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT); // ���ø߶�
    std::cout << cam.get(cv::CAP_PROP_FRAME_WIDTH) << "*" << cam.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    /*������������ͼ����ʾ*/
    cv::namedWindow("windows1", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("windows2", cv::WINDOW_NORMAL); // ����һ������

    /*��ʼ��֡����ʱ���*/
    int frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();
    double fps = 0;

    while (true) { // ��ʼѭ��
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

        cv::Mat frame; // ��ȡԭͼ��
        cv::Mat result; // ���ձ任���ͼ��
        bool ret = cam.read(frame); // �������ȡ�µ�һ֡
        if (!ret) { // ���û��ͼ��
            std::cout << "ERROR: �޷���ȡ����ͷ���棡" << std::endl;
            break;
        }
        int sui = 200; // �����λ��
        cv::Point2f pts1[4] = {
            cv::Point2f(sui, 0),
            cv::Point2f(CAM_WIDTH-1-sui, 0),
            cv::Point2f(0, CAM_HEIGHT-1),
            cv::Point2f(CAM_WIDTH-1, CAM_HEIGHT-1)
        };
        cv::Point2f pts2[4] = {
            cv::Point2f(sui, 0),
            cv::Point2f(CAM_WIDTH-1-sui, 0),
            cv::Point2f(sui, CAM_HEIGHT-1),
            cv::Point2f(CAM_WIDTH-1-sui, CAM_HEIGHT-1)
        };
        cv::Mat matrix = cv::getPerspectiveTransform(pts1, pts2); // ����͸�ӱ任����
        cv::warpPerspective(frame, result, matrix, cv::Size(CAM_WIDTH, CAM_HEIGHT)); // Ӧ��͸�ӱ任

        /*����Դͼ���С�Ŀ��ͼ���е��ĸ���*/
        for (auto &pt : pts1) cv::circle(frame, pt, 10, cv::Scalar(0, 255, 0), -1);
        for (auto &pt : pts2) cv::circle(result, pt, 10, cv::Scalar(0, 255, 0), -1);
        /*��ͼ����д��֡��ֵ*/
        cv::putText(frame, ss.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
        cv::putText(result, ss.str(), cv::Point(0, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        cv::imshow("windows1", frame); // �ڴ���1����ʾԭͼ��
        cv::imshow("windows2", result); // �ڴ���2����ʾ������ͼ��

        int KeyGet = cv::waitKey(1); // �ȴ�1ms��ȡ����ֵ
        if (KeyGet == KEY_ESC) break; // �������Esc�Ļ����˳�ѭ��

    }

    cam.release(); // �ͷ������Դ
    std::cout << "�����Դ���ͷ�" << std::endl; // �����־��������Դ�ͷ�
    cv::destroyAllWindows(); // �ر�����OpenCV�����Ĵ���
    std::cout << "���д����ѹر�" << std::endl; // �����־�����ٴ��ڹر�

    return 0;
}
