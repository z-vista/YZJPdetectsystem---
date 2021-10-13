// CamSetDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "YZJPdetectsystem.h"
#include "CamSetDlg.h"
#include "afxdialogex.h"
#include "findBound.h"
#include "YZJPdetectsystemDlg.h"
#include "ClibrationResultDlg.h"
#include "Calibration.h"
#include "LoggerEx.h"
// CCamSetDlg 对话框
extern YZJPdetectsystemDlg* g_dlg;
extern NetDataLog mlogger;
IMPLEMENT_DYNAMIC(CCamSetDlg, CDialogEx)

CCamSetDlg::CCamSetDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CAMSET_DIALOG, pParent)
	, m_RoiX(0)
	, m_RoiY(0)
	, m_RoiW(0)
	, m_RoiH(0)
	, m_thresh(200)
	, m_runRoiX(0)
	, m_runRoiY(0)
	, m_runRoiW(0)
	, m_runRoiH(0)
	, m_bReverseX(FALSE)
	, m_bReverseY(FALSE)
{
	m_nCamId = 0;
	//m_rect=cv::Rect(m_RoiX, m_RoiY, m_RoiW, m_RoiH);
	
}

CCamSetDlg::~CCamSetDlg()
{
	m_CaliMat.release();
}

void CCamSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ROIX, m_RoiX);
	DDX_Text(pDX, IDC_EDIT_ROIY, m_RoiY);
	DDX_Text(pDX, IDC_EDIT_ROIW, m_RoiW);
	DDX_Text(pDX, IDC_EDIT_ROIH, m_RoiH);
	DDX_Text(pDX, IDC_THRESH_EDIT, m_thresh);
	DDX_Control(pDX, IDC_BAR_EXPOSETIME_SLIDER, m_ExpoerTimeSlider);
	DDX_Control(pDX, IDC_BAR_GAIN_SLIDER, m_GainSlider);
	DDX_Control(pDX, IDC_BAR_LIGHTER_SLIDER, m_LigherSlider);
	DDX_Text(pDX, IDC_EDIT_RUNROIX, m_runRoiX);
	DDX_Text(pDX, IDC_EDIT_RUNROIY, m_runRoiY);
	DDX_Text(pDX, IDC_EDIT_RUNROIW, m_runRoiW);
	DDX_Text(pDX, IDC_EDIT_RUNROIH, m_runRoiH);
	DDX_Check(pDX, IDC_REVERSE_X_CHECK, m_bReverseX);
	DDX_Check(pDX, IDC_REVERSE_Y_CHECK, m_bReverseY);
}


BEGIN_MESSAGE_MAP(CCamSetDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CALIB_BUTTON, &CCamSetDlg::OnBnClickedCalibButton)
	ON_BN_CLICKED(IDC_OFFLINE_CALI_BUTTON, &CCamSetDlg::OnBnClickedOfflineCaliButton)
	ON_BN_CLICKED(IDC_PIC_TEST_BUTTON, &CCamSetDlg::OnBnClickedPicTestButton)
	ON_EN_CHANGE(IDC_THRESH_EDIT, &CCamSetDlg::OnEnChangeThreshEdit)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_BAR_EXPOSETIME_SLIDER, &CCamSetDlg::OnNMCustomdrawBarExposetimeSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_BAR_GAIN_SLIDER, &CCamSetDlg::OnNMCustomdrawBarGainSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_BAR_LIGHTER_SLIDER, &CCamSetDlg::OnNMCustomdrawBarLighterSlider)
	//ON_EN_CHANGE(IDC_EDIT_ROIX, &CCamSetDlg::OnEnChangeEditRoix)
	//ON_EN_CHANGE(IDC_EDIT_ROIW, &CCamSetDlg::OnEnChangeEditRoiw)
	//ON_EN_CHANGE(IDC_EDIT_ROIY, &CCamSetDlg::OnEnChangeEditRoiy)
	//ON_EN_CHANGE(IDC_EDIT_ROIH, &CCamSetDlg::OnEnChangeEditRoih)
	ON_BN_CLICKED(IDCANCEL, &CCamSetDlg::OnBnClickedCancel)
	//ON_EN_CHANGE(IDC_EDIT_RUNROIX, &CCamSetDlg::OnEnChangeEditRunroix)
	//ON_EN_CHANGE(IDC_EDIT_RUNROIY, &CCamSetDlg::OnEnChangeEditRunroiy)
	//ON_EN_CHANGE(IDC_EDIT_RUNROIW, &CCamSetDlg::OnEnChangeEditRunroiw)
	//ON_EN_CHANGE(IDC_EDIT_RUNROIH, &CCamSetDlg::OnEnChangeEditRunroih)
	ON_EN_KILLFOCUS(IDC_EDIT_ROIX, &CCamSetDlg::OnEnKillfocusEditRoix)
	ON_EN_KILLFOCUS(IDC_EDIT_ROIW, &CCamSetDlg::OnEnKillfocusEditRoiw)
	ON_EN_KILLFOCUS(IDC_EDIT_ROIY, &CCamSetDlg::OnEnKillfocusEditRoiy)
	ON_EN_KILLFOCUS(IDC_EDIT_ROIH, &CCamSetDlg::OnEnKillfocusEditRoih)
	ON_EN_KILLFOCUS(IDC_EDIT_RUNROIX, &CCamSetDlg::OnEnKillfocusEditRunroix)
	ON_EN_KILLFOCUS(IDC_EDIT_RUNROIW, &CCamSetDlg::OnEnKillfocusEditRunroiw)
	ON_EN_KILLFOCUS(IDC_EDIT_RUNROIY, &CCamSetDlg::OnEnKillfocusEditRunroiy)
	ON_EN_KILLFOCUS(IDC_EDIT_RUNROIH, &CCamSetDlg::OnEnKillfocusEditRunroih)
END_MESSAGE_MAP()


// CCamSetDlg 消息处理程序


BOOL CCamSetDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//设置窗口按桌面工作区的大小,调用一次onsize()
	RECT rt;
	SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rt, 0);//获取桌面工作区的大小
	MoveWindow(&rt);


	GetClientRect(&m_DlgRect);  //获取对话框的大小
	CString strbb_value;
	strbb_value.Format("%d", m_Calirect.x);
	GetDlgItem(IDC_EDIT_ROIX)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Calirect.y);
	GetDlgItem(IDC_EDIT_ROIY)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Calirect.width);
	GetDlgItem(IDC_EDIT_ROIW)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Calirect.height);
	GetDlgItem(IDC_EDIT_ROIH)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Runrect.x);
	GetDlgItem(IDC_EDIT_ROIX)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Runrect.y);
	GetDlgItem(IDC_EDIT_ROIY)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Runrect.width);
	GetDlgItem(IDC_EDIT_ROIW)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Runrect.height);
	GetDlgItem(IDC_EDIT_ROIH)->SetWindowText(strbb_value);
	// TODO:  在此添加额外的初始化
	//m_kaRectTracker.m_nStyle = CRectTracker::resizeInside | CRectTracker::dottedLine;//设置RectTracker样式
	//m_kaRectTracker.m_nHandleSize = 5; //控制柄的像素大小
	//m_kaRectTracker.m_rect.SetRect(0,0,0,0); //初始化m_rect的值

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CCamSetDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnSysCommand(nID, lParam);
}


void CCamSetDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialogEx::OnPaint()
	if (IsIconic())
	{

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	
	
		//m_kaRectTracker.Draw(&dc);//将这个四边形选区画出来
	

	}
}


