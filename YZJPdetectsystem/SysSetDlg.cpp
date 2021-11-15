// SysSetDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "YZJPdetectsystem.h"
#include "SysSetDlg.h"
#include "afxdialogex.h"
#include "YZJPdetectsystemDlg.h"

extern YZJPdetectsystemDlg* g_dlg;
extern NetDataLog mlogger;

// CSysSetDlg 对话框

IMPLEMENT_DYNAMIC(CSysSetDlg, CDialogEx)

CSysSetDlg::CSysSetDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SYS_SET, pParent)
	, m_bSaveSource(FALSE)
	, m_bSaveResult(FALSE)
	, m_nFrequency(0)
	, m_nResultNum(0)
	, m_fStandart(0)
	, m_fReduce(0)
	, m_nInterval1(0)
	, m_nInterval2(0)
	, m_OSTransDS(FALSE)
	, m_bTilerFactorIs5(FALSE)
	, m_bCoreFactorIs5(FALSE)
	, m_comNum(0)
	, m_timeLength(0)
	, m_waitTimeLength(0)
	, m_nBwInterl(0)
	, m_nCount(0)
{

}

CSysSetDlg::~CSysSetDlg()
{
}

void CSysSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SOURCE_SAVE, m_bSaveSource);
	DDX_Check(pDX, IDC_CHECK_RESULT_SAVE, m_bSaveResult);
	DDX_Text(pDX, IDC_EDIT_SAVE_FREQUEN, m_nFrequency);
	DDX_Text(pDX, IDC_EDIT_RESULT_NUM, m_nResultNum);
	DDX_Control(pDX, IDC_SPIN_SAVE_FREQUE, m_SpinSaveFre);
	DDX_Control(pDX, IDC_SPIN_RESULT_NUM, m_SpinResultNum);
	DDX_Text(pDX, IDC_EDIT_STANDARD, m_fStandart);
	DDX_Text(pDX, IDC_EDIT_REDUCED, m_fReduce);
	DDX_Text(pDX, IDC_EDIT_BLINDAREA1, m_nInterval1);
	DDX_Text(pDX, IDC_EDIT_BLINDAREA2, m_nInterval2);
	DDX_Check(pDX, IDC_CHECK_OSTRANSDS, m_OSTransDS);
	DDX_Check(pDX, IDC_TILER_FACTOR_CHECK, m_bTilerFactorIs5);
	DDX_Check(pDX, IDC_CORE_FACTOR_CHECK, m_bCoreFactorIs5);
	DDX_Text(pDX, IDC_EDIT_SER_PORT, m_comNum);
	DDX_Text(pDX, IDC_EDIT_TIME_LEN, m_timeLength);
	DDX_Text(pDX, IDC_EDIT_WAIT_TIME_LEN, m_waitTimeLength);
	DDX_Text(pDX, IDC_EDIT_B_INTERL, m_nBwInterl);
	DDX_Text(pDX, IDC_EDIT_COUNT, m_nCount);
}


BEGIN_MESSAGE_MAP(CSysSetDlg, CDialogEx)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDOK, &CSysSetDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSysSetDlg::OnBnClickedCancel)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SAVE_FREQUE, &CSysSetDlg::OnDeltaposSpinSaveFreque)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RESULT_NUM, &CSysSetDlg::OnDeltaposSpinResultNum)
END_MESSAGE_MAP()


// CSysSetDlg 消息处理程序


