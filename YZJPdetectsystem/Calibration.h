#pragma once
#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/core/core.hpp>
typedef struct XRESOULUTIONTAB
{
	float X;
	float Resolution;
} XRESOLUTIONTAB;


extern void mysharpen(cv::Mat& src);
extern void drawLine(cv::Mat& image, double theta, double rho, cv::Scalar color);
extern void getContoursLine(cv::Mat src, cv::Mat& dst);

extern bool GetResolutionTab(cv::Mat src, std::vector<cv::Point/*XRESOLUTIONTAB*/>& m_ResultTable, int phyRatio,int thresh);