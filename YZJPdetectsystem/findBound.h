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
	//定位灰白交界  //不返回有无纸状态
	void BoundLoc(Mat srcimage, Point &loc);
	//定位灰白交界，返回有无纸状态， 1有纸 0无纸  -1输入参数有问题
	int BoundLocWithState(Mat srcimage, Point& loc);
	//软件用接口： 定位灰白交界，返回有无纸状态， 1有纸 0无纸  -1输入参数有问题  输入proType:12背景黑   34背景亮
	int BoundLocWithState(Mat srcimage, Point& loc, int proType);
	void getMaxContour(Mat& src, std::vector<std::vector<cv::Point>> contours);
	void refineLoc(Mat src, Point& loc);
	bool isFreeState(Mat src, Point loc);
	bool isFreeState2(Mat src, Point loc);
	bool isFreeState(Mat src, Point loc, int proType);
	void refineLocByFitting(Mat src, Point& loc);
	bool polynomial_curve_fit(std::vector<cv::Point>& key_point, int n, cv::Mat& CoefMat);
};