BOOL CSysSetDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	RECT rt;
	SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rt, 0);//获取桌面工作区的大小
	MoveWindow(&rt);

	
	m_SpinSaveFre.SetBuddy(GetDlgItem(m_nFrequency));
	m_SpinSaveFre.SetRange(0, 900);
	m_SpinResultNum.SetBuddy(GetDlgItem(m_nResultNum));
	m_SpinResultNum.SetRange(0, 100);
	GetClientRect(&m_DlgRect);  //获取对话框的大小



	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CSysSetDlg::InitUI()
{
	////初始化界面参数UI
	int nValue = 0; char chAttribute[20];
	
	nValue = GetPrivateProfileInt("Set ImageFormat", "Interval1", 1000, g_dlg->lpPath);//
	m_nInterval1 = nValue;
	nValue = GetPrivateProfileInt("Set ImageFormat", "Interval2", 1000, g_dlg->lpPath);//
	m_nInterval2 = nValue;
	nValue = GetPrivateProfileInt("Set ImageFormat", "SourceImageSave_choice", 0, g_dlg->lpPath);//
	m_bSaveSource = (nValue == 1 ? TRUE : FALSE);
	nValue = GetPrivateProfileInt("Set ImageFormat", "ResultImageSave_choice", 0, g_dlg->lpPath);//
	m_bSaveResult = (nValue == 1 ? TRUE : FALSE);
	m_bSaveResult = nValue;
	memset(&chAttribute, 0, sizeof(chAttribute));
	GetPrivateProfileString("Set Detection Range", "StandardDeviation", "2.0", chAttribute,20,g_dlg->lpPath);
	m_fStandart = atof(chAttribute);
	memset(&chAttribute, 0, sizeof(chAttribute));
	GetPrivateProfileString("Set Detection Range", "ReducedValue", "2.0", chAttribute, 20, g_dlg->lpPath);
	m_fReduce = atof(chAttribute);
	nValue = GetPrivateProfileInt("Set Sfconfig", "sleepNum_save", 200, g_dlg->lpPath);//
	m_nFrequency = nValue;
	nValue = GetPrivateProfileInt("Set Sfconfig", "result_num", 5, g_dlg->lpPath);//
	m_nResultNum = nValue;
	nValue = GetPrivateProfileInt("Set Sfconfig", "OStransDS_on", 0, g_dlg->lpPath);//
	m_OSTransDS = (nValue == 1 ? TRUE : FALSE);
	nValue = GetPrivateProfileInt("Set Sfconfig", "TilerFactorIs5", 0, g_dlg->lpPath);//
	m_bTilerFactorIs5 = (nValue == 1 ? TRUE : FALSE);
	nValue = GetPrivateProfileInt("Set Sfconfig", "CoreFactorIs5", 0, g_dlg->lpPath);//
	m_bCoreFactorIs5 = (nValue == 1 ? TRUE : FALSE);
	CString strbb_value;
	strbb_value.Format("%d", m_nInterval1);//瓦纸视野盲区
	GetDlgItem(IDC_EDIT_BLINDAREA1)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_nInterval2);//芯纸视野盲区
	GetDlgItem(IDC_EDIT_BLINDAREA2)->SetWindowText(strbb_value);
	((CButton*)GetDlgItem(IDC_CHECK_SOURCE_SAVE))->SetCheck(m_bSaveSource == TRUE ? 1 : 0); //保存原始图开关	
	((CButton*)GetDlgItem(IDC_CHECK_RESULT_SAVE))->SetCheck(m_bSaveResult == TRUE ? 1 : 0); //保存结果图开关
	strbb_value.Format("%2f", m_fStandart);//标准差
	GetDlgItem(IDC_EDIT_STANDARD)->SetWindowText(strbb_value);
	strbb_value.Format("%2f", m_fReduce);//过滤值
	GetDlgItem(IDC_EDIT_REDUCED)->SetWindowText(strbb_value);

	strbb_value.Format("%d", m_nFrequency);//保存图片频率
	GetDlgItem(IDC_EDIT_SAVE_FREQUEN)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_nResultNum);//综合结果合成频率
	GetDlgItem(IDC_EDIT_RESULT_NUM)->SetWindowText(strbb_value);

	((CButton*)GetDlgItem(IDC_CHECK_OSTRANSDS))->SetCheck(m_OSTransDS == TRUE ? 1 : 0); //操作驱动侧的互换开关
	((CButton*)GetDlgItem(IDC_TILER_FACTOR_CHECK))->SetCheck(m_bTilerFactorIs5 == TRUE ? 1 : 0); //瓦纸比例==5开关
	((CButton*)GetDlgItem(IDC_CORE_FACTOR_CHECK))->SetCheck(m_bCoreFactorIs5 == TRUE ? 1 : 0); //芯纸比例==5开关

	nValue = GetPrivateProfileInt("Blowpipe set", "Count", 5, g_dlg->lpPath);//
	m_nCount = nValue;
	nValue = GetPrivateProfileInt("Blowpipe set", "COMid", 4, g_dlg->lpPath);//
	m_comNum = nValue;
	nValue = GetPrivateProfileInt("Blowpipe set", "Timelength", 30000, g_dlg->lpPath);//
	m_timeLength = nValue;
	nValue = GetPrivateProfileInt("Blowpipe set", "WaitTimelength", 30000, g_dlg->lpPath);//
	m_waitTimeLength = nValue;
	nValue = GetPrivateProfileInt("Blowpipe set", "Interval", 1200000, g_dlg->lpPath);//
	m_nBwInterl = nValue;
	strbb_value.Format("%d", m_nCount);
	GetDlgItem(IDC_EDIT_COUNT)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_comNum);
	GetDlgItem(IDC_EDIT_SER_PORT)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_timeLength);
	GetDlgItem(IDC_EDIT_TIME_LEN)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_waitTimeLength);
	GetDlgItem(IDC_EDIT_WAIT_TIME_LEN)->SetWindowText(strbb_value);
	strbb_value.Format("%d", m_nBwInterl);
	GetDlgItem(IDC_EDIT_B_INTERL)->SetWindowText(strbb_value);
}

