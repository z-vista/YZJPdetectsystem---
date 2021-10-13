// ClibrationResultDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "YZJPdetectsystem.h"
#include "ClibrationResultDlg.h"
#include "afxdialogex.h"


//Gdi+相关
Gdiplus::GdiplusStartupInput  m_gdi_StartupInput; //Gdi+
ULONG_PTR m_ptr_gdiPlusToken; //Gdi+
// CClibrationResultDlg 对话框

IMPLEMENT_DYNAMIC(CClibrationResultDlg, CDialogEx)

CClibrationResultDlg::CClibrationResultDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CALIBRATION_RESULT_DLG, pParent)
{
	//初始化gdi+
	GdiplusStartup(&m_ptr_gdiPlusToken, &m_gdi_StartupInput, NULL);
	m_zoom = 1.0f;
	m_imgX = 0.0f;
	m_imgY = 0.0f;
}

CClibrationResultDlg::~CClibrationResultDlg()
{//反初始化gdi+
	Gdiplus::GdiplusShutdown(m_ptr_gdiPlusToken);
	m_resultPic.release();
}

void CClibrationResultDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CClibrationResultDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CClibrationResultDlg 消息处理程序


BOOL CClibrationResultDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
void CClibrationResultDlg::DrawImageNewW(BYTE* image, int W, int H, int icolor, int  nID)
{//输入的图像数据宽度不需要为4的倍数，函数内会进行转换
	int tp_inputImageWidth = W;
	int tp_inputImageHeight = H;
	bool b_ReAlocation = false; //是否重新分配内存
								//位图图文件信息
								//使用gdi+进行绘图

								//位图的高度必须转换为4的倍数
	BYTE * tp_imageData = NULL;
	int tp_image_realWidth = 0;
	int colorType = 0;
	if (0 != (tp_inputImageWidth % 4))
	{
		b_ReAlocation = true;
		tp_image_realWidth = (tp_inputImageWidth / 4 + 1) * 4;
		if (icolor == 1)
		{
			tp_imageData = new BYTE[tp_image_realWidth*tp_inputImageHeight];
			colorType = 1;
		}
		else if (icolor == 24)
		{
			tp_imageData = new BYTE[tp_image_realWidth*tp_inputImageHeight * 3];
			colorType = 3;
		}
		else
		{
			return;
		}
		if (NULL == tp_imageData)
		{
			OutputDebugString(_T("xx.No morememory"));
		}
		//图像数据拷贝
		for (int i = 0; i< tp_inputImageHeight; i++)
		{
			memcpy(tp_imageData + i*tp_image_realWidth*colorType, image + i*tp_inputImageWidth*colorType, tp_inputImageWidth*colorType);
		}
	}
	else
	{//不需要重新分配内存的情况
		b_ReAlocation = false;
		tp_image_realWidth = tp_inputImageWidth;
		tp_imageData = image;
	}

	//BitMap建立
	PBITMAPINFO  tp_PbitmapInfo = NULL;
	char * tp_info = NULL;
	if (icolor == 1)
	{
		tp_info = new char[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) + 256 * 4];
		tp_PbitmapInfo = (PBITMAPINFO)tp_info;
		memset(tp_PbitmapInfo, 0, sizeof(BITMAPINFO));
		tp_PbitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		tp_PbitmapInfo->bmiHeader.biWidth = tp_image_realWidth;
		tp_PbitmapInfo->bmiHeader.biHeight = tp_inputImageHeight;
		tp_PbitmapInfo->bmiHeader.biPlanes = 1;
		tp_PbitmapInfo->bmiHeader.biClrUsed = false;
		tp_PbitmapInfo->bmiHeader.biBitCount = 8;
		tp_PbitmapInfo->bmiColors[0].rgbBlue = tp_info[sizeof(BITMAPINFOHEADER)];
		//位图颜色
		//位图颜色
		for (int i = 0; i<256; i++)
		{
			ASSERT(i == BYTE(i));
			tp_PbitmapInfo->bmiColors[i].rgbBlue = BYTE(i);
			tp_PbitmapInfo->bmiColors[i].rgbGreen = BYTE(i);
			tp_PbitmapInfo->bmiColors[i].rgbRed = BYTE(i);
			tp_PbitmapInfo->bmiColors[i].rgbReserved = 0;
		}
	}
	else
	{
		tp_PbitmapInfo = (PBITMAPINFO)new char[sizeof(BITMAPINFOHEADER) + 24 * 4];
		memset(tp_PbitmapInfo, 0, sizeof(BITMAPINFO));
		tp_PbitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		tp_PbitmapInfo->bmiHeader.biWidth = tp_image_realWidth;
		tp_PbitmapInfo->bmiHeader.biHeight = tp_inputImageHeight;
		tp_PbitmapInfo->bmiHeader.biPlanes = 1;
		tp_PbitmapInfo->bmiHeader.biClrUsed = false;
		tp_PbitmapInfo->bmiHeader.biBitCount = 24;
	}

	Gdiplus::Bitmap   tp_bitmap_showImage(tp_PbitmapInfo, tp_imageData);//位图
	Gdiplus::Status tp_status_return;

	HWND hwndPreview = NULL;
	hwndPreview = GetDlgItem(nID)->m_hWnd;
	//GetDlgItem(nID, &hwndPreview);//获取当前窗口句柄
	RECT tp_rect;
	::GetClientRect(hwndPreview, &tp_rect);//客户区矩形

	Gdiplus::Rect tp_GdiRect;//位图矩形

	REAL tp_zoom = 0.1f;//缩放比例
	REAL tp_angle = 0.0;//旋转角度
						//bool tp_isRotal = false;//

	tp_GdiRect.X = (INT)m_imgX;//图片左上角坐标偏移量
	tp_GdiRect.Y = (INT)m_imgY;
	tp_zoom = m_zoom;

	float dcWidth = (float)tp_rect.right - (float)tp_rect.left;
	float dcHeight = (float)tp_rect.bottom - (float)tp_rect.top;
	float ratio = 0;
	float ratiox = dcWidth / (float)W;
	float ratioy = dcHeight / (float)H;
	if (ratiox <= ratioy)
	{
		//按宽的比例
		ratio = ratiox;
	}
	else
	{
		//按高的比例
		ratio = ratioy;
	}