HCURSOR CCamSetDlg::OnQueryDragIcon()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return CDialogEx::OnQueryDragIcon();
}


void CCamSetDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	//// TODO: 在此添加消息处理程序代码和/或调用默认值
	//CRect cwrect;
	//(GetDlgItem(IDC_CAM_PIC))->GetWindowRect(&cwrect); //获取窗体中控件的区域
	//ScreenToClient(&cwrect); //转换为相对区域的CRect 
	//if(point.x>cwrect.left && point.y>cwrect.top    //确保按下的坐标在控件区域内   
	//	&& point.x<cwrect.right&&point.y<cwrect.bottom)
	//{
	//	if (m_kaRectTracker.HitTest(point) < 0)     //如果未击中矩形选择框,重新画选择框
	//	{
	//		m_kaRectTracker.TrackRubberBand(this, point, TRUE);
	//		m_kaRectTracker.m_rect.NormalizeRect();   //正规化矩形（关于正规化矩形下面有介绍）
	//		
	//	}
	//	else           //如果击中矩形选择框
	//	{
	//		m_kaRectTracker.Track(this, point, TRUE);
	//		m_kaRectTracker.m_rect.NormalizeRect();   //正规化矩形
	//		//SendMessage(WM_LBUTTONUP,NULL,NULL);
	//	}
	//	Invalidate();   //刷新窗口区域，使得CrectTracker对象重绘到窗口上
	//}
	//
	CDialogEx::OnLButtonDown(nFlags, point);
}


BOOL CCamSetDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//if (pWnd == this && m_rctCurTracker.SetCursor(this, nHitTest))
	//	return TRUE;

	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}


BOOL CCamSetDlg::PreTranslateMessage(MSG* pMsg)
{
	//// TODO: 在此添加专用代码和/或调用基类
	//if (pMsg->message == WM_KEYDOWN)
	//{


	//	switch (pMsg->wParam)
	//	{
	//	case VK_LEFT:
	//		dirct = 1;
	//		break;
	//	case VK_RIGHT:
	//		dirct = 2;
	//		break;
	//	case VK_UP:
	//		dirct = 3;
	//		break;
	//	case VK_DOWN:
	//		dirct = 4;
	//		break;
	//	case VK_DELETE:
	//		dirct = 5;
	//		break;
	//	default:
	//		dirct = 0;
	//	}
	//}

	//ChangeRectPt(dirct);

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CCamSetDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CCamSetDlg::SetCamId(int nCam)
{
	m_nCamId = nCam;
}
int CCamSetDlg::GetCamId()
{
	return m_nCamId;
}
void CCamSetDlg::SetCamRect(cv::Rect roi,cv::Rect Roi2,int width,int height)
{
	//标定Rect
	//这里图像大小为整图，所以需要对m_kaRectTracker.m_rect进行转换到屏幕坐标系

	m_Calirect = roi;
	m_Runrect=Roi2;

	m_RoiX = m_Calirect.x;
	m_RoiY = m_Calirect.y;
	m_RoiW = m_Calirect.width;
	m_RoiH = m_Calirect.height;
	m_runRoiX = m_Runrect.x;
	m_runRoiY = m_Runrect.y;
	m_runRoiW = m_Runrect.width;
	m_runRoiH = m_Runrect.height;
}
cv::Rect CCamSetDlg::GetCamRect()
{
	return m_Calirect;
}


void CCamSetDlg::SetLighter(int nLighter)
{
	m_nLighter = nLighter;
}
void CCamSetDlg::SetCamParaments(int nExposetime, int nExposetimeMin, int nExposetimeMax, int nGain, int nGainMin, int nGainMax)
{
	m_nExposetimeMin = nExposetimeMin;
	m_nExposetimeMax= nExposetimeMax;
	m_nExposetime= nExposetime;
	m_nGainMin= nGainMin;
	m_nGainMax= nGainMax;
	m_nGain= nGain;
}
void CCamSetDlg::InitUI()
{
	////初始化界面参数UI
	CString strbb_value;
	strbb_value.Format("%d", m_Calirect.x);
	GetDlgItem(IDC_EDIT_ROIX)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Calirect.y);
	GetDlgItem(IDC_EDIT_ROIY)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Calirect.width);
	GetDlgItem(IDC_EDIT_ROIW)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Calirect.height);
	GetDlgItem(IDC_EDIT_ROIH)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Runrect.x);
	GetDlgItem(IDC_EDIT_RUNROIX)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Runrect.y);
	GetDlgItem(IDC_EDIT_RUNROIY)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Runrect.width);
	GetDlgItem(IDC_EDIT_RUNROIW)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_Runrect.height);
	GetDlgItem(IDC_EDIT_RUNROIH)->SetWindowText(strbb_value);
	//初始化相机参数UI
	CString sTemp;

	m_ExpoerTimeSlider.SetRange(m_nExposetimeMin / 100, m_nExposetimeMax / 100);
	m_ExpoerTimeSlider.SetTicFreq(10);
	m_ExpoerTimeSlider.SetPos(m_nExposetime / 100);
	sTemp.Format(_T("%d"), m_nExposetime);
	GetDlgItem(IDC_EXPOSETIME_EDIT)->SetWindowText(sTemp);

	m_GainSlider.SetRange(m_nGainMin, m_nGainMax);
	m_GainSlider.SetTicFreq(10);
	m_GainSlider.SetPos(m_nGain);
	sTemp.Format(_T("%d"), m_nGain);
	GetDlgItem(IDC_GAIN_EDIT)->SetWindowText(sTemp);

	m_LigherSlider.SetRange(0, 255);
	m_LigherSlider.SetTicFreq(1);
	m_LigherSlider.SetPos(m_nLighter);
	sTemp.Format(_T("%d"), m_nLighter);
	GetDlgItem(IDC_LIGHTER_EDIT)->SetWindowText(sTemp);
	int nValue = 0;
	char* cp = new char[64];
	std::string tp_NodeStr;
	sprintf(cp, "ReverseX%d", m_nCamId);
	tp_NodeStr = cp;
	nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 0, g_dlg->lpPath);//
	m_bReverseX = (nValue == 1 ? TRUE : FALSE);
	((CButton*)GetDlgItem(IDC_REVERSE_X_CHECK))->SetCheck(nValue); //瓦纸比例==5开关
	sprintf(cp, "ReverseY%d", m_nCamId);
	tp_NodeStr = cp;
	nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 0, g_dlg->lpPath);//
	m_bReverseY = (nValue == 1 ? TRUE : FALSE);
	((CButton*)GetDlgItem(IDC_REVERSE_Y_CHECK))->SetCheck(nValue); //瓦纸比例==5开关
}
void CCamSetDlg::ShowImageToPic(cv::Mat& img, UINT ID)
{
	if (img.empty())
	{
		return;
	}
	cv::Mat imgTmp;
	//CRect rect;
	//if (m_bCali)//标定时拷贝图像
	//{
		img.copyTo(m_CaliMat);
		OutputDebugString("xxxx-- 拷贝 标定图像 \n");
	//}
		
	//GetDlgItem(ID)->GetClientRect(&rect);  // 获取控件大小

	CRect rect_ctr;
	(GetDlgItem(ID))->GetWindowRect(&rect_ctr);
	ScreenToClient(rect_ctr);
	//RedrawWindow(CRect(0, 0, rect.Width(), rect.Height()));//重绘指定区域
	//Sleep(10);
	CString strOutput;
	
	strOutput.Format("xxxx--  rect_ctr left=%d，top=%d，right=%d，bottom=%d  ！\n", rect_ctr.left, rect_ctr.top, rect_ctr.right, rect_ctr.bottom);
	OutputDebugString(strOutput);

	cv::Size mysize = cv::Size(rect_ctr.Width(), rect_ctr.Height());
	cv::rectangle(img, m_Runrect, cv::Scalar(0, 0, 255), 5);
	cv::rectangle(img, m_Calirect, cv::Scalar(255, 0, 0), 5);
	cv::resize(img, imgTmp, mysize, 0, 0, CV_INTER_AREA);// 缩放Mat并备份

	// 转一下格式,这段可以放外面

	switch (imgTmp.channels())

	{

	case 1:

		cv::cvtColor(imgTmp, imgTmp, CV_GRAY2BGRA); // GRAY单通道

		break;

	case 3:

		cv::cvtColor(imgTmp, imgTmp, CV_BGR2BGRA);  // BGR三通道

		break;

	default:

		break;

	}

	int pixelBytes = imgTmp.channels() * (imgTmp.depth() + 1); // 计算一个像素多少个字节

	// 制作bitmapinfo(数据头)

	BITMAPINFO bitInfo;

	bitInfo.bmiHeader.biBitCount = 8 * pixelBytes;

	bitInfo.bmiHeader.biWidth = imgTmp.cols;

	bitInfo.bmiHeader.biHeight = -imgTmp.rows;

	bitInfo.bmiHeader.biPlanes = 1;

	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	bitInfo.bmiHeader.biCompression = BI_RGB;

	bitInfo.bmiHeader.biClrImportant = 0;

	bitInfo.bmiHeader.biClrUsed = 0;

	bitInfo.bmiHeader.biSizeImage = 0;

	bitInfo.bmiHeader.biXPelsPerMeter = 0;

	bitInfo.bmiHeader.biYPelsPerMeter = 0;

	// Mat.data+ bitmap数据头 -> MFC

	CDC* pDC = GetDlgItem(ID)->GetDC();

	
	////显示到左下角
	//::StretchDIBits(
	//	pDC->GetSafeHdc(),
	//	0, 0, rect.Width(), rect.Height(),
	//	0, 0, rect.Width(), rect.Height(),
	//	imgTmp.data,
	//	&bitInfo,
	//	DIB_RGB_COLORS,
	//	SRCCOPY
	//	);
	//矩形区域转换到图像上
	

	//if (imgTmp.cols >= rect.Width() && imgTmp.rows >= rect.Height())
	//{
		//显示到左上角
		::StretchDIBits(
			pDC->GetSafeHdc(),
			0, imgTmp.rows - rect_ctr.Height(), rect_ctr.Width(), rect_ctr.Height(),
			0, 0, rect_ctr.Width(), rect_ctr.Height(),
			imgTmp.data,
			&bitInfo,
			DIB_RGB_COLORS,
			SRCCOPY
		);
	//}
	//else
	//{
	//	//显示到中心
	//	::StretchDIBits(
	//		pDC->GetSafeHdc(),
	//		-imgTmp.cols * 0.5 + rect.Width() * 0.5, (imgTmp.rows * 0.5 - rect.Height() * 0.5), rect.Width(), rect.Height(),
	//		0, 0, rect.Width(), rect.Height(),
	//		imgTmp.data,
	//		&bitInfo,
	//		DIB_RGB_COLORS,
	//		SRCCOPY
	//	);
	//}
		//CString strbb_value;
		//strbb_value.Format("%d", m_rect.x);
		//GetDlgItem(IDC_EDIT_ROIX)->SetWindowText(strbb_value);
		//strbb_value.Format("%d", m_rect.y);
		//GetDlgItem(IDC_EDIT_ROIY)->SetWindowText(strbb_value);
		//strbb_value.Format("%d", m_rect.width);
		//GetDlgItem(IDC_EDIT_ROIW)->SetWindowText(strbb_value);
		//strbb_value.Format("%d", m_rect.height);
		//GetDlgItem(IDC_EDIT_ROIH)->SetWindowText(strbb_value);

	ReleaseDC(pDC);

}


