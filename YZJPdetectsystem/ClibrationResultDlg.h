#pragma once
#include "stdafx.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
// CClibrationResultDlg �Ի���
#include <GdiPlus.h>  
using namespace Gdiplus;
class CClibrationResultDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CClibrationResultDlg)

public:
	CClibrationResultDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CClibrationResultDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CALIBRATION_RESULT_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
protected:
	cv::Mat m_resultPic;
	
	void DrawImageNewW(BYTE* image, int W, int H, int icolor, int  nID);
	
	float m_zoom;  //����ϵ��
	CRect m_Rect; //�Ի���ľ�������
	float m_imgX; //ͼƬ���Ͻ�X��ƫ����
	float m_imgY; //ͼƬ���Ͻ�Y��ƫ����
	Gdiplus::PointF m_PtStart;    //��������������¼��ǰ��m_imgX,m_imgY
	Gdiplus::PointF m_mouseDown;  //����������ʱ�������ͼƬ�ؼ��е�λ�ã�
	bool m_mousepressed; //��¼�Ƿ�����������

public:
	void SetShowPic(cv::Mat mat);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