#ifdef OUTPUTDEBUGSTRING
	CString outdebug = NULL;
	outdebug.Format(_T("xxx- 图片左上角坐标偏移量(%2f,%2f),放大系数%2f,图片左上角实际偏移像素点(%2f,%2f) \n"), m_imgX, m_imgY, tp_zoom, m_imgX / ratio, m_imgY / ratio);
	OutputDebugString(outdebug);
#endif
	tp_GdiRect.Width = (INT)(W*ratio);
	tp_GdiRect.Height = (INT)(H*ratio);

	if (NULL == hwndPreview)
	{
		return;
	}

	HDC hdcPreview = ::GetDC(hwndPreview);

	//要注意顺序，先平移在缩放后旋转
	Gdiplus::Graphics tp_graphics(hdcPreview);

	//1.直接绘制
	//tp_status_return = tp_graphics.DrawImage(&tp_bitmap_showImage,tp_GdiRect);
	//2.双缓冲绘制方案1
	/*Gdiplus::Bitmap tp_membitmap((INT)dcWidth, (INT)dcHeight);
	Gdiplus::Graphics tp_memGraphics(&tp_membitmap);//
	tp_memGraphics.Clear(Color(255, 255, 255));
	tp_memGraphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
	tp_memGraphics.ScaleTransform(tp_zoom, tp_zoom);
	tp_memGraphics.DrawImage(&tp_bitmap_showImage, tp_GdiRect,0,0,W,H,Gdiplus::UnitPixel);
	tp_status_return = tp_graphics.DrawImage(&tp_membitmap, 0,0);*/
	//3、双缓冲绘制方案2
	Gdiplus::Bitmap tp_membitmap((INT)dcWidth, (INT)dcHeight);
	Gdiplus::Graphics tp_memGraphics(&tp_membitmap);
	tp_memGraphics.Clear(Color(255, 255, 255));
	tp_memGraphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
	tp_memGraphics.ScaleTransform(tp_zoom, tp_zoom);
	tp_memGraphics.DrawImage(&tp_bitmap_showImage, tp_GdiRect, 0, 0, W, H, Gdiplus::UnitPixel);

	CachedBitmap tp_cachedBmp(&tp_membitmap, &tp_graphics);
	tp_status_return = tp_graphics.DrawCachedBitmap(&tp_cachedBmp, 0, 0);


	if (Ok != tp_status_return)
	{
		OutputDebugString(_T("xxx.No DrawImage faild"));
	}

	if (NULL != tp_PbitmapInfo)
	{
		delete[](char *)tp_PbitmapInfo;
	}

	if (NULL != tp_imageData)
	{
		if (b_ReAlocation)
		{
			delete[] tp_imageData;
		}
	}
	else
	{
		//CString strMsg;
		//strMsg.LoadString(IDS_IMAGEDATARELEASE);

		//MessageBox(strMsg, _T("Error"), MB_SYSTEMMODAL | MB_OK | MB_ICONSTOP);
		//MessageBox(_T("tp_imageData 被释放"));
	}

	::ReleaseDC(hwndPreview, hdcPreview);
	//end new yuanbin 
};

void CClibrationResultDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialogEx::OnPaint()
	cv::Mat tp_drawImageMat = m_resultPic.clone();
	flip(tp_drawImageMat, tp_drawImageMat,0);
	DrawImageNewW(tp_drawImageMat.data, tp_drawImageMat.cols, tp_drawImageMat.rows, 24, IDC_STATIC_RESULT_PIC);///显示图像
}
void CClibrationResultDlg::SetShowPic(cv::Mat mat)
{
	m_resultPic = mat.clone();
}

void CClibrationResultDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nFlags & MK_LBUTTON)
	{
		if (!m_mousepressed)
		{
			GetDlgItem(IDC_STATIC_RESULT_PIC)->SetFocus();
			m_mousepressed = true;
			m_mouseDown.X = (REAL)point.x;
			m_mouseDown.Y = (REAL)point.y;
			m_PtStart.X = m_imgX;
			m_PtStart.Y = m_imgY;

		}

	}
	CDialogEx::OnLButtonDown(nFlags, point);
}


BOOL CClibrationResultDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect tp_RectInWindow;
	GetDlgItem(IDC_STATIC_RESULT_PIC)->GetWindowRect(&tp_RectInWindow);
	if (tp_RectInWindow.PtInRect(pt))
	{
		REAL oldzoom = 0;
		oldzoom = m_zoom;
		if (zDelta == 120)
		{
			m_zoom =m_zoom+ 0.6f;
		}if (zDelta == -120)
		{
			m_zoom = m_zoom - 0.6f;
		}
		if (m_zoom < 1.0f)
		{
			m_zoom = 1.0f;//最小缩小比例
						  //m_minwarning = true;
		}
		CRect rect;
		GetWindowRect(rect); //注意这里的区域是窗口相对于整个屏幕的，因为cpoint pt这个点是相对于屏幕的
		CPoint ppt = pt;//点在屏幕上的坐标
		ppt.x -= rect.left; //计算该点在对话框中的位置
		ppt.y -= rect.top;

		float oldimagex = 0.0f;
		float oldimagey = 0.0f;

		float newimagex = 0.0f;
		float newimagey = 0.0f;
		if (tp_RectInWindow.PtInRect(pt))
		{
			GetDlgItem(IDC_STATIC_RESULT_PIC)->GetWindowRect(m_Rect);//获取控件相对自身左上角的矩形
			ScreenToClient(&m_Rect);
		}
		int x = ppt.x - m_Rect.left;//计算该点在picture control中的位置 
		int y = ppt.y - m_Rect.top;
		oldimagex = (x / oldzoom); //缩放前鼠标在原图中的位置
		oldimagey = (y / oldzoom);

		newimagex = (x / m_zoom); //缩放后鼠标在原图中的位置
		newimagey = (y / m_zoom);

		m_imgX = newimagex - oldimagex + m_imgX; //计算原图左上角应该的偏移
		m_imgY = newimagey - oldimagey + m_imgY;
		cv::Mat tp_drawImageMat = m_resultPic.clone();
		flip(tp_drawImageMat, tp_drawImageMat, 0);
		DrawImageNewW(tp_drawImageMat.data, tp_drawImageMat.cols, tp_drawImageMat.rows, 24, IDC_STATIC_RESULT_PIC);///显示图像
	}
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}


void CClibrationResultDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_mousepressed = false;
	CDialogEx::OnLButtonUp(nFlags, point);
}


void CClibrationResultDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint ppt = point;
	if (m_mousepressed)
	{
		CRect tp_dlgRect;
		GetWindowRect(tp_dlgRect);
		CPoint pt_currentPoint = point;
		pt_currentPoint.x += tp_dlgRect.left;
		pt_currentPoint.y += tp_dlgRect.top;//转换成点在屏幕上的坐标
		
		CRect tp_tempRectInWindow;

		GetDlgItem(IDC_STATIC_RESULT_PIC)->GetWindowRect(&tp_tempRectInWindow);
		
		if (tp_tempRectInWindow.PtInRect(point))
		{
			REAL deltaX = 0.0f;
			REAL deltaY = 0.0f;

			deltaX = (REAL)ppt.x - m_mouseDown.X; //x轴方向偏移
			deltaY = (REAL)ppt.y - m_mouseDown.Y; //y轴方向偏移

			m_imgX = (m_PtStart.X + (deltaX / m_zoom)); //在原始图像中X坐标偏移，这里计算的是在原始图片中的位移，原因在上面已经说明，全局的仿射变换会影响实际的位移
			m_imgY = (m_PtStart.Y + (deltaY / m_zoom)); //在原始图像中Y坐标偏移
														//重绘
			cv::Mat tp_drawImageMat = m_resultPic.clone();
			flip(tp_drawImageMat, tp_drawImageMat, 0);
			DrawImageNewW(tp_drawImageMat.data, tp_drawImageMat.cols, tp_drawImageMat.rows, 24, IDC_STATIC_RESULT_PIC);///显示图像
			tp_drawImageMat.release();
			 
		}
	}
	else//鼠标为按下左键移动
	{
		//计算鼠标在图片中的像素坐标

	}
	CDialogEx::OnMouseMove(nFlags, point);
}