void CCamSetDlg::SaveCamCalibration(std::vector<cv::Point2i>	tp_allRulerPosition,int tp_iCameraIndex)
{
	// TODO: 在此添加控件通知处理程序代码
	//>>>>保存标定结果
	CString tp_NodeStr;
	switch (tp_iCameraIndex)
	{
	case 0:
		tp_NodeStr = "CameraOne";
		break;
	case 1:
		tp_NodeStr = "CameraTwo";
		break;
	case 2:
		tp_NodeStr = "CameraThree";
		break;
	case 3:
		tp_NodeStr = "CameraFour";
		break;
	}
	
	char tp_writeChar[20];

	int num = tp_allRulerPosition.size();
	int nCount = 0;
	for (int i = 0; i < num; i++)
	{
		if (tp_allRulerPosition[i].x != 0 && tp_allRulerPosition[i].y != 0)
		{
			sprintf(tp_writeChar, "%d", tp_allRulerPosition[i].x);////x
			char tp_KeyNameChar[64];
			sprintf(tp_KeyNameChar, "Cam_CaliRulerPosition%dx", i);
			WritePrivateProfileStringA(tp_NodeStr, tp_KeyNameChar, tp_writeChar, g_dlg->lpPath);
			sprintf(tp_writeChar, "%d", tp_allRulerPosition[i].y);////y
			sprintf(tp_KeyNameChar, "Cam_CaliRulerPosition%dy", i);
			WritePrivateProfileStringA(tp_NodeStr, tp_KeyNameChar, tp_writeChar, g_dlg->lpPath);
			
			nCount++;
		}

	}
	
	sprintf(tp_writeChar, "%d", nCount);
	WritePrivateProfileStringA(tp_NodeStr, "Cam_CaliPositionNum", tp_writeChar, g_dlg->lpPath);

}

//控件适应屏幕变化模块

//nID为控件ID，x,y分别为对话框的当前长和宽
void CCamSetDlg::ChangeWidgetSize(UINT nID, int x, int y)
{
	CWnd* pWnd;
	pWnd = GetDlgItem(nID);
	if (pWnd != NULL)     //判断是否为空，因为在窗口创建的时候也会调用OnSize函数，但是此时各个控件还没有创建，Pwnd为空
	{
		CRect rec;
		pWnd->GetWindowRect(&rec);  //获取控件变化前的大小
		ScreenToClient(&rec);   //将控件大小装换位在对话框中的区域坐标
		rec.left = rec.left * x / m_DlgRect.Width();  //按照比例调整空间的新位置
		rec.top = rec.top * y / m_DlgRect.Height();
		rec.bottom = rec.bottom * y / m_DlgRect.Height();
		rec.right = rec.right * x / m_DlgRect.Width();

		CRect E_rec;
		pWnd->GetWindowRect(&E_rec);  //获取控件变化前的大小


		//如果是下拉框，则不改变其高度
		TCHAR szClass[65] = { 0 };
		//if (GetClassName(pWnd->m_hWnd, szClass, 64) && _tcsicmp(szClass, _T("COMBOBOX")) == 0)
		//{
		//	pWnd->SetWindowPos(NULL, rec.left, rec.top, rec.Width(), m_rect.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
		//}
		//else
		//	pWnd->SetWindowPos(NULL, rec.left, rec.top, rec.Width(), rec.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);

		if (GetClassName(pWnd->m_hWnd, szClass, 64) && _tcsicmp(szClass, _T("EDIT")) == 0)
		{
			pWnd->SetWindowPos(NULL, rec.left, rec.top, rec.Width(), E_rec.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);
		}
		else
			pWnd->SetWindowPos(NULL, rec.left, rec.top, rec.Width(), rec.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);


		//	pWnd->MoveWindow(rec);   //伸缩控件
	}
}

void CCamSetDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if (nType != SIZE_MINIMIZED)  //判断窗口是不是最小化了，因为窗口最小化之后 ，窗口的长和宽会变成0，当前一次变化的时就会出现除以0的错误操作
	{
		ChangeWidgetSize(IDC_EDIT_ROIX, cx, cy);
		ChangeWidgetSize(IDC_EDIT_ROIY, cx, cy);
		ChangeWidgetSize(IDC_EDIT_ROIW, cx, cy);
		ChangeWidgetSize(IDC_EDIT_ROIH, cx, cy);
		ChangeWidgetSize(IDC_STATIC_ROIX, cx, cy);
		ChangeWidgetSize(IDC_STATIC_ROIY, cx, cy);
		ChangeWidgetSize(IDC_STATIC_ROIW, cx, cy);
		ChangeWidgetSize(IDC_STATIC_ROIH, cx, cy);
		ChangeWidgetSize(IDC_STATIC_ROISET, cx, cy);
		ChangeWidgetSize(IDC_EDIT_RUNROIX, cx, cy);
		ChangeWidgetSize(IDC_EDIT_RUNROIY, cx, cy);
		ChangeWidgetSize(IDC_EDIT_RUNROIW, cx, cy);
		ChangeWidgetSize(IDC_EDIT_RUNROIH, cx, cy);
		ChangeWidgetSize(IDC_STATIC_RUNROIX, cx, cy);
		ChangeWidgetSize(IDC_STATIC_RUNROIY, cx, cy);
		ChangeWidgetSize(IDC_STATIC_RUNROIW, cx, cy);
		ChangeWidgetSize(IDC_STATIC_RUNROIH, cx, cy);
		ChangeWidgetSize(IDC_STATIC_GROUP_RUN, cx, cy);

		ChangeWidgetSize(IDC_CAM_PIC, cx, cy);

		ChangeWidgetSize(IDC_EXPOSETIME_EDIT, cx, cy);
		ChangeWidgetSize(IDC_BAR_EXPOSETIME_SLIDER, cx, cy);
		ChangeWidgetSize(IDC_EXPOSETIME_STATIC, cx, cy);
		ChangeWidgetSize(IDC_GAIN_EDIT, cx, cy);
		ChangeWidgetSize(IDC_BAR_GAIN_SLIDER, cx, cy);
		ChangeWidgetSize(IDC_GAIN_STATIC, cx, cy);
		ChangeWidgetSize(IDC_LIGHTER_EDIT, cx, cy);
		ChangeWidgetSize(IDC_BAR_LIGHTER_SLIDER, cx, cy);
		ChangeWidgetSize(IDC_LIGHTER_STATIC, cx, cy);

		ChangeWidgetSize(IDC_THRESH_STATIC, cx, cy);
		ChangeWidgetSize(IDC_THRESH_EDIT, cx, cy);
		ChangeWidgetSize(IDC_CALIB_BUTTON, cx, cy);
		ChangeWidgetSize(IDC_OFFLINE_CALI_BUTTON, cx, cy);
		ChangeWidgetSize(IDC_PIC_TEST_BUTTON, cx, cy);
		ChangeWidgetSize(IDC_REVERSE_X_CHECK, cx, cy);
		ChangeWidgetSize(IDC_REVERSE_Y_CHECK, cx, cy);

		ChangeWidgetSize(IDOK, cx, cy);
		ChangeWidgetSize(IDCANCEL, cx, cy);

		GetClientRect(&m_DlgRect);   //最后要更新对话框的大小，当做下一次变化的旧坐标；

		//Invalidate();//更新窗口0426
	}

	// TODO: 在此处添加消息处理程序代码
}

void CCamSetDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
		//记录矩形框
	UpdateData(TRUE);
	char tp_strToWrite[MAX_PATH];
	std::string tp_NodeStrX, tp_NodeStrY, tp_NodeStrW, tp_NodeStrH,tp_NodeStr;
	char* cp = new char[64]; 
	//保存标定框选区域
	sprintf(cp, "CaliImage%d", m_nCamId);
	tp_NodeStrX = cp;
	tp_NodeStrX += "_x";
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_Calirect.x);
	WritePrivateProfileString("Set ImageFormat", tp_NodeStrX.c_str(), tp_strToWrite, g_dlg->lpPath);
	tp_NodeStrY = cp;
	tp_NodeStrY += "_y";
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_Calirect.y);
	WritePrivateProfileString("Set ImageFormat", tp_NodeStrY.c_str(), tp_strToWrite, g_dlg->lpPath);
	tp_NodeStrW = cp;
	tp_NodeStrW += "_w";
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_Calirect.width);
	WritePrivateProfileString("Set ImageFormat", tp_NodeStrW.c_str(), tp_strToWrite, g_dlg->lpPath);
	tp_NodeStrH = cp;
	tp_NodeStrH += "_h";
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_Calirect.height);
	WritePrivateProfileString("Set ImageFormat", tp_NodeStrH.c_str(), tp_strToWrite, g_dlg->lpPath);
	//保存运行框选区域
	sprintf(cp, "RunImage%d", m_nCamId);
	tp_NodeStrX = cp;
	tp_NodeStrX += "_x";
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_Runrect.x);
	WritePrivateProfileString("Set ImageFormat", tp_NodeStrX.c_str(), tp_strToWrite, g_dlg->lpPath);
	tp_NodeStrY = cp;
	tp_NodeStrY += "_y";
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_Runrect.y);
	WritePrivateProfileString("Set ImageFormat", tp_NodeStrY.c_str(), tp_strToWrite, g_dlg->lpPath);
	tp_NodeStrW = cp;
	tp_NodeStrW += "_w";
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_Runrect.width);
	WritePrivateProfileString("Set ImageFormat", tp_NodeStrW.c_str(), tp_strToWrite, g_dlg->lpPath);
	tp_NodeStrH = cp;
	tp_NodeStrH += "_h";
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_Runrect.height);
	WritePrivateProfileString("Set ImageFormat", tp_NodeStrH.c_str(), tp_strToWrite, g_dlg->lpPath);
	//保存光源参数
	sprintf(cp, "LightValue%d", m_nCamId);
	tp_NodeStr = cp;
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_nLighter);
	WritePrivateProfileString("Set_Light", tp_NodeStr.c_str(), tp_strToWrite, g_dlg->lpPath);
	//设置光源
	// 
	//保存曝光时间
	sprintf(cp, "Exposetime%d", m_nCamId);
	tp_NodeStr = cp;
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", (int)m_nExposetime);
	WritePrivateProfileString("Set_Camera", tp_NodeStr.c_str(), tp_strToWrite, g_dlg->lpPath);
	sprintf(cp, "Gain%d", m_nCamId);
	tp_NodeStr = cp;
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", (int)m_nGain);
	WritePrivateProfileString("Set_Camera", tp_NodeStr.c_str(), tp_strToWrite, g_dlg->lpPath);
	//保存镜像设置
	sprintf(cp, "ReverseX%d", m_nCamId);
	tp_NodeStr = cp;
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_bReverseX==TRUE?1:0);
	WritePrivateProfileString("Set ImageFormat", tp_NodeStr.c_str(), tp_strToWrite, g_dlg->lpPath);
	sprintf(cp, "ReverseY%d", m_nCamId);
	tp_NodeStr = cp;
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_bReverseY == TRUE ? 1 : 0);
	WritePrivateProfileString("Set ImageFormat", tp_NodeStr.c_str(), tp_strToWrite, g_dlg->lpPath);

	g_dlg->m_control->StopGrabCamera(m_nCamId - 1);
	int nValue = 0;

	//设置镜像
	sprintf(cp, "ReverseX%d", m_nCamId);
	tp_NodeStr = cp;
	nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 0, g_dlg->lpPath);//
	g_dlg->m_control->SetCameraReverseX(m_nCamId - 1, nValue==1?true:false);
	sprintf(cp, "ReverseY%d", m_nCamId);
	tp_NodeStr = cp;
	nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 0, g_dlg->lpPath);//
	g_dlg->m_control->SetCameraReverseY(m_nCamId - 1, nValue == 1 ? true : false);

	sprintf(cp, "RunImage%d_w", m_nCamId);
	tp_NodeStr = cp;
	nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
	g_dlg->m_control->SetCameraWidth(m_nCamId - 1, nValue);

	sprintf(cp, "RunImage%d_h", m_nCamId);
	tp_NodeStr = cp;
	nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
	g_dlg->m_control->SetCameraHeight(m_nCamId - 1, nValue);

	sprintf(cp, "RunImage%d_x", m_nCamId);
	tp_NodeStr = cp;
	nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//

	g_dlg->m_control->SetCameraOffsetX(m_nCamId - 1, nValue);

	sprintf(cp, "RunImage%d_y", m_nCamId);
	tp_NodeStr = cp;
	nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
	g_dlg->m_control->SetCameraOffsetY(m_nCamId - 1, nValue);

	int nShutter = 0, nGain = 0;
	double  dOldShutter = 0, dOldGain = 0;
	g_dlg->m_control->GetExposetime(m_nCamId - 1, dOldShutter);
	nShutter = m_nExposetime;
	if (nShutter != dOldShutter) {
		g_dlg->m_control->SetExposetime(m_nCamId - 1, nShutter);
		mlogger.TraceInfo("xxxx--设置曝光:%d   ！\n", nShutter);
	}

	g_dlg->m_control->GetGain(m_nCamId - 1, dOldGain);	
	nGain = m_nGain;
	if (nGain != dOldGain) {
		g_dlg->m_control->SetGain(m_nCamId - 1, nGain);
		mlogger.TraceInfo("xxxx--设置增益:%d   ！\n", nGain);
	}
	g_dlg->m_control->StartGrabCamera(m_nCamId - 1);
	
	CDialogEx::OnOK();
}


