// track recognition for animated images ��̬ͼ��Ĺ켣ʶ��
// ��̬������ͼ��ʶ���ڳ���track_rcgn_static�ļ��������ȥ���������ȡ��ͼ��
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "test.hpp"

int test_track_rcgn_animated(void) {
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
    cv::namedWindow("���ź�Ĳ�ɫͼ", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("ԭ��ֵ��ͼ��", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("�����Ķ�ֵ��ͼ��", cv::WINDOW_NORMAL); // ����һ������

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
            resizedFrame.at<cv::Vec3b>(midPoint.back()) = cv::Vec3b(BGR_GREEN); // �����ұ���֮����е��ߣ���ѭ����
            resizedFrame.at<cv::Vec3b>(rightEdgePoints.at(i)) = cv::Vec3b(BGR_RED); // ���ұ���
        }
        double xl = calAverSlopeFromRowToRow(resizedFrame, midPoint, ROW_UP, ROW_DOWN); // �������ߵ�ƫб
        double py = calAverXCoordFromRowToRow(resizedFrame, midPoint, ROW_UP, ROW_DOWN); // ����X����ƽ��ֵ

        cv::imshow("���ź�Ĳ�ɫͼ", resizedFrame); // �ڶ�Ӧ��������ʾͼ��
        cv::imshow("�����Ķ�ֵ��ͼ��", binFrame); // �ڶ�Ӧ��������ʾͼ��

        clearScreen();
        std::cout << "֡�ʣ�" << ss_fps.str() << std::endl;
        std::cout << "��ֵ��" << th << std::endl;
        std::cout << "ƫб��" << xl << std::endl;
        std::cout << "ƫ�ƣ�" << py << std::endl;
        std::cout << "ת��" << ((xl < -0.2) ? "����" : ((xl > 0.2) ? "����" : "ֱ��")) << std::endl;



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

/**
 * @brief ����һ�Ŷ�ֵ��ͼ�񣬲������ҵı߽��ѹ�뵽����������
 * @note  ��Ѱ�����ǣ������ҡ����ϵ��£�����0������ʼ��
 * @param binImg ��ֵ��ͼ��
 * @param LeftEdgePoints �ռ���߽�����������
 * @param rightEdgePoints �ռ��ұ߽�����������
 */
// ʹ����һ�ַ��������ұ���
#define use_findEdgePix  5 // ʹ�õڼ��ַ���
void findEdgePix(const cv::Mat binImg, std::vector<cv::Point>& leftEdgePoints, std::vector<cv::Point>& rightEdgePoints) {
#if use_findEdgePix == 1
    // 1���򵥵ı���ʶ�𣺴���������Ѱ�� ��ת�� �� ��ת�� �ĵ㣬�ֱ��������ұ߽��===============================================
    bool findLeftEdge = false, findRightEdge = false;
    for (int r = 0; r < binImg.rows; r++) {
       findLeftEdge = true; findRightEdge = false;
       for (int c = 0; c < binImg.cols; c++) {
           uint8_t pix = binImg.at<uint8_t>(r, c);
           if (findLeftEdge && pix == BIN_WHITE) {
               leftEdgePoints.push_back(cv::Point(c, r));
               findLeftEdge = false;
               findRightEdge = true;
           }
           else if (findRightEdge && pix == BIN_BLACK) {
               rightEdgePoints.push_back(cv::Point(c, r));
               findLeftEdge = true;
               findLeftEdge = false;
               break;
           }
       }
    }

#elif use_findEdgePix == 2
    // 2�����л��ɵı���ʶ�𣺱�����ͬһ���У��ɺ�ת�ף�Ҫ����ʶ�𵽶����ɫ���أ������϶�Ϊ�����===============================================
    bool findLeftEdge = false, findRightEdge = false; // �Ƿ��ڲ�������ߣ��Ƿ��ڲ����ұ���
    bool leftEdge_huaiyi = false, rightEdge_huaiyi = false; // �����Ƿ�Ϊ������
    uint8_t leftEdge_huaiyi_cnt = 0, rightEdge_huaiyi_cnt = 0; // ���ɼ���
    constexpr auto leftEdge_huaiyi_cnt_MAX = 10, rightEdge_huaiyi_cnt_MAX = 5; // Ҫ������������ô��Ρ����Żᱻ�϶�Ϊ����
    cv::Point leftEdge_huaiyi_mark = cv::Point(0, 0), rightEdge_huaiyi_mark = cv::Point(0, 0); // �����Ƿ�Ϊ���ߵı�ǵ�
    for (int r = 0; r < binImg.rows; r++) {
        /*��ÿһ����ʼʱ*/
        findLeftEdge = true; findRightEdge = false;
        leftEdge_huaiyi = false; rightEdge_huaiyi = false;
        leftEdge_huaiyi_cnt = 0; rightEdge_huaiyi_cnt = 0;
        leftEdge_huaiyi_mark = cv::Point(0, 0); rightEdge_huaiyi_mark = cv::Point(0, 0);
        for (int c = 0; c < binImg.cols; c++) {
            uint8_t pix = binImg.at<uint8_t>(r, c);
            if (findLeftEdge) {
                /*û�л���ʱ�����ְ�ɫ���ˣ��û��ɣ�����¼��ǰ��ɫ������*/
                if (!leftEdge_huaiyi && pix == BIN_WHITE) {
                    leftEdge_huaiyi = true; // �û���
                    leftEdge_huaiyi_cnt++; // ���ɶ�+1
                    leftEdge_huaiyi_mark = cv::Point(c, r); // ��¼��ǰ��������
                }
                /*��û���ɵ����ޣ������˰�ɫ�飬���ɼ���*/
                else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt < leftEdge_huaiyi_cnt_MAX && pix == BIN_WHITE) {
                    leftEdge_huaiyi_cnt++; // ���ɶ�+1
                }
                /*��û���ɵ����ޣ����ֺ�ɫ���ˣ��������ɣ����ɶ�����*/
                else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt < leftEdge_huaiyi_cnt_MAX && pix == BIN_BLACK) {
                    leftEdge_huaiyi = false; // ��������
                    leftEdge_huaiyi_cnt = 0; // ���ɶ�����
                }
                /*���ɵ������ˣ�ȷ����������߽���*/
                else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt >= leftEdge_huaiyi_cnt_MAX) {
                    leftEdgePoints.push_back(leftEdge_huaiyi_mark); // ��¼�����߽��
                    findLeftEdge = false; findRightEdge = true; // ��ʼ��Ѱ�ұ߽�
                }
            }
            else if (findRightEdge) {
                /*���һֱû�ҵ��ұ���*/
                if (c == binImg.cols - 1) {
                    // ��û�л��ɣ���ֱ�Ӱ��������һ����ӽ�ȥ
                    if (!rightEdge_huaiyi) rightEdgePoints.push_back(cv::Point(c, r));
                    // �л����ˣ�����û�л��ɵ����ޣ���ѻ��ɵĵ�ӽ�ȥ
                    else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt <= rightEdge_huaiyi_cnt_MAX) {
                        rightEdgePoints.push_back(rightEdge_huaiyi_mark);
                    }
                }
                /*û�л���ʱ�����ֺ�ɫ���ˣ��û��ɣ�����¼��ǰ��ɫ������*/
                else if (!rightEdge_huaiyi && pix == BIN_BLACK) {
                    rightEdge_huaiyi = true; // �û���
                    rightEdge_huaiyi_cnt++; // ���ɶ�+1
                    rightEdge_huaiyi_mark = cv::Point(c, r); // ��¼��ǰ��������
                }
                /*��û���ɵ����ޣ������˺�ɫ�飬���ɼ���*/
                else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt < rightEdge_huaiyi_cnt_MAX && pix == BIN_BLACK) {
                    rightEdge_huaiyi_cnt++; // ���ɶ�+1
                }
                /*��û���ɵ����ޣ����ְ�ɫ���ˣ��������ɣ����ɶ�����*/
                else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt < rightEdge_huaiyi_cnt_MAX && pix == BIN_WHITE) {
                    rightEdge_huaiyi = false; // ��������
                    rightEdge_huaiyi_cnt = 0; // ���ɶ�����
                }
                /*���ɵ������ˣ�ȷ���������ұ߽���*/
                else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt >= rightEdge_huaiyi_cnt_MAX) {
                    rightEdgePoints.push_back(rightEdge_huaiyi_mark); // ��¼����ұ߽��
                    findLeftEdge = true; findRightEdge = false; // ��ʼ��Ѱ��߽�
                    break;
                }
            }
        }
    }

