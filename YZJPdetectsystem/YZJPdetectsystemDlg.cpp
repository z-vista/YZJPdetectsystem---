
// YZJPdetectsystemDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "YZJPdetectsystem.h"
#include "YZJPdetectsystemDlg.h"
#include "afxdialogex.h"

#include "CamSetDlg.h"

#include <numeric>//计算均值头文件
#include "findBound.h"//算法头文件

#include "DeleteFile.h"


//dog
#include "dog_api.h"
#include "dog_vcode.h"
#include "encryption_xvision.h"

#include "createDir.h"

#include "LoggerEx.h"
#include "Logger.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define dog_onoff 0

//Logger mlogger = Logger("./log/", LogLevelAll);
NetDataLog mlogger= NetDataLog("./log/");
Logger mlogger_sendData = Logger("./log/", LogLevelAll);
YZJPdetectsystemDlg* g_dlg = NULL;
shared_ptr<CGxCamManager> CGxCamManager::m_Instance(new CGxCamManager(), CGxCamManager::Destory);

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// YZJPdetectsystemDlg 对话框
double average(vector<double> list)
{
	double mean = 0;
	if (!list.empty())
	{
		double sum = std::accumulate(std::begin(list), std::end(list), 0.0);
		if (list.size() > 0)
			mean = sum / list.size(); //均值
		//std::cout << "当前均值为：" << mean << "\n";
	}

	return mean;
}
//标准差
double StandardDeviation(vector<double> list)
{

	double pj = 0;        //平均数
	double pfcount = -1;//平方和
	if (!list.empty())
	{
		pj = average(list);
		for (int i = 0; i < list.size(); i++) {
			double cha = abs(list.at(i) - pj);
			pfcount += pow((double)cha, 2);        //离差平方和
		}
		if (list.size() > 0)
		{
			pfcount = pfcount / list.size();            //离差平方和平均
		}
		else
			pfcount = 0;
	}
	return pfcount;
}


YZJPdetectsystemDlg::YZJPdetectsystemDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_YZJPDETECTSYSTEM_DIALOG, pParent)
{

	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);//IDR_MAINFRAME
	m_nTabItemCount1 = 0;
	m_nTabItemCount2 = 0;
	m_nTabItemCount3 = 0;
	m_nTabItemCount4 = 0;
	m_CalibTable1.clear();
	m_CalibTable2.clear();
	m_CalibTable3.clear();
	m_CalibTable4.clear();


	ready_wait = true; // 等待全局标志位,初始不等待
}

void YZJPdetectsystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MESSAGE, m_xclist);
}

BEGIN_MESSAGE_MAP(YZJPdetectsystemDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_CLOSE()

	ON_BN_CLICKED(IDC_CAM1_BUTTON, &YZJPdetectsystemDlg::OnBnClickedCam1Button)
	ON_BN_CLICKED(IDC_CAM2_BUTTON, &YZJPdetectsystemDlg::OnBnClickedCam2Button)
	ON_BN_CLICKED(IDC_CAM3_BUTTON, &YZJPdetectsystemDlg::OnBnClickedCam3Button)
	ON_BN_CLICKED(IDC_CAM4_BUTTON, &YZJPdetectsystemDlg::OnBnClickedCam4Button)
	ON_BN_CLICKED(IDC_BUTTON_SYS_SET, &YZJPdetectsystemDlg::OnBnClickedButtonSysSet)
END_MESSAGE_MAP()


// YZJPdetectsystemDlg 消息处理程序
long   __stdcall   callback(_EXCEPTION_POINTERS* excp)
{
	Sleep(500);
	CHAR FileName[MAX_PATH] = { 0 };
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	char chInfo[512];//字符显示
	SYSTEMTIME st;
	GetLocalTime(&st);
	sprintf(chInfo, "%4d%02d%02d-%02d:%02d:%02d:崩溃重启!\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	mlogger.TraceKeyInfo(chInfo);

	g_dlg->OnClose();
	GetModuleFileNameA(NULL, FileName, MAX_PATH);

	WinExec(FileName, SW_SHOW);
	return EXCEPTION_EXECUTE_HANDLER;
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return NULL;
}

BOOL PreventSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
	if (hKernel32 == NULL) return FALSE;
	void* pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if (pOrgEntry == NULL) return FALSE;
	unsigned char newJump[100];
	DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
	dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
	void* pNewFunc = &MyDummySetUnhandledExceptionFilter;
	DWORD dwNewEntryAddr = (DWORD)pNewFunc;
	DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

	newJump[0] = 0xE9;  // JMP absolute
	memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
		pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
	return bRet;
}

// EXCEPTION_EXECUTE_HANDLER equ 1 表示我已经处理了异常, 可以优雅地结束了
// EXCEPTION_CONTINUE_SEARCH equ 0 表示我不处理, 其他人来吧, 于是windows调用默认的处理程序显示一个错误框, 并结束
// EXCEPTION_CONTINUE_EXECUTION equ - 1 表示错误已经被修复, 请从异常发生处继续执行
LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* lpTopLevelExceptionFilter)
{
	mlogger_sendData.TraceKeyInfo("发生异常，并且错误被修复,从异常处继续执行\n");
	FatalAppExit(0, _T("OK, You get an Unhandled Exception !"));
	return EXCEPTION_CONTINUE_EXECUTION;
}

BOOL YZJPdetectsystemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	g_dlg = this;
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
	BOOL bRet = PreventSetUnhandledExceptionFilter();
	_tprintf(_T("Prevented: %d"), bRet);

	//SetUnhandledExceptionFilter(callback);
	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	
	
	m_control = CGxCamManager::GetInstance();
	// TODO: 在此添加额外的初始化代码
	mlogger.SetLogLevel(LogLevelAll);
	mlogger.setMaxFileSize(30);
	mlogger.setFileCount(100);
	mlogger.TraceKeyInfo("############当前系统版本号V1.0.0.1###############");
	DeleteAllFile(".\\log\\",5);//开机删除日志文件
	DeleteAllFile(".\\imageYZJP\\", 5);//开机删除图片文件
	//设置窗口按桌面工作区的大小,调用一次onsize()
	RECT rt;
	SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rt, 0);//获取桌面工作区的大小
	MoveWindow(&rt);


	GetClientRect(&m_rect);  //获取对话框的大小


	//dog
#if dog_onoff
	BOOL bValid = TRUE;
	if (!CheckCopyright())
		bValid = FALSE;

	if (!bValid)
	{
		//AfxMessageBox(_T("非法软件拷贝！"));
		AfxMessageBox(_T("请插入加密狗后再启动程序"));
		mlogger.TraceKeyInfo("……请插入加密狗后再启动程序……");
		exit(0);
	}

#endif
	//配置表路径
	
	lpPath = "./pHdcConfig.ini";
	//是否放入初始化中
	int retval = PathFileExists(lpPath);//检查ini是否存在
	if (0 == retval) //不存在，创建ini;
	{
		mlogger.TraceError("错误：系统初始化失败！\n可能原因：缺少系统配置文件！");
		MessageBox(_T("错误：系统初始化失败！\n可能原因：缺少系统配置文件！"), _T("提示"), MB_ICONHAND);
		exit(0);
	}

	GetDlgItem(IDC_EDIT_SEND_COUNT)->SetWindowText("0");
	GetDlgItem(IDC_EDIT_PAPER1)->SetWindowText("0");
	GetDlgItem(IDC_EDIT_PAPER2)->SetWindowText("0");
	GetDlgItem(IDC_EDIT_PAPER3)->SetWindowText("0");
	GetDlgItem(IDC_EDIT_PAPER4)->SetWindowText("0");
	GetDlgItem(IDC_EDIT_POSITION1)->SetWindowText("0");
	GetDlgItem(IDC_EDIT_POSITION2)->SetWindowText("0");
	GetDlgItem(IDC_EDIT_POSITION3)->SetWindowText("0");
	GetDlgItem(IDC_EDIT_POSITION4)->SetWindowText("0");

	////创建文件夹名

	CTime t = CTime::GetCurrentTime();
	m_s = t.Format(".\\imageYZJP\\%Y-%m-%d\\");
	int retCreate = createDirectoryEx(m_s);
	mlogger.TraceKeyInfo("xxxx--创建图片保存文件夹%s=%d", m_s.c_str(), retCreate);
	//CString path = m_s.c_str();
	//if (!PathIsDirectory(path))
	//{
//		BOOL ret=::CreateDirectory(path, 0);
//		mlogger.TraceKeyInfo("xxxx--创建图片保存文件夹%s=%d", path,ret);

//	}
	////读取相机标定参数
	for(int i = 0;i<4;i++)
		LoadRatioTable(i);
	

	//打开相机
	bool ret = false;

	//获取相机SN
	char chAttribute[20];
	memset(&chAttribute, 0, sizeof(chAttribute));
	GetPrivateProfileString("Set_CameraInfo", "SN_Camera1", "LT0200120615", chAttribute, 20, lpPath);
	m_control->SetCameraSN(chAttribute, 0);
	memset(&chAttribute, 0, sizeof(chAttribute));
	GetPrivateProfileString("Set_CameraInfo", "SN_Camera2", "LT0200120615", chAttribute, 20, lpPath);
	m_control->SetCameraSN(chAttribute, 1);
	memset(&chAttribute, 0, sizeof(chAttribute));
	GetPrivateProfileString("Set_CameraInfo", "SN_Camera3", "LT0200120615", chAttribute, 20, lpPath);
	m_control->SetCameraSN(chAttribute, 2);
	memset(&chAttribute, 0, sizeof(chAttribute));
	GetPrivateProfileString("Set_CameraInfo", "SN_Camera4", "LT0200120615", chAttribute, 20, lpPath);
	m_control->SetCameraSN(chAttribute, 3);
	ret=m_control->InitCamera();
	if (true == ret)
	{
		size_t nCamNum = m_control->GetCamNum();
		if (nCamNum < MaxCameraNumber)
		{
			MessageBox(_T("错误：相机初始化失败！\n可能原因：相机数量小于4个！"), _T("提示"), MB_ICONHAND);
			return TRUE;
		}
		for (size_t i = 0; i < nCamNum; i++)
		{
			////读取相机标定参数
			//LoadRatioTable(i);

			if (!m_control->OpenCamera(i))
			{
				MessageBox(_T("错误：相机打开失败！\n可能原因：相机配置文件错误或相机被占用！"), _T("提示"), MB_ICONHAND);
				return TRUE;
			}
			if (!m_control->StartGrabCamera(i))
			{
				MessageBox(_T("错误：相机打开失败！\n可能原因：相机采集流错误！"), _T("提示"), MB_ICONHAND);
				return TRUE;
			}
		}
	}

	//打开光源
	InitLightParameters();

	InitializeCameraLightDevice(0);///< 初始化光源设备

	InitializeCameraLightDevice(1);
	if (m_bFSTLight)
	{
		for (int num = 0; num < 2; num++)
			m_LightCom[num].SetLuminance(m_hCom[num], 0, 0);
	}
	else
	{
		for (int num = 0; num < 2; num++)
			m_LightCom[num].SetLuminanceEx(m_hCom[num], 0, 0);
	}

	OnBnClickedBtnOpenlight();
	//m_camSetDlg.m_hResDll = m_hResDll;
	m_camSetDlg.Create(IDD_CAMSET_DIALOG, this);
	m_camSetDlg.SetCamId(1);
	int CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_x", 1, lpPath);//算法开关
	int CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_y", 1, lpPath);//算法开关
	int CaliImageW = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_w", 100, lpPath);//算法开关
	int CaliImageH = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_h", 64, lpPath);//算法开关
	int RunImageX = GetPrivateProfileInt("Set ImageFormat", "RunImage1_x", 1, lpPath);//标定Rect
	int RunImageY = GetPrivateProfileInt("Set ImageFormat", "RunImage1_y", 1, lpPath);//标定Rect
	int RunImageW = GetPrivateProfileInt("Set ImageFormat", "RunImage1_w", 100, lpPath);//标定Rect
	int RunImageH = GetPrivateProfileInt("Set ImageFormat", "RunImage1_h", 64, lpPath);//标定Rect
	Rect rectROI = Rect(CaliImageX, CaliImageY, CaliImageW, CaliImageH);//标定感兴趣区域ROI
	Rect rectROI2 = Rect(RunImageX, RunImageY, RunImageW, RunImageH);//运行感兴趣区域ROI
	m_camSetDlg.SetCamRect(rectROI, rectROI2,5496,3276);

	m_sysSetDlg.Create(IDD_DIALOG_SYS_SET, this);
	
	OnBnClickedButstartsystem();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void YZJPdetectsystemDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void YZJPdetectsystemDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

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
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR YZJPdetectsystemDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//控件适应屏幕变化模块
#if 1
//nID为控件ID，x,y分别为对话框的当前长和宽
void YZJPdetectsystemDlg::ChangeWidgetSize(UINT nID, int x, int y)
{
	CWnd *pWnd;
	pWnd = GetDlgItem(nID);
	if (pWnd != NULL)     //判断是否为空，因为在窗口创建的时候也会调用OnSize函数，但是此时各个控件还没有创建，Pwnd为空
	{
		CRect rec;
		pWnd->GetWindowRect(&rec);  //获取控件变化前的大小
		ScreenToClient(&rec);   //将控件大小装换位在对话框中的区域坐标
		rec.left = rec.left*x / m_rect.Width();  //按照比例调整空间的新位置
		rec.top = rec.top*y / m_rect.Height();
		rec.bottom = rec.bottom*y / m_rect.Height();
		rec.right = rec.right*x / m_rect.Width();

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
//缩放控件，消息函数
void YZJPdetectsystemDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (nType != SIZE_MINIMIZED)  //判断窗口是不是最小化了，因为窗口最小化之后 ，窗口的长和宽会变成0，当前一次变化的时就会出现除以0的错误操作
	{

		ChangeWidgetSize(IDC_PIC_LEFT, cx, cy);
		ChangeWidgetSize(IDC_PIC_RIGHT, cx, cy);
		ChangeWidgetSize(IDC_PIC_LEFT2, cx, cy);
		ChangeWidgetSize(IDC_PIC_RIGHT2, cx, cy);
		ChangeWidgetSize(IDC_STATIC_CONFIG, cx, cy);
		ChangeWidgetSize(IDC_CAM1_BUTTON, cx, cy);
		ChangeWidgetSize(IDC_CAM2_BUTTON, cx, cy);
		ChangeWidgetSize(IDC_CAM3_BUTTON, cx, cy);
		ChangeWidgetSize(IDC_CAM4_BUTTON, cx, cy);
		ChangeWidgetSize(IDC_STATIC_TOPLIMIT, cx, cy);
		ChangeWidgetSize(IDC_EDIT_TOPLIMIT, cx, cy);
		ChangeWidgetSize(IDC_STATIC_LOWERLIMIT, cx, cy);
		ChangeWidgetSize(IDC_EDIT_LOWERLIMIT, cx, cy);
		ChangeWidgetSize(IDC_LIST_MESSAGE, cx, cy);
	

		ChangeWidgetSize(IDC_STATIC_2PLC, cx, cy);
		ChangeWidgetSize(IDC_STATIC_SEND_COUNT, cx, cy);
		ChangeWidgetSize(IDC_STATIC_CAM1, cx, cy);
		ChangeWidgetSize(IDC_STATIC_CAM2, cx, cy);
		ChangeWidgetSize(IDC_STATIC_CAM3, cx, cy);
		ChangeWidgetSize(IDC_STATIC_CAM4, cx, cy);
		ChangeWidgetSize(IDC_EDIT_SEND_COUNT, cx, cy);
		ChangeWidgetSize(IDC_EDIT_POSITION1, cx, cy);
		ChangeWidgetSize(IDC_EDIT_POSITION2, cx, cy);
		ChangeWidgetSize(IDC_EDIT_POSITION3, cx, cy);
		ChangeWidgetSize(IDC_EDIT_POSITION4, cx, cy);
		ChangeWidgetSize(IDC_EDIT_PAPER1, cx, cy);
		ChangeWidgetSize(IDC_EDIT_PAPER2, cx, cy);
		ChangeWidgetSize(IDC_EDIT_PAPER3, cx, cy);
		ChangeWidgetSize(IDC_EDIT_PAPER4, cx, cy);
		ChangeWidgetSize(IDC_STATIC_WZFK, cx, cy);
		ChangeWidgetSize(IDC_STATIC_XZFK, cx, cy);
		ChangeWidgetSize(IDC_EDIT_WZFK, cx, cy);
		ChangeWidgetSize(IDC_EDIT_XZFK, cx, cy);

		GetClientRect(&m_rect);   //最后要更新对话框的大小，当做下一次变化的旧坐标；

		//Invalidate();//更新窗口0426
	}
}

#endif


//dog
#if 1
int CheckKey_verify(dog_handle_t  hXVisionDog)
{
	unsigned char *bufData = NULL;
	int nStatus = 0;
	int i, j;
	i = j = 0;

	srand((unsigned)time(NULL));

	bufData = (unsigned char *)malloc(ENCRYPTDATA_LEN1);
	memset(bufData, 0, ENCRYPTDATA_LEN1);

	//Generate a random index number
	i = rand() % GENERATE_COUNT1;
	memcpy(bufData, encryptionArray1[i][0], ENCRYPTDATA_LEN1);

	/*
	* dog_encrypt
	*   encrypts a block of data using SuperDog
	*   (minimum buffer size is 16 bytes)
	*/
	nStatus = dog_encrypt(hXVisionDog, bufData, ENCRYPTDATA_LEN1);
	if (DOG_STATUS_OK != nStatus)
	{
		free(bufData);
		dog_logout(hXVisionDog);
		return -1;
	}

	//Check the encrypted data is right or wrong
	for (j = 0; j < ENCRYPTDATA_LEN1; ++j)
	{
		if (bufData[j] ^ encryptionArray1[i][1][j])
		{
			//printf( "Encrypted data is wrong.\n" );      
			free(bufData);
			dog_logout(hXVisionDog);
			//return -1; 
			return -1;
		}
	}

	free(bufData);
	dog_logout(hXVisionDog);

	//printf( "Check Dog using encryption array success.\n" );  

	return DOG_STATUS_OK;
}

BOOL YZJPdetectsystemDlg::CheckCopyright()
{
	dog_handle_t  hXVisionDog;
	TCHAR tmp[128];
	dog_status_t status = dog_login(GENARR_FEATUREID1,
		vendor_code,
		&hXVisionDog);
	if (DOG_STATUS_OK != status)
	{
		switch (status)
		{
		case DOG_INV_VCODE:
			_stprintf_s(tmp, _T("Invalid vendor code.\n"));
			break;

		case DOG_UNKNOWN_VCODE:
			_stprintf_s(tmp, _T("Vendor Code not recognized by API.\n"));
			break;
		case DOG_NOT_FOUND:
			_stprintf_s(tmp, _T("未检测到加密狗，请插入加密狗后再启动程序\n"));
			break;

		default:
			_stprintf_s(tmp, _T("Login to feature: %d failed with status %d\n"), GENARR_FEATUREID1, status);
			break;

		}
		//AfxMessageBox(tmp);
		return FALSE;
	}
	else
	{
		if (CheckKey_verify(hXVisionDog) != DOG_STATUS_OK)
		{
			//AfxMessageBox(_T("Invalid software copy！"));
			return FALSE;
		}
	}

	return TRUE;
}
#endif


#if 0
void YZJPdetectsystemDlg::ShowImageToPic(Mat &img, UINT ID)
{

	if (img.empty())
	{
		return;
	}
	cv::Mat imgTmp;
	CRect rect;

	GetDlgItem(ID)->GetClientRect(&rect);  // 获取控件大小

	cv::resize(img, imgTmp, cv::Size(rect.Width(), rect.Height()), 0, 0, CV_INTER_AREA);// 缩放Mat并备份


																						// 转一下格式,这段可以放外面,

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

	int pixelBytes = imgTmp.channels()*(imgTmp.depth() + 1); // 计算一个像素多少个字节

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

	CDC *pDC = GetDlgItem(ID)->GetDC();

	::StretchDIBits(

		pDC->GetSafeHdc(),

		0, 0, rect.Width(), rect.Height(),

		0, 0, rect.Width(), rect.Height(),

		imgTmp.data,

		&bitInfo,

		DIB_RGB_COLORS,

		SRCCOPY

	);

	ReleaseDC(pDC);
}
#else 
void YZJPdetectsystemDlg::ShowImageToPic(Mat &img, UINT ID)
{


	if (img.empty())
	{
		return;
	}
	cv::Mat imgTmp;
	CRect rect;

	GetDlgItem(ID)->GetClientRect(&rect);  // 获取控件大小

	//RedrawWindow(CRect(0, 0, rect.Width(), rect.Height()));//重绘指定区域
	//Sleep(10);

	Size mysize = cv::Size(rect.Width(), rect.Height());

	float xx = (float)img.cols / rect.Width();
	float yy = (float)img.rows / rect.Height();

	if (xx < 1 && yy < 1)
	{
		img.copyTo(imgTmp);
	}
	else
	{
		if (yy < xx)
		{
			//	mysize = cv::Size(rect.Width(), (int)rect.Height() / xx);
			if(img.rows>rect.Height())mysize = cv::Size(rect.Width(), (int)img.rows / xx);
			else mysize = cv::Size(rect.Width(), rect.Height()/4/*(int)img.rows / xx*/);
		}
		else if (yy > xx)
		{
			//mysize = cv::Size(rect.Width() / yy, (int)rect.Height());
			mysize = cv::Size((int)img.cols / yy, rect.Height());
		}

		cv::resize(img, imgTmp, mysize, 0, 0, CV_INTER_AREA);// 缩放Mat并备份
	}

	//flip(imgTmp, imgTmp,0);



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

	int pixelBytes = imgTmp.channels()*(imgTmp.depth() + 1); // 计算一个像素多少个字节

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

	CDC *pDC = GetDlgItem(ID)->GetDC();

	//显示到左下角
	//::StretchDIBits(
	//	pDC->GetSafeHdc(),
	//	0, 0, rect.Width(), rect.Height(),
	//	0, 0, rect.Width(), rect.Height(),
	//	imgTmp.data,
	//	&bitInfo,
	//	DIB_RGB_COLORS,
	//	SRCCOPY
	//	);

	if (imgTmp.cols >= rect.Width() && imgTmp.rows >= rect.Height())
	{
		//显示到左上角
		::StretchDIBits(
			pDC->GetSafeHdc(),
			0, imgTmp.rows - rect.Height(), rect.Width(), rect.Height(),
			0, 0, rect.Width(), rect.Height(),
			imgTmp.data,
			&bitInfo,
			DIB_RGB_COLORS,
			SRCCOPY
		);
	}
	else
	{
		//显示到中心
		::StretchDIBits(
			pDC->GetSafeHdc(),
			-imgTmp.cols *0.5 + rect.Width() *0.5, (imgTmp.rows *0.5 - rect.Height() *0.5), rect.Width(), rect.Height(),
			0, 0, rect.Width(), rect.Height(),
			imgTmp.data,
			&bitInfo,
			DIB_RGB_COLORS,
			SRCCOPY
		);
	}

	ReleaseDC(pDC);
}
#endif

//获取当前系统时间
CString YZJPdetectsystemDlg::myGetCurrentTime()
{
	//获取系统当前时间
	CTime curTime = CTime::GetCurrentTime(); ///构造CTime对象
	CString strTime;  // 用于将CTime对象格式化为字符串 
					  // 将当前时间curTime对象格式化为字符串   
	strTime = curTime.Format(_T("%Y-%m-%d %H:%M:%S	"));


	return strTime;
}

//光源//----------------------------------------------------------------------------------------------------
///@int num; >> 光源设备组代号，0表示第一组设备；1表示第二组设备;光源通道1和2分别用数值 iValue
///@初始化相机光源设备
int YZJPdetectsystemDlg::InitializeCameraLightDevice(int num)
{
	//about light controller
	//COM 口  灯
	CString strKeyName = NULL;
	strKeyName.Format("Communication%d", num);
	int iValue;
	iValue = GetPrivateProfileInt("Set_Light", strKeyName, num, lpPath);

	BOOL bs;
	bs = m_LightCom[num].InitLightCom(&m_hCom[num], iValue);
	if (bs) {
		bs = m_LightCom[num].SetComParm(m_hCom[num]);
		m_struct_ParameterOfGxCameraLight.b_CameraLight_Online = TRUE;
	}
	else {
		//CString tp_showMessage;
		//tp_showMessage.LoadStringA(IDS_STRING109);
		//MessageBox(tp_showMessage);
		m_struct_ParameterOfGxCameraLight.b_CameraLight_Online = FALSE;
	}

	return 0;
}
///@光源配置文件参数读取
void YZJPdetectsystemDlg::InitLightParameters()
{
	unsigned long iValue;
	//iValue = GetPrivateProfileInt("Parameters", "Communication0", 1, m_strSettingFile);
	//m_iLightCom[0] = iValue;
	//iValue = GetPrivateProfileInt("Parameters", "Communication1", 2, m_strSettingFile);
	//m_iLightCom[1] = iValue;
	iValue = GetPrivateProfileInt("Set_Light", "FSTLight", 0, lpPath);//光源供应商
	m_bFSTLight = (iValue == 1 ? true : false);
	iValue = GetPrivateProfileInt("Set_Light", "LightValue1", 0, lpPath);
	m_iLightValue[0] = iValue;

	iValue = GetPrivateProfileInt("Set_Light", "LightValue2", 0, lpPath);
	m_iLightValue[1] = iValue;

	iValue = GetPrivateProfileInt("Set_Light", "LightValue3", 0, lpPath);
	m_iLightValue[2] = iValue;

	iValue = GetPrivateProfileInt("Set_Light", "LightValue4", 0, lpPath);
	m_iLightValue[3] = iValue;

}
///@光源配置文件参数保存
void YZJPdetectsystemDlg::WriterLightParameters()
{
	char tp_strToWrite[MAX_PATH];
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_iLightValue[0]);
	WritePrivateProfileString("Set_Light", "LightValue1", tp_strToWrite, lpPath);
	sprintf(tp_strToWrite, "%d", m_iLightValue[1]);
	WritePrivateProfileString("Set_Light", "LightValue2", tp_strToWrite, lpPath);
	sprintf(tp_strToWrite, "%d", m_iLightValue[2]);
	WritePrivateProfileString("Set_Light", "LightValue3", tp_strToWrite, lpPath);
	sprintf(tp_strToWrite, "%d", m_iLightValue[3]);
	WritePrivateProfileString("Set_Light", "LightValue4", tp_strToWrite, lpPath);
}
///@打开光源
void YZJPdetectsystemDlg::OnBnClickedBtnOpenlight()
{
	// TODO: 在此添加控件通知处理程序代码
	//for (int num = 0; num < 2; num++)
	//{
	//	m_LightCom[num].SetLuminance(m_hCom[num], m_iLightValue[num],m_iLightValue[num + 1]);
	//}
	if (m_bFSTLight)
	{
		m_LightCom[0].SetLuminance(m_hCom[0], m_iLightValue[0 + 1], m_iLightValue[0]);
		m_LightCom[1].SetLuminance(m_hCom[1], m_iLightValue[2 + 1], m_iLightValue[2]);
	}
	else
	{
		m_LightCom[0].SetLuminanceEx(m_hCom[0], m_iLightValue[0], m_iLightValue[1]);
		m_LightCom[1].SetLuminanceEx(m_hCom[1], m_iLightValue[2], m_iLightValue[3]);
	}
}
///@关闭光源
void YZJPdetectsystemDlg::OnBnClickedBtnCloselight()
{
	// TODO: 在此添加控件通知处理程序代码

	if (m_bFSTLight)
	{
		for (int num = 0; num < 2; num++)
			m_LightCom[num].SetLuminance(m_hCom[num], 0, 0);
	}
	else
	{
		for (int num = 0; num < 2; num++)
			m_LightCom[num].SetLuminanceEx(m_hCom[num], 0, 0);
	}




}
//光源end//------------------------------------------------------------------------------------------------
float YZJPdetectsystemDlg::FindRatio(int camId, float pos)
{//获取像素与实际坐标相绑定的值
	int count = m_nTabItemCount1;
	vector<cv::Point> pTab = m_CalibTable1;
	if (camId == 1)
	{
		pTab = m_CalibTable1;
		count = m_nTabItemCount1;
	}
	else if (camId == 2)
	{
		pTab = m_CalibTable2;
		count = m_nTabItemCount2;
	}
	else if (camId == 3)
	{
		pTab = m_CalibTable3;
		count = m_nTabItemCount3;
	}
	else if (camId == 4)
	{
		pTab = m_CalibTable4;
		count = m_nTabItemCount4;
	}

	for (int i = 0; i < count; i++)
	{
		if (pTab[i].x > pos)
		{
			if (i < 1)
				return pTab[0].y;
			else
				return pTab[i - 1].y;
		}
	}
	//ASSERT(FALSE);
	return -1.0f;
}