void CCamSetDlg::OnBnClickedCalibButton()//自动标定
{
	// TODO: 在此添加控件通知处理程序代码
	//1、获取图像
	m_bCali = true;
	mlogger.TraceInfo(_T("xxxx--在线标定相机%dbegin \n"));
	//2、截取图像
	if (m_CaliMat.empty())
	{
		m_bCali = false;
		mlogger.TraceWarning(_T("xxxx-- 在线标定拷贝图像为NULL \n"));
		return;
	}
	else
	{
		cv::Mat imageROI = cv::Mat(m_Calirect.height, m_Calirect.width, CV_8UC3);

	
			cv::Mat imageSrc;
			m_CaliMat.copyTo(imageSrc);
			char chresulName[64];
			sprintf(chresulName, ".\\设备%dSrc_Cali.bmp", m_nCamId);
			imwrite(chresulName, imageSrc);
		
			imageSrc(m_Calirect).copyTo(imageROI);
			sprintf(chresulName, ".\\设备%dSrc_ROI.bmp", m_nCamId);
			cv::imwrite(chresulName, imageROI);

			imageSrc.release();
			mlogger.TraceInfo(_T("xxxx-- 在线标定ROI 图像 \n"));

			m_nResultTableItemCount = 0;
			//memset(m_ResultTable, 0, sizeof(m_vecResultTable));
			m_vecResultTable.clear();
			//int count = MAX_RATIO_TABLE_ITEM;
			if (GetResolutionTab(imageROI, m_vecResultTable, 5, m_thresh))
			{
				m_nResultTableItemCount = m_vecResultTable.size();
				for (int i = 0; i < m_nResultTableItemCount; i++)
				{
					//if (m_vecResultTable[i].X != 0)
					//cv::circle(imageROI, cv::Point2d(m_vecResultTable[i].X, m_rect.height / 2), 5, cv::Scalar(255, 0, 0), -1, 8);
					cv::circle(imageROI, cv::Point2d(m_vecResultTable[i].x, m_vecResultTable[i].y), 5, cv::Scalar(255, 0, 0), -1, 8);
				}
			}
			else
			{
				MessageBox(_T("错误：在线标定失败！\n可能原因：图像亮度不合适！"), _T("提示"), MB_ICONHAND);
			}
	
		CClibrationResultDlg* pReultDlg = new CClibrationResultDlg();
		pReultDlg->SetShowPic(imageROI);

		if (IDOK == pReultDlg->DoModal())
		{

			char chresulName[64];
			sprintf(chresulName, ".\\设备%dcolorresult.bmp", m_nCamId);
			cv::imwrite(chresulName, imageROI);

			///保存结果
			//g_dlg->SaveRatioTable(m_nCamId, m_vecResultTable, m_nResultTableItemCount);
		}
		imageROI.release();
		if (pReultDlg)
		{
			delete pReultDlg;
		}
	}

	m_bCali = false;
}


