#pragma once

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/core/core.hpp>
#include "WinDef.h"
#include <windows.h>
#include <iostream>

using namespace cv;
using namespace std;

#define FINDBOUND_API __declspec(dllexport)


#define PI 3.1415926


class FINDBOUND_API CFindBound
{
public:
	CFindBound();
	~CFindBound(); 

public:
	///***************
	//��λ�Ұ׽���  //����������ֽ״̬
	void BoundLoc(Mat srcimage, Point &loc);
	//��λ�Ұ׽��磬��������ֽ״̬�� 1��ֽ 0��ֽ  -1�������������
	int BoundLocWithState(Mat srcimage, Point& loc);
	//����ýӿڣ� ��λ�Ұ׽��磬��������ֽ״̬�� 1��ֽ 0��ֽ  -1�������������  ����proType:12������   34������
	int BoundLocWithState(Mat srcimage, Point& loc, int proType);
	void getMaxContour(Mat& src, std::vector<std::vector<cv::Point>> contours);
	void refineLoc(Mat src, Point& loc);
	bool isFreeState(Mat src, Point loc);
	bool isFreeState2(Mat src, Point loc);
	bool isFreeState(Mat src, Point loc, int proType);
	void refineLocByFitting(Mat src, Point& loc);
	bool polynomial_curve_fit(std::vector<cv::Point>& key_point, int n, cv::Mat& CoefMat);
};