#elif use_findEdgePix == 3
    // 3�����л��ɣ����е��������ұ��ߣ�Ҫ����ʶ�𵽶����ɫ���أ������϶�Ϊ����===============================================
    bool huaiyi = false; // �Ƿ���
    uint8_t huaiyi_cnt = 0; // ���ɼ���
    constexpr int huaiyi_cnt_MAX = 5; // ���ɼ������ֵ
    cv::Point leftEdge_huaiyi_mark = cv::Point(0, 0), rightEdge_huaiyi_mark = cv::Point(0, 0); // �����Ƿ�Ϊ���ߵı�ǵ�
    for (int r=0; r<binImg.rows; r++) {
        /*ÿһ����ʼ*/
        leftEdge_huaiyi_mark = cv::Point(0, 0); rightEdge_huaiyi_mark = cv::Point(0, 0);
        /*��Ѱ��߽�*/
        huaiyi = false;
        huaiyi_cnt = 0;
        for (int c=binImg.cols/2; c>=0; c--) {
            /*���е��������*/
            uint8_t pix = binImg.at<uint8_t>(r, c);
            /*���һֱû�ҵ������*/
            if(c == 0) {
                // ��û�л��ɣ���ֱ�Ӱ���Ѱ�����һ����ӽ�ȥ
                if(!huaiyi) leftEdgePoints.push_back(cv::Point(c, r));
                // �л����ˣ�����û�л��ɵ����ޣ���ѻ��ɵĵ�ӽ�ȥ
                else if (huaiyi && huaiyi_cnt <= huaiyi_cnt_MAX) {
                    leftEdgePoints.push_back(leftEdge_huaiyi_mark);
                }
            }
            /*û�л���ʱ�����ֺ�ɫ���ˣ��û��ɣ�����¼��ǰ��ɫ������*/
            else if(!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // �û���
                huaiyi_cnt++; // ���ɶ�+1
                leftEdge_huaiyi_mark = cv::Point(c, r); // ��¼��ǰ��������
            }
            /*��û���ɵ����ޣ������˺�ɫ�飬���ɼ���*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_BLACK) {
                huaiyi_cnt++; // ���ɶ�+1
            }
            /*��û���ɵ����ޣ����ְ�ɫ���ˣ��������ɣ����ɶ�����*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_WHITE) {
                huaiyi = false; // ��������
                huaiyi_cnt = 0; // ���ɶ�����
            }
            /*���ɵ������ˣ�ȷ����������߽���*/
            else if(huaiyi && huaiyi_cnt >= huaiyi_cnt_MAX) {
                leftEdgePoints.push_back(leftEdge_huaiyi_mark); // ��¼�����߽��
                huaiyi = false; // �������ˣ�ȷ����
                huaiyi_cnt = 0; // ���ɶ�����
                break; // �˳�ѭ������ʼ��Ѱ�ұ߽�
            }
        }
        /*��Ѱ�ұ߽�*/
        huaiyi = false;
        huaiyi_cnt = 0;
        for (int c=binImg.cols/2; c<binImg.cols; c++) {
            /*���е����ұ���*/
            uint8_t pix = binImg.at<uint8_t>(r, c);
            /*���һֱû�ҵ��ұ���*/
            if(c == binImg.cols - 1) {
                // ��û�л��ɣ���ֱ�Ӱ���Ѱ�����һ����ӽ�ȥ
                if(!huaiyi) rightEdgePoints.push_back(cv::Point(c, r));
                // �л����ˣ�����û�л��ɵ����ޣ���ѻ��ɵĵ�ӽ�ȥ
                else if (huaiyi && huaiyi_cnt <= huaiyi_cnt_MAX) {
                    rightEdgePoints.push_back(rightEdge_huaiyi_mark);
                }
            }
            /*û�л���ʱ�����ֺ�ɫ���ˣ��û��ɣ�����¼��ǰ��ɫ������*/
            else if(!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // �û���
                huaiyi_cnt++; // ���ɶ�+1
                rightEdge_huaiyi_mark = cv::Point(c, r); // ��¼��ǰ��������
            }
            /*��û���ɵ����ޣ������˺�ɫ�飬���ɼ���*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_BLACK) {
                huaiyi_cnt++; // ���ɶ�+1
            }
            /*��û���ɵ����ޣ����ְ�ɫ���ˣ��������ɣ����ɶ�����*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_WHITE) {
                huaiyi = false; // ��������
                huaiyi_cnt = 0; // ���ɶ�����
            }
            /*���ɵ������ˣ�ȷ���������ұ߽���*/
            else if(huaiyi && huaiyi_cnt >= huaiyi_cnt_MAX) {
                rightEdgePoints.push_back(rightEdge_huaiyi_mark); // ��¼����ұ߽��
                huaiyi = false; // �������ˣ�ȷ����
                huaiyi_cnt = 0; // ���ɶ�����
                break; // �˳�ѭ��
            }
        }
    }
