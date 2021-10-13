#include "stdafx.h"
#include "ContrlLight.h"


CContrlLight::CContrlLight(void)
{
}


CContrlLight::~CContrlLight(void)
{
}

BOOL CContrlLight::InitLightCom(HANDLE* phCom,int iPort)
{
	BOOL rb = TRUE;
	if (iPort<=0) return FALSE;
	//重叠方式
	char chPort[5]={0};
	sprintf(chPort,"COM%d",iPort);
	*phCom = CreateFile(chPort,
						GENERIC_READ|GENERIC_WRITE,
						0,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_OVERLAPPED,
						NULL);

	if ((*phCom) == (HANDLE)-1)
	{
		rb = FALSE;
	}

	return rb;
}


BOOL CContrlLight::SetComParm(HANDLE hCom)
{
	BOOL rb = TRUE;
	DCB  comDCB;

	if (hCom == NULL) return FALSE;

	COMMTIMEOUTS TimeOuts;
	SetupComm(hCom,4096,4096);
	// 把间隔超时设为最大，把总超时设为0将导致ReadFile立即返回并完成操作
	TimeOuts.ReadIntervalTimeout=MAXDWORD; 
	TimeOuts.ReadTotalTimeoutMultiplier=0; 
	TimeOuts.ReadTotalTimeoutConstant=0; 
	/* 设置写超时以指定WriteComm成员函数中的GetOverlappedResult函数的等待时间*/
	TimeOuts.WriteTotalTimeoutMultiplier=20;
	TimeOuts.WriteTotalTimeoutConstant=2000;
	SetCommTimeouts(hCom, &TimeOuts);

	rb = GetCommState(hCom,&comDCB);
	if (!rb)
		return rb;

	if (comDCB.BaudRate != 19200) comDCB.BaudRate = 19200;
	if (comDCB.ByteSize != 8) comDCB.ByteSize = 8;
	if (comDCB.StopBits != 0) comDCB.StopBits = 0;
	if (comDCB.Parity != 0) comDCB.Parity = 0;

	rb = SetCommState(hCom,&comDCB);
	if (!rb)
		OutputDebugString("xx--设置串口失败!");

	SetCommMask(hCom, EV_RXCHAR|EV_TXEMPTY);
	return rb;
}

BOOL CContrlLight::UninitLightCom(HANDLE hCom)
{
	BOOL rb = TRUE;
	if (hCom == NULL) return FALSE;
	rb = CloseHandle(hCom);
	return rb;
}

BOOL CContrlLight::SetLuminance(HANDLE hCom,int ivalue1,int ivalue2)
{
	BOOL rb = TRUE;
	int iSent = 0;
	unsigned char bytesSent[12] ={0};
	char c1,c2;
	DWORD dcount=0;

	if (hCom == NULL) return FALSE;
	OVERLAPPED ov_Write={0};
	

	if (ivalue1>0) 
		c1 = 'T';
	else
		c1 = 'F';

	if (ivalue2>0)
		c2 = 'T';
	else
		c2 = 'F';

	sprintf((char*)bytesSent,"S%03d%c%03d%cC#",ivalue1,c1,ivalue2,c2);

	COMSTAT ComStat;
	DWORD dwErrorFlags;
	ClearCommError(hCom,&dwErrorFlags,&ComStat);
	PurgeComm(hCom,PURGE_TXCLEAR);//清除发送缓冲区
	PurgeComm(hCom,PURGE_RXCLEAR);//清除接受缓冲区

	rb = WriteFile(hCom,bytesSent,11,&dcount,&ov_Write);
	if(!rb)
	{
		DWORD dwnError=GetLastError();
		if(dwnError ==ERROR_IO_PENDING)
		{
			rb = GetOverlappedResult(hCom,&ov_Write,&dcount,TRUE);// 等待
		}
	}
	
	return rb;
}

BOOL CContrlLight::SetLuminanceEx(HANDLE hCom, int ivalue1, int ivalue2)
{
	BOOL rb = TRUE;
	int iSent = 0;
	unsigned char bytesSent[29] = { 0 };
	
	DWORD dcount = 0;

	if (hCom == NULL) return FALSE;
	OVERLAPPED ov_Write = { 0 };

	
	sprintf((char*)bytesSent, "SA%04d#SB%04d#SC%04d#SD%04d#", ivalue1, ivalue1, ivalue2, ivalue2);
	CString str;
	str.Format("xxx--Light:亮度1为%d,亮度2为%d，发送%s,长度%d\n", ivalue1, ivalue2, bytesSent, sizeof(bytesSent) / sizeof(bytesSent[0]));
	OutputDebugString(str);
	
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	ClearCommError(hCom, &dwErrorFlags, &ComStat);
	PurgeComm(hCom, PURGE_TXCLEAR);//清除发送缓冲区
	PurgeComm(hCom, PURGE_RXCLEAR);//清除接受缓冲区

	rb = WriteFile(hCom, bytesSent, 28, &dcount, &ov_Write);
	if (!rb)
	{
		DWORD dwnError = GetLastError();
		if (dwnError == ERROR_IO_PENDING)
		{
			rb = GetOverlappedResult(hCom, &ov_Write, &dcount, TRUE);// 等待
		}
	}

	return rb;
}


