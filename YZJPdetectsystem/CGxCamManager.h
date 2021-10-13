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
// �����������
#define MaxCameraNumber 4
//---------------------------------------------------------------------------------
/**
\brief   ���߻ص�����
\param   pUserParam   �û�����

\return  void
*/
//----------------------------------------------------------------------------------

///��������ṹ��
typedef struct CAMER_INFO
{
	//BITMAPINFO*					pBmpInfo;		  ///< BITMAPINFO �ṹָ�룬��ʾͼ��ʱʹ��
	BYTE*						pImageBuffer;	  ///< ָ�򾭹�������ͼ�����ݻ�����
	BYTE*						pRawBuffer;		  ///< ָ��ԭʼRAWͼ������
	//char						chBmpBuf[2048];	  ///< BIMTAPINFO �洢��������m_pBmpInfo��ָ��˻�����	
	int64_t                     nPayLoadSise;     ///< ͼ����С
	int64_t					    nImageWidth;	  ///< ͼ����
	int64_t					    nImageHeight;	  ///< ͼ��߶�	
	int64_t					    nBayerLayout;	  ///< Bayer�Ų���ʽ
	bool						bIsColorFilter;	  ///< �ж��Ƿ�Ϊ��ɫ���
	BOOL						bIsOpen;		  ///< ����Ѵ򿪱�־
	BOOL						bIsSnap;		  ///< ������ڲɼ���־
	bool                        bIsOffLine;       ///< ������߱�ʶ
	float						fFps;			  ///< ֡��
}CAMER_INFO;


class CGxCamManager
{
private:

	CGxCamManager();
	CGxCamManager(const CGxCamManager &);
	CGxCamManager& operator=(const CGxCamManager &);	
	~CGxCamManager();
	static void Destory(CGxCamManager*) { m_Instance->ReleasesGx(); OutputDebugString("�������������Destory\n"); };

	static shared_ptr<CGxCamManager> m_Instance;

	//����Ψһ��������������������ɾ��CSingleton��ʵ�� 

public:
	static shared_ptr<CGxCamManager> GetInstance();

	void ShowErrorString(GX_STATUS emErrorStatus);	/// ��ȡ������Ϣ����
	GX_STATUS SetPixelFormat8bit(GX_DEV_HANDLE hDevice);
private:
	//�������
	GX_DEV_HANDLE *m_Cameras;												///< ���������
	GX_EVENT_CALLBACK_HANDLE  *m_hEventCB;                                  ///< ���߻ص����
	GX_DEVICE_BASE_INFO* m_pBaseinfo; 										///< �豸��Ϣ�ṹ��
	CAMER_INFO* m_pstCam;													///< ������ݽṹ��
	uint32_t m_nCamCount;

	string m_strCamSn[MaxCameraNumber];//���ر�������sn

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
	
	///�����ע����Ӧ�Ļص���Ŀǰ���֧��4���
	void RegisterCallback(int nCamIndex);
	void RegisterOffLineCallback(int nCamIndex);
#if 1
	///0������ص�
	static void __stdcall OnFrameCallbackFun1(GX_FRAME_CALLBACK_PARAM* pFrame);
	///1������ص�
	static void __stdcall OnFrameCallbackFun2(GX_FRAME_CALLBACK_PARAM* pFrame);
	///2������ص�
	static void __stdcall OnFrameCallbackFun3(GX_FRAME_CALLBACK_PARAM* pFrame);
	///3������ص�
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