float YZJPdetectsystemDlg::ComputePosition(int isupCamera, std::vector<cv::Point>& vecAllPosition, cv::Point crossPoint)
{
	int position = 0;
	double inch = 0;
	double dPaperWidth = 0;
	int dLeftInch = 0;//交点左边区间个数
	int dRigthInch = 0;//交点右边区间个数

	double dFactors = 10;//计算比例因子

	int iValue = GetPrivateProfileInt("Set Sfconfig", "TilerFactorIs5", 0, lpPath);//比例计算因子	
	bool bTilerFactorIs5 = (iValue == 1 ? true : false);
	iValue = GetPrivateProfileInt("Set Sfconfig", "CoreFactorIs5", 0, lpPath);//比例计算因子
	bool bCoreFactorIs5 = (iValue == 1 ? true : false);
	if (vecAllPosition.size() == 0)
	{
		mlogger.TraceKeyInfo("相机%d没有标定数据", isupCamera);
		return 0;
	}
	if ((vecAllPosition.at(0).x > crossPoint.x) || (vecAllPosition.at(0).x == crossPoint.x))//交点在最小刻度的左边
	{
		double tp_dSection = double(vecAllPosition.at(1).x - vecAllPosition.at(0).x);
		//double tp_dSection = sqrt(pow(double(vecAllPosition.at(1).x - vecAllPosition.at(0).x), 2) + pow(double(vecAllPosition.at(1).y - vecAllPosition.at(0).y), 2));
		if (tp_dSection != 0)
			inch = double((vecAllPosition.at(0).x - crossPoint.x)) / tp_dSection;
			//inch = sqrt(pow(double(vecAllPosition.at(0).x - crossPoint.x), 2) + pow(double(vecAllPosition.at(0).y - crossPoint.y), 2)) / tp_dSection;
		dLeftInch = 0;
		//modify by zhouqian 20200611
		dRigthInch = (int)vecAllPosition.size() - 1;
		//dRigthInch = (int)vecAllPosition.size();
	
		dPaperWidth = (dLeftInch + dRigthInch + inch) * dFactors;
		//if (isupCamera == 1 || isupCamera == 2)
		if(bTilerFactorIs5)
			dPaperWidth=(dLeftInch + dRigthInch + inch)* dFactors / 2;
		if(bCoreFactorIs5)
			dPaperWidth = (dLeftInch + dRigthInch + inch) * dFactors / 2;
	//	mlogger.TraceKeyInfo("相机%d纸边计算，crosspoint(%d,%d)在最小刻度左边", isupCamera,crossPoint.x,crossPoint.y);
		return dPaperWidth;
	}
	else if ((vecAllPosition.at(vecAllPosition.size() - 1).x < crossPoint.x) || (vecAllPosition.at(vecAllPosition.size() - 1).x == crossPoint.x))//交点在最大刻度的右边
	{
		double tp_dSection = double(vecAllPosition.at(vecAllPosition.size() - 1).x - vecAllPosition.at(vecAllPosition.size() - 2).x);
		//double tp_dSection = sqrt(pow(double(vecAllPosition.at(vecAllPosition.size() - 1).x - vecAllPosition.at(vecAllPosition.size() - 2).x), 2) + pow(double(vecAllPosition.at(vecAllPosition.size() - 1).y - vecAllPosition.at(vecAllPosition.size() - 2).y), 2));
		if (tp_dSection != 0)
			inch = double((crossPoint.x-vecAllPosition.at(vecAllPosition.size() - 1).x)) / tp_dSection;
		//	inch = sqrt(pow(double(crossPoint.x - vecAllPosition.at(vecAllPosition.size() - 1).x), 2) + pow(double(crossPoint.y - vecAllPosition.at(vecAllPosition.size() - 1).y), 2)) / tp_dSection;
		dLeftInch = (int)vecAllPosition.size() - 1;
		dRigthInch = 0;
		dPaperWidth = (dLeftInch + dRigthInch + inch) * dFactors;
		
		if (bTilerFactorIs5)
			dPaperWidth = (dLeftInch + dRigthInch + inch) * dFactors/2;
		if (bCoreFactorIs5)
			dPaperWidth = (dLeftInch + dRigthInch + inch) * dFactors / 2;
	//	mlogger.TraceKeyInfo("相机%d纸边计算，crosspoint(%d,%d)在最大刻度右边", isupCamera, crossPoint.x, crossPoint.y);
		return dPaperWidth;
	}
	else//交点不超出标定区域刻度
	{
		for (int i = 0; i < vecAllPosition.size() - 1; i++)
		{
			//double tp_dCoefficient = sqrt(pow(double(vecAllPosition.at(i).x - vecAllPosition.at(i + 1).x), 2) + pow(double(vecAllPosition.at(i).y - vecAllPosition.at(i + 1).y), 2));//每一段的像素距离
			double tp_dCoefficient = double(vecAllPosition.at(i + 1).x - vecAllPosition.at(i).x);

			if (tp_dCoefficient > 0)
			{
				
				if (vecAllPosition.at(i + 1).x < crossPoint.x || vecAllPosition.at(i + 1).x == crossPoint.x)//区间在交点左边
				{
					dLeftInch++;//交点左边区间个数			 
				}
				else if (vecAllPosition.at(i).x > crossPoint.x || vecAllPosition.at(i).x == crossPoint.x)
				{
					dRigthInch++;//区间在交点右边
				}
				else//在标定尺当前刻度区间内
				{
					////计算在此区间的像素距离/整个区间的像素
					/*inch = sqrt(pow(double(vecAllPosition.at(i + 1).x - crossPoint.x), 2) + pow(double(vecAllPosition.at(i + 1).y - crossPoint.y), 2)) / tp_dCoefficient;*/
					inch = double((vecAllPosition.at(i + 1).x - crossPoint.x)) / tp_dCoefficient;

				}

			}
			else
			{
			}
		}
		//dLeftInch = dLeftInch - 1;
	
		if (isupCamera == 1)//左边的相机中纸的宽度和纸的位置
		{
			dPaperWidth = (dRigthInch + inch) * dFactors;
			if (bTilerFactorIs5)
			dPaperWidth = (dRigthInch + inch) * dFactors *0.5;//这里使用新的标定尺
				
		}
		else if (isupCamera == 2)
		{
			dPaperWidth = (dLeftInch + (1 - inch)) * dFactors;
			if (bTilerFactorIs5)
			dPaperWidth = (dLeftInch + (1 - inch)) * dFactors *0.5;
		}
		else if (isupCamera == 3)
		{
			dPaperWidth = (dRigthInch + inch) * dFactors;
			if (bCoreFactorIs5)
			dPaperWidth = (dRigthInch + inch) * dFactors*0.5;
		}
		else//右边的相机中纸的宽度
		{			
			dPaperWidth = (dLeftInch + (1 - inch)) * dFactors;
			if (bCoreFactorIs5)
				dPaperWidth = (dLeftInch + (1 - inch)) * dFactors*0.5;
		}
	//	mlogger.TraceKeyInfo("相机%d纸边计算，crosspoint(%d,%d)在区间,左边区间个数%d,右边区间个数%d,inch=%2f", isupCamera, crossPoint.x, crossPoint.y,dLeftInch, dRigthInch,inch);
		return dPaperWidth;
	}
	
}