void CCamSetDlg::OnBnClickedOfflineCaliButton()
{
	// TODO: 在此添加控件通知处理程序代码


	CString strFileName;
	CString filt = "bmp (*.bmp);raw (*.raw)|*.bmp;*.raw|All files (*.*)|*.*||";
	CFileDialog dlgFile(TRUE, NULL, NULL, NULL, filt, NULL);
	//BYTE* pBuffer, *pTemp;
	//FILE* fp;
	char chFileName[MAX_PATH] = { 0 };


	//int LW, LH, ire;
	OutputDebugString("xxxxx--打开标定图像路径\n");
	if (dlgFile.DoModal() == IDCANCEL) return;

	strFileName = dlgFile.GetPathName();

	cv::Mat src = cv::imread(strFileName.GetBuffer());
	OutputDebugString("xxxxx--读取标定图像\n");
	//cv::Rect rect = cv::Rect(m_RoiX, m_RoiY, m_RoiW, m_RoiH);
	//cv::Mat imageROI = cv::Mat(rect.height, rect.width, CV_8UC3);
	cv::Mat imageROI = cv::Mat(src.rows, src.cols, CV_8UC3,src.data);
	//src(rect).copyTo(imageROI);
	char chresulName[64];
	sprintf(chresulName, ".\\roi设备%d.bmp", m_nCamId);
	cv::imwrite(chresulName, imageROI);
	OutputDebugString("xxxxx--获取标定图像，保存\n");
	//3、算法提取刻度		
	m_nResultTableItemCount = 0;
	//memset(m_ResultTable, 0, sizeof(m_vecResultTable));
	m_vecResultTable.clear();
	//int count = MAX_RATIO_TABLE_ITEM;
	if (GetResolutionTab(imageROI, m_vecResultTable, 5,m_thresh))
	{
		OutputDebugString("xxxxx--标定1\n");
		m_nResultTableItemCount = m_vecResultTable.size();
		for (int i = 0; i < m_nResultTableItemCount; i++)
		{
			//if (m_vecResultTable[i].X != 0)
			//cv::circle(imageROI, cv::Point2d(m_vecResultTable[i].X, m_rect.height / 2), 5, cv::Scalar(255, 0, 0), -1, 8);
			cv::circle(imageROI, cv::Point2d(m_vecResultTable[i].x, m_vecResultTable[i].y), 5, cv::Scalar(255, 0, 0), -1, 8);
		}
		OutputDebugString("xxxxx--标定2\n");
	}
	else
	{
		MessageBox(_T("错误：在线标定失败！\n可能原因：图像亮度不合适！"), _T("提示"), MB_ICONHAND);
	}



	CClibrationResultDlg* pReultDlg = new CClibrationResultDlg();
	pReultDlg->SetShowPic(imageROI);
	OutputDebugString("xxxxx--标定3\n");
	if (IDOK == pReultDlg->DoModal())
	{

		char chresulName[64];
		sprintf(chresulName, ".\\设备%dcolorresult.bmp", m_nCamId);
		cv::imwrite(chresulName, imageROI);
		
		///保存结果
		SaveCamCalibration(m_vecResultTable,m_nCamId-1);
		//g_dlg->SaveRatioTable(m_nCamId, m_vecResultTable, m_nResultTableItemCount);
	}
	imageROI.release();
	if (pReultDlg)
	{
		delete pReultDlg;
	}
}


void CCamSetDlg::OnBnClickedPicTestButton()
{
	// TODO: 在此添加控件通知处理程序代码

	CString strFileName;
	CString filt = "bmp (*.bmp);raw (*.raw)|*.bmp;*.raw|All files (*.*)|*.*||";
	CFileDialog dlgFile(TRUE, NULL, NULL, NULL, filt, NULL);
	
	char chFileName[MAX_PATH] = { 0 };



	if (dlgFile.DoModal() == IDCANCEL) return;

	strFileName = dlgFile.GetPathName();

	cv::Mat src = cv::imread(strFileName.GetBuffer(),0);

	Vec4i L;//检边位置
	cv::Point m_loc;
	double position = 0;
	int result = 0;

	
	if (!src.empty())
	{
		std::chrono::time_point<std::chrono::steady_clock> beginTime, endTime;
		std::chrono::duration<double, std::milli> elapsed_milliseconds;
		beginTime = std::chrono::high_resolution_clock::now();
		//图像截取
	
		cv::Rect tpRect = cv::Rect(m_Runrect.x, m_Runrect.y, m_Runrect.width, m_Runrect.height);
		
		Mat tmpImage = src/*(tpRect)*/;
		//src(m_rect).copyTo(imageROI);
		CFindBound pFindBound;
		result = pFindBound.BoundLocWithState(tmpImage, m_loc, 1);
		m_loc.x += tpRect.x;
		m_loc.y += tpRect.y;
		mlogger.TraceInfo("xxxx--相机1算法结束 result=%d  m_loc(%d,%d)\n", result, m_loc.x, m_loc.y);
		tmpImage.release();
		vector<cv::Point>  m_CalibTable;
		int nTabCount = 0;
		//计算物理位置
		if (m_nCamId==1)
		{
			 m_CalibTable=g_dlg->m_CalibTable1;
			 nTabCount= g_dlg->m_nTabItemCount1;
		}
		else if (m_nCamId == 2)
		{
			m_CalibTable = g_dlg->m_CalibTable2;
			nTabCount = g_dlg->m_nTabItemCount2;
		}
		else if (m_nCamId == 3)
		{
			m_CalibTable = g_dlg->m_CalibTable3;
			nTabCount = g_dlg->m_nTabItemCount3;
		}
		else if (m_nCamId == 4)
		{
			m_CalibTable = g_dlg->m_CalibTable4;
			nTabCount = g_dlg->m_nTabItemCount4;
		}

		if (m_CalibTable.size()> 0)
		{
			position = g_dlg->ComputePosition(m_nCamId, m_CalibTable, m_loc);//纸宽

			Mat imgRGB;
			cv::cvtColor(src, imgRGB, COLOR_GRAY2RGB);

			//画矩形
			cv::rectangle(imgRGB, m_Runrect, Scalar(0, 0, 255), 5);
			cv::rectangle(imgRGB, m_Calirect, Scalar(255, 0, 0), 5);
		
			L[0] = m_loc.x;//初始化
			L[1] = tpRect.y;
			L[2] = m_loc.x;
			L[3] = tpRect.y+tpRect.height;

			cv::Point p1(L[0], L[1]);
			cv::Point p2(L[2], L[3]);
			mlogger.TraceInfo("xxxx--相机1 p1(%d ,%d) p2(%d,%d)\n", L[0], L[1], L[2], L[3]);
			cv::line(imgRGB, p1, p2, Scalar(0, 255, 0), 5);
			ShowImageToPic(imgRGB, IDC_CAM_PIC);
		}
		else
		{
			MessageBox(_T("错误：计算纸边宽度！\n可能原因：标定数据为NULL！"), _T("提示"), MB_ICONHAND);
		}
		src.release();
		//位置
		//position += 原点与相机边缘;
		mlogger.TraceInfo("xxxx--相机1中纸宽position=%d\n", position);
		endTime = std::chrono::high_resolution_clock::now();
		elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
		mlogger.TraceInfo(_T("xxxx--相机1算法时间：%.2lf ms"), elapsed_milliseconds.count());
	}
	else {
		L[0] = m_Runrect.x + 1;//初始化
		L[1] = m_Runrect.y;
		L[2] = m_Runrect.x + 1;
		L[3] = m_Runrect.y + m_Runrect.height - 1;

	}
}


void CCamSetDlg::OnEnChangeThreshEdit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	
	// TODO:  在此添加控件通知处理程序代码

	UpdateData(TRUE);
}


void CCamSetDlg::OnNMCustomdrawBarExposetimeSlider(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CString sTemp;
	int exposertime = m_ExpoerTimeSlider.GetPos();

	sTemp.Format(_T("%d"), exposertime*100);
	GetDlgItem(IDC_EXPOSETIME_EDIT)->SetWindowText(sTemp);
	if (exposertime > 0)
	{
		bool ret = g_dlg->m_control->SetExposetime(m_nCamId - 1, (double)(exposertime * 100));
		if (ret == false)
			MessageBox(_T("错误：设置曝光时间失败！\n可能原因：超过设备曝光时间范围！"), _T("提示"), MB_ICONHAND);
		m_nExposetime = exposertime*100;
		/*TCHAR stemp[64] = _T("");
		_stprintf(stemp, _T("%d"), int(exposertime*100));*/
	}
	*pResult = 0;
}


void CCamSetDlg::OnNMCustomdrawBarGainSlider(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码


	CString sTemp;
	int gain = m_GainSlider.GetPos();

	sTemp.Format(_T("%d"), gain);
	GetDlgItem(IDC_GAIN_EDIT)->SetWindowText(sTemp);
	
	if (gain > 0)
	{
	/*	TCHAR stemp[64] = _T("");
		_stprintf(stemp, _T("%d"), int(gain));*/
		bool ret = g_dlg->m_control->SetGain(m_nCamId - 1, (double)gain);
		if (ret == false)
			MessageBox(_T("错误：设置增益失败！\n可能原因：超过设备增益范围！"), _T("提示"), MB_ICONHAND);
		m_nGain = gain;
	}

	*pResult = 0;
}


