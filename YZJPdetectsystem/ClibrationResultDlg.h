#pragma once
#include "stdafx.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
// CClibrationResultDlg 对话框
#include <GdiPlus.h>  
using namespace Gdiplus;
class CClibrationResultDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CClibrationResultDlg)

public:
	CClibrationResultDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CClibrationResultDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CALIBRATION_RESULT_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
protected:
	cv::Mat m_resultPic;
	
	void DrawImageNewW(BYTE* image, int W, int H, int icolor, int  nID);
	
	float m_zoom;  //缩放系数
	CRect m_Rect; //对话框的矩形区域
	float m_imgX; //图片左上角X轴偏移量
	float m_imgY; //图片左上角Y轴偏移量
	Gdiplus::PointF m_PtStart;    //按下鼠标左键，记录当前的m_imgX,m_imgY
	Gdiplus::PointF m_mouseDown;  //按下鼠标左键时，鼠标在图片控件中的位置；
	bool m_mousepressed; //记录是否按下了鼠标左键

public:
	void SetShowPic(cv::Mat mat);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
