#pragma once 

#include<iostream>
#include<memory>
#include<opencv2/opencv.hpp>

#include"GxIApi.h"
#include"DxImageProc.h"

//Log
#include "Logger.h"

using namespace std;
using namespace cv;
//typedef GX_DEV_HANDLE Camera_t;
// 相机最大的数量
#define MaxCameraNumber 4
//---------------------------------------------------------------------------------
/**
\brief   掉线回调函数
\param   pUserParam   用户参数

\return  void
*/
//----------------------------------------------------------------------------------

///相机参数结构体
typedef struct CAMER_INFO
{
	//BITMAPINFO*					pBmpInfo;		  ///< BITMAPINFO 结构指针，显示图像时使用
	BYTE*						pImageBuffer;	  ///< 指向经过处理后的图像数据缓冲区
	BYTE*						pRawBuffer;		  ///< 指向原始RAW图缓冲区
	//char						chBmpBuf[2048];	  ///< BIMTAPINFO 存储缓冲区，m_pBmpInfo即指向此缓冲区	
	int64_t                     nPayLoadSise;     ///< 图像块大小
	int64_t					    nImageWidth;	  ///< 图像宽度
	int64_t					    nImageHeight;	  ///< 图像高度	
	int64_t					    nBayerLayout;	  ///< Bayer排布格式
	bool						bIsColorFilter;	  ///< 判断是否为彩色相机
	BOOL						bIsOpen;		  ///< 相机已打开标志
	BOOL						bIsSnap;		  ///< 相机正在采集标志
	bool                        bIsOffLine;       ///< 相机掉线标识
	float						fFps;			  ///< 帧率
}CAMER_INFO;


class CGxCamManager
{
private:

	CGxCamManager();
	CGxCamManager(const CGxCamManager &);
	CGxCamManager& operator=(const CGxCamManager &);	
	~CGxCamManager();
	static void Destory(CGxCamManager*) { m_Instance->ReleasesGx(); OutputDebugString("相机管理类析构Destory\n"); };

	static shared_ptr<CGxCamManager> m_Instance;

	//它的唯一工作就是在析构函数中删除CSingleton的实例 

public:
	static shared_ptr<CGxCamManager> GetInstance();

	void ShowErrorString(GX_STATUS emErrorStatus);	/// 获取错误信息描述
	GX_STATUS SetPixelFormat8bit(GX_DEV_HANDLE hDevice);
private:
	//相机对象
	GX_DEV_HANDLE *m_Cameras;												///< 相机对象句柄
	GX_EVENT_CALLBACK_HANDLE  *m_hEventCB;                                  ///< 掉线回调句柄
	GX_DEVICE_BASE_INFO* m_pBaseinfo; 										///< 设备信息结构体
	CAMER_INFO* m_pstCam;													///< 相机数据结构体
	uint32_t m_nCamCount;

	string m_strCamSn[MaxCameraNumber];//本地保存的相机sn

	uint32_t GetCameraIndex(uint32_t iCam);
	




public:
	

	void InitializeGx(/*UINT Msg*/);
	void ReleasesGx();
	bool InitCamera();
	bool OpenCamera(int CameraIndex);
	bool CloseCamera(int CameraIndex);
	bool StartGrabCamera(int CameraIndex);
	bool StopGrabCamera(int CameraIndex);
	bool GetCameraMaxWidth(int CameraIndex, int& nMaxWdith);
	bool GetCameraMaxHeight(int CameraIndex, int& nMaxHeight);
	bool SetCameraOffsetX(int CameraIndex, int offsetx);
	bool SetCameraOffsetY(int CameraIndex, int offsety);
	bool SetCameraWidth(int CameraIndex, int width);
	bool SetCameraHeight(int CameraIndex, int height);
	bool SetCameraReverseX(int CameraIndex, bool reverse);
	bool SetCameraReverseY(int CameraIndex, bool reverse);
	bool GetExposetimeRange(int CameraIndex, int& nMin, int& nMax);
	bool GetGainRange(int CameraIndex, int& nMin, int& nMax);
	bool SetExposetime(int CameraIndex, double dShutter);
	bool GetExposetime(int CameraIndex, double &dShutter);
	bool SetGain(int CameraIndex, double dGain);
	bool GetGain(int CameraIndex, double &dGain);
	bool ImportConfigFile(int CameraIndex);
	bool ExportConfigFile(int CameraIndex);
	
	///给相机注册相应的回调，目前最多支持4相机
	void RegisterCallback(int nCamIndex);
	void RegisterOffLineCallback(int nCamIndex);
#if 1
	///0号相机回调
	static void __stdcall OnFrameCallbackFun1(GX_FRAME_CALLBACK_PARAM* pFrame);
	///1号相机回调
	static void __stdcall OnFrameCallbackFun2(GX_FRAME_CALLBACK_PARAM* pFrame);
	///2号相机回调
	static void __stdcall OnFrameCallbackFun3(GX_FRAME_CALLBACK_PARAM* pFrame);
	///3号相机回调
	static void __stdcall OnFrameCallbackFun4(GX_FRAME_CALLBACK_PARAM* pFrame);
#endif
	static void __stdcall OnDeviceOfflineCallbackFun1(void* pUserParam);
	static void __stdcall OnDeviceOfflineCallbackFun2(void* pUserParam);
	static void __stdcall OnDeviceOfflineCallbackFun3(void* pUserParam);
	static void __stdcall OnDeviceOfflineCallbackFun4(void* pUserParam);

	void GetCameraSN(string strSN, int iCam);
	void SetCameraSN(string strSN, int iCam);
	size_t GetCamNum() { return m_nCamCount; };
	int GetImageWidth(int CameraIndex);
	int GetImageHeight(int CameraIndex);
	int GrabImage(int CameraIndex, Mat& outImage);

	void ImageUpToDown(UCHAR* src, int width, int height, int iColor);
	void ImageLeftToRight(BYTE* Buffer, int width, int height, int iColor);
	
};
