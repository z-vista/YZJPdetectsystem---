#pragma once

#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include "Calibration.h"
#include "LoggerEx.h"

// CCamSetDlg 对话框
#define MAX_RATIO_TABLE_ITEM 119


class CCamSetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCamSetDlg)

public:
	CCamSetDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CCamSetDlg();
	//HINSTANCE m_hResDll;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAMSET_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	HICON m_hIcon;
	//CRectTracker m_kaRectTracker;
	//CRectTracker m_rctCurTracker;   //当前选中的矩形区域
	//CRectTracker m_rctTracker[MAX_RECT_NUM]; //用于存储已画的矩形区域
	//bool m_IsChose;  //标记是否被选中
	//bool m_IsDraw;   //标记“绘制”按钮是否按下
	//int m_rectNum;   //当前实际已经画的矩形的个数
	//int m_rctChoseNum;//当前选中的矩形的编号
	//int m_FlaMoveStep;//键盘方向键每响应一次的图像移动的像素单位上的步长
	//int dirct;     //用于标记那个方向键按下。1：左，2：右，3：上，4：下，5：delete（删除）

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	DECLARE_MESSAGE_MAP()
protected:
	int m_nCamId;
	cv::Rect m_Calirect;
	cv::Rect m_Runrect;
	bool m_bCali;
	cv::Mat m_CaliMat;
	int m_nResultTableItemCount;
	//std::vector<XRESOLUTIONTAB>  m_vecResultTable/*[MAX_RATIO_TABLE_ITEM]*/;
	std::vector<cv::Point>  m_vecResultTable;
	int m_nLighter;
	int m_nExposetimeMin;
	int m_nExposetimeMax;
	double m_nExposetime;
	int m_nGainMin;
	int m_nGainMax;
	double m_nGain;
public:
	int m_RoiX;
	int m_RoiY;
	int m_RoiW;
	int m_RoiH;
	int m_thresh;
	void SetCamId(int nCam);
	int GetCamId();
	void SetCamRect(cv::Rect roi, cv::Rect Roi2, int width, int height);
	cv::Rect GetCamRect();
	void SetLighter(int nLighter);
	void SetCamParaments(int nExposetime, int nExposetimeMin, int nExposetimeMax, int nGain, int nGainMin, int nGainMax);
	void InitUI();
	CRect m_DlgRect;
	void ChangeWidgetSize(UINT nID, int x, int y);
	void SaveCamCalibration(std::vector<cv::Point2i>	tp_allRulerPosition, int tp_iCameraIndex);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	void ShowImageToPic(cv::Mat& img, UINT ID);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();
	afx_msg void OnBnClickedCalibButton();
	afx_msg void OnBnClickedOfflineCaliButton();
	afx_msg void OnBnClickedPicTestButton();
	
	afx_msg void OnEnChangeThreshEdit();
	CSliderCtrl m_ExpoerTimeSlider;
	CSliderCtrl m_GainSlider;
	CSliderCtrl m_LigherSlider;
	afx_msg void OnNMCustomdrawBarExposetimeSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawBarGainSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMCustomdrawBarLighterSlider(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnEnChangeEditRoix();
	//afx_msg void OnEnChangeEditRoiw();
	//afx_msg void OnEnChangeEditRoiy();
	//afx_msg void OnEnChangeEditRoih();
	afx_msg void OnBnClickedCancel();
	int m_runRoiX;
	int m_runRoiY;
	int m_runRoiW;
	int m_runRoiH;
	//afx_msg void OnEnChangeEditRunroix();
	//afx_msg void OnEnChangeEditRunroiy();
	//afx_msg void OnEnChangeEditRunroiw();
	//afx_msg void OnEnChangeEditRunroih();
	BOOL m_bReverseX;
	BOOL m_bReverseY;
	afx_msg void OnEnKillfocusEditRoix();
	afx_msg void OnEnKillfocusEditRoiw();
	afx_msg void OnEnKillfocusEditRoiy();
	afx_msg void OnEnKillfocusEditRoih();
	afx_msg void OnEnKillfocusEditRunroix();
	afx_msg void OnEnKillfocusEditRunroiw();
	afx_msg void OnEnKillfocusEditRunroiy();
	afx_msg void OnEnKillfocusEditRunroih();
};