void YZJPdetectsystemDlg::LoadRatioTable(int nId)
{
	TCHAR sKey[32] = _T(""),
		sSec[24] = _T("CameraOne");
	int count = 0;
	int CaliImageX = 0;
	int CaliImageY = 0;


	switch (nId + 1)
	{
	case 1:
		_tcscpy(sSec, _T("CameraOne"));
		//pTab = m_CalibTable1;
		count = GetPrivateProfileInt(sSec, _T("Cam_CaliPositionNum"), 0, lpPath);

		CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_x", 1, lpPath);//标定Rect
		CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_y", 1, lpPath);//标定Rect

		
		m_nTabItemCount1 = count;
		m_CalibTable1.clear();
		for (int i = 0; i < count; i++)
		{
			cv::Point tmp;
			_stprintf(sKey, _T("Cam_CaliRulerPosition%dx"), i);
			tmp.x = GetPrivateProfileInt(sSec, sKey, 0, lpPath);
			_stprintf(sKey, _T("Cam_CaliRulerPosition%dy"), i);
			tmp.y = GetPrivateProfileInt(sSec, sKey, 0, lpPath);
			m_CalibTable1.push_back(tmp);
		}
		for (int n = 0; n < m_CalibTable1.size(); n++)
		{
			m_CalibTable1.at(n).x += CaliImageX;
			m_CalibTable1.at(n).y += CaliImageY;
		}
		break;
	case 2:
		_tcscpy(sSec, _T("CameraTwo"));
	//	pTab = m_CalibTable2;
		count = GetPrivateProfileInt(sSec, _T("Cam_CaliPositionNum"), 0, lpPath);
		CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage2_x", 1, lpPath);//标定Rect
		CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage2_y", 1, lpPath);//标定Rect
		m_nTabItemCount2 = count;
		m_CalibTable2.clear();
		for (int i = 0; i < count; i++)
		{
			cv::Point tmp;
			_stprintf(sKey, _T("Cam_CaliRulerPosition%dx"), i);
			/*GetPrivateProfileString(sSec, sKey, _T("100000.0"), sTemp, 31, lpPath);
			tmp.x = (float)_tstof(sTemp);*/
			tmp.x = GetPrivateProfileInt(sSec, sKey, 0, lpPath);


			_stprintf(sKey, _T("Cam_CaliRulerPosition%dy"), i);
			tmp.y = GetPrivateProfileInt(sSec, sKey, 0, lpPath);
			/*	GetPrivateProfileString(sSec, sKey, _T("-1.0"), sTemp, 31, lpPath);
				tmp.y = (float)_tstof(sTemp);*/
			
			m_CalibTable2.push_back(tmp);
		
		}
		for (int n = 0; n < m_CalibTable2.size(); n++)
		{
			m_CalibTable2.at(n).x += CaliImageX;
			m_CalibTable2.at(n).y += CaliImageY;
		}
		break;
	case 3:
		_tcscpy(sSec, _T("CameraThree"));
		//pTab = m_CalibTable3;
		count = GetPrivateProfileInt(sSec, _T("Cam_CaliPositionNum"), 0, lpPath);
		CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage3_x", 1, lpPath);//标定Rect
		CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage3_y", 1, lpPath);//标定Rect
	
		m_nTabItemCount3 = count;
		m_CalibTable3.clear();
		for (int i = 0; i < count; i++)
		{
			cv::Point tmp;
			_stprintf(sKey, _T("Cam_CaliRulerPosition%dx"), i);
			/*GetPrivateProfileString(sSec, sKey, _T("100000.0"), sTemp, 31, lpPath);
			tmp.x = (float)_tstof(sTemp);*/
			tmp.x = GetPrivateProfileInt(sSec, sKey, 0, lpPath);
			

			_stprintf(sKey, _T("Cam_CaliRulerPosition%dy"), i);
			tmp.y = GetPrivateProfileInt(sSec, sKey, 0, lpPath);
		/*	GetPrivateProfileString(sSec, sKey, _T("-1.0"), sTemp, 31, lpPath);
			tmp.y = (float)_tstof(sTemp);*/
			m_CalibTable3.push_back(tmp);
		}
		for (int n = 0; n < m_CalibTable3.size(); n++)
		{
			m_CalibTable3.at(n).x += CaliImageX;
			m_CalibTable3.at(n).y += CaliImageY;
		}
		break;
	case 4:
		_tcscpy(sSec, _T("CameraFour"));
		//pTab = m_CalibTable4;
		count = GetPrivateProfileInt(sSec, _T("Cam_CaliPositionNum"), 0, lpPath);
		CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage4_x", 1, lpPath);//标定Rect
		CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage4_y", 1, lpPath);//标定Rect

		m_nTabItemCount4 = count;
		m_CalibTable4.clear();
		for (int i = 0; i < count; i++)
		{
			cv::Point tmp;
			_stprintf(sKey, _T("Cam_CaliRulerPosition%dx"), i);
			/*GetPrivateProfileString(sSec, sKey, _T("100000.0"), sTemp, 31, lpPath);
			tmp.x = (float)_tstof(sTemp);*/
			tmp.x = GetPrivateProfileInt(sSec, sKey, 0, lpPath);


			_stprintf(sKey, _T("Cam_CaliRulerPosition%dy"), i);
			tmp.y = GetPrivateProfileInt(sSec, sKey, 0, lpPath);
			/*	GetPrivateProfileString(sSec, sKey, _T("-1.0"), sTemp, 31, lpPath);
				tmp.y = (float)_tstof(sTemp);*/
			m_CalibTable4.push_back(tmp);
		}
		for (int n = 0; n < m_CalibTable4.size(); n++)
		{
			m_CalibTable4.at(n).x += CaliImageX;
			m_CalibTable4.at(n).y += CaliImageY;
		}
		break;
	}

	mlogger.TraceInfo(_T("xxxx--标定结果%d：count=%d \n"), nId+1, count);

}

//启动程序
void YZJPdetectsystemDlg::OnBnClickedButstartsystem()
{
	// TODO:  在此添加控件通知处理程序代码

	CString countsrc;
	countsrc.Format("%s 系统运行中", myGetCurrentTime());
	m_xclist.InsertString(0, countsrc);
	m_xclist.SetCurSel(0);

	int nComId = GetPrivateProfileInt("Blowpipe set", "COMid", 4, lpPath);
	//串口的初始化
	if (!mySerialPort.InitPort(nComId, CBR_9600, 'N', 8, 1, EV_RXCHAR))
	{
		mlogger.TraceError( "initPort fail!" );
		//exit(0);
	}
	else
	{
		thread th(&YZJPdetectsystemDlg::receiveData, this);
		th.detach();
	}
#if 1


	//相机1
	thread th1(&YZJPdetectsystemDlg::gatherImage, this);
	th1.detach();

	//process image
	thread th2(&YZJPdetectsystemDlg::proImage, this);
	th2.detach();

	//save image
	thread th3(&YZJPdetectsystemDlg::saveImage, this);
	th3.detach();


	////相机2
	thread th11(&YZJPdetectsystemDlg::gatherImage2, this);
	th11.detach();

	//process image
	thread th21(&YZJPdetectsystemDlg::proImage2, this);
	th21.detach();

	//save image
	thread th31(&YZJPdetectsystemDlg::saveImage2, this);
	th31.detach();

	////相机3
	thread th12(&YZJPdetectsystemDlg::gatherImage3, this);
	th12.detach();

	//process image
	thread th22(&YZJPdetectsystemDlg::proImage3, this);
	th22.detach();

	//save image
	thread th32(&YZJPdetectsystemDlg::saveImage3, this);
	th32.detach();

	////相机4
	thread th13(&YZJPdetectsystemDlg::gatherImage4, this);
	th13.detach();

	//process image
	thread th23(&YZJPdetectsystemDlg::proImage4, this);
	th23.detach();

	//save image
	thread th33(&YZJPdetectsystemDlg::saveImage4, this);
	th33.detach();

#endif // 0	

#if 1
	//modbus发送线程1
	thread th5(&YZJPdetectsystemDlg::modbus2PLC, this);
	th5.detach();
	
#endif

	


	//m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	//m_pThread = AfxBeginThread(SendThreadProc, this);//数据发送到plc
	//ASSERT(m_hEvent && m_pThread);
	//m_bQuit = FALSE;

	//定时删除文件线程
	/*thread th7(&YZJPdetectsystemDlg::hxDeleteFile, this);
	th7.detach();
	*/
	//在系统运行之间将当前日期前n天的系统日志进行删除（由于我们的机器会最长7天进行一次关机）


	mlogger.TraceKeyInfo("……系统开始运行……");

}



//单相机1图像拼接
void YZJPdetectsystemDlg::gatherImage()
{

	stopgather = false;
	
	int coutsave = 0;

	Mat dstimage;
	while (!stopgather)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

		if (srcMat_src.empty())
			continue;

		Mat midimage;
		srcMat_src.wait_and_pop(midimage);
		//flip(midimage, midimage, 0);
		int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum"), 10, lpPath);

		if (coutsave == sleepNum)
		{
			if (midMat_pro.size() < 1)
			{
#if 1 //裁剪图像大小？

#endif
				midMat_pro.push(midimage);//transprot image to process
				srcMat_save.push(midimage);//transprot image to save 
			}
		}

		coutsave++;//必须放在此处

		if (coutsave > sleepNum)
		{
			coutsave = 0;//归0
		}
		
	}
	mlogger.TraceKeyInfo("相机1采集线程：线程结束");
}
//检测算法1
void YZJPdetectsystemDlg::proImage()
{
	int vcout = 0;
	//double preposition = 0;//记录前次结果-综合结果
	
	double rehightOld=0;
	//int resultOld=0;
	vector<double> rehighData;
	vector<int> resultData;
	bool bFristResult = false;
	int minPos = 0,maxPos=0;//边线临界值
	int nId = 1;
	vector<cv::Point> tmp_CalibTable = m_CalibTable1;
	int tmp_nTabItemCount = m_nTabItemCount1;
	mlogger.TraceKeyInfo("相机%d处理图像线程：线程开始",nId);

	stop_proImage = false;
	while (!stop_proImage)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

		if (!midMat_pro.empty())
		{
			std::chrono::time_point<std::chrono::steady_clock> beginTimeOne, endTimeOne;
			std::chrono::duration<double, std::milli> elapsed_millisecondsalOne;
			beginTimeOne = std::chrono::high_resolution_clock::now();

			Mat proImage;
			midMat_pro.wait_and_pop(proImage);
		/*	flip(proImage, proImage,0);*/
			//画感兴趣区域ROI【用于传给算法的图像】	
			Vec4i L;//检边位置
			cv::Point m_loc(0,0);
			double position = 0;
			int result = 0;
			int ImagePro_on = GetPrivateProfileInt("Set ImageFormat", "ImagePro_on", 1, lpPath);//算法开关
			int CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_x", 1, lpPath);//标定Rect
			int CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_y", 1, lpPath);//标定Rect
			int CaliImageW = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_w", 100, lpPath);//标定Rect
			int CaliImageH = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_h", 64, lpPath);//标定Rect
			int RunImageX = GetPrivateProfileInt("Set ImageFormat", "RunImage1_x", 1, lpPath);//标定Rect
			int RunImageY = GetPrivateProfileInt("Set ImageFormat", "RunImage1_y", 1, lpPath);//标定Rect
			int RunImageW = GetPrivateProfileInt("Set ImageFormat", "RunImage1_w", 100, lpPath);//标定Rect
			int RunImageH = GetPrivateProfileInt("Set ImageFormat", "RunImage1_h", 64, lpPath);//标定Rect
			int nMaxPic = GetPrivateProfileInt(_T("Set_Camera"), _T("useMaxPic"), 1, lpPath);;
			int ORG = GetPrivateProfileInt("Set ImageFormat", "Interval1", 1000, lpPath);//算法开关
			
			Rect rectROI = Rect(RunImageX, RunImageY, RunImageW, RunImageH);//感兴趣区域ROI
			Rect rectROI2 = Rect(CaliImageX, CaliImageY, CaliImageW, CaliImageH);//感兴趣区域ROI
			char chAttribute[20];
			memset(&chAttribute, 0, sizeof(chAttribute));
			GetPrivateProfileString("Set Detection Range", "StandardDeviation", "0", chAttribute, 20, lpPath);//标准差差
			double dMtandardDeviation = atof(chAttribute);
			memset(&chAttribute, 0, sizeof(chAttribute));
			GetPrivateProfileString("Set Detection Range", "ReducedValue", "0", chAttribute, 20, lpPath);//均值对比值
			double dReducedValue = atof(chAttribute);
#if 1				//modify 
			std::chrono::time_point<std::chrono::steady_clock> beginTime, endTime;
			std::chrono::duration<double, std::milli> elapsed_milliseconds;
			beginTime = std::chrono::high_resolution_clock::now();
				
			int resultShow = GetPrivateProfileInt(_T("Set Sfconfig"), _T("ResultShow"), 0, lpPath);
			if (resultShow)//显示图像
			{
				if (g_dlg->m_camSetDlg.IsWindowVisible() && g_dlg->m_camSetDlg.GetCamId() == nId)//在设置界面显示
				{
					if (ImagePro_on && !proImage.empty())
					{
						//图像截取
						Mat tmpImage = proImage(rectROI);
						CFindBound pFindBound;
						result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
						m_loc.x += rectROI.x;
						m_loc.y += rectROI.y;
						//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId, result, m_loc.x, m_loc.y);
						double dPaperWidth = 0;
						//计算物理位置
						if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
						{
							dPaperWidth = ComputePosition(nId, tmp_CalibTable, m_loc);
							//求位置
							position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
						//	mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%d\n", nId, ORG / 2);
						}
						else
						{
							position = ORG / 2;
						}
						mlogger.TraceInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
						L[0] = m_loc.x;
						//L[1] = m_loc.y - 32;
						L[1] = RunImageY;
						L[2] = m_loc.x;
						//L[3] = m_loc.y + 32;
						L[3] = RunImageY + RunImageH;


					}
					else
					{
						L[0] = RunImageX;//初始化
						L[1] = RunImageY;
						L[2] = RunImageX;
						L[3] = RunImageY + RunImageH;
					}
					Mat imgRGB;
					cv::cvtColor(proImage, imgRGB, COLOR_GRAY2RGB);
					Point p1(L[0] , L[1]);
					Point p2(L[2], L[3]);
					//mlogger.TraceKeyInfo("xxxx--相机%d p1(%d ,%d) p2(%d,%d)\n", nId, L[0], L[1], L[2], L[3]);
					cv::line(imgRGB, p1, p2, Scalar(0, 0, 255), 8);
					g_dlg->m_camSetDlg.ShowImageToPic(imgRGB, IDC_CAM_PIC);
					imgRGB.release();
				}
				else//在主界面
				{
					if (ImagePro_on && !proImage.empty())
					{
						Mat tmpImage = proImage.clone();
						CFindBound pFindBound;
						result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
						//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId, result, m_loc.x, m_loc.y);
						double dPaperWidth = 0;
						//计算物理位置
						if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
						{
							if (m_loc.x > 0)
							{
								cv::Point tmp;
								tmp.x = m_loc.x + rectROI.x;
								tmp.y = m_loc.y + rectROI.y;
								dPaperWidth = ComputePosition(nId, tmp_CalibTable, tmp);
								//求位置
								position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
							}
							else
							{
								position = ORG / 2;
							}
						//	mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%2f,point1(%d,%d)\n", nId, double(ORG / 2), m_loc.x, m_loc.y);
							L[0] = m_loc.x;//初始化
							//L[1] = m_loc.y - 32;
							L[1] = 0;
							L[2] = m_loc.x;
							//L[3] = m_loc.y + 32;
							L[3] = RunImageH;
						}
						else
						{
							position = ORG / 2;
							L[0] = m_loc.x;//初始化
							L[1] = 0;
							L[2] = m_loc.x;
							L[3] = RunImageH;
						}
						mlogger.TraceInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
					
					}
					else
					{
						L[0] = 1;//初始化
						L[1] = 0;
						L[2] = 1;
						L[3] = RunImageH;
					}
					Mat imgRGB;
					cv::cvtColor(proImage, imgRGB, COLOR_GRAY2RGB);
					//画检测到的边线	
					Point p1(L[0], L[1]);
					Point p2(L[2], L[3]);
				//	mlogger.TraceKeyInfo("xxxx--相机%d p1(%d ,%d) p2(%d,%d)\n", nId, L[0], L[1], L[2], L[3]);
					cv::line(imgRGB, p1, p2, Scalar(0, 0, 255), 8);
					ShowImageToPic(imgRGB, IDC_PIC_LEFT);
					imgRGB.release();
				}
			}
			else//不显示图像
			{
				if (ImagePro_on && !proImage.empty())
				{
					Mat tmpImage = proImage.clone();
					CFindBound pFindBound;
					result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
					//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId, result, m_loc.x, m_loc.y);
					double dPaperWidth = 0;
					//计算物理位置
					if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
					{
						if (m_loc.x > 0)
						{
							cv::Point tmp;
							tmp.x = m_loc.x + rectROI.x;
							tmp.y = m_loc.y + rectROI.y;
							dPaperWidth = ComputePosition(nId, tmp_CalibTable, tmp);
							position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
						}
						else
						{
							position = ORG / 2;
						}
					//	mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%2f,point1(%d,%d)\n", nId, double(ORG / 2), m_loc.x, m_loc.y);
					}
					else
					{
						position = ORG / 2;
					}
					mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
					L[0] = m_loc.x;//初始化
					L[1] = 0;
					L[2] = m_loc.x;
					L[3] = RunImageH;
				}
				else
				{
					L[0] = 1;//初始化
					L[1] = 0;
					L[2] = 1;
					L[3] = RunImageH;
				}
			}

			endTime = std::chrono::high_resolution_clock::now();
			elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
			mlogger.TraceInfo(_T("xxxx--相机%d算法时间：%.2lf ms"), nId,elapsed_milliseconds.count());
#endif				//modify end

#if 0
			if (ImagePro_on && !proImage.empty())
			{
				std::chrono::time_point<std::chrono::steady_clock> beginTime, endTime;
				std::chrono::duration<double, std::milli> elapsed_milliseconds;
				beginTime = std::chrono::high_resolution_clock::now();
				//mlogger.TraceKeyInfo("xxxx--相机1算法开始 \n ");
				if (nMaxPic == 1)
				{
					//图像截取
					Mat tmpImage = proImage(rectROI);
					CFindBound pFindBound;
					result = pFindBound.BoundLocWithState(tmpImage, m_loc, 1);
					m_loc.x += rectROI.x;
					m_loc.y += rectROI.y;
					mlogger.TraceKeyInfo("xxxx--相机1算法结束 result=%d  m_loc(%d,%d)\n", result, m_loc.x, m_loc.y);
					double dPaperWidth = 0;
					//计算物理位置
					if (result == 1 && m_nTabItemCount1 > 0 && m_CalibTable1.size() > 0)
					{
						dPaperWidth = ComputePosition(1, m_CalibTable1, m_loc);
						//求位置
						position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
						mlogger.TraceKeyInfo("xxxx--相机1视野盲区=%d\n", ORG / 2);
					}
					else
					{
						position = ORG / 2;
					}
					mlogger.TraceKeyInfo("xxxx--相机1物理位置纸宽度%2f,位置position=%2f\n", dPaperWidth, position);
					

					L[0] = m_loc.x;
					L[1] = m_loc.y - 32;
					L[2] = m_loc.x;
					L[3] = m_loc.y + 32;
				}
				else
				{
					Mat tmpImage = proImage.clone();
					CFindBound pFindBound;
					result = pFindBound.BoundLocWithState(tmpImage, m_loc, 1);
					mlogger.TraceKeyInfo("xxxx--相机1算法结束 result=%d  m_loc(%d,%d)\n", result, m_loc.x, m_loc.y);
					double dPaperWidth = 0;
					//计算物理位置
					if (result == 1 && m_nTabItemCount1 > 0 && m_CalibTable1.size() > 0)
					{
						if (m_loc.x > 0)
						{
							cv::Point tmp;
							tmp.x=m_loc.x + rectROI.x;
							tmp.y=m_loc.y + rectROI.y;							
							dPaperWidth = ComputePosition(1, m_CalibTable1, tmp);
							//求位置
							position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
						}
						else
						{
							position = ORG / 2;
						}
						mlogger.TraceKeyInfo("xxxx--相机1视野盲区=%2f,point1(%d,%d)\n", double(ORG / 2), m_loc.x, m_loc.y);
					}
					else
					{
						position = ORG / 2;
					}
					mlogger.TraceKeyInfo("xxxx--相机1物理位置纸宽度%2f,位置position=%2f\n", dPaperWidth, position);
			
					L[0] = m_loc.x;//初始化
					L[1] = m_loc.y-32;
					L[2] = m_loc.x;
					L[3] = m_loc.y+32;
				}
				endTime = std::chrono::high_resolution_clock::now();
				elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
				mlogger.TraceInfo(_T("xxxx--相机1算法时间：%.2lf ms"), elapsed_milliseconds.count());
			}
			else
			{
				if (nMaxPic == 1)
				{
					L[0] = RunImageX;//初始化
					L[1] = RunImageY;
					L[2] = RunImageX;
					L[3] = RunImageY + RunImageH;
				}
				else
				{
					L[0] = 0;//初始化
					L[1] = 0;
					L[2] = 0;
					L[3] = 64;
				}

			}
#endif
			//综合结果合成数量
			int result_num = GetPrivateProfileInt(_T("Set Sfconfig"), _T("result_num"), 15, lpPath);
			float meanhigh = 0.0;
			int meanresult = 0;
			if (rehighData.size() < result_num)//单次结果处理流程
			{
				if (position != 0)
				{
					resultData.push_back(result);
					if (result == 0 || position <= 0 || position > 3300/*&& (preposition != 0)*/)//滤掉异常值
					{
						//rehighData.push_back(preposition);
						rehighData.push_back(rehightOld);
					}
					else
					{
						rehighData.push_back(position);
					}
				}
	
			}
			else//综合结果处理
			{

	
				if (!rehighData.empty()&& !resultData.empty())
				{
					if (!resultData.empty())
					{//当无纸状态大于个数的一半，则为无纸，否则为有纸
						int num = count(resultData.begin(), resultData.end(), 0);
						if (num > resultData.size() / 2)
							meanresult = 0;
						else
							meanresult = 1;
					}
					vector<int>().swap(resultData);
					
					if (!bFristResult)
					{
						double pfcount = StandardDeviation(rehighData);//求标准差
																	   //2、判断标准差是否符合小于条件a   
						if (pfcount >= 0 && pfcount < dMtandardDeviation)
						{
							//求均值
							rehightOld = average(rehighData);
							vector<double>().swap(rehighData);
							bFristResult = true;
						}
						else
						{
							//输入下一组数据
							vector<double>().swap(rehighData);
						}
					}
					else
					{
						double pfcount = StandardDeviation(rehighData);//求标准差																		
						if (pfcount >= 0 && pfcount < dMtandardDeviation)//判断标准差是否符合小于条件a   
						{
							meanhigh = average(rehighData);//求均值							
							if (abs(meanhigh - rehightOld) < dReducedValue)//均值与第一次的参考值比较
							{
								//发送参考值
								meanhigh = rehightOld;
							}
							else
							{
								//发送均值；参考值=均值
								rehightOld=meanhigh;								
							}
						}
						else//发送参考值；
						{
							meanhigh = rehightOld;
						}
						vector<double>().swap(rehighData);//清空数据
					}
					if (send_Data.size() < 1)
					{
						PositonResult tp_result;
						tp_result.result = meanresult;
						tp_result.position = meanhigh;
						send_Data.push(tp_result);
					}
				}
			}


			vcout++;
			CString countsrc1;
			countsrc1.Format("相机%d处理图像线程：纸边:(%d,%d)", nId, L[0], L[1]);
			//mlogger.TraceKeyInfo(countsrc1);
			//输出到列表框
			m_xclist.InsertString(0, countsrc1);
			m_xclist.SetCurSel(0);

			endTimeOne = std::chrono::high_resolution_clock::now();
			elapsed_millisecondsalOne = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeOne - beginTimeOne);
			mlogger.TraceInfo(_T("xxxx--相机%d单次时间：%.2lf ms"), nId, elapsed_millisecondsalOne.count());
		
		}
		
	}
	mlogger.TraceKeyInfo("相机%d处理图像线程：线程结束", nId);

}
//保存原图和结果图
void YZJPdetectsystemDlg::saveImage()
{
	int coutsave = 0;
	int coutsave2 = 0;
	stop_saveImage = false;
	while (!stop_saveImage)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

#if 1
		//保存原图
		if (!srcMat_save.empty())
		{

			Mat midimage;
			//srcMat_save.wait_and_pop(midimage);


			int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum_save"), 10, lpPath);

			if (coutsave == sleepNum)
			{
				srcMat_save.wait_and_pop(midimage);
				mlogger.TraceInfo("xxxx--相机1保存图像\n");

				//选择保存图像的格式，0-jpg,1-all bmp,2-halfImage bmp, 3-fullImage bmp, 4-bbImage bmp
				int ImageFormat_choice = GetPrivateProfileInt("Set ImageFormat", "ImageFormat_choice", 0, lpPath);
				int SourceImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "SourceImageSave_choice", 0, lpPath);

				if (1 == SourceImageSave_choice)
				{
					SYSTEMTIME sys;
					GetLocalTime(&sys);
					char  strTime[200];
					memset(strTime, 0, 200 * sizeof(CHAR));
					sprintf(strTime, "%d%02d%02d%02d%02d%02d%03d_1",
						sys.wYear,
						sys.wMonth,
						sys.wDay,
						sys.wHour,
						sys.wMinute,
						sys.wSecond,
						sys.wMilliseconds);

					string sspath = m_s + "sourceimage";
					CString path2 = sspath.c_str();
					if (!PathIsDirectory(path2))
					{
						//::CreateDirectory(path2, 0);
						BOOL ret = ::CreateDirectory(path2, 0);
						mlogger.TraceInfo("xxxx--相机1创建图片保存文件夹%s=%d", path2, ret);
					}

					char  strname[300];
					memset(strname, 0, 300 * sizeof(CHAR));

					if (1 == ImageFormat_choice || 2 == ImageFormat_choice)
					{
						sprintf(strname, "\\%s.bmp", strTime);
					}
					else
					{
						sprintf(strname, "\\%s.jpg", strTime);
					}


					std::string strnamepath = "";
					strnamepath = strname;
					strnamepath = sspath + strnamepath;
					imwrite(strnamepath, midimage);
					
				}
			}
			else
			{
				Mat pqmidimage;
				srcMat_save.wait_and_pop(pqmidimage);
			}

			coutsave++;//必须放在此处

			if (coutsave > sleepNum)
			{
				coutsave = 0;//归0
			}
		}
