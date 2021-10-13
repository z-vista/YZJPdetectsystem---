
// YZJPdetectsystemDlg.h: 头文件
//

#pragma once

#include <ctime>
#include <string>
#include "afxwin.h"

#include "SerialPort.h"
#include "CGxCamManager.h"
#include "CamSetDlg.h"
#include "SysSetDlg.h"

#include <string>

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


#include <thread> //多线程
#include <queue>
#include <memory>
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>    // std::condition_variable

//Modbus
//#include "M_Client.h"

#include <stdio.h>
#include <modbus.h>

//Light
#include "ContrlLight.h"
//
//#include "NetSocket.h"

//算法
#include"findBound.h"

using namespace cv;
using namespace std;


struct PositonResult
{
	double position;
	int result;
};
//typedef struct  _tagSettingInfo {
//	BOOL  bUdp;
//	WORD  btDevNo;
//	TCHAR sServerIp[32];
//	WORD  wServerPort;
//	WORD  wSendCycle;
//}SETTINGINFO;
//==================================================================
//函数名：		template<typename T> class threadsafe_queue
//作者：		lyc
//日期：		2016/07/10
//功能：		线程安全的queue接口
//输入参数：	typename T
//输出参数：	
//返回值：		这是一个类
//修改记录：	无
//待解决问题：	无
//==================================================================
template<typename T>
class threadsafe_queue
{

private:
	mutable std::mutex mut;
	std::queue<std::shared_ptr<T>> data_queue;
	std::condition_variable data_cond;
public:
	int queuelength;
	threadsafe_queue() {}
	// 	threadsafe_queue(threadsafe_queue const& other)
	// 	{
	// 		std::lock_guard<std::mutex> lk(other.mut);
	// 		data_queue = other.data_queue;
	// 	}
	void push(T new_value)//入队操作
	{
		std::shared_ptr<T> data(std::make_shared<T>(std::move(new_value)));
		std::lock_guard<std::mutex> lk(mut);
		data_queue.push(data);
		data_cond.notify_one();
	}
	void wait_and_pop(T& value)//直到有元素可以删除为止
	{
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [this] {return !data_queue.empty(); });

		queuelength = data_queue.size();

		value = std::move(*data_queue.front());
		data_queue.pop();
	}


	std::shared_ptr<T> wait_and_pop()
	{
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [this] {return !data_queue.empty(); });
		std::shared_ptr<T> res = data_queue.front();
		data_queue.pop();
		return res;
	}

	bool try_pop(T& value)//不管有没有队首元素直接返回
	{
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty())
			return false;
		value = std::move(*data_queue.front());
		data_queue.pop();
		return true;
	}

	std::shared_ptr<T> try_pop()
	{
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty())
			return std::shared_ptr<T>();
		std::shared_ptr<T> res = data_queue.front();
		data_queue.pop();
		return res;
	}
	bool empty() const
	{
		std::lock_guard<std::mutex> lk(mut);
		return data_queue.empty();
	}

	int size() const
	{   // return length of queue  
		return (data_queue.size());
	}
};
// YZJPdetectsystemDlg 对话框
class YZJPdetectsystemDlg : public CDialogEx
{
// 构造
public:
	YZJPdetectsystemDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_YZJPDETECTSYSTEM_DIALOG};
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


public:
	void ChangeWidgetSize(UINT nID, int x, int y);
	CRect  m_rect;   //用于保存原对话框大小

	BOOL CheckCopyright();
	void OnBnClickedButstartsystem();
	CString myGetCurrentTime();

public:


	int staticCount3;
	int staticCount4;
	int cameraAllNumber;

	//Logger mlogger;//日志文件

public:
	shared_ptr<CGxCamManager> m_control = NULL;
	CSerialPort mySerialPort;
public:

	
	//线程通信队列
	threadsafe_queue<Mat> srcMat_src;//transport src image from camera call-back function
	threadsafe_queue<Mat> srcMat_save;//transport src image to save
	threadsafe_queue<Mat> dstMat_save;//transport result image to save
	threadsafe_queue<Mat> midMat_pro;//transport image to pro

	threadsafe_queue<Mat> srcMat_src2;//transport src image from camera call-back function
	threadsafe_queue<Mat> srcMat_save2;//transport src image to save
	threadsafe_queue<Mat> dstMat_save2;//transport result image to save
	threadsafe_queue<Mat> midMat_pro2;//transport image to pro


	threadsafe_queue<Mat> srcMat_src3;//transport src image from camera call-back function
	threadsafe_queue<Mat> srcMat_save3;//transport src image to save
	threadsafe_queue<Mat> dstMat_save3;//transport result image to save
	threadsafe_queue<Mat> midMat_pro3;//transport image to pro

	threadsafe_queue<Mat> srcMat_src4;//transport src image from camera call-back function
	threadsafe_queue<Mat> srcMat_save4;//transport src image to save
	threadsafe_queue<Mat> dstMat_save4;//transport result image to save
	threadsafe_queue<Mat> midMat_pro4;//transport image to pro

	threadsafe_queue<PositonResult> send_Data;//transport data to modbus
	threadsafe_queue<PositonResult> send_Data2;//transport data to modbus
	threadsafe_queue<PositonResult> send_Data3;//transport data to modbus
	threadsafe_queue<PositonResult> send_Data4;//transport data to modbus