void CCamSetDlg::OnNMCustomdrawBarLighterSlider(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CString sTemp;
	int lighter = m_LigherSlider.GetPos();

	sTemp.Format(_T("%d"), lighter);
	GetDlgItem(IDC_LIGHTER_EDIT)->SetWindowText(sTemp);
	if (lighter < 0)
	{
		lighter = 0;
	}
	if (lighter > 255)
	{
		lighter = 255;
	}
	g_dlg->m_iLightValue[m_nCamId-1] = lighter;
	//switch (m_nCamId)
	//{
	//case 1:
	//	g_dlg->m_iLightValue[0] = lighter;
	//	break;
	//case 2:
	//	g_dlg->m_iLightValue[1] = lighter;
	//	break;
	//case 3:
	//	g_dlg->m_iLightValue[2] = lighter;
	//	break;
	//case 4:
	//	g_dlg->m_iLightValue[3] = lighter;
	//	break;
	//default:
	//	g_dlg->m_iLightValue[0] = lighter;
	//	break;
	//}
	
	if (g_dlg->m_bFSTLight==true)
	{
		mlogger.TraceInfo("xxxx--菲视特设置光源%d亮度:%d   ！\n", m_nCamId, lighter);
		g_dlg->m_LightCom[0].SetLuminance(g_dlg->m_hCom[0], g_dlg->m_iLightValue[1], g_dlg->m_iLightValue[0]);
		g_dlg->m_LightCom[1].SetLuminance(g_dlg->m_hCom[1], g_dlg->m_iLightValue[3], g_dlg->m_iLightValue[2]);
	}
	else
	{
		mlogger.TraceInfo("xxxx--康视达设置光源%d亮度:%d   ！\n", m_nCamId, lighter);
		g_dlg->m_LightCom[0].SetLuminanceEx(g_dlg->m_hCom[0], g_dlg->m_iLightValue[0], g_dlg->m_iLightValue[1]);
		g_dlg->m_LightCom[1].SetLuminanceEx(g_dlg->m_hCom[1], g_dlg->m_iLightValue[2], g_dlg->m_iLightValue[3]);
	}

	if (lighter > 0)
	{
		/*TCHAR stemp[64] = _T("");
		_stprintf(stemp, _T("%d"), int(lighter));*/
		m_nLighter = lighter;
	}

	*pResult = 0;
}


//void CCamSetDlg::OnEnChangeEditRoix()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	UpdateData(TRUE);
//	
//	m_Calirect.x = m_RoiX;
//}


//void CCamSetDlg::OnEnChangeEditRoiw()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	UpdateData(TRUE);
//	int nMaxWidth = 0;
//	g_dlg->m_control->GetCameraMaxWidth(m_nCamId, nMaxWidth);
//	if (nMaxWidth - m_RoiW <= 0)
//		m_RoiW = nMaxWidth;
//	m_Calirect.width = m_RoiW;
//}


//void CCamSetDlg::OnEnChangeEditRoiy()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	UpdateData(TRUE);
//	m_Calirect.y = m_RoiY;
//}


//void CCamSetDlg::OnEnChangeEditRoih()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	UpdateData(TRUE);
//	int nMaxHeight = 0;
//	g_dlg->m_control->GetCameraMaxWidth(m_nCamId, nMaxHeight);
//	if (nMaxHeight - m_RoiH <= 0)
//		m_RoiH = nMaxHeight;
//	m_Calirect.height = m_RoiH;
//}


void CCamSetDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	g_dlg->m_control->StopGrabCamera(m_nCamId - 1);
	char* cp = new char[64];
	std::string tp_NodeStr;
	int nMaxPic = GetPrivateProfileInt(_T("Set_Camera"), _T("useMaxPic"), 0, g_dlg->lpPath);;
	if (nMaxPic)
	{
		//设置
	
	}
	else
	{
		int nValue = 0;
		sprintf(cp, "ReverseX%d", m_nCamId);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 0, g_dlg->lpPath);//
		g_dlg->m_control->SetCameraReverseX(m_nCamId - 1, nValue == 1 ? true : false);
		sprintf(cp, "ReverseY%d", m_nCamId);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 0, g_dlg->lpPath);//
		g_dlg->m_control->SetCameraReverseY(m_nCamId - 1, nValue == 1 ? true : false);

	
		sprintf(cp, "RunImage%d_w", m_nCamId);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
		g_dlg->m_control->SetCameraWidth(m_nCamId - 1, nValue);

		sprintf(cp, "RunImage%d_h", m_nCamId);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
		g_dlg->m_control->SetCameraHeight(m_nCamId - 1, nValue);
		
		sprintf(cp, "RunImage%d_x", m_nCamId);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
	
		g_dlg->m_control->SetCameraOffsetX(m_nCamId-1, nValue);
		
		sprintf(cp, "RunImage%d_y", m_nCamId);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
		g_dlg->m_control->SetCameraOffsetY(m_nCamId-1, nValue);

	}

	//设置曝光与增益
	int nShutter = 0, nGain = 0;
	double  dOldShutter = 0, dOldGain = 0;
	g_dlg->m_control->GetExposetime(m_nCamId - 1, dOldShutter);
	sprintf(cp, "Exposetime%d", m_nCamId);
	tp_NodeStr = cp;
	nShutter = GetPrivateProfileInt("Set_Camera", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
	if (nShutter != dOldShutter) {
		g_dlg->m_control->SetExposetime(m_nCamId-1, nShutter);
		mlogger.TraceInfo("xxxx--设置曝光:%d   ！\n", nShutter);
	}

	g_dlg->m_control->GetGain(m_nCamId-1, dOldGain);
	sprintf(cp, "Gain%d", m_nCamId);
	tp_NodeStr = cp;
	nGain = GetPrivateProfileInt("Set_Camera", tp_NodeStr.c_str(), 0, g_dlg->lpPath);//
	if (nGain != dOldGain) {
		g_dlg->m_control->SetGain(m_nCamId-1, nGain);
		mlogger.TraceInfo("xxxx--设置增益:%d   ！\n", nGain);
	}


	g_dlg->m_control->StartGrabCamera(m_nCamId - 1);
	CDialogEx::OnCancel();
}


//void CCamSetDlg::OnEnChangeEditRunroix()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	UpdateData(TRUE);
//	m_Runrect.x = m_runRoiX;
//}


//void CCamSetDlg::OnEnChangeEditRunroiy()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	UpdateData(TRUE);
//	m_Runrect.y = m_runRoiY;
//}


//void CCamSetDlg::OnEnChangeEditRunroiw()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	UpdateData(TRUE);
//	m_Runrect.width = m_runRoiW;
//}


//void CCamSetDlg::OnEnChangeEditRunroih()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//	UpdateData(TRUE);
//	m_Runrect.height = m_runRoiH;
//}


