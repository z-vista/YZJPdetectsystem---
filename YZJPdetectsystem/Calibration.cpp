
#include "stdafx.h"
#include "Calibration.h"
using namespace std;
using namespace cv;
#define PI  3.1415926
//最新标定所用函数
void  mysharpen(cv::Mat& src)
{

	cv::Mat dst = src.clone();
	cv::Mat kernel(3, 3, CV_32F, cv::Scalar(-1));

	kernel.at<float>(1, 1) = (float)8.9;
	filter2D(src, dst, src.depth(), kernel);
	src = dst.clone();

}

void drawLine(cv::Mat& image, double theta, double rho, cv::Scalar color)
{
	if (theta < PI / 4. || theta > 3. * PI / 4.)// ~vertical line
	{
		cv::Point pt1(rho / cos(theta), 0);
		cv::Point pt2((rho - image.rows * sin(theta)) / cos(theta), image.rows);
		cv::line(image, pt1, pt2, cv::Scalar(255), 1);
	}
	else
	{
		cv::Point pt1(0, rho / sin(theta));
		cv::Point pt2(image.cols, (rho - image.cols * cos(theta)) / sin(theta));
		cv::line(image, pt1, pt2, color, 1);
	}
}

void getContoursLine(cv::Mat src, cv::Mat& dst)//求轮廓和轮廓图
{
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarcy;
	vector<int>selectIndexContours;
	cv::Mat srccopy = src.clone();
	cv::findContours(src, contours, hierarcy, /*CV_RETR_TREE*/CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	if (contours.size() == 0)
	{
		return;
	}
	vector<cv::Rect> tp_boundRect(contours.size());  //定义外接矩形集合
	vector<cv::RotatedRect> tp_box(contours.size()); //定义最小外接矩形集合
												 //Point2f rect[4];
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() < 20)
			continue;
		cv::Rect tp_rect;
		tp_box[i] = minAreaRect(cv::Mat(contours[i]));  //计算每个轮廓最小外接矩形
		tp_rect = boundingRect(cv::Mat(contours[i]));

		if (tp_rect.width > 50 /*30*/)continue;
		if (tp_rect.height < 20)continue;
		//if (contours[i].size() / (tp_rect.width*tp_rect.height) > 0.8 && contours[i].size() > 2000)continue;

		//拟合一条直线
		//绘制直线，并返回图像
		cv::Vec4f Line;
		cv::fitLine(contours[i], Line, CV_DIST_HUBER, 0, 0.01, 0.01);

		double cos_theta = Line[0];
		double sin_theta = Line[1];
		double x0 = Line[2], y0 = Line[3];

		double phi = atan2(sin_theta, cos_theta) + PI / 2.0;
		double rho = y0 * cos_theta - x0 * sin_theta;

		/*std::cout << "phi = " << phi / PI * 180 << std::endl;
		std::cout << "rho = " << rho << std::endl;*/

		drawLine(dst, phi, rho, cv::Scalar(0));
	}

}


bool GetResolutionTab(cv::Mat src, vector<cv::Point/*XRESOLUTIONTAB*/>& m_ResultTable, int phyRatio,int thresh)
{
	//Mat imagesource = imagesource1(Rect(0, 0, imagesource1.cols, imagesource1.rows));
	Mat srcHSV, imagebin;
	cvtColor(src, srcHSV, CV_BGR2HSV);
	//cvtColor(y, yHSV, CV_BGR2HSV);
	//兰色
	inRange(srcHSV, Scalar(117, 175, 203), Scalar(119, 178, 205), imagebin);
	
#if 0
	Mat imageGray, imagebin;
	mysharpen(src);
	//mysharpen(imagesource);
	cvtColor(src, imageGray, CV_BGR2GRAY);

	medianBlur(imageGray, imageGray, 3);//中值滤波消除椒盐噪声
	medianBlur(imageGray, imageGray, 3);//中值滤波消除椒盐噪声
	medianBlur(imageGray, imageGray, 3);//中值滤波消除椒盐噪声

	mysharpen(imageGray);
	medianBlur(imageGray, imageGray, 3);//中值滤波消除椒盐噪
	medianBlur(imageGray, imageGray, 3);//中值滤波消除椒盐噪声
										//medianBlur(imageGray, imageGray, 3);//中值滤波消除椒盐噪声
										/*mysharpen(imageGray);*/

										/*equalizeHist(imageGray, imageGray);*/

	
	//int thresh = 40;
	//do
	//{
		//m_ResultTable.clear();
		//thresh--;
		threshold(imageGray, imagebin, thresh, 255, CV_THRESH_BINARY);

		medianBlur(imagebin, imagebin, 3);//中值滤波消除椒盐噪声
		medianBlur(imagebin, imagebin, 3);//中值滤波消除椒盐噪声
#endif
		Mat imagedst = imagebin.clone();
		imagedst.setTo(0);
		getContoursLine(imagebin, imagedst);//求轮廓和轮廓图

		vector<cv::Point>findPoint;
	
		for (int i = 0; i < imagedst.cols; i++)
		{
			//Mat::at<float>(j,i)获取像素值，并与阈值比较
			if (/*(int)*/imagedst.at<uchar>(imagedst.rows / 2, i) == 255)
			{
				findPoint.push_back(cv::Point(i, imagedst.rows / 2));
				circle(src, cv::Point(i, imagedst.rows / 2), 3, Scalar(10, 10, 255), 2, 8, 0);
			}
		}
		//遍历一遍输出
		if (findPoint.size() > 0)
		{
			for (int j = 0; j < findPoint.size() - 1; j++)
			{
				if ((findPoint.at(j + 1).x - findPoint.at(j).x) > 5)
				{
					//XRESOLUTIONTAB temp_m_Result;
					//temp_m_Result.X = findPoint.at(j).x;
					//temp_m_Result.Resolution = (float)phyRatio / (float)(findPoint.at(j + 1).x - findPoint.at(j).x);
					cv::Point temp_m_Result;
					temp_m_Result.x = findPoint.at(j).x;
					temp_m_Result.y = findPoint.at(j).y;
					m_ResultTable.push_back(temp_m_Result);
				}
			}
			/*if(m_ResultTable.size()>120)
				break;*/
		}
	//} while (thresh >30);

	


	
	//if ((findPoint.at(findPoint.size() - 1).x - findPoint.at(findPoint.size() - 2).x) > 5)
	//{
	//	XRESOLUTIONTAB Last_m_Result;
	//	Last_m_Result.X = findPoint.at(findPoint.size() - 1).x;
	//	Last_m_Result.Resolution = 0;
	//	m_ResultTable.push_back(Last_m_Result);
	//}
	
	return true;
}