//控件适应屏幕变化模块
//nID为控件ID，x,y分别为对话框的当前长和宽
void CSysSetDlg::ChangeWidgetSize(UINT nID, int x, int y)
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

void CSysSetDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (nType != SIZE_MINIMIZED)  //判断窗口是不是最小化了，因为窗口最小化之后 ，窗口的长和宽会变成0，当前一次变化的时就会出现除以0的错误操作
	{
		ChangeWidgetSize(IDC_STATIC_SAVE_GROUP, cx, cy);
		ChangeWidgetSize(IDC_CHECK_SOURCE_SAVE, cx, cy);
		ChangeWidgetSize(IDC_CHECK_RESULT_SAVE, cx, cy);
		ChangeWidgetSize(IDC_STATIC_SAVE_FREQUEN, cx, cy);
		ChangeWidgetSize(IDC_EDIT_SAVE_FREQUEN, cx, cy);
		ChangeWidgetSize(IDC_SPIN_SAVE_FREQUE, cx, cy);
		ChangeWidgetSize(IDC_STATIC_REULST_NUM, cx, cy);
		ChangeWidgetSize(IDC_EDIT_RESULT_NUM, cx, cy);
		ChangeWidgetSize(IDC_SPIN_RESULT_NUM, cx, cy);	
		ChangeWidgetSize(IDC_STATIC_BLINDAREA1, cx, cy);
		ChangeWidgetSize(IDC_STATIC_BLINDAREA2, cx, cy);
		ChangeWidgetSize(IDC_EDIT_BLINDAREA1, cx, cy);
		ChangeWidgetSize(IDC_EDIT_BLINDAREA2, cx, cy);
		ChangeWidgetSize(IDC_STATIC_FILTRATION_GROUP, cx, cy);
		ChangeWidgetSize(IDC_STATIC_STANDARD, cx, cy);
		ChangeWidgetSize(IDC_EDIT_STANDARD, cx, cy);
		ChangeWidgetSize(IDC_STATIC_REDUCED, cx, cy);
		ChangeWidgetSize(IDC_EDIT_REDUCED, cx, cy);
		ChangeWidgetSize(IDC_CHECK_OSTRANSDS, cx, cy);
		ChangeWidgetSize(IDC_TILER_FACTOR_CHECK, cx, cy);
		ChangeWidgetSize(IDC_CORE_FACTOR_CHECK, cx, cy);
		ChangeWidgetSize(IDOK, cx, cy);
		ChangeWidgetSize(IDCANCEL, cx, cy);
		ChangeWidgetSize(IDC_EDIT_SER_PORT, cx, cy);
		ChangeWidgetSize(IDC_STATIC_COM, cx, cy);
		ChangeWidgetSize(IDC_EDIT_TIME_LEN, cx, cy);
		ChangeWidgetSize(IDC_STATIC_TIME_LEN, cx, cy);
		ChangeWidgetSize(IDC_EDIT_WAIT_TIME_LEN, cx, cy);
		ChangeWidgetSize(IDC_STATIC_WAIT_TIME_LEN, cx, cy);
		ChangeWidgetSize(IDC_EDIT_B_INTERL, cx, cy);
		ChangeWidgetSize(IDC_STATIC_B_INTREL, cx, cy);
		ChangeWidgetSize(IDC_EDIT_COUNT, cx, cy);
		ChangeWidgetSize(IDC_STATIC_COUNT, cx, cy);
		
		GetClientRect(&m_DlgRect);   //最后要更新对话框的大小，当做下一次变化的旧坐标；
	}
}


void CSysSetDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//保存所有参数
	UpdateData(true);
	char tp_strToWrite[MAX_PATH];
	//标定后瓦纸视野盲区
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_nInterval1);
	WritePrivateProfileString("Set ImageFormat", "Interval1", tp_strToWrite, g_dlg->lpPath);
	//标定后芯纸视野盲区
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_nInterval2);
	WritePrivateProfileString("Set ImageFormat", "Interval2", tp_strToWrite, g_dlg->lpPath);
	//保存原图开关
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_bSaveSource==TRUE?1:0);
	WritePrivateProfileString("Set ImageFormat", "SourceImageSave_choice", tp_strToWrite, g_dlg->lpPath);
	//保存结果图开关
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_bSaveResult==TRUE?1:0);
	WritePrivateProfileString("Set ImageFormat", "ResultImageSave_choice", tp_strToWrite, g_dlg->lpPath);

	//标准差
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%2f", m_fStandart);
	WritePrivateProfileString("Set Detection Range", "StandardDeviation", tp_strToWrite, g_dlg->lpPath);
	//值变化大小上限
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%2f", m_fReduce);
	WritePrivateProfileString("Set Detection Range", "ReducedValue", tp_strToWrite, g_dlg->lpPath);
	
	//图像保存频率
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_nFrequency);
	WritePrivateProfileString("Set Sfconfig", "sleepNum_save", tp_strToWrite, g_dlg->lpPath);
	//结果合成频率
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_nResultNum);
	WritePrivateProfileString("Set Sfconfig", "result_num", tp_strToWrite, g_dlg->lpPath);
	//操作侧驱动侧调换开关
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_OSTransDS==TRUE?1:0);
	WritePrivateProfileString("Set Sfconfig", "OStransDS_on", tp_strToWrite, g_dlg->lpPath);
	//瓦纸比例开关
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_bTilerFactorIs5 == TRUE ? 1 : 0);
	WritePrivateProfileString("Set Sfconfig", "TilerFactorIs5", tp_strToWrite, g_dlg->lpPath);
	//芯纸比例开关
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_bCoreFactorIs5 == TRUE ? 1 : 0);
	WritePrivateProfileString("Set Sfconfig", "CoreFactorIs5", tp_strToWrite, g_dlg->lpPath);

	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_nCount);
	WritePrivateProfileString("Blowpipe set", "Count", tp_strToWrite, g_dlg->lpPath);
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_comNum);
	WritePrivateProfileString("Blowpipe set", "COMid", tp_strToWrite, g_dlg->lpPath);
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_timeLength);
	WritePrivateProfileString("Blowpipe set", "Timelength", tp_strToWrite, g_dlg->lpPath);
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_waitTimeLength);
	WritePrivateProfileString("Blowpipe set", "WaitTimelength", tp_strToWrite, g_dlg->lpPath);
	memset(tp_strToWrite, 0x00, MAX_PATH);
	sprintf(tp_strToWrite, "%d", m_nBwInterl);
	WritePrivateProfileString("Blowpipe set", "Interval", tp_strToWrite, g_dlg->lpPath);
	CDialogEx::OnOK();
}


void CSysSetDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


void CSysSetDlg::OnDeltaposSpinSaveFreque(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);
	
	if (pNMUpDown->iDelta == -1) // 如果此值为-1 , 说明点击了Spin的往下的箭头  
	{
		m_nFrequency = m_nFrequency - 10;	
	}
	else if (pNMUpDown->iDelta == 1) // 如果此值为1, 说明点击了Spin的往上的箭头  
	{		
		m_nFrequency = m_nFrequency + 10;	
	}
	UpdateData(false);
	*pResult = 0;
}


void CSysSetDlg::OnDeltaposSpinResultNum(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(true);

	if (pNMUpDown->iDelta == -1) // 如果此值为-1 , 说明点击了Spin的往下的箭头  
	{
		m_nResultNum = m_nResultNum - 1;
	}
	else if (pNMUpDown->iDelta == 1) // 如果此值为1, 说明点击了Spin的往上的箭头  
	{
		m_nResultNum = m_nResultNum + 1;
	}
	UpdateData(false);
	*pResult = 0;
}