void CCamSetDlg::OnEnKillfocusEditRoix()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_RoiX = abs(m_RoiX);
	int nValue = 0;
	int nMaxWidth = 0;
	g_dlg->m_control->GetCameraMaxWidth(m_nCamId - 1, nMaxWidth);
	mlogger.TraceError("xxxx--MaxWidth==%d ！\n", nMaxWidth);
	if (m_RoiX > nMaxWidth)
		m_RoiX = nMaxWidth;
	if (m_RoiX == 0 || m_RoiX % 4 == 0)
	{
		if (((m_RoiX + m_RoiW) - nMaxWidth) > 0)
			m_RoiX = nMaxWidth - m_RoiW;
		nValue = m_RoiX;
	}
	else
	{
		int i = m_RoiX / 4;
		m_RoiX = (i + 1) * 4;
		if (((m_RoiX + m_RoiW) - nMaxWidth) > 0)
			m_RoiX = nMaxWidth - m_RoiW;
		nValue = m_RoiX;
	}
	m_Calirect.x = nValue;
	UpdateData(FALSE);
}


void CCamSetDlg::OnEnKillfocusEditRoiw()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_RoiW = abs(m_RoiW);
	int nValue = 0;
	int nMaxWidth = 0;
	g_dlg->m_control->GetCameraMaxWidth(m_nCamId - 1, nMaxWidth);

	if (m_RoiW == 0 || m_RoiW % 4 == 0)
	{
		if (((m_RoiX + m_RoiW) - nMaxWidth) > 0)
			m_RoiW = nMaxWidth - m_RoiX;
		nValue = m_RoiW;
	}
	else
	{
		int i = m_RoiW / 4;
		m_RoiW = (i + 1) * 4;
		if (((m_RoiX + m_RoiW) - nMaxWidth) > 0)
			m_RoiW = nMaxWidth - m_RoiX;
		nValue = m_RoiW;
	}
	UpdateData(FALSE);
	m_Calirect.width = nValue;
}


void CCamSetDlg::OnEnKillfocusEditRoiy()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_RoiY = abs(m_RoiY);
	int nValue = 0;
	int nMaxHeight = 0;
	g_dlg->m_control->GetCameraMaxHeight(m_nCamId - 1, nMaxHeight);

	if (m_RoiY == 0 || m_RoiY % 4 == 0)
	{
		if (((m_RoiH + m_RoiY) - nMaxHeight) > 0)
			m_RoiY = nMaxHeight - m_RoiH;
		nValue = m_RoiY;
	}
	else
	{
		int i = m_RoiY / 4;
		m_RoiY = (i + 1) * 4;
		if (((m_RoiH + m_RoiY) - nMaxHeight) > 0)
			m_RoiY = nMaxHeight - m_RoiH;
		nValue = m_RoiY;
	}
	m_Calirect.y = m_RoiY;
	UpdateData(FALSE);
}


void CCamSetDlg::OnEnKillfocusEditRoih()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_RoiH = abs(m_RoiH);
	int nValue = 0;
	int nMaxHeight = 0;
	g_dlg->m_control->GetCameraMaxHeight(m_nCamId - 1, nMaxHeight);

	if (m_RoiH == 0 || m_RoiH % 4 == 0)
	{
		if (((m_RoiH + m_RoiY) - nMaxHeight) > 0)
			m_RoiH = nMaxHeight - m_RoiY;
		nValue = m_RoiH;
	}
	else
	{
		int i = m_RoiH / 4;
		m_RoiH = (i + 1) * 4;	
		if (((m_RoiH +m_RoiY) -nMaxHeight) > 0)
			m_RoiH = nMaxHeight-m_RoiY;
		nValue = m_RoiH;
	}
	UpdateData(FALSE);
	m_Calirect.height = nValue;
}


void CCamSetDlg::OnEnKillfocusEditRunroix()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_runRoiX = abs(m_runRoiX);
	int nValue = 0;
	int nMaxWidth = 0;
	g_dlg->m_control->GetCameraMaxWidth(m_nCamId - 1, nMaxWidth);
	if (m_runRoiX > nMaxWidth)
		m_runRoiX = nMaxWidth;
	if (m_runRoiX == 0 || m_runRoiX % 4 == 0)
	{
		if (((m_runRoiX + m_runRoiW) - nMaxWidth) > 0)
			m_runRoiX = nMaxWidth - m_runRoiW;
		nValue = m_runRoiX;
	}
	else
	{
		int i = m_runRoiX / 4;
		m_runRoiX = (i + 1) * 4;
		if (((m_runRoiX + m_runRoiW) - nMaxWidth) > 0)
			m_runRoiX = nMaxWidth - m_runRoiW;
		nValue = m_runRoiX;
	}
	m_Runrect.x = nValue;
	UpdateData(FALSE);
}


void CCamSetDlg::OnEnKillfocusEditRunroiw()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_runRoiW = abs(m_runRoiW);
	int nValue = 0;
	int nMaxWidth = 0;
	g_dlg->m_control->GetCameraMaxWidth(m_nCamId - 1, nMaxWidth);
	if (m_runRoiW > nMaxWidth)
		m_runRoiW = nMaxWidth;
	if (m_runRoiW == 0 || m_runRoiW % 4 == 0)
	{
		if (((m_runRoiX + m_runRoiW) - nMaxWidth) > 0)
			m_runRoiW = nMaxWidth - m_runRoiX;
		nValue = m_runRoiW;
	}
	else
	{
		int i = m_runRoiW / 4;
		m_runRoiW = (i + 1) * 4;
		if (((m_runRoiX + m_runRoiW) - nMaxWidth) > 0)
			m_runRoiW = nMaxWidth - m_runRoiX;
		nValue = m_runRoiW;
	}
	m_Runrect.width = nValue;
	UpdateData(FALSE);
}


void CCamSetDlg::OnEnKillfocusEditRunroiy()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_runRoiY = abs(m_runRoiY);
	int nValue = 0;
	int nMaxHeight = 0;
	g_dlg->m_control->GetCameraMaxWidth(m_nCamId - 1, nMaxHeight);
	if (m_runRoiY > nMaxHeight)
		m_runRoiY = nMaxHeight;
	if (m_runRoiY == 0 || m_runRoiY % 4 == 0)
	{
		if (((m_runRoiY + m_runRoiH) - nMaxHeight) > 0)
			m_runRoiY = nMaxHeight - m_runRoiH;
		nValue = m_runRoiY;
	}
	else
	{
		int i = m_runRoiY / 4;
		m_runRoiY = (i + 1) * 4;
		if (((m_runRoiY + m_runRoiH) - nMaxHeight) > 0)
			m_runRoiY = nMaxHeight - m_runRoiH;
		nValue = m_runRoiY;
	}
	m_Runrect.y = nValue;
	UpdateData(FALSE);
}


void CCamSetDlg::OnEnKillfocusEditRunroih()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	m_runRoiH = abs(m_runRoiH);
	int nValue = 0;
	int nMaxHeight = 0;
	g_dlg->m_control->GetCameraMaxWidth(m_nCamId - 1, nMaxHeight);
	if (m_runRoiH > nMaxHeight)
		m_runRoiH = nMaxHeight;
	if (m_runRoiH == 0 || m_runRoiH % 4 == 0)
	{
		if (((m_runRoiY + m_runRoiH) - nMaxHeight) > 0)
			m_runRoiH = nMaxHeight - m_runRoiY;
		nValue = m_runRoiH;
	}
	else
	{
		int i = m_runRoiH / 4;
		m_runRoiH = (i + 1) * 4;
		if (((m_runRoiY + m_runRoiH) - nMaxHeight) > 0)
			m_runRoiH = nMaxHeight - m_runRoiY;
		nValue = m_runRoiH;
	}
	m_Runrect.height = nValue;
	UpdateData(FALSE);
}