#endif // 1


#if 0
		//保存结果图
		if (!dstMat_save.empty())
		{
			//CString countsrc;
			//countsrc.Format(_T("结果存图1大小:%d\n"), dstMat_save.size());
			//OutputDebugString(countsrc);

			Mat dstimage;

			int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum_save"), 10, lpPath);

			if (coutsave2 == sleepNum)
			{
				dstMat_save.wait_and_pop(dstimage);

				//选择保存图像的格式，0-jpg,1-all bmp,2-halfImage bmp, 3-fullImage bmp, 4-bbImage bmp
				int ImageFormat_choice = GetPrivateProfileInt("Set ImageFormat", "ImageFormat_choice", 0, lpPath);
				//	int SourceImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "SourceImageSave_choice", 1, lpPath);
				int ResultImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "ResultImageSave_choice", 1, lpPath);
				if (1 == ResultImageSave_choice)
				{
					SYSTEMTIME sys;
					GetLocalTime(&sys);
					char  strTime[200];
					memset(strTime, 0, 200 * sizeof(CHAR));
					sprintf(strTime, "%d%02d%02d%02d%02d%02d%03d_1",
						sys.wYear,
						sys.wMonth,
						sys.wDay,
						sys.wHour,
						sys.wMinute,
						sys.wSecond,
						sys.wMilliseconds);

					//2018.09.12
					CTime t = CTime::GetCurrentTime();
					string s = t.Format("D://imageFDZ//%Y-%m-%d");
					CString path = s.c_str();
					if (!PathIsDirectory(path))
					{
						::CreateDirectory(path, 0);
					}

					string sspath = s + "//resultimage";
					CString path2 = sspath.c_str();
					if (!PathIsDirectory(path2))
					{
						::CreateDirectory(path2, 0);
					}

					char  strname[300];
					memset(strname, 0, 300 * sizeof(CHAR));

					if (1 == ImageFormat_choice || 2 == ImageFormat_choice)
					{
						sprintf(strname, "//%s.bmp", strTime);
					}
					else
					{
						sprintf(strname, "//%s.jpg", strTime);
					}


					std::string strnamepath = "";
					strnamepath = strname;
					strnamepath = sspath + strnamepath;
					cv::imwrite(strnamepath, dstimage);
				}

			}
			else
			{
				Mat pqmidimage;
				dstMat_save.wait_and_pop(pqmidimage);
			}

			coutsave2++;//必须放在此处

			if (coutsave2 > sleepNum)
			{
				coutsave2 = 0;//归0
			}
		}
#endif // 1

	}

}

//单相机2图像拼接
void YZJPdetectsystemDlg::gatherImage2()
{

	stopgather2 = false;
	int coutsave = 0;

	Mat dstimage;
	while (!stopgather2)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

		if (srcMat_src2.empty())
			continue;

		{
			Mat midimage;
			srcMat_src2.wait_and_pop(midimage);
			int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum"), 10, lpPath);

			if (coutsave == sleepNum)
			{
				if (midMat_pro2.size() < 1)
				{
#if 1 //裁剪图像大小？

#endif
					midMat_pro2.push(midimage);//transprot image to process
					srcMat_save2.push(midimage);//transprot image to save 
				}
			}

			coutsave++;//必须放在此处

			if (coutsave > sleepNum)
			{
				coutsave = 0;//归0
			}
		}
	}


	mlogger.TraceKeyInfo("相机2采集线程：线程结束");
}

//检测算法2
void YZJPdetectsystemDlg::proImage2()
{
	int vcout = 0;
	double rehightOld = 0;//记录前次结果-综合结果

	bool bFristResult = false;
	vector<double> rehighData;
	vector<int> resultData;
	int nId = 2;
	vector<cv::Point> tmp_CalibTable = m_CalibTable2;
	int tmp_nTabItemCount = m_nTabItemCount2;
	int minPos = 0, maxPos = 0;//边线临界值
	mlogger.TraceKeyInfo("相机%d处理图像线程：线程开始", nId);

	stop_proImage2 = false;
	while (!stop_proImage2)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

		if (!midMat_pro2.empty())
		{
			std::chrono::time_point<std::chrono::steady_clock> beginTimeOne, endTimeOne;
			std::chrono::duration<double, std::milli> elapsed_millisecondsalOne;
			beginTimeOne = std::chrono::high_resolution_clock::now();

			Mat proImage;
			midMat_pro2.wait_and_pop(proImage);
			/*	flip(proImage, proImage,0);*/
				//画感兴趣区域ROI【用于传给算法的图像】	
			Vec4i L;//检边位置
			cv::Point m_loc(0, 0);
			double position = 0;
			int result = 0;
			int ImagePro_on = GetPrivateProfileInt("Set ImageFormat", "ImagePro_on", 1, lpPath);//算法开关
			int CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage2_x", 1, lpPath);//标定Rect
			int CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage2_y", 1, lpPath);//标定Rect
			int CaliImageW = GetPrivateProfileInt("Set ImageFormat", "CaliImage2_w", 100, lpPath);//标定Rect
			int CaliImageH = GetPrivateProfileInt("Set ImageFormat", "CaliImage2_h", 64, lpPath);//标定Rect
			int RunImageX = GetPrivateProfileInt("Set ImageFormat", "RunImage2_x", 1, lpPath);//标定Rect
			int RunImageY = GetPrivateProfileInt("Set ImageFormat", "RunImage2_y", 1, lpPath);//标定Rect
			int RunImageW = GetPrivateProfileInt("Set ImageFormat", "RunImage2_w", 100, lpPath);//标定Rect
			int RunImageH = GetPrivateProfileInt("Set ImageFormat", "RunImage2_h", 64, lpPath);//标定Rect
			int nMaxPic = GetPrivateProfileInt(_T("Set_Camera"), _T("useMaxPic"), 1, lpPath);;
			int ORG = GetPrivateProfileInt("Set ImageFormat", "Interval1", 1000, lpPath);//算法开关

			Rect rectROI = Rect(RunImageX, RunImageY, RunImageW, RunImageH);//感兴趣区域ROI
			Rect rectROI2 = Rect(CaliImageX, CaliImageY, CaliImageW, CaliImageH);//感兴趣区域ROI
			char chAttribute[20];
			memset(&chAttribute, 0, sizeof(chAttribute));
			GetPrivateProfileString("Set Detection Range", "StandardDeviation", "0", chAttribute, 20, lpPath);//标准差差
			double dMtandardDeviation = atof(chAttribute);
			memset(&chAttribute, 0, sizeof(chAttribute));
			GetPrivateProfileString("Set Detection Range", "ReducedValue", "0", chAttribute, 20, lpPath);//均值对比值
			double dReducedValue = atof(chAttribute);
#if 1				//modify 
			std::chrono::time_point<std::chrono::steady_clock> beginTime, endTime;
			std::chrono::duration<double, std::milli> elapsed_milliseconds;
			beginTime = std::chrono::high_resolution_clock::now();

			int resultShow = GetPrivateProfileInt(_T("Set Sfconfig"), _T("ResultShow"), 0, lpPath);
			if (resultShow)//显示图像
			{
				if (g_dlg->m_camSetDlg.IsWindowVisible() && g_dlg->m_camSetDlg.GetCamId() == nId)//在设置界面显示
				{
					if (ImagePro_on && !proImage.empty())
					{
						//图像截取
						Mat tmpImage = proImage(rectROI);
						CFindBound pFindBound;
						result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
						m_loc.x += rectROI.x;
						m_loc.y += rectROI.y;
						//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId, result, m_loc.x, m_loc.y);
						double dPaperWidth = 0;
						//计算物理位置
						if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
						{
							dPaperWidth = ComputePosition(nId, tmp_CalibTable, m_loc);
							//求位置
							position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
						//	mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%d\n", nId, ORG / 2);
						}
						else
						{
							position = ORG / 2;
						}
						mlogger.TraceInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
					/*	L[0] = m_loc.x;
						L[1] = m_loc.y - 32;
						L[2] = m_loc.x;
						L[3] = m_loc.y + 32;*/
						L[0] = abs(m_loc.x);//初始化
						L[1] = RunImageY;
						L[2] = abs(m_loc.x);
						L[3] = RunImageY+RunImageH;


					}
					else
					{
						L[0] = RunImageX;//初始化
						L[1] = RunImageY;
						L[2] = RunImageX;
						L[3] = RunImageY + RunImageH;
					}
					Mat imgRGB;
					cv::cvtColor(proImage, imgRGB, COLOR_GRAY2RGB);
					Point p1(L[0], L[1] );
					Point p2(L[2], L[3]);
				//	mlogger.TraceKeyInfo("xxxx--相机%d p1(%d ,%d) p2(%d,%d)\n", nId, L[0], L[1], L[2], L[3]);
					cv::line(imgRGB, p1, p2, Scalar(0, 0, 255), 8);
					g_dlg->m_camSetDlg.ShowImageToPic(imgRGB, IDC_CAM_PIC);
					imgRGB.release();
				}
				else//在主界面
				{
					if (ImagePro_on && !proImage.empty())
					{
						Mat tmpImage = proImage.clone();
						CFindBound pFindBound;
						result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
						//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId, result, m_loc.x, m_loc.y);
						double dPaperWidth = 0;
						//计算物理位置
						if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
						{
#if 1
							if (m_loc.x > 0)
							{
								cv::Point tmp;
								tmp.x = m_loc.x + rectROI.x;
								tmp.y = m_loc.y + rectROI.y;
								dPaperWidth = ComputePosition(nId, tmp_CalibTable, tmp);
								//求位置
								position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
							}
							else
							{
								position = ORG / 2;
							}
#endif
						//	position = ORG / 2;
						//	mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%2f,point1(%d,%d)\n", nId, double(ORG / 2), m_loc.x, m_loc.y);
							//L[0] = abs(m_loc.x);//初始化
							//L[1] = 0;
							//L[2] = abs(m_loc.x);
							//L[3] = 64;
							L[0] = abs(m_loc.x);//初始化
							L[1] = 0;
							L[2] = abs(m_loc.x);
							L[3] = RunImageH;
						}
						else
						{
							position = ORG / 2;
							//L[0] = m_loc.x;//初始化
							//L[1] = 0;
							//L[2] = m_loc.x;
							//L[3] = 64;
							L[0] = abs(m_loc.x);//初始化
							L[1] = 0;
							L[2] = abs(m_loc.x);
							L[3] = RunImageH;
						}
						mlogger.TraceInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
					
					}
					else
					{
						L[0] = 1;//初始化
						L[1] = 0;
						L[2] = 1;
						L[3] = RunImageH;
					}
					Mat imgRGB;
					cv::cvtColor(proImage, imgRGB, COLOR_GRAY2RGB);
					//画检测到的边线	
					Point p1(L[0], L[1]);
					Point p2(L[2], L[3]);
				//	mlogger.TraceKeyInfo("xxxx--相机%d p1(%d ,%d) p2(%d,%d)\n", nId, L[0], L[1], L[2], L[3]);
					cv::line(imgRGB, p1, p2, Scalar(0, 0, 255), 8);
					ShowImageToPic(imgRGB, IDC_PIC_RIGHT);
					imgRGB.release();
				}
			}
			else//不显示图像
			{
				if (ImagePro_on && !proImage.empty())
				{
					Mat tmpImage = proImage.clone();
					CFindBound pFindBound;
					result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
					//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId, result, m_loc.x, m_loc.y);
					double dPaperWidth = 0;
					//计算物理位置
					if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
					{
						if (m_loc.x > 0)
						{
							cv::Point tmp;
							tmp.x = m_loc.x + rectROI.x;
							tmp.y = m_loc.y + rectROI.y;
							dPaperWidth = ComputePosition(nId, tmp_CalibTable, tmp);
							position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
						}
						else
						{
							position = ORG / 2;
						}
					//	mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%2f,point1(%d,%d)\n", nId, double(ORG / 2), m_loc.x, m_loc.y);
						L[0] = m_loc.x;//初始化
						L[1] = m_loc.y - 32;
						L[2] = m_loc.x;
						L[3] = m_loc.y + 32;
					}
					else
					{
						position = ORG / 2;
						L[0] = m_loc.x;//初始化
						L[1] = 0;
						L[2] = m_loc.x;
						L[3] = 64;
					}
					mlogger.TraceInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
				
				}
				else
				{
					L[0] = 1;//初始化
					L[1] = 0;
					L[2] = 1;
					L[3] = 64;
				}
			}

			endTime = std::chrono::high_resolution_clock::now();
			elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
			mlogger.TraceInfo(_T("xxxx--相机%d.算法时间：%.2lf ms"), nId,elapsed_milliseconds.count());
#endif				//modify end

			//综合结果合成数量
			int result_num = GetPrivateProfileInt(_T("Set Sfconfig"), _T("result_num"), 15, lpPath);
			float meanhigh = 0.0;
			int meanresult = 0;
			if (rehighData.size() < result_num)//单次结果处理流程
			{
				if (position != 0)
				{
					resultData.push_back(result);
					if (result == 0 || position <= 0 || position > 3300/*&& (preposition != 0)*/)//滤掉异常值
					{
						//rehighData.push_back(preposition);
						rehighData.push_back(rehightOld);
					}
					else
					{
						rehighData.push_back(position);
					}
				}
			}
			else//综合结果处理
			{
				if (!rehighData.empty() && !resultData.empty())
				{
					if (!resultData.empty())
					{//当无纸状态大于个数的一半，则为无纸，否则为有纸
						int num = count(resultData.begin(), resultData.end(), 0);
						if (num > resultData.size() / 2)
							meanresult = 0;
						else
							meanresult = 1;
					}
					vector<int>().swap(resultData);

					if (!bFristResult)
					{
						double pfcount = StandardDeviation(rehighData);//求标准差
																	   //2、判断标准差是否符合小于条件a   
						if (pfcount >= 0 && pfcount < dMtandardDeviation)
						{
							//求均值
							rehightOld = average(rehighData);
							vector<double>().swap(rehighData);
							bFristResult = true;
						}
						else
						{
							//输入下一组数据
							vector<double>().swap(rehighData);
						}
					}
					else
					{
						double pfcount = StandardDeviation(rehighData);//求标准差																		
						if (pfcount >= 0 && pfcount < dMtandardDeviation)//判断标准差是否符合小于条件a   
						{
							meanhigh = average(rehighData);//求均值							
							if (abs(meanhigh - rehightOld) < dReducedValue)//均值与第一次的参考值比较
							{
								//发送参考值
								meanhigh = rehightOld;
							}
							else
							{
								//发送均值；参考值=均值
								rehightOld = meanhigh;
							}
						}
						else//发送参考值；
						{
							meanhigh = rehightOld;
						}
						vector<double>().swap(rehighData);//清空数据
					}
					if (send_Data2.size() < 1)
					{
						PositonResult tp_result;
						tp_result.result = meanresult;
						tp_result.position = meanhigh;
						send_Data2.push(tp_result);
					}
				}
			}

			vcout++;
			CString countsrc1;
			countsrc1.Format("相机%d处理图像线程：纸边:(%d,%d)", nId, L[0], L[1]);
			//mlogger.TraceKeyInfo(countsrc1);
			//输出到列表框
			m_xclist.InsertString(0, countsrc1);
			m_xclist.SetCurSel(0);

			endTimeOne = std::chrono::high_resolution_clock::now();
			elapsed_millisecondsalOne = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeOne - beginTimeOne);
			mlogger.TraceInfo(_T("xxxx--相机%d单次时间：%.2lf ms"), nId, elapsed_millisecondsalOne.count());

		}

	}
	mlogger.TraceKeyInfo("相机%d处理图像线程：线程结束", nId);

}

