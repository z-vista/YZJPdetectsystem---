// ClibrationResultDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "YZJPdetectsystem.h"
#include "ClibrationResultDlg.h"
#include "afxdialogex.h"


//Gdi+���
Gdiplus::GdiplusStartupInput  m_gdi_StartupInput; //Gdi+
ULONG_PTR m_ptr_gdiPlusToken; //Gdi+
// CClibrationResultDlg �Ի���

IMPLEMENT_DYNAMIC(CClibrationResultDlg, CDialogEx)

CClibrationResultDlg::CClibrationResultDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CALIBRATION_RESULT_DLG, pParent)
{
	//��ʼ��gdi+
	GdiplusStartup(&m_ptr_gdiPlusToken, &m_gdi_StartupInput, NULL);
	m_zoom = 1.0f;
	m_imgX = 0.0f;
	m_imgY = 0.0f;
}

CClibrationResultDlg::~CClibrationResultDlg()
{//����ʼ��gdi+
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


// CClibrationResultDlg ��Ϣ�������


BOOL CClibrationResultDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}
void CClibrationResultDlg::DrawImageNewW(BYTE* image, int W, int H, int icolor, int  nID)
{//�����ͼ�����ݿ�Ȳ���ҪΪ4�ı����������ڻ����ת��
	int tp_inputImageWidth = W;
	int tp_inputImageHeight = H;
	bool b_ReAlocation = false; //�Ƿ����·����ڴ�
								//λͼͼ�ļ���Ϣ
								//ʹ��gdi+���л�ͼ

								//λͼ�ĸ߶ȱ���ת��Ϊ4�ı���
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
		//ͼ�����ݿ���
		for (int i = 0; i< tp_inputImageHeight; i++)
		{
			memcpy(tp_imageData + i*tp_image_realWidth*colorType, image + i*tp_inputImageWidth*colorType, tp_inputImageWidth*colorType);
		}
	}
	else
	{//����Ҫ���·����ڴ�����
		b_ReAlocation = false;
		tp_image_realWidth = tp_inputImageWidth;
		tp_imageData = image;
	}

	//BitMap����
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
		//λͼ��ɫ
		//λͼ��ɫ
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

	Gdiplus::Bitmap   tp_bitmap_showImage(tp_PbitmapInfo, tp_imageData);//λͼ
	Gdiplus::Status tp_status_return;

	HWND hwndPreview = NULL;
	hwndPreview = GetDlgItem(nID)->m_hWnd;
	//GetDlgItem(nID, &hwndPreview);//��ȡ��ǰ���ھ��
	RECT tp_rect;
	::GetClientRect(hwndPreview, &tp_rect);//�ͻ�������

	Gdiplus::Rect tp_GdiRect;//λͼ����

	REAL tp_zoom = 0.1f;//���ű���
	REAL tp_angle = 0.0;//��ת�Ƕ�
						//bool tp_isRotal = false;//

	tp_GdiRect.X = (INT)m_imgX;//ͼƬ���Ͻ�����ƫ����
	tp_GdiRect.Y = (INT)m_imgY;
	tp_zoom = m_zoom;

	float dcWidth = (float)tp_rect.right - (float)tp_rect.left;
	float dcHeight = (float)tp_rect.bottom - (float)tp_rect.top;
	float ratio = 0;
	float ratiox = dcWidth / (float)W;
	float ratioy = dcHeight / (float)H;
	if (ratiox <= ratioy)
	{
		//����ı���
		ratio = ratiox;
	}
	else
	{
		//���ߵı���
		ratio = ratioy;
	}

#ifdef OUTPUTDEBUGSTRING
	CString outdebug = NULL;
	outdebug.Format(_T("xxx- ͼƬ���Ͻ�����ƫ����(%2f,%2f),�Ŵ�ϵ��%2f,ͼƬ���Ͻ�ʵ��ƫ�����ص�(%2f,%2f) \n"), m_imgX, m_imgY, tp_zoom, m_imgX / ratio, m_imgY / ratio);
	OutputDebugString(outdebug);
#endif
	tp_GdiRect.Width = (INT)(W*ratio);
	tp_GdiRect.Height = (INT)(H*ratio);

	if (NULL == hwndPreview)
	{
		return;
	}

	HDC hdcPreview = ::GetDC(hwndPreview);

	//Ҫע��˳����ƽ�������ź���ת
	Gdiplus::Graphics tp_graphics(hdcPreview);

	//1.ֱ�ӻ���
	//tp_status_return = tp_graphics.DrawImage(&tp_bitmap_showImage,tp_GdiRect);
	//2.˫������Ʒ���1
	/*Gdiplus::Bitmap tp_membitmap((INT)dcWidth, (INT)dcHeight);
	Gdiplus::Graphics tp_memGraphics(&tp_membitmap);//
	tp_memGraphics.Clear(Color(255, 255, 255));
	tp_memGraphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
	tp_memGraphics.ScaleTransform(tp_zoom, tp_zoom);
	tp_memGraphics.DrawImage(&tp_bitmap_showImage, tp_GdiRect,0,0,W,H,Gdiplus::UnitPixel);
	tp_status_return = tp_graphics.DrawImage(&tp_membitmap, 0,0);*/
	//3��˫������Ʒ���2
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
		//MessageBox(_T("tp_imageData ���ͷ�"));
	}

	::ReleaseDC(hwndPreview, hdcPreview);
	//end new yuanbin 
};

void CClibrationResultDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: �ڴ˴������Ϣ����������
					   // ��Ϊ��ͼ��Ϣ���� CDialogEx::OnPaint()
	cv::Mat tp_drawImageMat = m_resultPic.clone();
	flip(tp_drawImageMat, tp_drawImageMat,0);
	DrawImageNewW(tp_drawImageMat.data, tp_drawImageMat.cols, tp_drawImageMat.rows, 24, IDC_STATIC_RESULT_PIC);///��ʾͼ��
}
void CClibrationResultDlg::SetShowPic(cv::Mat mat)
{
	m_resultPic = mat.clone();
}

void CClibrationResultDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
			m_zoom = 1.0f;//��С��С����
						  //m_minwarning = true;
		}
		CRect rect;
		GetWindowRect(rect); //ע������������Ǵ��������������Ļ�ģ���Ϊcpoint pt��������������Ļ��
		CPoint ppt = pt;//������Ļ�ϵ�����
		ppt.x -= rect.left; //����õ��ڶԻ����е�λ��
		ppt.y -= rect.top;

		float oldimagex = 0.0f;
		float oldimagey = 0.0f;

		float newimagex = 0.0f;
		float newimagey = 0.0f;
		if (tp_RectInWindow.PtInRect(pt))
		{
			GetDlgItem(IDC_STATIC_RESULT_PIC)->GetWindowRect(m_Rect);//��ȡ�ؼ�����������Ͻǵľ���
			ScreenToClient(&m_Rect);
		}
		int x = ppt.x - m_Rect.left;//����õ���picture control�е�λ�� 
		int y = ppt.y - m_Rect.top;
		oldimagex = (x / oldzoom); //����ǰ�����ԭͼ�е�λ��
		oldimagey = (y / oldzoom);

		newimagex = (x / m_zoom); //���ź������ԭͼ�е�λ��
		newimagey = (y / m_zoom);

		m_imgX = newimagex - oldimagex + m_imgX; //����ԭͼ���Ͻ�Ӧ�õ�ƫ��
		m_imgY = newimagey - oldimagey + m_imgY;
		cv::Mat tp_drawImageMat = m_resultPic.clone();
		flip(tp_drawImageMat, tp_drawImageMat, 0);
		DrawImageNewW(tp_drawImageMat.data, tp_drawImageMat.cols, tp_drawImageMat.rows, 24, IDC_STATIC_RESULT_PIC);///��ʾͼ��
	}
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}


void CClibrationResultDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_mousepressed = false;
	CDialogEx::OnLButtonUp(nFlags, point);
}


void CClibrationResultDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CPoint ppt = point;
	if (m_mousepressed)
	{
		CRect tp_dlgRect;
		GetWindowRect(tp_dlgRect);
		CPoint pt_currentPoint = point;
		pt_currentPoint.x += tp_dlgRect.left;
		pt_currentPoint.y += tp_dlgRect.top;//ת���ɵ�����Ļ�ϵ�����
		
		CRect tp_tempRectInWindow;

		GetDlgItem(IDC_STATIC_RESULT_PIC)->GetWindowRect(&tp_tempRectInWindow);
		
		if (tp_tempRectInWindow.PtInRect(point))
		{
			REAL deltaX = 0.0f;
			REAL deltaY = 0.0f;

			deltaX = (REAL)ppt.x - m_mouseDown.X; //x�᷽��ƫ��
			deltaY = (REAL)ppt.y - m_mouseDown.Y; //y�᷽��ƫ��

			m_imgX = (m_PtStart.X + (deltaX / m_zoom)); //��ԭʼͼ����X����ƫ�ƣ�������������ԭʼͼƬ�е�λ�ƣ�ԭ���������Ѿ�˵����ȫ�ֵķ���任��Ӱ��ʵ�ʵ�λ��
			m_imgY = (m_PtStart.Y + (deltaY / m_zoom)); //��ԭʼͼ����Y����ƫ��
														//�ػ�
			cv::Mat tp_drawImageMat = m_resultPic.clone();
			flip(tp_drawImageMat, tp_drawImageMat, 0);
			DrawImageNewW(tp_drawImageMat.data, tp_drawImageMat.cols, tp_drawImageMat.rows, 24, IDC_STATIC_RESULT_PIC);///��ʾͼ��
			tp_drawImageMat.release();
			 
		}
	}
	else//���Ϊ��������ƶ�
	{
		//���������ͼƬ�е���������

	}
	CDialogEx::OnMouseMove(nFlags, point);
}