public:
	bool stopgather;
	bool stop_proImage;
	bool stop_saveImage;

	bool stopgather2;
	bool stop_proImage2;
	bool stop_saveImage2;

	bool stopgather3;
	bool stop_proImage3;
	bool stop_saveImage3;

	bool stopgather4;
	bool stop_proImage4;
	bool stop_saveImage4;

	bool stop_hxDeleteFile;

	CString lpPath;
	string m_s;
public:
	void gatherImage();
	void proImage();
	void saveImage();

	void gatherImage2();
	void proImage2();
	void saveImage2();

	void gatherImage3();
	void proImage3();
	void saveImage3();

	void gatherImage4();
	void proImage4();
	void saveImage4();

	void modbus2PLC();//发送数据到PLC
	//modbus多次连接
	void modbus2Connect();

	//文件删除线程
	void hxDeleteFile();

	void receiveData();
	void ShowImageToPic(Mat &img, UINT ID);
public:
	//M_Client m_mymodbus;//初始化对象
	bool stop_modbus2plc;
	bool stop_sendData2plc;
	//0420
	modbus_t* m_mymb;

	bool stop_receiveData;
	//0420
//0715
private:
	std::mutex mtx; // 全局互斥锁.
	std::condition_variable cv_wait; // 全局条件变量.
	bool ready_wait; // 全局标志位.

public:
	void hxthread();	//线程唤醒函数
	
	//光源
public:
	bool m_bFSTLight;//1：使用菲视特光源控制器，0：使用CST光源控制器
	HANDLE  m_hCom[2];
	CContrlLight m_LightCom[2];
	GxCAMERALIGHT_PARAMETER m_struct_ParameterOfGxCameraLight;
	//int m_iLightCom[2];
	int m_iLightValue[4];

	int InitializeCameraLightDevice(int num);///初始化光源
	void InitLightParameters();///@光源配置文件
	void WriterLightParameters();
	afx_msg void OnBnClickedBtnOpenlight();
	afx_msg void OnBnClickedBtnCloselight();
public:
	int m_nTabItemCount1;
	int m_nTabItemCount2;
	int m_nTabItemCount3;
	int m_nTabItemCount4;


	vector<cv::Point>  m_CalibTable1/*[MAX_RATIO_TABLE_ITEM]*/;
	vector<cv::Point>  m_CalibTable2/*[MAX_RATIO_TABLE_ITEM]*/;
	vector<cv::Point>  m_CalibTable3/*[MAX_RATIO_TABLE_ITEM]*/;
	vector<cv::Point>  m_CalibTable4/*[MAX_RATIO_TABLE_ITEM]*/;
	
	void LoadRatioTable(int nId);

	//std::vector<cv::Point> m_vecAllPosition[4];//标定系数
	CFindBound myfindBound;//算法对象
	float FindRatio(int camId, float pos);
	/*float ComputePaperWdith(vector<XRESOLUTIONTAB> sloutionTab, int camId, cv::Point crossPoint);*/
	float ComputePosition(int isupCamera, std::vector<cv::Point>& vecAllPosition, cv::Point crossPoint);//根据返回的点计算物理位置

//public:
//	static UINT SendThreadProc(LPVOID lpvoid);
//	static void NetSockCallback(int msg, int param, void* lpobj);
//	void OnNetSockCallback(int msg, int param, void* lpobj);
//	void LoadingNetSockSetting();
//	void InitNetSock();
//	void OnSockData();//接收数据解析
//	void ConnectToServer();
//	void ServerNotify(int Ref, int addr, int value, int value1, int value2, int value3, int value4, int IsPaper1, int IsPaper2, int value5, int value6);  //写20个寄存器，使用功能码16，修改后就可以写多个

	//SETTINGINFO m_set;
	//BOOL m_bQuit;
	//HANDLE m_hEvent;
	//CWinThread* m_pThread;
	//UINT SendProc();
	//enum {
	//	MAX_SEND_BUFF = 128,
	//	MAX_RECV_BUFF = 128
	//};
	//NetSocket   m_sock; //socket的句柄
	////WORD        m_wSendNo; //发送心跳标识
	//int			m_sendLen;//发送的长度
	//int			m_recvLen;
	//BYTE		m_sendBuff[MAX_SEND_BUFF];//发送的数据
	//BYTE        m_recvBuff[MAX_RECV_BUFF];
	//CCriticalSection m_lock;
	//bool m_bThread;
protected:
	HICON m_hIcon;
	HINSTANCE m_hResDll;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CCamSetDlg m_camSetDlg;
	CSysSetDlg m_sysSetDlg;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CListBox m_xclist;
	afx_msg void OnClose();
	afx_msg void OnBnClickedCam1Button();
	afx_msg void OnBnClickedCam2Button();
	afx_msg void OnBnClickedCam3Button();
	afx_msg void OnBnClickedCam4Button();
	afx_msg void OnBnClickedButtonSysSet();
};