//保存原图和结果图
void YZJPdetectsystemDlg::saveImage2()
{
	int coutsave = 0;
	int coutsave2 = 0;
	stop_saveImage2 = false;
	while (!stop_saveImage2)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

#if 1
		//保存原图
		if (!srcMat_save2.empty())
		{

			Mat midimage;
		//	srcMat_save2.wait_and_pop(midimage);

			int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum_save"), 10, lpPath);

			if (coutsave == sleepNum)
			{
				srcMat_save2.wait_and_pop(midimage);

				//选择保存图像的格式，0-jpg,1-all bmp,2-halfImage bmp, 3-fullImage bmp, 4-bbImage bmp
				int ImageFormat_choice = GetPrivateProfileInt("Set ImageFormat", "ImageFormat_choice", 0, lpPath);
				int SourceImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "SourceImageSave_choice", 1, lpPath);

				if (1 == SourceImageSave_choice)
				{
					SYSTEMTIME sys;
					GetLocalTime(&sys);
					char  strTime[200];
					memset(strTime, 0, 200 * sizeof(CHAR));
					sprintf(strTime, "%d%02d%02d%02d%02d%02d%03d",
						sys.wYear,
						sys.wMonth,
						sys.wDay,
						sys.wHour,
						sys.wMinute,
						sys.wSecond,
						sys.wMilliseconds);

					////2018.09.12
					//CTime t = CTime::GetCurrentTime();
					//string s = t.Format("D://imageFDZ//%Y-%m-%d");
					//CString path = s.c_str();
					//if (!PathIsDirectory(path))
					//{
					//	::CreateDirectory(path, 0);
					//}

					string sspath = m_s + "sourceimage2";
					CString path2 = sspath.c_str();
					if (!PathIsDirectory(path2))
					{
						::CreateDirectory(path2, 0);
					}

					char  strname[300];
					memset(strname, 0, 300 * sizeof(CHAR));

					if (1 == ImageFormat_choice || 2 == ImageFormat_choice)
					{
						sprintf(strname, "\\%s.bmp", strTime);
					}
					else
					{
						sprintf(strname, "\\%s.jpg", strTime);
					}


					std::string strnamepath = "";
					strnamepath = strname;
					strnamepath = sspath + strnamepath;
					imwrite(strnamepath, midimage);
				}
			}
			else
			{
				Mat pqmidimage;
				srcMat_save2.wait_and_pop(pqmidimage);
			}

			coutsave++;//必须放在此处

			if (coutsave > sleepNum)
			{
				coutsave = 0;//归0
			}
		}
#endif // 1

#if 0
		//保存结果图
		if (!dstMat_save2.empty())
		{
			//CString countsrc;
			//countsrc.Format(_T("结果存图1大小:%d\n"), dstMat_save2.size());
			//OutputDebugString(countsrc);

			Mat dstimage;

			int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum_save"), 10, lpPath);

			if (coutsave2 == sleepNum)
			{

				dstMat_save2.wait_and_pop(dstimage);

				//选择保存图像的格式，0-jpg,1-all bmp,2-halfImage bmp, 3-fullImage bmp, 4-bbImage bmp
				int ImageFormat_choice = GetPrivateProfileInt("Set ImageFormat", "ImageFormat_choice", 0, lpPath);
				//	int SourceImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "SourceImageSave_choice", 1, lpPath);
				int ResultImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "ResultImageSave_choice", 1, lpPath);
				if (1 == ResultImageSave_choice)
				{
					SYSTEMTIME sys;
					GetLocalTime(&sys);
					char  strTime[200];
					memset(strTime, 0, 200 * sizeof(CHAR));
					sprintf(strTime, "%d%02d%02d%02d%02d%02d%03d_1",
						sys.wYear,
						sys.wMonth,
						sys.wDay,
						sys.wHour,
						sys.wMinute,
						sys.wSecond,
						sys.wMilliseconds);

					//2018.09.12
					CTime t = CTime::GetCurrentTime();
					string s = t.Format("D://imageFDZ//%Y-%m-%d");
					CString path = s.c_str();
					if (!PathIsDirectory(path))
					{
						::CreateDirectory(path, 0);
					}

					string sspath = s + "//resultimage2";
					CString path2 = sspath.c_str();
					if (!PathIsDirectory(path2))
					{
						::CreateDirectory(path2, 0);
					}

					char  strname[300];
					memset(strname, 0, 300 * sizeof(CHAR));

					if (1 == ImageFormat_choice || 2 == ImageFormat_choice)
					{
						sprintf(strname, "//%s.bmp", strTime);
					}
					else
					{
						sprintf(strname, "//%s.jpg", strTime);
					}


					std::string strnamepath = "";
					strnamepath = strname;
					strnamepath = sspath + strnamepath;
					imwrite(strnamepath, dstimage);
				}
			}
			else
			{
				Mat pqmidimage;
				dstMat_save2.wait_and_pop(pqmidimage);
			}

			coutsave2++;//必须放在此处

			if (coutsave2 > sleepNum)
			{
				coutsave2 = 0;//归0
			}
		}
#endif // 1

	}

}

//单相机3图像拼接
void YZJPdetectsystemDlg::gatherImage3()
{
	stopgather3 = false;

	int coutsave = 0;

	Mat dstimage;
	while (!stopgather3)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

		if (srcMat_src3.empty())
			continue;

		{
			Mat midimage;
			srcMat_src3.wait_and_pop(midimage);
			int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum"), 10, lpPath);

			if (coutsave == sleepNum)
			{
				if (midMat_pro3.size() < 1)
				{
#if 1 //裁剪图像大小？

#endif
					midMat_pro3.push(midimage);//transprot image to process
					srcMat_save3.push(midimage);//transprot image to save 
				}
			}

			coutsave++;//必须放在此处

			if (coutsave > sleepNum)
			{
				coutsave = 0;//归0
			}
		}
	}
	
	mlogger.TraceKeyInfo("相机3采集线程：线程结束");
}

//检测算法3
void YZJPdetectsystemDlg::proImage3()
{
	int vcout = 0;
	double rehightOld = 0;//记录前次结果-综合结果
	bool bFristResult=false;
	vector<double> rehighData;
	vector<int> resultData;
	int minPos = 0, maxPos = 0;//边线临界值
	mlogger.TraceKeyInfo("相机3处理图像线程：线程开始");
	int nId = 3;
	vector<cv::Point> tmp_CalibTable = m_CalibTable3;
	int tmp_nTabItemCount = m_nTabItemCount3;
	stop_proImage3 = false;
	while (!stop_proImage3)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

		if (!midMat_pro3.empty())
		{
			std::chrono::time_point<std::chrono::steady_clock> beginTimeOne, endTimeOne;
			std::chrono::duration<double, std::milli> elapsed_millisecondsalOne;
			beginTimeOne = std::chrono::high_resolution_clock::now();

			Mat proImage;
			midMat_pro3.wait_and_pop(proImage);
			/*	flip(proImage, proImage,0);*/
				//画感兴趣区域ROI【用于传给算法的图像】	
			Vec4i L;//检边位置
			cv::Point m_loc(0, 0);
			double position = 0;
			int result = 0;
			int ImagePro_on = GetPrivateProfileInt("Set ImageFormat", "ImagePro_on", 1, lpPath);//算法开关
			int CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage3_x", 1, lpPath);//标定Rect
			int CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage3_y", 1, lpPath);//标定Rect
			int CaliImageW = GetPrivateProfileInt("Set ImageFormat", "CaliImage3_w", 100, lpPath);//标定Rect
			int CaliImageH = GetPrivateProfileInt("Set ImageFormat", "CaliImage3_h", 64, lpPath);//标定Rect
			int RunImageX = GetPrivateProfileInt("Set ImageFormat", "RunImage3_x", 1, lpPath);//标定Rect
			int RunImageY = GetPrivateProfileInt("Set ImageFormat", "RunImage3_y", 1, lpPath);//标定Rect
			int RunImageW = GetPrivateProfileInt("Set ImageFormat", "RunImage3_w", 100, lpPath);//标定Rect
			int RunImageH = GetPrivateProfileInt("Set ImageFormat", "RunImage3_h", 64, lpPath);//标定Rect
			int nMaxPic = GetPrivateProfileInt(_T("Set_Camera"), _T("useMaxPic"), 1, lpPath);;
			int ORG = GetPrivateProfileInt("Set ImageFormat", "Interval2", 1000, lpPath);//算法开关

			Rect rectROI = Rect(RunImageX, RunImageY, RunImageW, RunImageH);//感兴趣区域ROI
			Rect rectROI2 = Rect(CaliImageX, CaliImageY, CaliImageW, CaliImageH);//感兴趣区域ROI
			char chAttribute[20];
			memset(&chAttribute, 0, sizeof(chAttribute));
			GetPrivateProfileString("Set Detection Range", "StandardDeviation", "0", chAttribute, 20, lpPath);//标准差差
			double dMtandardDeviation = atof(chAttribute);
			memset(&chAttribute, 0, sizeof(chAttribute));
			GetPrivateProfileString("Set Detection Range", "ReducedValue", "0", chAttribute, 20, lpPath);//均值对比值
			double dReducedValue = atof(chAttribute);
				//modify 
			std::chrono::time_point<std::chrono::steady_clock> beginTime, endTime;
			std::chrono::duration<double, std::milli> elapsed_milliseconds;
			beginTime = std::chrono::high_resolution_clock::now();

			int resultShow = GetPrivateProfileInt(_T("Set Sfconfig"), _T("ResultShow"), 0, lpPath);
			if (resultShow)//显示图像
			{
				if (g_dlg->m_camSetDlg.IsWindowVisible() && g_dlg->m_camSetDlg.GetCamId() == nId)//在设置界面显示
				{
					if (ImagePro_on && !proImage.empty())
					{
						//图像截取
						Mat tmpImage = proImage(rectROI);
						CFindBound pFindBound;
						result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
						m_loc.x += rectROI.x;
						m_loc.y += rectROI.y;
						//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId, result, m_loc.x, m_loc.y);
						double dPaperWidth = 0;
						//计算物理位置
						if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
						{
							dPaperWidth = ComputePosition(nId, tmp_CalibTable, m_loc);
							//求位置
							position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
							//mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%d\n", nId,ORG / 2);
						}
						else
						{
							position = ORG / 2;
						}
						mlogger.TraceInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
					/*	L[0] = m_loc.x;
						L[1] = m_loc.y - 32;
						L[2] = m_loc.x;
						L[3] = m_loc.y + 32;*/
						L[0] = abs(m_loc.x);//初始化
						L[1] = RunImageY;
						L[2] = abs(m_loc.x);
						L[3] = RunImageY+RunImageH;


					}
					else
					{
						L[0] = RunImageX;//初始化
						L[1] = RunImageY;
						L[2] = RunImageX;
						L[3] = RunImageY + RunImageH;
					}
					Mat imgRGB;
					cv::cvtColor(proImage, imgRGB, COLOR_GRAY2RGB);
					Point p1(L[0], L[1] );
					Point p2(L[2], L[3]);
					//mlogger.TraceKeyInfo("xxxx--相机%d p1(%d ,%d) p2(%d,%d)\n", nId, L[0], L[1], L[2], L[3]);
					cv::line(imgRGB, p1, p2, Scalar(0, 0, 255), 8);
					g_dlg->m_camSetDlg.ShowImageToPic(imgRGB, IDC_CAM_PIC);
					imgRGB.release();
				}
				else//在主界面
				{
					if (ImagePro_on && !proImage.empty())
					{
						Mat tmpImage = proImage.clone();
						CFindBound pFindBound;
						result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
						//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId, result, m_loc.x, m_loc.y);
						double dPaperWidth = 0;
						//计算物理位置
						if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
						{
							if (m_loc.x > 0)
							{
								cv::Point tmp;
								tmp.x = m_loc.x + rectROI.x;
								tmp.y = m_loc.y + rectROI.y;
								dPaperWidth = ComputePosition(nId, tmp_CalibTable, tmp);
								//求位置
								position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
							}
							else
							{
								position = ORG / 2;
							}
						//	mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%2f,point1(%d,%d)\n", nId, double(ORG / 2), m_loc.x, m_loc.y);
							//L[0] = m_loc.x;//初始化
							//L[1] = m_loc.y - 32;
							//L[2] = m_loc.x;
							//L[3] = m_loc.y + 32;
							L[0] = abs(m_loc.x);//初始化
							L[1] = 0;
							L[2] = abs(m_loc.x);
							L[3] = RunImageH;
						}
						else
						{
							position = ORG / 2;
							//L[0] = m_loc.x;//初始化
							//L[1] = 0 ;
							//L[2] = m_loc.x;
							//L[3] = 64 ;
							L[0] = abs(m_loc.x);//初始化
							L[1] = 0;
							L[2] = abs(m_loc.x);
							L[3] = RunImageH;
						}
						mlogger.TraceInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
						
					}
					else
					{
						//L[0] = 1;//初始化
						//L[1] = 0;
						//L[2] = 1;
						//L[3] = 63;
						L[0] = abs(m_loc.x);//初始化
						L[1] = 0;
						L[2] = abs(m_loc.x);
						L[3] = RunImageH;
					}
					Mat imgRGB;
					cv::cvtColor(proImage, imgRGB, COLOR_GRAY2RGB);
					//画检测到的边线	
					Point p1(L[0], L[1]);
					Point p2(L[2], L[3]);
					//mlogger.TraceKeyInfo("xxxx--相机%d p1(%d ,%d) p2(%d,%d)\n", nId, L[0], L[1], L[2], L[3]);
					cv::line(imgRGB, p1, p2, Scalar(0, 0, 255), 8);
					ShowImageToPic(imgRGB, IDC_PIC_LEFT2);
					imgRGB.release();
				}
			}
			else//不显示图像
			{
				if (ImagePro_on && !proImage.empty())
				{
					Mat tmpImage = proImage.clone();
					CFindBound pFindBound;
					result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
					//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId,result, m_loc.x, m_loc.y);
					double dPaperWidth = 0;
					//计算物理位置
					if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
					{
						if (m_loc.x > 0)
						{
							cv::Point tmp;
							tmp.x = m_loc.x + rectROI.x;
							tmp.y = m_loc.y + rectROI.y;
							dPaperWidth = ComputePosition(nId, tmp_CalibTable, tmp);
							position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
						}
						else
						{
							position = ORG / 2;
						}
					//	mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%2f,point1(%d,%d)\n", nId, double(ORG / 2), m_loc.x, m_loc.y);
					}
					else
					{
						position = ORG / 2;
					}
					mlogger.TraceInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
					L[0] = m_loc.x;//初始化
					L[1] = m_loc.y - 32;
					L[2] = m_loc.x;
					L[3] = m_loc.y + 32;
				}
				else
				{
					L[0] = 1;//初始化
					L[1] = 0;
					L[2] = 1;
					L[3] = 64;
				}
			}

			endTime = std::chrono::high_resolution_clock::now();
			elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
			mlogger.TraceInfo(_T("xxxx--相机%d.算法时间：%.2lf ms"), nId, elapsed_milliseconds.count());
				//modify end

			//综合结果合成数量
			int result_num = GetPrivateProfileInt(_T("Set Sfconfig"), _T("result_num"), 15, lpPath);
			float meanhigh = 0.0;
			int meanresult = 0;
			if (rehighData.size() < result_num)//单次结果处理流程
			{
				if (position != 0)
				{
					resultData.push_back(result);
					if (result == 0 || position <= 0 || position > 1700/*&& (preposition != 0)*/)//滤掉异常值
					{
						//rehighData.push_back(preposition);
						rehighData.push_back(rehightOld);
					}
					else
					{
						rehighData.push_back(position);
					}
				}
			}
			else//综合结果处理
			{
				if (!rehighData.empty() && !resultData.empty())
				{
					if (!resultData.empty())
					{//当无纸状态大于个数的一半，则为无纸，否则为有纸
						int num = count(resultData.begin(), resultData.end(), 0);
						if (num > resultData.size() / 2)
							meanresult = 0;
						else
							meanresult = 1;
					}
					vector<int>().swap(resultData);

					if (!bFristResult)
					{
						double pfcount = StandardDeviation(rehighData);//求标准差
																	   //2、判断标准差是否符合小于条件a   
						//mlogger_sendData.TraceKeyInfo("xxx--第一次数据标准差%2lf", pfcount);
						if (pfcount>=0&&pfcount < dMtandardDeviation)
						{
							//求均值
							rehightOld = average(rehighData);
							vector<double>().swap(rehighData);
							bFristResult = true;
						}
						else
						{
							//输入下一组数据
							vector<double>().swap(rehighData);
						}
					}
					else
					{
						double pfcount = StandardDeviation(rehighData);//求标准差	
						//mlogger_sendData.TraceKeyInfo("xxx--标准差%2lf", pfcount);
						if (pfcount >= 0 && pfcount < dMtandardDeviation)//判断标准差是否符合小于条件a   
						{
							meanhigh = average(rehighData);//求均值							
							if (abs(meanhigh - rehightOld) < dReducedValue)//均值与第一次的参考值比较
							{
								//发送参考值
								meanhigh = rehightOld;
							}
							else
							{
								//发送均值；参考值=均值
								rehightOld = meanhigh;
							}
						}
						else//发送参考值；
						{
							meanhigh = rehightOld;
						}
						vector<double>().swap(rehighData);//清空数据
					}
					if (send_Data3.size() < 1)
					{
						PositonResult tp_result;
						tp_result.result = meanresult;
						tp_result.position = meanhigh;
						send_Data3.push(tp_result);
					}
				}
			
			}

			vcout++;
			CString countsrc1;
			countsrc1.Format("相机%d处理图像线程：纸边:(%d,%d)", nId, L[0], L[1]);
			//mlogger.TraceKeyInfo(countsrc1);
			//输出到列表框
			m_xclist.InsertString(0, countsrc1);
			m_xclist.SetCurSel(0);

			endTimeOne = std::chrono::high_resolution_clock::now();
			elapsed_millisecondsalOne = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeOne - beginTimeOne);
			mlogger.TraceInfo(_T("xxxx--相机%d单次时间：%.2lf ms"), nId, elapsed_millisecondsalOne.count());

		}

	}
	mlogger.TraceKeyInfo("相机%d处理图像线程：线程结束", nId);

}

