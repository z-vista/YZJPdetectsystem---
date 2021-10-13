#pragma once


// CSysSetDlg 对话框

class CSysSetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSysSetDlg)

public:
	CSysSetDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSysSetDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SYS_SET };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	void InitUI();
	CRect m_DlgRect;
	void ChangeWidgetSize(UINT nID, int x, int y);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	BOOL m_bSaveSource;
	BOOL m_bSaveResult;
	int m_nFrequency;
	int m_nResultNum;
	CSpinButtonCtrl m_SpinSaveFre;
	CSpinButtonCtrl m_SpinResultNum;
	afx_msg void OnDeltaposSpinSaveFreque(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinResultNum(NMHDR* pNMHDR, LRESULT* pResult);
	float m_fStandart;
	float m_fReduce;
	int m_nInterval1;
	int m_nInterval2;
	BOOL m_OSTransDS;
	BOOL m_bTilerFactorIs5;
	BOOL m_bCoreFactorIs5;
	int m_comNum;
	int m_timeLength;
	int m_nBwInterl;
};