#elif use_findEdgePix == 4
    // 4���ȴӵײ���ʼ���Ե�2�ַ����ҵ�һ��߽�㣬���û�ҵ�������������ң�
    //    ���ҵ�һ��߽��󣬾Ϳ�ʼ�����ұ��ߣ�ÿ�ֵ���һ�У�ֻ����ǰһ���߽������긽���ұ߽��
    bool hasFoundBasis = false; // �Ƿ��Ѿ��ҵ�����ʶ���
    constexpr int FIND_ZONE = 15; // ��������
    bool findLeftEdge = false, findRightEdge = false; // �Ƿ��ڲ�������ߣ��Ƿ��ڲ����ұ���
    bool leftEdge_huaiyi = false, rightEdge_huaiyi = false; // �����Ƿ�Ϊ������
    uint8_t leftEdge_huaiyi_cnt = 0, rightEdge_huaiyi_cnt = 0; // ���ɼ���
    constexpr auto leftEdge_huaiyi_cnt_MAX = 10, rightEdge_huaiyi_cnt_MAX = 20; // Ҫ������������ô��Ρ����Żᱻ�϶�Ϊ����
    cv::Point leftEdge_huaiyi_mark = cv::Point(0, 0), rightEdge_huaiyi_mark = cv::Point(0, 0); // �����Ƿ�Ϊ���ߵı�ǵ�
    for (int r=binImg.rows-1; r>=0; r--) {
        /*������п�ʼ���Ȳ���һ������ʶ��㣬�������ƽ����õ��ǵ�2�ֲ��ұ��ߵķ�����*/
        if (!hasFoundBasis) { // �����û���ҵ�����ʶ���
            /*��ÿһ����ʼʱ*/
            findLeftEdge = true; findRightEdge = false;
            leftEdge_huaiyi = false; rightEdge_huaiyi = false;
            leftEdge_huaiyi_cnt = 0; rightEdge_huaiyi_cnt = 0;
            leftEdge_huaiyi_mark = cv::Point(0, 0); rightEdge_huaiyi_mark = cv::Point(0, 0);
            for (int c = 0; c < binImg.cols; c++) {
                uint8_t pix = binImg.at<uint8_t>(r, c);
                if (findLeftEdge) {
                    /*û�л���ʱ�����ְ�ɫ���ˣ��û��ɣ�����¼��ǰ��ɫ������*/
                    if (!leftEdge_huaiyi && pix == BIN_WHITE) {
                        leftEdge_huaiyi = true; // �û���
                        leftEdge_huaiyi_cnt++; // ���ɶ�+1
                        leftEdge_huaiyi_mark = cv::Point(c, r); // ��¼��ǰ��������
                    }
                    /*��û���ɵ����ޣ������˰�ɫ�飬���ɼ���*/
                    else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt < leftEdge_huaiyi_cnt_MAX && pix == BIN_WHITE) {
                        leftEdge_huaiyi_cnt++; // ���ɶ�+1
                    }
                    /*��û���ɵ����ޣ����ֺ�ɫ���ˣ��������ɣ����ɶ�����*/
                    else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt < leftEdge_huaiyi_cnt_MAX && pix == BIN_BLACK) {
                        leftEdge_huaiyi = false; // ��������
                        leftEdge_huaiyi_cnt = 0; // ���ɶ�����
                    }
                    /*���ɵ������ˣ�ȷ����������߽���*/
                    else if (leftEdge_huaiyi && leftEdge_huaiyi_cnt >= leftEdge_huaiyi_cnt_MAX) {
                        leftEdgePoints.push_back(leftEdge_huaiyi_mark); // ��¼�����߽��
                        findLeftEdge = false; findRightEdge = true; // ��ʼ��Ѱ�ұ߽�
                    }
                }
                else if (findRightEdge) {
                    /*���һֱû�ҵ��ұ���*/
                    if (c == binImg.cols - 1) {
                        // ��û�л��ɣ���ֱ�Ӱ��������һ����ӽ�ȥ
                        if (!rightEdge_huaiyi) rightEdgePoints.push_back(cv::Point(c, r));
                        // �л����ˣ�����û�л��ɵ����ޣ���ѻ��ɵĵ�ӽ�ȥ
                        else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt <= rightEdge_huaiyi_cnt_MAX) {
                            rightEdgePoints.push_back(rightEdge_huaiyi_mark);
                        }
                    }
                    /*û�л���ʱ�����ֺ�ɫ���ˣ��û��ɣ�����¼��ǰ��ɫ������*/
                    else if (!rightEdge_huaiyi && pix == BIN_BLACK) {
                        rightEdge_huaiyi = true; // �û���
                        rightEdge_huaiyi_cnt++; // ���ɶ�+1
                        rightEdge_huaiyi_mark = cv::Point(c, r); // ��¼��ǰ��������
                    }
                    /*��û���ɵ����ޣ������˺�ɫ�飬���ɼ���*/
                    else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt < rightEdge_huaiyi_cnt_MAX && pix == BIN_BLACK) {
                        rightEdge_huaiyi_cnt++; // ���ɶ�+1
                    }
                    /*��û���ɵ����ޣ����ְ�ɫ���ˣ��������ɣ����ɶ�����*/
                    else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt < rightEdge_huaiyi_cnt_MAX && pix == BIN_WHITE) {
                        rightEdge_huaiyi = false; // ��������
                        rightEdge_huaiyi_cnt = 0; // ���ɶ�����
                    }
                    /*���ɵ������ˣ�ȷ���������ұ߽���*/
                    else if (rightEdge_huaiyi && rightEdge_huaiyi_cnt >= rightEdge_huaiyi_cnt_MAX) {
                        rightEdgePoints.push_back(rightEdge_huaiyi_mark); // ��¼����ұ߽��
                        findLeftEdge = true; findRightEdge = false; // ��ʼ��Ѱ��߽�
                        break;
                    }
                }
            }
            if (leftEdgePoints.empty() || rightEdgePoints.empty()) { // �����һ�߻�û���ҵ�����ʶ��㣨�߽�㣩
                hasFoundBasis = false; // ����Ϊfalse����ʾ��û���ҵ�����ʶ���
                leftEdgePoints.erase(leftEdgePoints.begin(), leftEdgePoints.end()); // �������ߵ�
                rightEdgePoints.erase(rightEdgePoints.begin(), rightEdgePoints.end()); // ����ұ��ߵ�
                // �����Ϊ���ܱ���Ѱ�ҵ��ĵ��������꣨�кţ�����
            }
            else { // ����ҵ��˱���
                hasFoundBasis = true; // ��Ϊtrue����ʾ�Ѿ��ҵ�����ʶ�����
            }
        }
        else { // �õģ��Ѿ��ҵ�����ʶ����ˣ���ʼ�������ϲ���
            cv::Point leftLastMark(leftEdgePoints.back().x, leftEdgePoints.back().y); // ��������һ����
            for (int cl=leftLastMark.x+FIND_ZONE; cl>=leftLastMark.x-FIND_ZONE; cl--) { // ����ߣ����������ұ߽�
                if (isBinImgRectRoiAllColor(
                        binImg,
                        cv::Rect(
                            (leftLastMark.x-FIND_ZONE<0) ? 0 : leftLastMark.x-FIND_ZONE, // ������ʼ������ֵ�����С��0�Ļ�����ֱ��Ϊ0������Ϊԭ��
                            r, // ������ʼ������ֵ
                            (leftLastMark.x+FIND_ZONE>binImg.cols) ? binImg.cols-(leftLastMark.x-FIND_ZONE) : FIND_ZONE*2+1, // ���ǿ��ֵ���������ͼ���ȵĻ�����ֱ��Ϊͼ���ȼ�ȥ��ʼ������ֵ������ΪFIND_ZONE*2+1
                            1 // ���Ǹ߶�ֵ
                        ),
                        BIN_BLACK)// �����������Ǻ�ɫ��
                ) {
                    leftEdgePoints.push_back(cv::Point(leftLastMark.x, r)); // ������һ�μ�¼�ĺ�����ֵ
                    break; // ����ѭ��
                }
                else if(isBinImgRectRoiAllColor(
                        binImg,
                        cv::Rect(
                            (leftLastMark.x-FIND_ZONE<0) ? 0 : leftLastMark.x-FIND_ZONE, // ������ʼ������ֵ�����С��0�Ļ�����ֱ��Ϊ0������Ϊԭ��
                            r, // ������ʼ������ֵ
                            (leftLastMark.x+FIND_ZONE>binImg.cols) ? binImg.cols-(leftLastMark.x-FIND_ZONE) : FIND_ZONE*2+1, // ���ǿ��ֵ���������ͼ���ȵĻ�����ֱ��Ϊͼ���ȼ�ȥ��ʼ������ֵ������ΪFIND_ZONE*2+1
                            1 // ���Ǹ߶�ֵ
                        ),
                        BIN_WHITE) // �����������ǰ�ɫ��
                ) {
                    leftEdgePoints.push_back(cv::Point(leftLastMark.x, r)); // ������һ�μ�¼�ĺ�����ֵ
                    break; // ����ѭ��
                }
                else if (cl >= binImg.cols) continue; // Խ���ˣ�ֱ����һ��ѭ��
                else if (cl < 0) {
                    leftEdgePoints.push_back(cv::Point(leftLastMark.x, r)); // ������һ�μ�¼�ĺ�����ֵ
                    break; // ����ѭ��
                }
                else if (binImg.at<uint8_t>(r,cl) == BIN_BLACK) {
                    leftEdgePoints.push_back(cv::Point(cl, r)); // ��¼����߽��
                    break; // ����ѭ��
                }
            }
            cv::Point rightLastMark(rightEdgePoints.back().x, rightEdgePoints.back().y); // �ұ������һ����
            for (int cr=rightLastMark.x-FIND_ZONE; cr<=rightLastMark.x+FIND_ZONE; cr++) { // �ұ��ߣ����������ұ߽�
                if (isBinImgRectRoiAllColor(
                        binImg,
                        cv::Rect(
                            (rightLastMark.x-FIND_ZONE<0) ? 0 : rightLastMark.x-FIND_ZONE, // ������ʼ������ֵ�����С��0�Ļ�����ֱ��Ϊ0������Ϊԭ��
                            r, // ������ʼ������ֵ
                            (rightLastMark.x+FIND_ZONE>binImg.cols)?binImg.cols-(rightLastMark.x-FIND_ZONE):FIND_ZONE*2+1, // ���ǿ��ֵ���������ͼ���ȵĻ�����ֱ��Ϊͼ���ȼ�ȥ��ʼ������ֵ������ΪFIND_ZONE*2+1
                            1 // ���Ǹ߶�ֵ
                        ),
                        BIN_BLACK) // �����������Ǻ�ɫ��
                ) {
                    rightEdgePoints.push_back(cv::Point(rightLastMark.x, r)); // ������һ�μ�¼�ĺ�����ֵ
                    break; // ����ѭ��
                }
                else if(isBinImgRectRoiAllColor(
                        binImg,
                        cv::Rect(
                            (rightLastMark.x-FIND_ZONE<0) ? 0 : rightLastMark.x-FIND_ZONE, // ������ʼ������ֵ�����С��0�Ļ�����ֱ��Ϊ0������Ϊԭ��
                            r, // ������ʼ������ֵ
                            (rightLastMark.x+FIND_ZONE>binImg.cols)?binImg.cols-(rightLastMark.x-FIND_ZONE):FIND_ZONE*2+1, // ���ǿ��ֵ���������ͼ���ȵĻ�����ֱ��Ϊͼ���ȼ�ȥ��ʼ������ֵ������ΪFIND_ZONE*2+1
                            1 // ���Ǹ߶�ֵ
                        ),
                        BIN_WHITE) // �����������ǰ�ɫ��
                ) {
                    rightEdgePoints.push_back(cv::Point(rightLastMark.x, r)); // ������һ�μ�¼�ĺ�����ֵ
                    break; // ����ѭ��
                }
                else if (cr < 0) continue; // Խ���ˣ�ֱ����һ��ѭ��
                else if (cr >= binImg.cols) {
                    rightEdgePoints.push_back(cv::Point(rightLastMark.x, r)); // ������һ�μ�¼�ĺ�����ֵ
                    break; // ����ѭ��
                }
                else if (binImg.at<uint8_t>(r, cr) == BIN_BLACK) {
                    rightEdgePoints.push_back(cv::Point(cr, r)); // ��¼����߽��
                    break; // ����ѭ��
                }
            }
        }
    }
