// track ������ for static images ʹ�ð������㷨�Ծ�̬ͼ����й켣ʶ�𣨸о�Ч����̫�ã�
// �ο���Bվup�����嶫ͷ�޵е�СϹ�ӡ�(uid:385282905)�ĳ���
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include "test.hpp"

#define PIC_FILE_NUM 2 // ����ͼƬ�ļ��е�����
#define TRACK_PIC_NUM  20 // ����ͼƬ
int blue=0, green=0, red=0;
int test_track_bly_static(void) {
    cv::namedWindow("���ź�Ĳ�ɫͼ", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("ԭ��ֵ��ͼ��", cv::WINDOW_NORMAL); // ����һ������
    cv::namedWindow("�����Ķ�ֵ��ͼ��", cv::WINDOW_NORMAL); // ����һ������
    int pic_file = 0, pic_index = 0;
    while (true) {
        std::string filename = "./saideao_" + std::to_string(pic_file) + "/" + std::to_string(pic_index) + ".jpg";
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

        binImgFilter(binFrame); // �Զ�ֵͼ������˲�
        binImgDrawRect(binFrame); // �ڶ�ֵͼ��һ���߿�

        // ��תС����ڰ����򷽷�1
        // invertSmallRegion_1(binFrame, MIN_AREA_THRESH); // ��ת���С��MIN_AREA_THRESH������

        // ��תС����ڰ����򷽷�2
        // invertSmallRegion_2(binFrame, MIN_AREA_THRESH); // ��ת���С��MIN_AREA_THRESH������

        // ����ʶ��
        std::vector<cv::Point> leftEdgePoints, rightEdgePoints; // �洢���ߵ����������
        std::vector<int> leftEdgePointDir, rightEdgePointDir; // �洢���ߵ������������������
        blyfindEdgePoint(binFrame, leftEdgePoints, rightEdgePoints, leftEdgePointDir, rightEdgePointDir, binFrame.rows, binFrame.rows);

        // ���켣
        for (auto& pt: leftEdgePoints) blue = blue==255?0:blue+1,resizedFrame.at<cv::Vec3b>(pt) = cv::Vec3b(blue,green,red);
        for (auto& pt: rightEdgePoints) resizedFrame.at<cv::Vec3b>(pt) = cv::Vec3b(BGR_RED);



        cv::imshow("���ź�Ĳ�ɫͼ", resizedFrame); // �ڶ�Ӧ��������ʾͼ��
        cv::imshow("�����Ķ�ֵ��ͼ��", binFrame); // �ڶ�Ӧ��������ʾͼ��

        clearScreen();
        std::cout << "��ֵ��" << th << std::endl;
        std::cout << "ͼƬ��saideao" << pic_file << "/" << pic_index << ".jpg" << std::endl;
        //====================================================================================================|




        int KeyGet = cv::waitKey(0);
        if (KeyGet == KEY_ESC) break;
        else if ((KeyGet == 47) && (pic_file == PIC_FILE_NUM - 1)) pic_file = 0;
        else if ((KeyGet == 44 && pic_index == 0) || (KeyGet == 46 && pic_index == TRACK_PIC_NUM - 1))pic_index = pic_index;
        else if (KeyGet == 47) pic_file++;
        else if (KeyGet == 46) pic_index++;
        else if (KeyGet == 44) pic_index--;
        else continue;
    }

    return 0;
}

/**
 * @brief ���������
 * @param binImg �����ֵ��ͼ��
 * @param leftEdgePoints �����߽������
 * @param rightEdgePoints ����ұ߽������
 * @param leftEdgePointDir �����߽��������������±��leftEdgePoints�Ƕ�Ӧ�ģ�����leftEdgePoints[0]�������������leftEdgePointDir[0]��
 *                         �������һ�����ʱ���Ѿ�û����һ�����ˣ�Ҳ��û�����������ˣ�����leftEdgePointDir�����һ��Ԫ������Ч�ġ�
 * @param rightEdgePointDir ����ұ߽��������������±��rightEdgePoints�Ƕ�Ӧ�ģ�����rightEdgePoints[0]�������������rightEdgePointDir[0]
 *                          �������һ�����ʱ���Ѿ�û����һ�����ˣ�Ҳ��û�����������ˣ�����rightEdgePointDir�����һ��Ԫ������Ч�ġ�
 * @param leftEdgePointsNum ��Ҫ������߽��ĸ���
 * @param rightEdgePointsNum ��Ҫ�����ұ߽��ĸ���
 */
void blyfindEdgePoint(cv::Mat& binImg,
                       std::vector<cv::Point>& leftEdgePoints, std::vector<cv::Point>& rightEdgePoints,
                       std::vector<int>& leftEdgePointDir, std::vector<int>& rightEdgePointDir,
                       int leftEdgePointsNum, int rightEdgePointsNum) {
    /*�����������*/
    leftEdgePoints.erase(leftEdgePoints.begin(), leftEdgePoints.end());
    rightEdgePoints.erase(rightEdgePoints.begin(), rightEdgePoints.end());
    leftEdgePointDir.erase(leftEdgePointDir.begin(), leftEdgePointDir.end());
    rightEdgePointDir.erase(rightEdgePointDir.begin(), rightEdgePointDir.end());

    /*�ڶ�ֵͼ����Χ��һ�����Ϊ2�ľ��α߿�����Ϊ���ڰ�����ʶ��ʱ���ᳬ��ͼ��߽�*/
    // cv::rectangle(binImg, cv::Rect(0,0,binImg.cols,binImg.rows),cv::Scalar(BIN_BLACK), 1);
    // cv::rectangle(binImg, cv::Rect(1,1,binImg.cols-2,binImg.rows-2),cv::Scalar(BIN_BLACK), 1);
    
    /*�Ȳ���һ��������ʼ�㣬���м������߲��ҵģ����л��ɣ�����Ҫ�������ҵ�����ڵ�����϶�Ϊ�߽�*/
    bool huaiyi = false; // �Ƿ���
    uint8 huaiyiCnt = 0; // ���ɼ�����
    constexpr int huaiyiCntMax = 5; // ���ɼ������ֵ
    cv::Point leftEdge_huaiyi_mark = cv::Point(0, 0), rightEdge_huaiyi_mark = cv::Point(0, 0); // �����Ƿ�Ϊ���ߵı�ǵ�
    for (int r=binImg.rows-3; r>=0; r--) { // Ϊʲô��binImg.rows-3��ʼ����Ϊǰ�߻�Զ�ֵͼ����Χ��һ����Ȼ�һ�����Ϊ2�ĺ�ɫ�߿�
        /*��Ѱ��߽�*/
        huaiyi = false; // ÿһ����ʼʱ���������
        huaiyiCnt = 0; // ���ɼ���������
        for (int c=binImg.cols/2; c>=0; c--) {
            /*���м��������*/
            uint8 pix = binImg.at<uint8>(r,c);
            /*���һֱû�ҵ������*/
            if (c == 0) {
                // ��û�л��ɣ���ֱ�Ӱ���Ѱ�����һ����ӽ�ȥ
                if (!huaiyi) leftEdgePoints.push_back(cv::Point(c,r));
                // �л����ˣ�����û�л��ɵ����ޣ���ѻ��ɵĵ�ӽ�ȥ
                else if (huaiyi && huaiyiCnt < huaiyiCntMax) {
                    leftEdgePoints.push_back(leftEdge_huaiyi_mark);
                }
            }
            /*û�л���ʱ�����ֺ�ɫ���ˣ��û��ɣ�����¼��ǰ��ɫ������*/
            else if (!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // �û���
                huaiyiCnt++; // ���ɶ�+1
                leftEdge_huaiyi_mark = cv::Point(c,r); // ��¼��ǰ��������
            }
            /*��û���ɵ����ޣ������˺�ɫ�飬���ɼ���*/
            else if (huaiyi && huaiyiCnt < huaiyiCntMax && pix == BIN_BLACK) {
                huaiyiCnt++; // ���ɶ�+1
            }
            /*��û���ɵ����ޣ����ְ�ɫ���ˣ��������ɣ����ɶ�����*/
            else if (huaiyi && huaiyiCnt < huaiyiCntMax && pix == BIN_WHITE) {
                huaiyi = false; // ��������
                huaiyiCnt = 0; // ���ɶ�����
            }
            /*���ɵ������ˣ�ȷ����������߽���*/
            else if (huaiyi && huaiyiCnt >= huaiyiCntMax) {
                leftEdgePoints.push_back(leftEdge_huaiyi_mark); // ��¼�����߽��
                huaiyi = false; // �������ˣ�ȷ����
                huaiyiCnt = 0; // ���ɶ�����
                break; // �˳�ѭ��
            }
        }
        /*��Ѱ�ұ߽�*/
        huaiyi = false; // ÿһ����ʼʱ���������
        huaiyiCnt = 0; // ���ɼ���������
        for (int c=binImg.cols/2; c<binImg.cols; c++) {
            /*���м����ұ���*/
            uint8 pix = binImg.at<uint8>(r,c);
            /*���һֱû�ҵ��ұ���*/
            if (c == binImg.cols - 1) {
                // ��û�л��ɣ���ֱ�Ӱ���Ѱ�����һ����ӽ�ȥ
                if (!huaiyi) rightEdgePoints.push_back(cv::Point(c,r));
                // �л����ˣ�����û�л��ɵ����ޣ���ѻ��ɵĵ�ӽ�ȥ
                else if (huaiyi && huaiyiCnt < huaiyiCntMax) {
                    rightEdgePoints.push_back(rightEdge_huaiyi_mark);
                }
            }
            /*û�л���ʱ�����ֺ�ɫ���ˣ��û��ɣ�����¼��ǰ��ɫ������*/
            else if (!huaiyi && pix == BIN_BLACK) {
                huaiyi = true; // �û���
                huaiyiCnt++; // ���ɶ�+1
                rightEdge_huaiyi_mark = cv::Point(c,r); // ��¼��ǰ��������
            }
            /*��û���ɵ����ޣ������˺�ɫ�飬���ɼ���*/
            else if (huaiyi && huaiyiCnt < huaiyiCntMax && pix == BIN_BLACK) {
                huaiyiCnt++; // ���ɶ�+1
            }
            /*��û���ɵ����ޣ����ְ�ɫ���ˣ��������ɣ����ɶ�����*/
            else if (huaiyi && huaiyiCnt < huaiyiCntMax && pix == BIN_WHITE) {
                huaiyi = false; // ��������
                huaiyiCnt = 0; // ���ɶ�����
            }
            /*���ɵ������ˣ�ȷ���������ұ߽���*/
            else if (huaiyi && huaiyiCnt >= huaiyiCntMax) {
                rightEdgePoints.push_back(rightEdge_huaiyi_mark); // ��¼����ұ߽��
                huaiyi = false; // �������ˣ�ȷ����
                huaiyiCnt = 0; // ���ɶ�����
                break; // �˳�ѭ��
            }
        }
        if (leftEdgePoints.empty() || rightEdgePoints.empty()) { // �����һ�߻�û���ҵ�����ʶ��㣨�߽�㣩
            leftEdgePoints.erase(leftEdgePoints.begin(), leftEdgePoints.end()); // �����߽��
            rightEdgePoints.erase(rightEdgePoints.begin(), rightEdgePoints.end()); // ����ұ߽��
            // �����Ϊ���ܱ���Ѱ�ҵ��Ļ���ʶ��������������
            continue; // ����������һ��
        }
        else { // ������߶��ҵ��˻���ʶ��㣨�߽�㣩
            leftEdgePointsNum--; // ���ҵ�һ����߽���ˣ�leftEdgePointsNum��1
            rightEdgePointsNum--; // ���ҵ�һ���ұ߽���ˣ�rightEdgePointsNum��1
            break; // �˳����һ���ʶ����forѭ��
        }
    }

    cv::Point leftCenPoint; // ���������ʱ����ߵ����ĵ㣬��ʼ��Ϊ����ʶ���
    cv::Point rightCenPoint; // ���������ʱ�����ұߵ����ĵ㣬��ʼ��Ϊ����ʶ���
    std::array<cv::Point,8> leftFindField, rightFindField; // ���ڴ�Ų��ҵİ˸����������

    // ���������ʱ��һЩ����
    std::array<cv::Point,8> leftSeeds{
        cv::Point(0,1), cv::Point(-1,1), cv::Point(-1,0), cv::Point(-1,-1),
        cv::Point(0,-1), cv::Point(1,-1), cv::Point(1,0), cv::Point(1,1)
    };
    std::array<cv::Point,8> rightSeeds{
        cv::Point(0,1), cv::Point(1,1), cv::Point(1,0), cv::Point(1,-1),
        cv::Point(0,-1), cv::Point(-1,-1), cv::Point(-1,0), cv::Point(-1,1)
    };
    // �����������˳ʱ��                     // �����ұ�������ʱ��
    // {-1,-1}, {0,-1}, {+1,-1},           // {-1,-1}, {0,-1}, {+1,-1},
    // {-1, 0},         {+1, 0},           // {-1, 0},         {+1, 0},
    // {-1,+1}, {0,+1}, {+1,+1},           // {-1,+1}, {0,+1}, {+1,+1},

    /*��Ѱ��߽��*/
    while (leftEdgePointsNum--) {
        leftCenPoint = leftEdgePoints.back(); // ���������ĵ�
        for (int i=0; i<8; i++) leftFindField[i] = leftCenPoint + leftSeeds[i]; // ����������ĵ�˸�����������ŵ�leftFindField��
        for (int i=0; i<8; i++) {
            if (binImg.at<uint8>(leftFindField[i])==BIN_BLACK && binImg.at<uint8>(leftFindField[(i+1)%8])==BIN_WHITE) {
                leftEdgePoints.push_back(leftFindField[i]);
                leftEdgePointDir.push_back(i);
                break;
            }
        }
    }
    /*��Ѱ�ұ߽��*/
    while (rightEdgePointsNum--) {
        rightCenPoint = rightEdgePoints.back(); // ���������ĵ�
        for (int i=0; i<8; i++) rightFindField[i] = rightCenPoint + rightSeeds[i];
        for (int i=0; i<8; i++) {
            if (binImg.at<uint8>(rightFindField[i])==BIN_BLACK && binImg.at<uint8>(rightFindField[(i+1)%8])==BIN_WHITE) {
                rightEdgePoints.push_back(rightFindField[i]);
                rightEdgePointDir.push_back(i);
                break;
            }
        }
    }
}

/**
 * @brief �Զ�ֵͼ������˲������ú�����Դ���Ķ�ֵͼ������˲�����������Χ����ֵ��ͳ�ƽ�����޸��������ص�ֵ��
 *
 * @param binImg ��Ҫ�����˲�����Ķ�ֵͼ��
 */
// �������ͺ͸�ʴ����ֵ����
#define threshold_max  255*5 //�˲����ɸ����Լ����������
#define threshold_min  255*2 //�˲����ɸ����Լ����������
void binImgFilter(cv::Mat& binImg) {
    uint32 num = 0;
    for (int i=1; i<RESIZED_HEIGHT-1; i++) {
        for (int j=1; j<RESIZED_WIDTH-1; j++) {
            //ͳ�ư˸����������ֵ
            num = binImg.at<uint8>(i-1,j-1) + binImg.at<uint8>(i-1,j) + binImg.at<uint8>(i-1,j+1)
                + binImg.at<uint8>(i,j-1) + binImg.at<uint8>(i,j+1)
                + binImg.at<uint8>(i+1,j-1) + binImg.at<uint8>(i+1,j) + binImg.at<uint8>(i+1,j+1);
            if (num >= threshold_max && binImg.at<uint8>(i,j) == 0) {
                binImg.at<uint8>(i,j) = BIN_WHITE;//��  ���Ը�ɺ궨�壬�������
            }
            if (num <= threshold_min && binImg.at<uint8>(i,j) == BIN_WHITE) {
                binImg.at<uint8>(i,j) = BIN_BLACK;//��
            }
        }
    }
}

/*�ڶ�ֵͼ������ܻ�һ�����Ϊ2�ľ��α߿�*/
void binImgDrawRect(cv::Mat& binImg) {
    for (int i=0; i<RESIZED_HEIGHT; i++) {
        binImg.at<uint8>(i,0) = 0;
        binImg.at<uint8>(i,1) = 0;
        binImg.at<uint8>(i,RESIZED_WIDTH - 1) = 0;
        binImg.at<uint8>(i,RESIZED_WIDTH - 2) = 0;
    }
    for (int i=0; i<RESIZED_WIDTH; i++) {
        binImg.at<uint8>(0,i) = 0;
        binImg.at<uint8>(1,i) = 0;
    }
}

// /**
//  * @brief ����һ�����ڰ������ұ߽���ǰ���������ô˺������ڵײ����ҵ�һ����ʼ�����ұ߽�㣬Ȼ���Դ�Ϊ�����������ϲ���
//  * @param binImg �����ֵ��ͼ��
//  * @param start_row ������ʼ������
//  * @param leftStartPoint �����߽���ʼ������
//  * @param rightStartPoint ����ұ߽���ʼ������
//  */
// bool blyfindStartEdgePoint(cv::Mat& binImg, uint16 start_row, cv::Point& leftStartPoint, cv::Point& rightStartPoint) {
//     bool hasLeftEdgeFound=false, hasRightEdgeFound=false;
//     /*���߽���ʼ����ֵ���㣬�±�0��x����ֵ���±�1��y����ֵ*/
//     leftStartPoint.x = 0; leftStartPoint.y = 0;
//     rightStartPoint.x = 0; rightStartPoint.y = 0;

//     /*���м������*/
//     for (int i=RESIZED_WIDTH/2; i>BORDER_MIN; i--) {
//         leftStartPoint.x = i;
//         leftStartPoint.y = start_row;
//         if (binImg.at<uint8>(leftStartPoint.y, leftStartPoint.x) == 255 &&
//              binImg.at<uint8>(leftStartPoint.y, leftStartPoint.x-1) == 0) {
//             hasLeftEdgeFound = true;
//             break;
//         }
//     }
//     for (int i=RESIZED_WIDTH/2; i<BORDER_MAX; i++) {
//         start_point_r[0] = i;
//         start_point_r[1] = start_row;
//         if (binImg.at<uint8>(rightStartPoint.y, rightStartPoint.x) == 255 &&
//              binImg.at<uint8>(rightStartPoint.y, rightStartPoint.x+1) == 0) {
//             hasRightEdgeFound = true;
//             break;
//         }
//     }

//     if(hasLeftEdgeFound && hasRightEdgeFound) return true; // ������ҵ���
//     else return false; // ���û���ҵ�
// }

// #define USE_NUM	 RESIZED_HEIGHT*3	//�����ҵ�������Ա��������˵300�����ܷ��£�������Щ�������ȷʵ�Ѷ����ඨ����һ��
// std::array<cv::Point, USE_NUM> leftPoints; // ��������ߵ������
// std::array<cv::Point, USE_NUM> rightPoints; // �����ұ��ߵ������
// std::array<int, USE_NUM> leftDir; // ��������ߵ�������������
// std::array<int, USE_NUM> rightDir; // �����ұ��ߵ�������������
// int data_stastics_l = 0; //ͳ������ҵ���ĸ���
// int data_stastics_r = 0; //ͳ���ұ��ҵ���ĸ���
// uint8 hightest = 0; //��ߵ�
// /**
//  * @brief ��ʼ��������ұ߽��
//  * @param binImg �����ֵ��ͼ��
//  * @param leftStartPoint ��Ѱ��߽���ʼ�����꣬�ɺ���blyfindStartEdgePoint()�õ�
//  * @param rightStartPoint ��Ѱ�ұ߽���ʼ�����꣬�ɺ���blyfindStartEdgePoint()�õ�
//  * @param leftPointsNum �ҵ�����߽��ĸ���
//  * @param rightPointsNum �ҵ����ұ߽��ĸ���
//  * @param findTimes ���ҵĴ���
//  * @return ��
//  */
// void blyfindAllEdgePoint(cv::Mat& binImg, cv::Point& leftStartPoint, cv::Point& rightStartPoint
//                          int& leftPointsNum, int& rightPointsNum
//                          int findTimes) {
//     // ��������ߵı���
//     cv::Point leftCenPoint; // �˿���������ߵ����ĵ�
//     std::array<cv::Point, 8> leftSearchField; // ������������ʱĳ�����������
//     int leftIndex = 0;
//     std::array<cv::Point, 8> tempL;
//     std::array<cv::Point, 8> leftSeeds{
//         cv::Point(0,1), cv::Point(-1,1), cv::Point(-1,0), cv::Point(-1,-1),
//         cv::Point(0,-1), cv::Point(1,-1), cv::Point(1,0), cv::Point(1,1)
//     }; // �����˳ʱ��
//     // {-1,-1}, {0,-1}, {+1,-1},
//     // {-1, 0},         {+1, 0},
//     // {-1,+1}, {0,+1}, {+1,+1},

//     // �����ұ��ߵı���
//     cv::Point rightCenPoint; // �˿������ұ��ߵ����ĵ�
//     std::array<cv::Point, 8> rightSearchField; // ��������ұ���ʱĳ�����������
//     int rightIndex = 0;
//     std::array<cv::Point, 8> tempR;
//     std::array<cv::Point, 8> rightSeeds{
//         cv::Point(0,1), cv::Point(1,1), cv::Point(1,0), cv::Point(1,-1),
//         cv::Point(0,-1), cv::Point(-1,-1), cv::Point(-1,0), cv::Point(-1,1)
//     }; // �������ʱ��
//     // {-1,-1}, {0,-1}, {+1,-1},
//     // {-1, 0},         {+1, 0},
//     // {-1,+1}, {0,+1}, {+1,+1},

//     leftPointsNum = 0; // ��ʼ������ߵ�ĸ���
//     rightPointsNum = 0; // ��ʼ���ұ��ߵ�ĸ���

//     leftCenPoint = leftStartPoint; // ��ʼ������ߵ����ĵ�
//     rightCenPoint = rightStartPoint; // ��ʼ���ұ��ߵ����ĵ�

//     while (findTimes--) {
//         for (int i=0; i<8; i++) {
//             leftSearchField[i] = leftCenPoint + leftSeeds[i];
//         }
//         leftPoints[leftPointsNum] = leftCenPoint; // ����ǰ�����ĵ���뵽����ߵ��������
//         leftPointsNum ++;

//         for (int i=0; i<8; i++) {
//             rightSearchField[i] = rightCenPoint + rightSeeds[i];
//         }
//         rightPoints[rightPointsNum] = rightCenPoint; // ����ǰ�����ĵ���뵽�ұ��ߵ��������

//         leftIndex = 0; // �����㣬��ʹ��
//         for (int i=0; i<8; i++) {
//             tempL[i] = cv::Point(0,0); // ����
//         }

//         /*��ʽ��Ѱ�����*/
//         for (int i=0; i<8; i++) {
//             if (binImg.at<uint8>(leftSearchField[i]) == BIN_BLACK && binImg.at<uint8>(leftSearchField[(i+1)%8]) == BIN_WHITE) {
//                 tempL[leftIndex] = leftSearchField[i]; // ������ҵ��ĵ�
//                 leftIndex++;
//                 leftDir[leftPointsNum-1] = i; // ��¼�ҵ�����������������
//                 if (leftIndex) { // ����ҵ����ɺ�ת�׵ĵ�
//                     leftCenPoint = tempL[0];
//                     for (int j=0; j<leftIndex; j++) {
//                         if (leftCenPoint.y > tempL[j].y) { // �ҵ�������ĵ�
//                             leftCenPoint = tempL[j];
//                         }
//                     }
//                 }
//             }
//         }
//         if ((points_r[r_data_statics][0]== points_r[r_data_statics-1][0]&& points_r[r_data_statics][0] == points_r[r_data_statics - 2][0]
//            && points_r[r_data_statics][1] == points_r[r_data_statics - 1][1] && points_r[r_data_statics][1] == points_r[r_data_statics - 2][1])
//            ||(points_l[l_data_statics-1][0] == points_l[l_data_statics - 2][0] && points_l[l_data_statics-1][0] == points_l[l_data_statics - 3][0]
//                 && points_l[l_data_statics-1][1] == points_l[l_data_statics - 2][1] && points_l[l_data_statics-1][1] == points_l[l_data_statics - 3][1]))
//         {
//             //printf("���ν���ͬһ���㣬�˳�\n");
//             break;
//         }
//     }
// }