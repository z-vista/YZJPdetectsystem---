#pragma once
//�����Դ
struct GxCAMERALIGHT_PARAMETER
{
	BOOL b_CameraLight_Online; //�����Դ�Ƿ�����
	BOOL b_NeedWarning;  //��������Ƿ���Ҫͨ����ʾ�ƽ�����ʾ
	int i_Com; //�����Դcom��
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