#elif use_findEdgePix == 5
    // 5�����л��ɣ��ӵײ����ϣ����е��������ұ��ߣ����ҵ�һ����ߺ����ҵ��ı��߼�����ĵ���Ϊ�µ��е㣬
    //    �ֵ���һ�У���������µ��е����������ߣ�Ҫ����ʶ�𵽶����ɫ���أ������϶�Ϊ����
    bool huaiyi = false; // �Ƿ���
    uint8_t huaiyi_cnt = 0; // ���ɼ���
    constexpr int huaiyi_cnt_MAX = 5; // ���ɼ������ֵ
    cv::Point midPointMark = cv::Point(binImg.cols/2, binImg.rows-1); // �е��ǣ���ʼΪͼ��ײ��е�
    cv::Point leftEdge_huaiyi_mark = cv::Point(0, 0), rightEdge_huaiyi_mark = cv::Point(0, 0); // �����Ƿ�Ϊ���ߵı�ǵ�
    for (int r=binImg.rows-1; r>=0; r--) { // �ӵײ�����
        /*ÿһ����ʼ*/
        leftEdge_huaiyi_mark = cv::Point(0, 0); rightEdge_huaiyi_mark = cv::Point(0, 0);
        /*��Ѱ��߽�*/
        huaiyi = false;
        huaiyi_cnt = 0;
        for (int c=midPointMark.x; c>=0; c--) {
            /*���е��������*/
            uint8_t pix = binImg.at<uint8_t>(r, c);
            /*���һֱû�ҵ������*/
            if(c == 0) {
                // ��û�л��ɣ���ֱ�Ӱ���Ѱ�����һ����ӽ�ȥ
                if(!huaiyi) leftEdgePoints.push_back(cv::Point(c, r));
                // �л����ˣ�����û�л��ɵ����ޣ���ѻ��ɵĵ�ӽ�ȥ
                else if (huaiyi && huaiyi_cnt <= huaiyi_cnt_MAX) {
                    leftEdgePoints.push_back(leftEdge_huaiyi_mark);
                }
            }
            /*û�л���ʱ�����ֺ�ɫ���ˣ��û��ɣ�����¼��ǰ��ɫ������*/
            else if(!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // �û���
                huaiyi_cnt++; // ���ɶ�+1
                leftEdge_huaiyi_mark = cv::Point(c, r); // ��¼��ǰ��������
            }
            /*��û���ɵ����ޣ������˺�ɫ�飬���ɼ���*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_BLACK) {
                huaiyi_cnt++; // ���ɶ�+1
            }
            /*��û���ɵ����ޣ����ְ�ɫ���ˣ��������ɣ����ɶ�����*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_WHITE) {
                huaiyi = false; // ��������
                huaiyi_cnt = 0; // ���ɶ�����
            }
            /*���ɵ������ˣ�ȷ����������߽���*/
            else if(huaiyi && huaiyi_cnt >= huaiyi_cnt_MAX) {
                leftEdgePoints.push_back(leftEdge_huaiyi_mark); // ��¼�����߽��
                huaiyi = false; // �������ˣ�ȷ����
                huaiyi_cnt = 0; // ���ɶ�����
                break; // �˳�ѭ������ʼ��Ѱ�ұ߽�
            }
        }
        /*��Ѱ�ұ߽�*/
        huaiyi = false;
        huaiyi_cnt = 0;
        for (int c=midPointMark.x; c<binImg.cols; c++) {
            /*���е����ұ���*/
            uint8_t pix = binImg.at<uint8_t>(r, c);
            /*���һֱû�ҵ��ұ���*/
            if(c == binImg.cols - 1) {
                // ��û�л��ɣ���ֱ�Ӱ���Ѱ�����һ����ӽ�ȥ
                if(!huaiyi) rightEdgePoints.push_back(cv::Point(c, r));
                // �л����ˣ�����û�л��ɵ����ޣ���ѻ��ɵĵ�ӽ�ȥ
                else if (huaiyi && huaiyi_cnt <= huaiyi_cnt_MAX) {
                    rightEdgePoints.push_back(rightEdge_huaiyi_mark);
                }
            }
            /*û�л���ʱ�����ֺ�ɫ���ˣ��û��ɣ�����¼��ǰ��ɫ������*/
            else if(!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // �û���
                huaiyi_cnt++; // ���ɶ�+1
                rightEdge_huaiyi_mark = cv::Point(c, r); // ��¼��ǰ��������
            }
            /*��û���ɵ����ޣ������˺�ɫ�飬���ɼ���*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_BLACK) {
                huaiyi_cnt++; // ���ɶ�+1
            }
            /*��û���ɵ����ޣ����ְ�ɫ���ˣ��������ɣ����ɶ�����*/
            else if(huaiyi && huaiyi_cnt < huaiyi_cnt_MAX && pix == BIN_WHITE) {
                huaiyi = false; // ��������
                huaiyi_cnt = 0; // ���ɶ�����
            }
            /*���ɵ������ˣ�ȷ���������ұ߽���*/
            else if(huaiyi && huaiyi_cnt >= huaiyi_cnt_MAX) {
                rightEdgePoints.push_back(rightEdge_huaiyi_mark); // ��¼����ұ߽��
                huaiyi = false; // �������ˣ�ȷ����
                huaiyi_cnt = 0; // ���ɶ�����
                break; // �˳�ѭ��
            }
        }
        if (leftEdgePoints.empty() || rightEdgePoints.empty()) continue;
        else midPointMark = calMidPoint(leftEdgePoints.back(), rightEdgePoints.back()); // �����µ��е�
    }