//保存原图和结果图
void YZJPdetectsystemDlg::saveImage3()
{
	int coutsave = 0;
	int coutsave2 = 0;
	stop_saveImage3 = false;
	while (!stop_saveImage3)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

#if 1
		//保存原图
		if (!srcMat_save3.empty())
		{

			Mat midimage;
			//	srcMat_save2.wait_and_pop(midimage);

			int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum_save"), 10, lpPath);

			if (coutsave == sleepNum)
			{
				srcMat_save3.wait_and_pop(midimage);

				//选择保存图像的格式，0-jpg,1-all bmp,2-halfImage bmp, 3-fullImage bmp, 4-bbImage bmp
				int ImageFormat_choice = GetPrivateProfileInt("Set ImageFormat", "ImageFormat_choice", 0, lpPath);
				int SourceImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "SourceImageSave_choice", 1, lpPath);

				if (1 == SourceImageSave_choice)
				{
					SYSTEMTIME sys;
					GetLocalTime(&sys);
					char  strTime[200];
					memset(strTime, 0, 200 * sizeof(CHAR));
					sprintf(strTime, "%d%02d%02d%02d%02d%02d%03d",
						sys.wYear,
						sys.wMonth,
						sys.wDay,
						sys.wHour,
						sys.wMinute,
						sys.wSecond,
						sys.wMilliseconds);


					string sspath = m_s + "sourceimage3";
					CString path2 = sspath.c_str();
					if (!PathIsDirectory(path2))
					{
						::CreateDirectory(path2, 0);
					}

					char  strname[300];
					memset(strname, 0, 300 * sizeof(CHAR));

					if (1 == ImageFormat_choice || 2 == ImageFormat_choice)
					{
						sprintf(strname, "\\%s.bmp", strTime);
					}
					else
					{
						sprintf(strname, "\\%s.jpg", strTime);
					}


					std::string strnamepath = "";
					strnamepath = strname;
					strnamepath = sspath + strnamepath;
					imwrite(strnamepath, midimage);
				}
			}
			else
			{
				Mat pqmidimage;
				srcMat_save3.wait_and_pop(pqmidimage);
			}

			coutsave++;//必须放在此处

			if (coutsave > sleepNum)
			{
				coutsave = 0;//归0
			}
		}
#endif // 1

#if 0
		//保存结果图
		if (!dstMat_save3.empty())
		{
			//CString countsrc;
			//countsrc.Format(_T("结果存图1大小:%d\n"), dstMat_save2.size());
			//OutputDebugString(countsrc);

			Mat dstimage;

			int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum_save"), 10, lpPath);

			if (coutsave2 == sleepNum)
			{

				dstMat_save3.wait_and_pop(dstimage);

				//选择保存图像的格式，0-jpg,1-all bmp,2-halfImage bmp, 3-fullImage bmp, 4-bbImage bmp
				int ImageFormat_choice = GetPrivateProfileInt("Set ImageFormat", "ImageFormat_choice", 0, lpPath);
				//	int SourceImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "SourceImageSave_choice", 1, lpPath);
				int ResultImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "ResultImageSave_choice", 1, lpPath);
				if (1 == ResultImageSave_choice)
				{
					SYSTEMTIME sys;
					GetLocalTime(&sys);
					char  strTime[200];
					memset(strTime, 0, 200 * sizeof(CHAR));
					sprintf(strTime, "%d%02d%02d%02d%02d%02d%03d",
						sys.wYear,
						sys.wMonth,
						sys.wDay,
						sys.wHour,
						sys.wMinute,
						sys.wSecond,
						sys.wMilliseconds);
				
					string sspath = m_s + "\\resultimage3";
					CString path = sspath.c_str();
					if (!PathIsDirectory(path))
					{
						::CreateDirectory(path, 0);
					}
					char  strname[300];
					memset(strname, 0, 300 * sizeof(CHAR));

					if (1 == ImageFormat_choice || 2 == ImageFormat_choice)
					{
						sprintf(strname, "\\%s.bmp", strTime);
					}
					else
					{
						sprintf(strname, "\\%s.jpg", strTime);
					}


					std::string strnamepath = "";
					strnamepath = strname;
					strnamepath = sspath + strnamepath;
					imwrite(strnamepath, dstimage);
				}
			}
			else
			{
				Mat pqmidimage;
				dstMat_save3.wait_and_pop(pqmidimage);
			}

			coutsave2++;//必须放在此处

			if (coutsave2 > sleepNum)
			{
				coutsave2 = 0;//归0
			}
		}
#endif // 1

	}

}

//单相机4图像拼接
void YZJPdetectsystemDlg::gatherImage4()
{
	stopgather4 = false;

	int coutsave = 0;

	Mat dstimage;
	while (!stopgather4)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

		if (srcMat_src4.empty())
			continue;

		{
			Mat midimage;
			srcMat_src4.wait_and_pop(midimage);
			int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum"), 10, lpPath);

			if (coutsave == sleepNum)
			{
				if (midMat_pro4.size() < 1)
				{
#if 1 //裁剪图像大小？
					//Mat roi3 = midimage(Rect(10, 10, 5368, 100));
					//midMat_pro4.push(roi3);//transprot image to process
					//srcMat_save4.push(roi3);//transprot image to save 
#endif
					midMat_pro4.push(midimage);//transprot image to process
					srcMat_save4.push(midimage);//transprot image to save 
				}
			}

			coutsave++;//必须放在此处

			if (coutsave > sleepNum)
			{
				coutsave = 0;//归0
			}
		}
	}

	mlogger.TraceKeyInfo("相机4采集线程：线程结束");
}

//检测算法4
void YZJPdetectsystemDlg::proImage4()
{
	int vcout = 0;
	double rehightOld = 0;//记录前次结果-综合结果
	bool bFristResult = false;
	vector<double> rehighData;
	vector<int> resultData;
	int minPos = 0, maxPos = 0;//边线临界值
	int nId = 4;
	vector<cv::Point> tmp_CalibTable = m_CalibTable4;
	int tmp_nTabItemCount = m_nTabItemCount4;
	mlogger.TraceKeyInfo("相机4处理图像线程：线程开始");

	stop_proImage4 = false;
	while (!stop_proImage4)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

		if (!midMat_pro4.empty())
		{
			std::chrono::time_point<std::chrono::steady_clock> beginTimeOne, endTimeOne;
			std::chrono::duration<double, std::milli> elapsed_millisecondsalOne;
			beginTimeOne = std::chrono::high_resolution_clock::now();

			Mat proImage;
			midMat_pro4.wait_and_pop(proImage);
			/*	flip(proImage, proImage,0);*/
				//画感兴趣区域ROI【用于传给算法的图像】	
			Vec4i L;//检边位置
			cv::Point m_loc(0, 0);
			double position = 0;
			int result = 0;
			int ImagePro_on = GetPrivateProfileInt("Set ImageFormat", "ImagePro_on", 1, lpPath);//算法开关
			int CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage4_x", 1, lpPath);//标定Rect
			int CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage4_y", 1, lpPath);//标定Rect
			int CaliImageW = GetPrivateProfileInt("Set ImageFormat", "CaliImage4_w", 100, lpPath);//标定Rect
			int CaliImageH = GetPrivateProfileInt("Set ImageFormat", "CaliImage4_h", 64, lpPath);//标定Rect
			int RunImageX = GetPrivateProfileInt("Set ImageFormat", "RunImage4_x", 1, lpPath);//标定Rect
			int RunImageY = GetPrivateProfileInt("Set ImageFormat", "RunImage4_y", 1, lpPath);//标定Rect
			int RunImageW = GetPrivateProfileInt("Set ImageFormat", "RunImage4_w", 100, lpPath);//标定Rect
			int RunImageH = GetPrivateProfileInt("Set ImageFormat", "RunImage4_h", 64, lpPath);//标定Rect
			int nMaxPic = GetPrivateProfileInt(_T("Set_Camera"), _T("useMaxPic"), 1, lpPath);;
			int ORG = GetPrivateProfileInt("Set ImageFormat", "Interval2", 1000, lpPath);//算法开关

			Rect rectROI = Rect(RunImageX, RunImageY, RunImageW, RunImageH);//感兴趣区域ROI
			Rect rectROI2 = Rect(CaliImageX, CaliImageY, CaliImageW, CaliImageH);//感兴趣区域ROI
			char chAttribute[20];
			memset(&chAttribute, 0, sizeof(chAttribute));
			GetPrivateProfileString("Set Detection Range", "StandardDeviation", "0", chAttribute, 20, lpPath);//标准差差
			double dMtandardDeviation = atof(chAttribute);
			memset(&chAttribute, 0, sizeof(chAttribute));
			GetPrivateProfileString("Set Detection Range", "ReducedValue", "0", chAttribute, 20, lpPath);//均值对比值
			double dReducedValue = atof(chAttribute);
				//modify 
			std::chrono::time_point<std::chrono::steady_clock> beginTime, endTime;
			std::chrono::duration<double, std::milli> elapsed_milliseconds;
			beginTime = std::chrono::high_resolution_clock::now();

			int resultShow = GetPrivateProfileInt(_T("Set Sfconfig"), _T("ResultShow"), 0, lpPath);
			if (resultShow)//显示图像
			{
				if (g_dlg->m_camSetDlg.IsWindowVisible() && g_dlg->m_camSetDlg.GetCamId() == nId)//在设置界面显示
				{
					if (ImagePro_on && !proImage.empty())
					{
						//图像截取
						Mat tmpImage = proImage(rectROI);
						CFindBound pFindBound;
						result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
						m_loc.x += rectROI.x;
						m_loc.y += rectROI.y;
						//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId, result, m_loc.x, m_loc.y);
						double dPaperWidth = 0;
						//计算物理位置
						if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
						{
							dPaperWidth = ComputePosition(nId, tmp_CalibTable, m_loc);
							//求位置
							position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
						//	mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%d\n", nId, ORG / 2);
						}
						else
						{
							position = ORG / 2;
						}
						mlogger.TraceInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
				/*		L[0] = m_loc.x;
						L[1] = m_loc.y - 32;
						L[2] = m_loc.x;
						L[3] = m_loc.y + 32;*/
						L[0] = abs(m_loc.x);//初始化
						L[1] = RunImageY;
						L[2] = abs(m_loc.x);
						L[3] = RunImageY+RunImageH;


					}
					else
					{
						L[0] = RunImageX;//初始化
						L[1] = RunImageY;
						L[2] = RunImageX;
						L[3] = RunImageY + RunImageH;
					}
					Mat imgRGB;
					cv::cvtColor(proImage, imgRGB, COLOR_GRAY2RGB);
					Point p1(L[0] , L[1] );
					Point p2(L[2], L[3] );
					mlogger.TraceInfo("xxxx--相机%d p1(%d ,%d) p2(%d,%d)\n", nId, L[0], L[1], L[2], L[3]);
					cv::line(imgRGB, p1, p2, Scalar(0, 0, 255), 8);
					g_dlg->m_camSetDlg.ShowImageToPic(imgRGB, IDC_CAM_PIC);
					imgRGB.release();
				}
				else//在主界面
				{
					if (ImagePro_on && !proImage.empty())
					{
						Mat tmpImage = proImage.clone();
						CFindBound pFindBound;
						result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
						//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId, result, m_loc.x, m_loc.y);
						double dPaperWidth = 0;
						//计算物理位置
						if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
						{
							if (m_loc.x > 0)
							{
								cv::Point tmp;
								tmp.x = m_loc.x + rectROI.x;
								tmp.y = m_loc.y + rectROI.y;
								dPaperWidth = ComputePosition(nId, tmp_CalibTable, tmp);
								//求位置
								position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
							}
							else
							{
								position = ORG / 2;
							}
						//	mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%2f,point1(%d,%d)\n", nId, double(ORG / 2), m_loc.x, m_loc.y);
							//L[0] = m_loc.x;//初始化
							//L[1] = m_loc.y - 32;
							//L[2] = m_loc.x;
							//L[3] = m_loc.y + 32;
							L[0] = abs(m_loc.x);//初始化
							L[1] = 0;
							L[2] = abs(m_loc.x);
							L[3] = RunImageH;
						}
						else
						{
							position = ORG / 2;
							//L[0] = m_loc.x;//初始化
							//L[1] = 0;
							//L[2] = m_loc.x;
							//L[3] = 64;
							L[0] = abs(m_loc.x);//初始化
							L[1] = 0;
							L[2] = abs(m_loc.x);
							L[3] = RunImageH;
						}
						mlogger.TraceInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
					
					}
					else
					{
						//L[0] = 1;//初始化
						//L[1] = 0;
						//L[2] = 1;
						//L[3] = 63;
						L[0] = 1;//初始化
						L[1] = 0;
						L[2] = 1;
						L[3] = RunImageH;
					}
					Mat imgRGB;
					cv::cvtColor(proImage, imgRGB, COLOR_GRAY2RGB);
					//画检测到的边线	
					Point p1(L[0], L[1]);
					Point p2(L[2], L[3]);
					cv::line(imgRGB, p1, p2, Scalar(0, 0, 255), 8);
					ShowImageToPic(imgRGB, IDC_PIC_RIGHT2);
					imgRGB.release();
				}
			}
			else//不显示图像
			{
				if (ImagePro_on && !proImage.empty())
				{
					Mat tmpImage = proImage.clone();
					CFindBound pFindBound;
					result = pFindBound.BoundLocWithState(tmpImage, m_loc, nId);
					//mlogger.TraceKeyInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d)\n", nId, result, m_loc.x, m_loc.y);
					double dPaperWidth = 0;
					//计算物理位置
					if (result == 1 && tmp_nTabItemCount > 0 && tmp_CalibTable.size() > 0)
					{
						if (m_loc.x > 0)
						{
							cv::Point tmp;
							tmp.x = m_loc.x + rectROI.x;
							tmp.y = m_loc.y + rectROI.y;
							dPaperWidth = ComputePosition(nId, tmp_CalibTable, tmp);
							position = ORG / 2 + dPaperWidth;	///第一个相机中纸的位置
						}
						else
						{
							position = ORG / 2;
						}
			//			mlogger.TraceKeyInfo("xxxx--相机%d视野盲区=%2f,point1(%d,%d)\n", nId, double(ORG / 2), m_loc.x, m_loc.y);
					}
					else
					{
						position = ORG / 2;
					}
					mlogger.TraceInfo("xxxx--相机%d算法结束 result=%d  m_loc(%d,%d),物理位置纸宽度%2f,位置position=%2f\n", nId, result, m_loc.x, m_loc.y, dPaperWidth, position);
					L[0] = m_loc.x;//初始化
					L[1] = m_loc.y - 32;
					L[2] = m_loc.x;
					L[3] = m_loc.y + 32;
				}
				else
				{
					L[0] = 1;//初始化
					L[1] = 0;
					L[2] = 1;
					L[3] = 64;
				}
			}

			endTime = std::chrono::high_resolution_clock::now();
			elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime);
			mlogger.TraceInfo(_T("xxxx--相机%d算法时间：%.2lf ms"), nId, elapsed_milliseconds.count());
			//modify end

			//综合结果合成数量
			int result_num = GetPrivateProfileInt(_T("Set Sfconfig"), _T("result_num"), 15, lpPath);
			float meanhigh = 0.0;
			int meanresult = 0;
			if (rehighData.size() < result_num)//单次结果处理流程
			{
				if (position != 0)
				{
					resultData.push_back(result);
					if (result == 0 || position <= 0 || position > 1600/*&& (preposition != 0)*/)//滤掉异常值
					{
						//rehighData.push_back(preposition);
						rehighData.push_back(rehightOld);
					}
					else
					{
						rehighData.push_back(position);
					}
				}

			}
			else//综合结果处理
			{

				if (!rehighData.empty() && !resultData.empty())
				{
					if (!resultData.empty())
					{//当无纸状态大于个数的一半，则为无纸，否则为有纸
						int num = count(resultData.begin(), resultData.end(), 0);
						if (num > resultData.size() / 2)
							meanresult = 0;
						else
							meanresult = 1;
					}
					vector<int>().swap(resultData);

					if (!bFristResult)
					{
						double pfcount = StandardDeviation(rehighData);//求标准差
																	   //2、判断标准差是否符合小于条件a   
						if (pfcount >= 0 && pfcount < dMtandardDeviation)
						{
							//求均值
							rehightOld = average(rehighData);
						
							bFristResult = true;
						}					
							//输入下一组数据
						vector<double>().swap(rehighData);
						
					}
					else
					{
						double pfcount = StandardDeviation(rehighData);//求标准差																		
						if (pfcount >= 0 && pfcount < dMtandardDeviation)//判断标准差是否符合小于条件a   
						{
							meanhigh = average(rehighData);//求均值							
							if (abs(meanhigh - rehightOld) < dReducedValue)//均值与第一次的参考值比较
							{
								//发送参考值
								meanhigh = rehightOld;
							}
							else
							{
								//发送均值；参考值=均值
								rehightOld = meanhigh;
							}
						}
						else//发送参考值；
						{
							meanhigh = rehightOld;
						}
						vector<double>().swap(rehighData);//清空数据
					}
					if (send_Data4.size() < 1)
					{
						PositonResult tp_result;
						tp_result.result = meanresult;
						tp_result.position = meanhigh;
						send_Data4.push(tp_result);
					}
				}

			}

			vcout++;
			CString countsrc1;
			countsrc1.Format("相机%d处理图像线程：纸边:(%d,%d)", nId, L[0], L[1]);
			//mlogger.TraceKeyInfo(countsrc1);
			//输出到列表框
			m_xclist.InsertString(0, countsrc1);
			m_xclist.SetCurSel(0);

			endTimeOne = std::chrono::high_resolution_clock::now();
			elapsed_millisecondsalOne = std::chrono::duration_cast<std::chrono::milliseconds>(endTimeOne - beginTimeOne);
			mlogger.TraceInfo(_T("xxxx--相机%d单次时间：%.2lf ms"), nId, elapsed_millisecondsalOne.count());

		}

	}
	mlogger.TraceKeyInfo("相机4处理图像线程：线程结束");

}

