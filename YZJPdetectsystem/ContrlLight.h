#pragma once
//相机光源
struct GxCAMERALIGHT_PARAMETER
{
	BOOL b_CameraLight_Online; //相机光源是否在线
	BOOL b_NeedWarning;  //在线情况是否需要通过警示灯进行提示
	int i_Com; //相机光源com口
};

class CContrlLight
{
public:
	CContrlLight(void);
	~CContrlLight(void);

	BOOL InitLightCom(HANDLE* hCom,int iPort);
	BOOL SetComParm(HANDLE hCom);
	BOOL SetLuminance(HANDLE hCom,int ivalue1,int ivalue2);
	BOOL SetLuminanceEx(HANDLE hCom, int ivalue, int LightNum);
	BOOL UninitLightCom(HANDLE hCom);
};