#endif
}

// ������������֮����е�����
cv::Point calMidPoint(const cv::Point pt1, const cv::Point& pt2) {
    return cv::Point((pt1.x + pt2.x) / 2, (pt1.y + pt2.y) / 2);
}

// ʹ���Զ���ֵ������ɫͼ��תΪָ�����صĶ�ֵͼ�񣬷���ֵΪת��Ϊ��ֵ��ͼ��ʱ����ֵ
double bgr2bin(const cv::Mat& bgrImg, cv::Mat& resizedImg, cv::Mat& grayImg, cv::Mat& binImg,
               const cv::Size& imgSize) {
    cv::resize(bgrImg, resizedImg, imgSize);
    cv::cvtColor(resizedImg, grayImg, cv::COLOR_BGR2GRAY); // ��ԭʼͼ��ת��Ϊ�Ҷ�ͼ
    return cv::threshold(grayImg, binImg, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
}

// ָ����ֵ������ɫͼ��תΪָ�����صĶ�ֵͼ��
double bgr2bin(const cv::Mat& bgrImg, cv::Mat& resizedImg, cv::Mat& grayImg, cv::Mat& binImg,
             const cv::Size& imgSize, double thresh, double maxval) {
    cv::resize(bgrImg, resizedImg, imgSize);
    cv::cvtColor(resizedImg, grayImg, cv::COLOR_BGR2GRAY); // ��ԭʼͼ��ת��Ϊ�Ҷ�ͼ
    cv::threshold(grayImg, binImg, thresh, maxval, cv::THRESH_BINARY);
    return thresh;
}

/*�ж϶�ֵ��ͼ���е�ĳ�����������Ƿ�Ϊcolor��ɫ���ǵĻ�����true����Ļ�����false���������ľ��η�Χ����ͼ��ߴ�Ҳ�᷵��false*/
bool isBinImgRectRoiAllColor(const cv::Mat& binImg, const cv::Rect& rect, uint8_t color) {
    if (rect.x < 0 || rect.y < 0 || rect.width <= 0 || rect.height <= 0) return false; // ��������Ƿ�Ϸ�
    if (rect.x + rect.width > binImg.cols || rect.y + rect.height > binImg.rows) return false; // �����������Ƿ���ͼ��Χ��
    cv::Mat imgRoi = binImg(rect); // ��ȡ��������
    for (int i = 0; i < imgRoi.rows; ++i) // �������������е���������
        for (int j = 0; j < imgRoi.cols; ++j)
            if (imgRoi.at<uint8_t>(i,j) != color) return false; // ������ַǺ�ɫ���أ�����false
    return true; // �������ض��Ǻ�ɫ������true
}

/**
 * @brief �ڶ�ֵͼ���ϵ����ұ߽���ƺ�ɫ�߿�
 * @param binImg ����Ķ�ֵͼ��
 */
void drawLeftRightBlackBorder(cv::Mat& binImg) {
    if (binImg.type() != CV_8UC1) { // ���ͼ���Ƿ�Ϊ��ֵͼ��
        std::cerr << "Image is not a binary image!" << std::endl;
        return;
    }
    int rows = binImg.rows;
    int cols = binImg.cols;
    for (int r=0; r<rows; r++) {
        binImg.at<uint8>(r,0) = BIN_BLACK; // ��߽�
        binImg.at<uint8>(r,cols-1) = BIN_BLACK; // �ұ߽�
    }
}

/**
 * @brief �ڶ�ֵͼ�񶥲��͵ײ����ƺ�ɫ�߿�
 * @param binImg ��ֵͼ��
 */
void drawTopBottomBlackBorder(cv::Mat& binImg) {
    if (binImg.type() != CV_8UC1) { // ���ͼ���Ƿ�Ϊ��ֵͼ��
        std::cerr << "Image is not a binary image!" << std::endl;
        return;
    }
    int rows = binImg.rows;
    int cols = binImg.cols;
    for (int c=0; c<cols; c++) {
        binImg.at<uint8>(0,c) = BIN_BLACK; // �ϱ߽�
        binImg.at<uint8>(rows-1,c) = BIN_BLACK; // �±߽�
    }
}