//保存原图和结果图
void YZJPdetectsystemDlg::saveImage4()
{
	int coutsave = 0;
	int coutsave2 = 0;
	stop_saveImage4 = false;
	while (!stop_saveImage4)
	{
		Sleep(1);//休息1ms，降低cpu占用率 

#if 1
		//保存原图
		if (!srcMat_save4.empty())
		{

			Mat midimage;
			//	srcMat_save2.wait_and_pop(midimage);

			int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum_save"), 10, lpPath);

			if (coutsave == sleepNum)
			{
				srcMat_save4.wait_and_pop(midimage);

				//选择保存图像的格式，0-jpg,1-all bmp,2-halfImage bmp, 3-fullImage bmp, 4-bbImage bmp
				int ImageFormat_choice = GetPrivateProfileInt("Set ImageFormat", "ImageFormat_choice", 0, lpPath);
				int SourceImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "SourceImageSave_choice", 1, lpPath);

				if (1 == SourceImageSave_choice)
				{
					SYSTEMTIME sys;
					GetLocalTime(&sys);
					char  strTime[200];
					memset(strTime, 0, 200 * sizeof(CHAR));
					sprintf(strTime, "%d%02d%02d%02d%02d%02d%03d",
						sys.wYear,
						sys.wMonth,
						sys.wDay,
						sys.wHour,
						sys.wMinute,
						sys.wSecond,
						sys.wMilliseconds);

					////2018.09.12
					//CTime t = CTime::GetCurrentTime();
					//string s = t.Format("D://imageFDZ//%Y-%m-%d");
					//CString path = s.c_str();
					//if (!PathIsDirectory(path))
					//{
					//	::CreateDirectory(path, 0);
					//}

					string sspath = m_s + "sourceimage4";
					CString path2 = sspath.c_str();
					if (!PathIsDirectory(path2))
					{
						::CreateDirectory(path2, 0);
					}

					char  strname[300];
					memset(strname, 0, 300 * sizeof(CHAR));

					if (1 == ImageFormat_choice || 2 == ImageFormat_choice)
					{
						sprintf(strname, "\\%s.bmp", strTime);
					}
					else
					{
						sprintf(strname, "\\%s.jpg", strTime);
					}


					std::string strnamepath = "";
					strnamepath = strname;
					strnamepath = sspath + strnamepath;
					imwrite(strnamepath, midimage);
				}
			}
			else
			{
				Mat pqmidimage;
				srcMat_save4.wait_and_pop(pqmidimage);
			}

			coutsave++;//必须放在此处

			if (coutsave > sleepNum)
			{
				coutsave = 0;//归0
			}
		}
#endif // 1

#if 0
		//保存结果图
		if (!dstMat_save4.empty())
		{
			//CString countsrc;
			//countsrc.Format(_T("结果存图1大小:%d\n"), dstMat_save2.size());
			//OutputDebugString(countsrc);

			Mat dstimage;

			int sleepNum = GetPrivateProfileInt(_T("Set Sfconfig"), _T("sleepNum_save"), 10, lpPath);

			if (coutsave2 == sleepNum)
			{

				dstMat_save4.wait_and_pop(dstimage);

				//选择保存图像的格式，0-jpg,1-all bmp,2-halfImage bmp, 3-fullImage bmp, 4-bbImage bmp
				int ImageFormat_choice = GetPrivateProfileInt("Set ImageFormat", "ImageFormat_choice", 0, lpPath);
				//	int SourceImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "SourceImageSave_choice", 1, lpPath);
				int ResultImageSave_choice = GetPrivateProfileInt("Set ImageFormat", "ResultImageSave_choice", 1, lpPath);
				if (1 == ResultImageSave_choice)
				{
					SYSTEMTIME sys;
					GetLocalTime(&sys);
					char  strTime[200];
					memset(strTime, 0, 200 * sizeof(CHAR));
					sprintf(strTime, "%d%02d%02d%02d%02d%02d%03d_1",
						sys.wYear,
						sys.wMonth,
						sys.wDay,
						sys.wHour,
						sys.wMinute,
						sys.wSecond,
						sys.wMilliseconds);

					//2018.09.12
					CTime t = CTime::GetCurrentTime();
					string s = t.Format("D://imageFDZ//%Y-%m-%d");
					CString path = s.c_str();
					if (!PathIsDirectory(path))
					{
						::CreateDirectory(path, 0);
					}

					string sspath = s + "//resultimage4";
					CString path2 = sspath.c_str();
					if (!PathIsDirectory(path2))
					{
						::CreateDirectory(path2, 0);
					}

					char  strname[300];
					memset(strname, 0, 300 * sizeof(CHAR));

					if (1 == ImageFormat_choice || 2 == ImageFormat_choice)
					{
						sprintf(strname, "//%s.bmp", strTime);
					}
					else
					{
						sprintf(strname, "//%s.jpg", strTime);
					}


					std::string strnamepath = "";
					strnamepath = strname;
					strnamepath = sspath + strnamepath;
					imwrite(strnamepath, dstimage);
				}
			}
			else
			{
				Mat pqmidimage;
				dstMat_save4.wait_and_pop(pqmidimage);
			}

			coutsave2++;//必须放在此处

			if (coutsave2 > sleepNum)
			{
				coutsave2 = 0;//归0
			}
		}
#endif // 1

	}

}

//分开发送
#if 1
//modbus发送数据线程函数
void YZJPdetectsystemDlg::modbus2PLC()
{

#if 1

	CString countsrc1;

	thread th_m2plc(&YZJPdetectsystemDlg::modbus2Connect, this);//连接plc1的线程函数
	th_m2plc.detach();

	int btaddr = GetPrivateProfileInt(_T("Commu"), _T("Devaddr"), 0, lpPath);

	int wAllCount = 0;

	int wSendcout = 0;
	int position1 = 0;
	int position2 = 0;
	int position3 = 0;
	int position4 = 0;
	int width1 = abs(position1) + abs(position2);
	int width2 = abs(position3) + abs(position4);
	int isPaper1 = 0, isPaper2 = 0;

	//stop_sendData2plc = false;
	while (!stop_sendData2plc)
	{
#if 1
		Sleep(250);
		if (!send_Data.empty() && !send_Data2.empty() && !send_Data3.empty() && !send_Data4.empty())
		{
			PositonResult tp_result1;
			PositonResult tp_result2;
			PositonResult tp_result3;
			PositonResult tp_result4;
			send_Data.wait_and_pop(tp_result1);
			send_Data2.wait_and_pop(tp_result2);
			send_Data3.wait_and_pop(tp_result3);
			send_Data4.wait_and_pop(tp_result4);
			//操作侧驱动侧调换
			int OStransDS_on = GetPrivateProfileInt("Set Sfconfig", "OStransDS_on", 1, lpPath);//调换方向开关
			if (OStransDS_on)
			{
				PositonResult tmp;
				tmp = tp_result1;
				tp_result1 = tp_result2;
				tp_result2 = tmp;
				tmp = tp_result3;
				tp_result3 = tp_result4;
				tp_result4 = tmp;
			}
			tp_result1.position = -tp_result1.position;
			tp_result3.position = -tp_result3.position;
			//mlogger.TraceKeyInfo("modbus发送数据线程：获取数据结束\n");
			CString strbb_value;
			strbb_value.Format("%d", wSendcout);
			GetDlgItem(IDC_EDIT_SEND_COUNT)->SetWindowText(strbb_value);
			strbb_value.Format("%2f", tp_result1.position);
			GetDlgItem(IDC_EDIT_POSITION1)->SetWindowText(strbb_value);
			strbb_value.Format("%s", tp_result1.result == 1 ? "有纸" : "无纸");
			GetDlgItem(IDC_EDIT_PAPER1)->SetWindowText(strbb_value);
			strbb_value.Format("%2f", tp_result2.position);
			GetDlgItem(IDC_EDIT_POSITION2)->SetWindowText(strbb_value);
			strbb_value.Format("%s", tp_result2.result == 1 ? "有纸" : "无纸");
			GetDlgItem(IDC_EDIT_PAPER2)->SetWindowText(strbb_value);
			strbb_value.Format("%2f", tp_result3.position);
			GetDlgItem(IDC_EDIT_POSITION3)->SetWindowText(strbb_value);
			strbb_value.Format("%s", tp_result3.result == 1 ? "有纸" : "无纸");
			GetDlgItem(IDC_EDIT_PAPER3)->SetWindowText(strbb_value);
			strbb_value.Format("%2f", tp_result4.position);
			GetDlgItem(IDC_EDIT_POSITION4)->SetWindowText(strbb_value);
			strbb_value.Format("%s", tp_result4.result == 1 ? "有纸" : "无纸");
			GetDlgItem(IDC_EDIT_PAPER4)->SetWindowText(strbb_value);
			strbb_value.Format("%2f", (abs(tp_result1.position) + abs(tp_result2.position)));
			GetDlgItem(IDC_EDIT_WZFK)->SetWindowText(strbb_value);
			strbb_value.Format("%2f", (abs(tp_result3.position) + abs(tp_result4.position)));
			GetDlgItem(IDC_EDIT_XZFK)->SetWindowText(strbb_value);
			UpdateData(false);
			//mlogger.TraceKeyInfo("modbus发送数据线程：更新界面\n");
			//发送plc1
			int position1_tmp = (int)(tp_result1.position * 10);
			int position2_tmp = (int)(tp_result2.position * 10);
			int position3_tmp = (int)(tp_result3.position * 10);
			int position4_tmp = (int)(tp_result4.position * 10);
			int width1_tmp = abs(position1_tmp) + abs(position2_tmp);
			int width2_tmp = abs(position3_tmp) + abs(position4_tmp);
			int isPaper1_tmp = 0, isPaper2_tmp = 0;
			if (tp_result1.result == 1 && tp_result2.result == 1)
				isPaper1_tmp = 1;
			if (tp_result3.result == 1 && tp_result4.result == 1)
				isPaper2_tmp = 1;
			//if(position1_tmp != position1) 
				position1 = position1_tmp;				
			//if(position2_tmp != position2) 
				position2 = position2_tmp;
			//if(position3_tmp != position3) 
				position3 = position3_tmp; 
			//if(position4_tmp != position4) 
				position4 = position4_tmp;
			//if(width1_tmp != width1) 
				width1 = width1_tmp;
			//if(width2_tmp != width2) 
				width2 = width2_tmp;
			//if(isPaper1_tmp != isPaper1) 
				isPaper1 = isPaper1_tmp; 
			//if(isPaper2_tmp != isPaper2)
				isPaper2 = isPaper2_tmp;
			
								
			if (ready_wait)//为真，连接失败，重新连接
			{
				mlogger.TraceKeyInfo("modbus发送数据线程：modubus正在连接plc1中");
				mlogger_sendData.TraceInfo("modbus发送数据线程：modubus正在连接plc1中");
			}
			else
			{
				if (isPaper1 == 0 || isPaper2 == 0)
				{
					countsrc1.Format("@@@modbus发送数据线程：不发送---1号机%s：%d,2号机%s：%d,3号机%s：%d,4号机%s：%d @@@",  tp_result1.result == 1 ? "有纸" : "无纸", position1, \
						tp_result2.result == 1 ? "有纸" : "无纸", position2, tp_result3.result == 1 ? "有纸" : "无纸", position3, \
						tp_result4.result == 1 ? "有纸" : "无纸", position4);
					mlogger.TraceKeyInfo(countsrc1);
					mlogger_sendData.TraceInfo(countsrc1);//记录发送数据
					m_xclist.InsertString(0, countsrc1);
					m_xclist.SetCurSel(0);
				}
				else
				{
					uint16_t tab_reg[20];
					tab_reg[0] = wSendcout;
					tab_reg[1] = position1;
					tab_reg[2] = position2;
					tab_reg[3] = width1;
					tab_reg[4] = isPaper1;
					tab_reg[5] = 0;
					tab_reg[6] = 0;
					tab_reg[7] = 0;
					tab_reg[8] = 0;
					tab_reg[9] = 0;
					tab_reg[10] = position3;
					tab_reg[11] = position4;
					tab_reg[12] = width2;
					tab_reg[13] = isPaper2;
					tab_reg[14] = 0;
					tab_reg[15] = 0;
					tab_reg[16] = 0;
					tab_reg[17] = 0;
					tab_reg[18] = 0;
					tab_reg[19] = 0;
			
					int iErrMsg = 0;
					iErrMsg = modbus_write_registers(m_mymb, btaddr, 20, tab_reg);;
					if (-1==iErrMsg)
					{
						mlogger.TraceKeyInfo("modbus发送数据线程：数据发送错误，可能网络中断,重新连接网络:%s", modbus_strerror(errno));
						mlogger_sendData.TraceInfo("modbus发送数据线程：数据发送错误，可能网络中断,重新连接网络:%s", modbus_strerror(errno));
					
						thread th_hx(&YZJPdetectsystemDlg::hxthread, this);//唤醒线程开始,解除等待
						th_hx.detach();
					}
					countsrc1.Format("@@@modbus发送数据线程%7d：1号机%s：%d,2号机%s：%d,3号机%s：%d,4号机%s：%d @@@", wSendcout, tp_result1.result == 1 ? "有纸" : "无纸", position1, \
						tp_result2.result == 1 ? "有纸" : "无纸", position2, tp_result3.result == 1 ? "有纸" : "无纸", position3, \
						tp_result4.result == 1 ? "有纸" : "无纸", position4);
					mlogger.TraceKeyInfo(countsrc1);
					mlogger_sendData.TraceInfo(countsrc1);
					m_xclist.InsertString(0, countsrc1);
					m_xclist.SetCurSel(0);
					wAllCount++;
					wSendcout++;//心跳值
					if (wSendcout > 20000)
					{
						wSendcout = 0;
					}
				}
			}
		}

#endif
	}
	
		//mlogger.TraceKeyInfo("释放modbus通讯字");
		//mlogger_sendData.TraceKeyInfo("释放modbus通讯字");
		//modbus_close(m_mymb);
		//modbus_free(m_mymb);
	
#endif // 0
}

#endif // 1

#if 1
//返回值0-成功，1-超时
void YZJPdetectsystemDlg::modbus2Connect()
{
	 stop_modbus2plc=false;
	while (!stop_modbus2plc)
	{
	//	Sleep(1);
	//	mlogger.TraceKeyInfo("1\n1\n1\n1\n.");
		//线程中断
		std::unique_lock <std::mutex> lck(mtx);
		cv_wait.wait(lck, [this] {return ready_wait; });
		//

			//服务器1
		CString countsrc1;
		LPSTR  sServerIp = new char[150];
		GetPrivateProfileString(_T("Commu"), _T("IPAddr"), _T("127.0.0.1"), sServerIp, 31, lpPath);
		int wServerPort = GetPrivateProfileInt(_T("Commu"), _T("Port"), 502, lpPath);
		int btDevNo = GetPrivateProfileInt(_T("Commu"), _T("DevNo"), 0, lpPath);
		m_mymb = modbus_new_tcp(sServerIp, wServerPort);//创建连接
		if (m_mymb == NULL)
		{
		/*	fprintf(stderr, "new tcp failed: %s\n", modbus_strerror(errno));*/
			countsrc1.Format("modbus发送数据线程：创建sServerIp:%s, wServerPort:%d, btDevNo:%d 失败:%s.", sServerIp, wServerPort, btDevNo, modbus_strerror(errno));
			mlogger.TraceKeyInfo(countsrc1);
			mlogger_sendData.TraceKeyInfo(countsrc1);
			stop_sendData2plc = true;
		}
		else
		{
			countsrc1.Format("modbus发送数据线程：创建sServerIp:%s, wServerPort:%d, btDevNo:%d 成功.", sServerIp, wServerPort, btDevNo);
			mlogger.TraceKeyInfo(countsrc1);
			mlogger_sendData.TraceKeyInfo(countsrc1);

			int rlt = modbus_connect(m_mymb);//建立连接
		
			if (-1== rlt)
			{				
				countsrc1.Format("modbus连接plc1：modbus连接失败，继续连接.err:%s", modbus_strerror(errno));
				mlogger.TraceKeyInfo(countsrc1);
				mlogger_sendData.TraceKeyInfo(countsrc1);
				//modbus_free(m_mymb);//释放实例对象
			

				thread th_hx(&YZJPdetectsystemDlg::hxthread, this);//唤醒线程开始,解除等待
				th_hx.join();
			}
			else
			{
				stop_sendData2plc = false;
				ready_wait = false;//开始等待
				mlogger.TraceKeyInfo("modbus连接plc1：modbus连接成功");
				mlogger_sendData.TraceKeyInfo("modbus连接plc1：modbus连接成功");
			}
		}
	}

}


#endif
//唤醒线程
void YZJPdetectsystemDlg::hxthread()
{
	//std::unique_lock <std::mutex> lck(mtx);
	mlogger_sendData.TraceKeyInfo("唤醒线程.");
	ready_wait = true;//停止等待
	modbus_close(m_mymb);
	mlogger_sendData.TraceKeyInfo("关闭modbus通讯");
	modbus_free(m_mymb);
	mlogger.TraceKeyInfo("释放modbus通讯字");
	mlogger_sendData.TraceKeyInfo("释放modbus通讯字");
	cv_wait.notify_all();
}


//删除文件及文件夹
void YZJPdetectsystemDlg::hxDeleteFile()
{
	string filepath = ".\\log\\";
	string fileFolderpath = ".\\imageYZJP\\";

	myDeleteFile mydeletefile;
	stop_hxDeleteFile = false;
	while (!stop_hxDeleteFile)
	{
		Sleep(1);


		//获取当前系统时间
		//time_t curTime;
		//struct tm * pTimeInfo = NULL;
		//time(&curTime);
		//pTimeInfo = localtime(&curTime);

		//if (16 == pTimeInfo->tm_hour)
		//{
		//	if (28 == pTimeInfo->tm_min)
		//	{
		//		if (pTimeInfo->tm_sec< 1)
		//		{
		//mlogger.TraceKeyInfo("开始执行删除日志\n");
			//		int vback=mydeletefile.deleteFile(filepath, 3);//删日志
					//mlogger.TraceKeyInfo("删除日志成功\n");
				//	int vfolderback=mydeletefile.deleteFileFolder(fileFolderpath, 5);//删图

				//	CString countsrc1;
				//	countsrc1.Format("删除文件及文件夹线程：删除文件:%d, 删除文件夹:%d.", vback, vfolderback);
				//	mlogger.TraceKeyInfo(countsrc1);

			//	}

			//}
		//}


	}

}

//系统关闭
void YZJPdetectsystemDlg::OnClose()
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (MessageBox(_T("确定关闭系统"), _T("关闭系统"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
	{
#if 1
		stop_sendData2plc = true;
		stop_modbus2plc = true;
		thread th_hx(&YZJPdetectsystemDlg::hxthread, this);//唤醒线程开始,解除等待
		th_hx.detach();
		
		Sleep(200);
#endif
		stopgather = true;
		stop_proImage = true;
		stop_saveImage = true;


		stopgather2 = true;
		stop_proImage2 = true;
		stop_saveImage2 = true;


		stopgather3 = true;
		stop_proImage3 = true;
		stop_saveImage3 = true;


		stopgather4 = true;
		stop_proImage4 = true;
		stop_saveImage4 = true;
		mlogger.TraceKeyInfo("xxxx--OnClose() stopthread \n");
		
		stop_receiveData = true;
		/*modbus_close(m_mymb);
		modbus_free(m_mymb);*/

	//	m_mymodbus.Close();//关闭modbus


		//stop_hxDeleteFile = true;


		//关闭相机
		int nCamNum = m_control->GetCamNum();
		mlogger.TraceKeyInfo("xxxx--OnClose()  nCamnum=%d \n", nCamNum);
		for (size_t i = 0; i < nCamNum; i++)
		{
			m_control->CloseCamera(i);
		}
		mlogger.TraceKeyInfo("xxxx--OnClose() closelight \n");
		//关闭光源
		OnBnClickedBtnCloselight();
		mlogger.TraceKeyInfo("xxxx--OnClose() closelight \n");


		m_camSetDlg.DestroyWindow();
		m_sysSetDlg.DestroyWindow();
		mlogger.TraceKeyInfo("系统完成关闭.");
		CDialogEx::OnClose();

	}

}



void YZJPdetectsystemDlg::OnBnClickedCam1Button()
{
	// TODO: 在此添加控件通知处理程序代码
	int nId = 0;
	m_camSetDlg.SetCamId((int)(nId+1));
	int CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_x", 1, lpPath);//标定Rect
	int CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_y", 1, lpPath);//标定Rect
	int CaliImageW = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_w", 100, lpPath);//标定Rect
	int CaliImageH = GetPrivateProfileInt("Set ImageFormat", "CaliImage1_h", 64, lpPath);//标定Rect
	int RunImageX = GetPrivateProfileInt("Set ImageFormat", "RunImage1_x", 1, lpPath);//标定Rect
	int RunImageY = GetPrivateProfileInt("Set ImageFormat", "RunImage1_y", 1, lpPath);//标定Rect
	int RunImageW = GetPrivateProfileInt("Set ImageFormat", "RunImage1_w", 100, lpPath);//标定Rect
	int RunImageH = GetPrivateProfileInt("Set ImageFormat", "RunImage1_h", 64, lpPath);//标定Rect
	Rect rectROI = Rect(CaliImageX, CaliImageY, CaliImageW, CaliImageH);//标定感兴趣区域ROI
	Rect rectROI2 = Rect(RunImageX, RunImageY, RunImageW, RunImageH);//运行感兴趣区域ROI
	int LighterValue = GetPrivateProfileInt("Set_Light", "LightValue1", 255, lpPath);//光源亮度 
	//设置对话框名称
	char title[64];
	_tcscpy(title, _T("相机1设置"));
	m_camSetDlg.SetWindowText(title);
	m_camSetDlg.SetCamRect(rectROI, rectROI2,m_control->GetImageWidth(nId), m_control->GetImageHeight(nId));
	int nExposetimeMin = 0;
	int nExposetimeMax = 0;
	m_control->GetExposetimeRange(nId, nExposetimeMin, nExposetimeMax);
	double nExposetime = 0;
	m_control->GetExposetime(nId, nExposetime);
	int nGainMin = 0;
	int nGainMax = 0;
	m_control->GetGainRange(nId, nGainMin, nGainMax);
	double nGain = 0;
	m_control->GetGain(nId, nGain);

	m_camSetDlg.SetCamParaments(nExposetime, nExposetimeMin, nExposetimeMax,nGain, nGainMin, nGainMax);
	m_camSetDlg.SetLighter(LighterValue);
	
	//关闭相机采集，重新设置相机图像大小，再打开采集
	g_dlg->m_control->StopGrabCamera(nId);
	//获取相机最大视野
		//设置
	int nMaxWidht = 0, nMaxHeight = 0;
	g_dlg->m_control->GetCameraMaxWidth(nId, nMaxWidht);
	g_dlg->m_control->GetCameraMaxHeight(nId, nMaxHeight);

	g_dlg->m_control->StopGrabCamera(nId);
	g_dlg->m_control->SetCameraOffsetX(nId, 0);
	g_dlg->m_control->SetCameraOffsetY(nId, 0);
	g_dlg->m_control->SetCameraWidth(nId, nMaxWidht);
	g_dlg->m_control->SetCameraHeight(nId, nMaxHeight);
	g_dlg->m_control->StartGrabCamera(nId);

	m_camSetDlg.InitUI();
	m_camSetDlg.ShowWindow(SW_SHOW);
}


void YZJPdetectsystemDlg::OnBnClickedCam2Button()
{
	// TODO: 在此添加控件通知处理程序代码
	int nId = 1;
	
	m_camSetDlg.SetCamId((int)(nId + 1));

	int CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage2_x", 1, lpPath);//标定Rect
	int CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage2_y", 1, lpPath);//标定Rect
	int CaliImageW = GetPrivateProfileInt("Set ImageFormat", "CaliImage2_w", 100, lpPath);//标定Rect
	int CaliImageH = GetPrivateProfileInt("Set ImageFormat", "CaliImage2_h", 64, lpPath);//标定Rect
	int RunImageX = GetPrivateProfileInt("Set ImageFormat", "RunImage2_x", 1, lpPath);//运行Rect
	int RunImageY = GetPrivateProfileInt("Set ImageFormat", "RunImage2_y", 1, lpPath);//运行Rect
	int RunImageW = GetPrivateProfileInt("Set ImageFormat", "RunImage2_w", 100, lpPath);//运行Rect
	int RunImageH = GetPrivateProfileInt("Set ImageFormat", "RunImage2_h", 64, lpPath);//运行Rect
	Rect rectROI = Rect(CaliImageX, CaliImageY, CaliImageW, CaliImageH);//标定感兴趣区域ROI
	Rect rectROI2 = Rect(RunImageX, RunImageY, RunImageW, RunImageH);//运行感兴趣区域ROI
	int LighterValue = GetPrivateProfileInt("Set_Light", "LightValue2", 255, lpPath);//算法开关 
	//设置对话框名称
	char title[64];
	_tcscpy(title, _T("相机2设置"));
	m_camSetDlg.SetWindowText(title);
	m_camSetDlg.SetCamRect(rectROI, rectROI2, m_control->GetImageWidth(nId), m_control->GetImageHeight(nId));
	int nExposetimeMin = 0;
	int nExposetimeMax = 0;
	m_control->GetExposetimeRange(nId, nExposetimeMin, nExposetimeMax);
	double nExposetime = 0;
	m_control->GetExposetime(nId, nExposetime);
	int nGainMin = 0;
	int nGainMax = 0;
	m_control->GetGainRange(nId, nGainMin, nGainMax);
	double nGain = 0;
	m_control->GetGain(nId, nGain);

	m_camSetDlg.SetCamParaments(nExposetime, nExposetimeMin, nExposetimeMax, nGain, nGainMin, nGainMax);
	m_camSetDlg.SetLighter(LighterValue);

	//关闭相机采集，重新设置相机图像大小，再打开采集
	g_dlg->m_control->StopGrabCamera(nId);
	//获取相机最大视野
		//设置
	int nMaxWidht = 0, nMaxHeight = 0;
	g_dlg->m_control->GetCameraMaxWidth(nId, nMaxWidht);
	g_dlg->m_control->GetCameraMaxHeight(nId, nMaxHeight);

	g_dlg->m_control->StopGrabCamera(nId);
	g_dlg->m_control->SetCameraOffsetX(nId, 0);
	g_dlg->m_control->SetCameraOffsetY(nId, 0);
	g_dlg->m_control->SetCameraWidth(nId, nMaxWidht);
	g_dlg->m_control->SetCameraHeight(nId, nMaxHeight);
	g_dlg->m_control->StartGrabCamera(nId);

	m_camSetDlg.InitUI();
	m_camSetDlg.ShowWindow(SW_SHOW);
}


void YZJPdetectsystemDlg::OnBnClickedCam3Button()
{
	// TODO: 在此添加控件通知处理程序代码
	int nId = 2;
	m_camSetDlg.SetCamId(3);

	int CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage3_x", 1, lpPath);//标定Rect
	int CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage3_y", 1, lpPath);//标定Rect
	int CaliImageW = GetPrivateProfileInt("Set ImageFormat", "CaliImage3_w", 100, lpPath);//标定Rect
	int CaliImageH = GetPrivateProfileInt("Set ImageFormat", "CaliImage3_h", 64, lpPath);//标定Rect
	int RunImageX = GetPrivateProfileInt("Set ImageFormat", "RunImage3_x", 1, lpPath);//运行Rect
	int RunImageY = GetPrivateProfileInt("Set ImageFormat", "RunImage3_y", 1, lpPath);//运行Rect
	int RunImageW = GetPrivateProfileInt("Set ImageFormat", "RunImage3_w", 100, lpPath);//运行Rect
	int RunImageH = GetPrivateProfileInt("Set ImageFormat", "RunImage3_h", 64, lpPath);//运行Rect
	Rect rectROI = Rect(CaliImageX, CaliImageY, CaliImageW, CaliImageH);//标定感兴趣区域ROI
	Rect rectROI2 = Rect(RunImageX, RunImageY, RunImageW, RunImageH);//运行感兴趣区域ROI
	int LighterValue = GetPrivateProfileInt("Set_Light", "LightValue3", 255, lpPath);//算法开关 
	//设置对话框名称
	char title[64];
	_tcscpy(title, _T("相机3设置"));
	m_camSetDlg.SetWindowText(title);
	m_camSetDlg.SetCamRect(rectROI, rectROI2,m_control->GetImageWidth(nId), m_control->GetImageHeight(nId));
	int nExposetimeMin = 0;
	int nExposetimeMax = 0;
	m_control->GetExposetimeRange(nId, nExposetimeMin, nExposetimeMax);
	double nExposetime = 0;
	m_control->GetExposetime(nId, nExposetime);
	int nGainMin = 0;
	int nGainMax = 0;
	m_control->GetGainRange(nId, nGainMin, nGainMax);
	double nGain = 0;
	m_control->GetGain(nId, nGain);

	m_camSetDlg.SetCamParaments(nExposetime, nExposetimeMin, nExposetimeMax, nGain, nGainMin, nGainMax);
	m_camSetDlg.SetLighter(LighterValue);

	//关闭相机采集，重新设置相机图像大小，再打开采集
	g_dlg->m_control->StopGrabCamera(nId);
	//获取相机最大视野
		//设置
	int nMaxWidht = 0, nMaxHeight = 0;
	g_dlg->m_control->GetCameraMaxWidth(nId, nMaxWidht);
	g_dlg->m_control->GetCameraMaxHeight(nId, nMaxHeight);

	g_dlg->m_control->StopGrabCamera(nId);
	g_dlg->m_control->SetCameraOffsetX(nId, 0);
	g_dlg->m_control->SetCameraOffsetY(nId, 0);
	g_dlg->m_control->SetCameraWidth(nId, nMaxWidht);
	g_dlg->m_control->SetCameraHeight(nId, nMaxHeight);
	g_dlg->m_control->StartGrabCamera(nId);

	m_camSetDlg.InitUI();
	m_camSetDlg.ShowWindow(SW_SHOW);
}


void YZJPdetectsystemDlg::OnBnClickedCam4Button()
{
	// TODO: 在此添加控件通知处理程序代码
	
	int nId = 3;
	m_camSetDlg.SetCamId((int)(nId + 1));
	int CaliImageX = GetPrivateProfileInt("Set ImageFormat", "CaliImage4_x", 1, lpPath);//标定Rect
	int CaliImageY = GetPrivateProfileInt("Set ImageFormat", "CaliImage4_y", 1, lpPath);//标定Rect
	int CaliImageW = GetPrivateProfileInt("Set ImageFormat", "CaliImage4_w", 100, lpPath);//标定Rect
	int CaliImageH = GetPrivateProfileInt("Set ImageFormat", "CaliImage4_h", 64, lpPath);//标定Rect
	int RunImageX = GetPrivateProfileInt("Set ImageFormat", "RunImage4_x", 1, lpPath);//运行Rect
	int RunImageY = GetPrivateProfileInt("Set ImageFormat", "RunImage4_y", 1, lpPath);//运行Rect
	int RunImageW = GetPrivateProfileInt("Set ImageFormat", "RunImage4_w", 100, lpPath);//运行Rect
	int RunImageH = GetPrivateProfileInt("Set ImageFormat", "RunImage4_h", 64, lpPath);//运行Rect
	Rect rectROI = Rect(CaliImageX, CaliImageY, CaliImageW, CaliImageH);//标定感兴趣区域ROI
	Rect rectROI2 = Rect(RunImageX, RunImageY, RunImageW, RunImageH);//运行感兴趣区域ROI
	int LighterValue = GetPrivateProfileInt("Set_Light", "LightValue4", 255, lpPath);//算法开关 
	//设置对话框名称
	char title[64];
	_tcscpy(title, _T("相机4设置"));
	m_camSetDlg.SetWindowText(title);
	m_camSetDlg.SetCamRect(rectROI, rectROI2, m_control->GetImageWidth(nId), m_control->GetImageHeight(nId));
	int nExposetimeMin = 0;
	int nExposetimeMax = 0;
	m_control->GetExposetimeRange(nId, nExposetimeMin, nExposetimeMax);
	double nExposetime = 0;
	m_control->GetExposetime(nId, nExposetime);
	int nGainMin = 0;
	int nGainMax = 0;
	m_control->GetGainRange(nId, nGainMin, nGainMax);
	double nGain = 0;
	m_control->GetGain(nId, nGain);

	m_camSetDlg.SetCamParaments(nExposetime, nExposetimeMin, nExposetimeMax, nGain, nGainMin, nGainMax);
	m_camSetDlg.SetLighter(LighterValue);
	//关闭相机采集，重新设置相机图像大小，再打开采集
	g_dlg->m_control->StopGrabCamera(nId);
	//获取相机最大视野
		//设置
	int nMaxWidht = 0, nMaxHeight = 0;
	g_dlg->m_control->GetCameraMaxWidth(nId, nMaxWidht);
	g_dlg->m_control->GetCameraMaxHeight(nId, nMaxHeight);

	g_dlg->m_control->StopGrabCamera(nId);
	g_dlg->m_control->SetCameraOffsetX(nId, 0);
	g_dlg->m_control->SetCameraOffsetY(nId, 0);
	g_dlg->m_control->SetCameraWidth(nId, nMaxWidht);
	g_dlg->m_control->SetCameraHeight(nId, nMaxHeight);
	g_dlg->m_control->StartGrabCamera(nId);
	m_camSetDlg.InitUI();
	m_camSetDlg.ShowWindow(SW_SHOW);
}


void YZJPdetectsystemDlg::OnBnClickedButtonSysSet()
{
	// TODO: 在此添加控件通知处理程序代码
	m_sysSetDlg.InitUI();
	m_sysSetDlg.ShowWindow(SW_SHOW);
}

void YZJPdetectsystemDlg::receiveData()
{
	stop_receiveData = false;

	bool b_once = true;

	while (!stop_receiveData)
	{
		Sleep(1);

		

		bool bk = true;

		unsigned char *temp = new unsigned char[8];//动态创建一个数组

		//打开继电器 1
		temp[0] = 0x01;
		temp[1] = 0x05;
		temp[2] = 0x00;
		temp[3] = 0x00;
		temp[4] = 0xFF;
		temp[5] = 0x00;
		temp[6] = 0x8C;
		temp[7] = 0x3A;
		bk = mySerialPort.WriteData(temp, 8);//这个函数就是给串口发送数据的函数，temp就是要发送的数组。
		int BlowpipeTimelength = GetPrivateProfileInt("Blowpipe set", "Timelength", 30000, lpPath);
		Sleep(BlowpipeTimelength);
		//关闭继电器 1
		temp[0] = 0x01;
		temp[1] = 0x05;
		temp[2] = 0x00;
		temp[3] = 0x00;
		temp[4] = 0x00;
		temp[5] = 0x00;
		temp[6] = 0xCD;
		temp[7] = 0xCA;
		bk = mySerialPort.WriteData(temp, 8);//这个函数就是给串口发送数据的函数，temp就是要发送的数组。

		if (!bk)
		{
			CString countsrc;
			countsrc.Format("%s 串口掉线", myGetCurrentTime());
			m_xclist.InsertString(0, countsrc);
			m_xclist.SetCurSel(0);
			//串口的初始化
			int nComId = GetPrivateProfileInt("Blowpipe set", "COMid", 4, lpPath);
			if (!mySerialPort.InitPort(nComId, CBR_9600, 'N', 8, 1, EV_RXCHAR))
			{
				countsrc.Format("%s 串口重连失败", myGetCurrentTime());
				m_xclist.InsertString(0, countsrc);
				m_xclist.SetCurSel(0);
				//	continue;
			}

			countsrc.Format("%s 串口重连成功", myGetCurrentTime());
			m_xclist.InsertString(0, countsrc);
			m_xclist.SetCurSel(0);
		}

		delete[]	temp;
		int BlowpipeInterval = GetPrivateProfileInt("Blowpipe set", "Interval", 1200000, lpPath);//默认间隔20分钟
		Sleep(BlowpipeInterval);
	}



}