#include "stdafx.h"
#include "CGxCamManager.h"
#include "YZJPdetectsystemDlg.h"

extern YZJPdetectsystemDlg* g_dlg;
extern NetDataLog mlogger;
CGxCamManager::CGxCamManager()
{

	InitializeGx();
	mlogger.TraceKeyInfo("��������๹��\n");
}
CGxCamManager::~CGxCamManager()
{
	mlogger.TraceKeyInfo("�������������\n");
}
// �������캯��
CGxCamManager::CGxCamManager(const CGxCamManager&)
{
	
}
// ��ֵ�����
CGxCamManager& CGxCamManager::operator = (const CGxCamManager&)
{
	return *this;
}
// ��ȡ���ʵ��
shared_ptr<CGxCamManager> CGxCamManager::GetInstance()
{
	
	/*m_Instance->mlogger.TraceKeyInfo("CGxCamManager::GetInstance()");*/
	return m_Instance;
}
void CGxCamManager::InitializeGx(/*UINT Msg*/)
{
	mlogger.TraceKeyInfo("xxxx--InitializeGx() begin \n");
	GX_STATUS emStatus = GX_STATUS_SUCCESS;


	emStatus = GXInitLib();
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--InitializeGx() err \n");
		ShowErrorString(emStatus);
	}
}

void CGxCamManager::ReleasesGx()
{
	mlogger.TraceKeyInfo("xxxx--ReleasesGx() begin \n");
	GX_STATUS emStatus = GX_STATUS_ERROR;
	//�ͷ��ڴ�
	if (m_pBaseinfo != NULL)
	{
		delete[]m_pBaseinfo;
		m_pBaseinfo = NULL;
	}
	if (m_Cameras != NULL)
	{
		delete[]m_Cameras;
		m_Cameras = NULL;
	}
	if (m_pstCam != NULL)
	{
		for (uint32_t i = 0; i < m_nCamCount; i++)
		{
			if (m_pstCam[i].pImageBuffer != NULL)
			{
				delete[]m_pstCam[i].pImageBuffer;
				m_pstCam[i].pImageBuffer = NULL;
			}
			if (m_pstCam[i].pRawBuffer != NULL)
			{
				delete[]m_pstCam[i].pRawBuffer;
				m_pstCam[i].pRawBuffer = NULL;
			}
		}
		delete[]m_pstCam;
		m_pstCam = NULL;
	}
	emStatus = GXCloseLib();
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--ReleasesGx() err \n");
		ShowErrorString(emStatus);
	}
}
//����豸��ʼ�����豸����
bool CGxCamManager::InitCamera()
{
	mlogger.TraceKeyInfo("xxxx--InitCamera() begin \n");
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	m_Cameras = NULL;									//��ʼ��������
	m_pstCam = NULL;									//��ʼ����������ṹ��
	m_pBaseinfo = NULL;									//��ʼ���豸��Ϣ�ṹ��
	m_nCamCount = 0;

	uint32_t          nDeviceNum = 0;     // �豸���� 
	emStatus = GXUpdateDeviceList(&nDeviceNum, 1000);

	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXUpdateDeviceList() err ��\n");
		ShowErrorString(emStatus);
	}

	if (nDeviceNum <= 0)//δ���������
		return false;

	if (nDeviceNum != MaxCameraNumber)//�����������ȷ
		return false;
	mlogger.TraceKeyInfo("xxxx--��ʼ�����������%d ��\n",nDeviceNum);
	m_pBaseinfo = new GX_DEVICE_BASE_INFO[nDeviceNum];
	
	//��ȡ����ö���������Ϣ
	size_t nSize = nDeviceNum * sizeof(GX_DEVICE_BASE_INFO);  //����ṹ���С
	emStatus = GXGetAllDeviceBaseInfo(m_pBaseinfo, &nSize);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetAllDeviceBaseInfo() err ��\n");
		ShowErrorString(emStatus);
		delete[]m_pBaseinfo;
		m_pBaseinfo = NULL;

		// ���豸������Ϊ0
		nDeviceNum = 0;

		return false;
	}
	mlogger.TraceInfo("xxxx--ƥ�������ӦSN�ͱ�� ��\n");

	//��ʼ����Դ
	m_Cameras = new GX_DEV_HANDLE[nDeviceNum];
	m_pstCam = new CAMER_INFO[nDeviceNum];
	for (uint32_t i = 0; i < nDeviceNum; i++)
	{
		
		//mlogger.TraceKeyInfo("xxxx--���%dSN: %s ��\n",i, m_pBaseinfo[i].szSN);
		//if (0 == m_strCamSn[0].compare(m_pBaseinfo[i].szSN)) {
		//	m_CameraIndex1 = i;
		//	mlogger.TraceKeyInfo("xxxx--m_CameraIndex1=%d ��\n", i);
		//	
		//}
		//else if (0 == m_strCamSn[1].compare(m_pBaseinfo[i].szSN)) {
		//	mlogger.TraceKeyInfo("xxxx--m_CameraIndex2=%d ��\n", i);
		//	m_CameraIndex2 = i;
		//}
		//else if (0 == m_strCamSn[2].compare(m_pBaseinfo[i].szSN)) {
		//	mlogger.TraceKeyInfo("xxxx--m_CameraIndex3=%d ��\n", i);
		//	m_CameraIndex3 = i;
		//}
		//else if (0 == m_strCamSn[3].compare(m_pBaseinfo[i].szSN)) {
		//	mlogger.TraceKeyInfo("xxxx--m_CameraIndex4=%d ��\n", i);
		//	m_CameraIndex4 = i;
		//}
		m_Cameras[i] = NULL;

		m_pstCam[i].bIsColorFilter = false;
		m_pstCam[i].bIsOpen = FALSE;
		m_pstCam[i].bIsSnap = FALSE;
		//m_pstCam[i].pBmpInfo = NULL;
		m_pstCam[i].pRawBuffer = NULL;
		m_pstCam[i].pImageBuffer = NULL;
		m_pstCam[i].fFps = 0.0;
		m_pstCam[i].nBayerLayout = 0;
		m_pstCam[i].nImageHeight = 0;
		m_pstCam[i].nImageWidth = 0;
		m_pstCam[i].nPayLoadSise = 0;
		//memset(m_pstCam[i].chBmpBuf, 0, sizeof(m_pstCam[i].chBmpBuf));
	}
	m_nCamCount =nDeviceNum;

	return true;

}

bool CGxCamManager::OpenCamera(int CameraIndex)
{
	mlogger.TraceInfo("xxxx--OpenCamera() %d begin \n", CameraIndex);
	
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	//CameraIndex=GetCameraIndex(CameraIndex);
	//mlogger.TraceKeyInfo("xxxx--GetCameraIndex()= %d begin \n", CameraIndex);
	
	char      szIndex[10] = { 0 };
	GX_OPEN_PARAM     stOpenParam;          // ���������
	stOpenParam.accessMode = GX_ACCESS_EXCLUSIVE;
	stOpenParam.openMode = GX_OPEN_SN;
	stOpenParam.pszContent = "";
	//mlogger.TraceKeyInfo("xxxx--_itoa1 \n");
	//_itoa(CameraIndex+1, szIndex, 10);
	//mlogger.TraceKeyInfo("xxxx--_itoa2 \n");

	if (m_Cameras[CameraIndex] != NULL)
	{
		mlogger.TraceInfo("xxxx--m_Cameras[%d] not null! close \n", CameraIndex);
		emStatus = GXCloseDevice(m_Cameras[CameraIndex]);
		m_Cameras[CameraIndex] = NULL;
	}
	// ͨ����֪��SN���豸
	stOpenParam.pszContent = (char*)m_strCamSn[CameraIndex].c_str();

	mlogger.TraceKeyInfo("xxxx--Open Device %d by SN %s \n", CameraIndex, stOpenParam.pszContent);
	emStatus = GXOpenDevice(&stOpenParam, &m_Cameras[CameraIndex]);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXOpenDevice() err ��\n");
		ShowErrorString(emStatus);
		return false;
	}

	//���������ļ�ǰҪ���豸ͼ���С����Ϊ�������ļ���ͼ���Сһ��
	int64_t nMaxWidth = 0, nMaxHeight = 0;
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_WIDTH_MAX, &nMaxWidth);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_INT_WIDTH_MAX() err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_HEIGHT_MAX, &nMaxHeight);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_INT_HEIGHT_MAX() err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	char* cp = new char[64];
	std::string tp_NodeStr;
	int nMaxPic = GetPrivateProfileInt(_T("Set_Camera"), _T("useMaxPic"), 1, g_dlg->lpPath);;
	if (nMaxPic)
	{
		//����
		emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_OFFSET_X, 0);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_INT_OFFSET_X() err ��\n");
			ShowErrorString(emStatus);
			return false;
		}
		emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_OFFSET_Y, 0);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_INT_OFFSET_Y() err ��\n");
			ShowErrorString(emStatus);
			return false;
		}

	
		emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_WIDTH, nMaxWidth);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_INT_WIDTH() err ��\n");
			ShowErrorString(emStatus);
			return false;
		}
		emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_HEIGHT, nMaxHeight);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_INT_HEIGHT() err ��\n");
			ShowErrorString(emStatus);
			return false;
		}

		// ���������ļ�
		if (!ImportConfigFile(CameraIndex))
		{
			mlogger.TraceError("xxxx--ImportConfigFile() err ��\n");
			return false;
		}
	}
	else
	{
		
		// ���������ļ�
		//if (!ImportConfigFile(CameraIndex))
		//{
		//	mlogger.TraceError("xxxx--ImportConfigFile() err ��\n");
		//	return false;
		//}
		int nValue = 0;
		sprintf(cp, "ReverseX%d", CameraIndex + 1);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
		emStatus = GXSetBool(m_Cameras[CameraIndex], GX_BOOL_REVERSE_X, nValue==1?true:false);//����ͼ����
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_BOOL_REVERSE_X() err ��\n");
			ShowErrorString(emStatus);
			return false;
		}
		sprintf(cp, "ReverseY%d", CameraIndex + 1);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
		emStatus = GXSetBool(m_Cameras[CameraIndex], GX_BOOL_REVERSE_Y, nValue == 1 ? true : false);//����ͼ����
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_BOOL_REVERSE_Y() err ��\n");
			ShowErrorString(emStatus);
			return false;
		}
		sprintf(cp, "RunImage%d_w", CameraIndex + 1);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
		if (0 < nValue && nValue <= nMaxWidth)
		{
			emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_WIDTH, nValue);
			if (emStatus != GX_STATUS_SUCCESS)
			{
				mlogger.TraceError("xxxx--GX_INT_WIDTH() err ��\n");
				ShowErrorString(emStatus);
				return false;
			}
		}
		sprintf(cp, "RunImage%d_h", CameraIndex + 1);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
		if (0 < nValue && nValue <= nMaxWidth)
		{
			emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_HEIGHT, nValue);
			if (emStatus != GX_STATUS_SUCCESS)
			{
				mlogger.TraceError("xxxx--GX_INT_HEIGHT() err ��\n");
				ShowErrorString(emStatus);
				return false;
			}
		}
		sprintf(cp, "RunImage%d_x", CameraIndex + 1);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
		if (0 < nValue && nValue <= nMaxWidth)
		{
			emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_OFFSET_X, nValue);
			if (emStatus != GX_STATUS_SUCCESS)
			{
				mlogger.TraceError("xxxx--GX_INT_OFFSET_X() err ��\n");
				ShowErrorString(emStatus);
				return false;
			}
		}
		sprintf(cp, "RunImage%d_y", CameraIndex + 1);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
		if (0 < nValue && nValue <= nMaxWidth)
		{
			emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_OFFSET_Y, nValue);
			if (emStatus != GX_STATUS_SUCCESS)
			{
				mlogger.TraceError("xxxx--GX_INT_OFFSET_Y() err ��\n");
				ShowErrorString(emStatus);
				return false;
			}
		}
		
	}

	//�����ع�������
	int nShutter = 0, nGain = 0;
	double  dOldShutter =0,dOldGain=0;
	
	GetExposetime(CameraIndex,dOldShutter);
	sprintf(cp, "Exposetime%d", CameraIndex+1);
	tp_NodeStr = cp;
	nShutter = GetPrivateProfileInt("Set_Camera", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
	if (nShutter != dOldShutter) {
		SetExposetime(CameraIndex, nShutter);
		mlogger.TraceKeyInfo("xxxx--�����ع�:%d   ��\n", nShutter);
	}
		
	GetGain(CameraIndex,dOldGain);
	sprintf(cp, "Gain%d", CameraIndex + 1);
	tp_NodeStr = cp;
	nGain = GetPrivateProfileInt("Set_Camera", tp_NodeStr.c_str(), 0, g_dlg->lpPath);//
	if(nGain !=dOldGain){
		SetGain(CameraIndex, nGain);
	mlogger.TraceKeyInfo("xxxx--��������:%d   ��\n", nGain);
	}

	// ��֪��ǰ���֧���ĸ�8λͼ�����ݸ�ʽ����ֱ������
	// �����豸֧�����ݸ�ʽGX_PIXEL_FORMAT_BAYER_GR8,��ɰ������´���ʵ��
	// emStatus = GXSetEnum(m_hDevice, GX_ENUM_PIXEL_FORMAT, GX_PIXEL_FORMAT_BAYER_GR8);
	// VERIFY_STATUS_RET(emStatus);

	// �������ǰ��������ݸ�ʽʱ�����Ե������º�����ͼ�����ݸ�ʽ����Ϊ8Bit
	emStatus = SetPixelFormat8bit(m_Cameras[CameraIndex]);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--SetPixelFormat8bit err ��\n");
		ShowErrorString(emStatus);
		return false;
	}


	//���òɼ�ģʽ�����ɼ�
	emStatus = GXSetEnum(m_Cameras[CameraIndex], GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXSetEnum  GX_ENUM_ACQUISITION_MODEerr ��\n");
		ShowErrorString(emStatus);
		return false;
	}

	//���ô���ģʽΪ��
	emStatus = GXSetEnum(m_Cameras[CameraIndex], GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_OFF);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXSetEnum GX_ENUM_TRIGGER_MODE err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	m_pstCam[CameraIndex].bIsOpen = TRUE;
	mlogger.TraceKeyInfo("xxxx--bIsOpen    sucessed��\n");
	return true;
}
bool CGxCamManager::CloseCamera(int CameraIndex)
{
	
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	/*CameraIndex = GetCameraIndex(CameraIndex);
	mlogger.TraceKeyInfo("xxxx--CloseCamera() %d begin \n", CameraIndex);*/
	if (m_pstCam[CameraIndex].bIsSnap)
	{
		//ֹͣ�ɼ�
		emStatus = GXSendCommand(m_Cameras[CameraIndex], GX_COMMAND_ACQUISITION_STOP);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_COMMAND_ACQUISITION_STOP() err ��\n");
			// ������
			ShowErrorString(emStatus);
			return false;
		}

		//ע���ص�
		emStatus = GXUnregisterCaptureCallback(m_Cameras[CameraIndex]);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			// ������
			mlogger.TraceError("xxxx--GXUnregisterCaptureCallback() err ��\n");
			ShowErrorString(emStatus);
			return false;
		}
		//�ͷ���Դ
		if (m_pstCam[CameraIndex].pImageBuffer != NULL)
		{
			delete[]m_pstCam[CameraIndex].pImageBuffer;
			m_pstCam[CameraIndex].pImageBuffer = NULL;
		}

		if (m_pstCam[CameraIndex].pRawBuffer != NULL)
		{
			delete[]m_pstCam[CameraIndex].pRawBuffer;
			m_pstCam[CameraIndex].pRawBuffer = NULL;
		}

		m_pstCam[CameraIndex].bIsSnap = FALSE;
	}
	/*emStatus = GXUnregisterDeviceOfflineCallback(m_Cameras[CameraIndex], m_hEventCB[CameraIndex]);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceKeyInfo("xxxx--GXUnregisterDeviceOfflineCallback() err ��\n");
		ShowErrorString(emStatus); 
		return false;
	}*/
	if (m_pstCam[CameraIndex].bIsOpen)
	{
		//�ر����
		emStatus = GXCloseDevice(m_Cameras[CameraIndex]);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GXCloseDevice() err ��\n");
			// ������
			ShowErrorString(emStatus);
			return false;
		}

		m_Cameras[CameraIndex] = NULL;
		m_pstCam[CameraIndex].bIsOpen = FALSE;
	}
	mlogger.TraceKeyInfo("xxxx--CloseCamera() %d end \n", CameraIndex);
	return true;
}
bool CGxCamManager::StartGrabCamera(int CameraIndex)
{

	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	int64_t   nValue = 0;
	bool      bIsImplemented = false;
	if (m_Cameras == NULL)
		return false;
	//��ȡ���Bayerת�����ͼ��Ƿ�֧�������ɫͼ��
	emStatus = GXIsImplemented(m_Cameras[CameraIndex], GX_ENUM_PIXEL_COLOR_FILTER, &bIsImplemented);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXIsImplemented  err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	mlogger.TraceKeyInfo("xxxx--bIsImplemented:%d   ��\n", bIsImplemented);
	m_pstCam[CameraIndex].bIsColorFilter = bIsImplemented;
	if (bIsImplemented)
	{
		emStatus = GXGetEnum(m_Cameras[CameraIndex], GX_ENUM_PIXEL_COLOR_FILTER, &nValue);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GXGetEnum GX_ENUM_PIXEL_COLOR_FILTER  err ��\n");
			ShowErrorString(emStatus);
			return false;
		}
		m_pstCam[CameraIndex].nBayerLayout = nValue;
	}

	// ��ȡͼ���
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_WIDTH, &nValue);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetInt GX_INT_WIDTH err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	m_pstCam[CameraIndex].nImageWidth = nValue;
	mlogger.TraceKeyInfo("xxxx--nImageWidth:%d   ��\n", nValue);
	// ��ȡͼ���
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_HEIGHT, &nValue);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetInt GX_INT_HEIGHT err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	m_pstCam[CameraIndex].nImageHeight = nValue;
	mlogger.TraceKeyInfo("xxxx--nImageHeight:%d   ��\n", nValue);
	// ��ȡԭʼͼ���С
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_PAYLOAD_SIZE, &nValue);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetInt GX_INT_PAYLOAD_SIZE err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	m_pstCam[CameraIndex].nPayLoadSise = nValue;
	mlogger.TraceKeyInfo("xxxx--nPayLoadSise:%d   ��\n", nValue);

	/*CameraIndex = GetCameraIndex(CameraIndex);
	mlogger.TraceKeyInfo("xxxx--StartGrabCamera() %d begin \n", CameraIndex);*/
	bool      bIsColorFilter = m_pstCam[CameraIndex].bIsColorFilter;

	//// ��ʼ��BMPͷ��Ϣ
	//m_pstCam[CameraIndex].pBmpInfo = (BITMAPINFO*)(m_pstCam[CameraIndex].chBmpBuf);
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biWidth = (LONG)m_pstCam[CameraIndex].nImageWidth;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biHeight = (LONG)m_pstCam[CameraIndex].nImageHeight;

	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biPlanes = 1;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biBitCount = bIsColorFilter ? 24 : 8; // ��ɫͼ��Ϊ24,�ڰ�ͼ��Ϊ8;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biCompression = BI_RGB;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biSizeImage = 0;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biXPelsPerMeter = 0;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biYPelsPerMeter = 0;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biClrUsed = 0;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biClrImportant = 0;

	//// �ڰ������Ҫ��ʼ����ɫ����Ϣ
	//if (!bIsColorFilter)
	//{
	//	// �ڰ������Ҫ���г�ʼ����ɫ�����
	//	for (int i = 0; i < 256; i++)
	//	{
	//		m_pstCam[CameraIndex].pBmpInfo->bmiColors[i].rgbBlue = i;
	//		m_pstCam[CameraIndex].pBmpInfo->bmiColors[i].rgbGreen = i;
	//		m_pstCam[CameraIndex].pBmpInfo->bmiColors[i].rgbRed = i;
	//		m_pstCam[CameraIndex].pBmpInfo->bmiColors[i].rgbReserved = i;
	//	}
	//}

	// ���Buffer
	if (m_pstCam[CameraIndex].pImageBuffer != NULL)
	{
		delete[]m_pstCam[CameraIndex].pImageBuffer;
		m_pstCam[CameraIndex].pImageBuffer = NULL;
	}

	if (m_pstCam[CameraIndex].pRawBuffer != NULL)
	{
		delete[]m_pstCam[CameraIndex].pRawBuffer;
		m_pstCam[CameraIndex].pRawBuffer = NULL;
	}

	// ������Դ
	m_pstCam[CameraIndex].pRawBuffer = new BYTE[(size_t)(m_pstCam[CameraIndex].nPayLoadSise)];
	if (m_pstCam[CameraIndex].pRawBuffer == NULL)
	{

		mlogger.TraceKeyInfo("xxxx--������Դʧ�� \n");
		//MessageBox("������Դʧ��!");
		return false;
	}

	// ����ǲ�ɫ�����Ϊ��ɫ���������Դ
	if (bIsColorFilter)
	{
		m_pstCam[CameraIndex].pImageBuffer = new BYTE[(size_t)(m_pstCam[CameraIndex].nImageWidth * m_pstCam[CameraIndex].nImageHeight * 3)];
		if (m_pstCam[CameraIndex].pImageBuffer == NULL)
		{
			mlogger.TraceKeyInfo("xxxx--������Դʧ�� \n");
			delete[]m_pstCam[CameraIndex].pRawBuffer;
			m_pstCam[CameraIndex].pRawBuffer = NULL;
			return false;
		}
	}

	// ����Ǻڰ������ Ϊ�ڰ����������Դ
	else
	{
		m_pstCam[CameraIndex].pImageBuffer = new BYTE[(size_t)(m_pstCam[CameraIndex].nImageWidth * m_pstCam[CameraIndex].nImageHeight)];
		if (m_pstCam[CameraIndex].pImageBuffer == NULL)
		{
			mlogger.TraceKeyInfo("xxxx--������Դʧ�� \n");
			delete[]m_pstCam[CameraIndex].pRawBuffer;
			m_pstCam[CameraIndex].pRawBuffer = NULL;
			return false;
		}
	}

	//ע��ص�
	RegisterCallback(CameraIndex);
	//��ʼ�ɼ�
	emStatus = GXSendCommand(m_Cameras[CameraIndex], GX_COMMAND_ACQUISITION_START);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		// ������
		ShowErrorString(emStatus);
		return false;
	}

	m_pstCam[CameraIndex].bIsSnap = TRUE;
	return true;
}
bool CGxCamManager::StopGrabCamera(int CameraIndex)
{
	GX_STATUS emStatus = GX_STATUS_ERROR;
	/*CameraIndex = GetCameraIndex(CameraIndex);
	mlogger.TraceKeyInfo("xxxx--StopGrabCamera() %d begin \n", CameraIndex);*/
	//ֹͣ�ɼ�
	if (m_Cameras== NULL)
		return false;
	emStatus = GXSendCommand(m_Cameras[CameraIndex], GX_COMMAND_ACQUISITION_STOP);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		// ������
		ShowErrorString(emStatus);
		return false;
	}
	//ע���ص�
	emStatus = GXUnregisterCaptureCallback(m_Cameras[CameraIndex]);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		// ������
		ShowErrorString(emStatus);
		return false;
	}
	//�ͷ���Դ
	if (m_pstCam[CameraIndex].pImageBuffer != NULL)
	{
		delete[]m_pstCam[CameraIndex].pImageBuffer;
		m_pstCam[CameraIndex].pImageBuffer = NULL;
	}

	if (m_pstCam[CameraIndex].pRawBuffer != NULL)
	{
		delete[]m_pstCam[CameraIndex].pRawBuffer;
		m_pstCam[CameraIndex].pRawBuffer = NULL;
	}

	m_pstCam[CameraIndex].bIsSnap = FALSE;
	return true;
}
void CGxCamManager::ShowErrorString(GX_STATUS emErrorStatus) {
	char* pchErrorInfo = NULL;
	size_t    nSize = 0;
	GX_STATUS emStatus = GX_STATUS_ERROR;

	// ��ȡ����������Ϣ����
	emStatus = GXGetLastError(&emErrorStatus, NULL, &nSize);

	pchErrorInfo = new char[nSize];
	if (pchErrorInfo == NULL)
	{
		OutputDebugString("xxxx--GXGetLastError�ӿڵ��� size <1��\n");
		mlogger.TraceError("xxxx--GXGetLastError�ӿڵ��� size <1��\n");
		return;
	}

	// ��ȡ������Ϣ����
	emStatus = GXGetLastError(&emErrorStatus, pchErrorInfo, &nSize);

	// ��ʾ����������Ϣ
	if (emStatus != GX_STATUS_SUCCESS)
	{
		OutputDebugString("xxxx--GXGetLastError�ӿڵ���ʧ�ܣ�\n");
		mlogger.TraceError("xxxx--GXGetLastError�ӿڵ���ʧ�ܣ�\n");
	}
	else
	{
		CString strErrorInfo = NULL;
		strErrorInfo.Format("xxxx--%s\n", (LPCTSTR)pchErrorInfo);
		OutputDebugString(strErrorInfo);
		mlogger.TraceError(strErrorInfo);

	}

	// �ͷ���Դ
	if (pchErrorInfo != NULL)
	{
		delete[]pchErrorInfo;
		pchErrorInfo = NULL;
	}
}	/// ��ȡ������Ϣ����
GX_STATUS CGxCamManager::SetPixelFormat8bit(GX_DEV_HANDLE hDevice)
{
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	int64_t   nPixelSize = 0;
	uint32_t  nEnmuEntry = 0;
	size_t    nBufferSize = 0;
	BOOL      bIs8bit = TRUE;

	GX_ENUM_DESCRIPTION* pEnumDescription = NULL;
	GX_ENUM_DESCRIPTION* pEnumTemp = NULL;

	// ��ȡ���ص��С
	emStatus = GXGetEnum(hDevice, GX_ENUM_PIXEL_SIZE, &nPixelSize);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		return emStatus;
	}

	// �ж�Ϊ8bitʱֱ�ӷ���,��������Ϊ8bit
	if (nPixelSize == GX_PIXEL_SIZE_BPP8)
	{
		return GX_STATUS_SUCCESS;
	}
	else
	{
		// ��ȡ�豸֧�ֵ����ظ�ʽö����
		emStatus = GXGetEnumEntryNums(hDevice, GX_ENUM_PIXEL_FORMAT, &nEnmuEntry);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			return emStatus;
		}

		// Ϊ��ȡ�豸֧�ֵ����ظ�ʽö��ֵ׼����Դ
		nBufferSize = nEnmuEntry * sizeof(GX_ENUM_DESCRIPTION);
		pEnumDescription = new GX_ENUM_DESCRIPTION[nEnmuEntry];

		// ��ȡ֧�ֵ�ö��ֵ
		emStatus = GXGetEnumDescription(hDevice, GX_ENUM_PIXEL_FORMAT, pEnumDescription, &nBufferSize);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			if (pEnumDescription != NULL)
			{
				delete[]pEnumDescription;
				pEnumDescription = NULL;
			}
			return emStatus;
		}

		// �����豸֧�ֵ����ظ�ʽ,�������ظ�ʽΪ8bit,
		// ���豸֧�ֵ����ظ�ʽΪMono10��Mono8��������ΪMono8
		for (uint32_t i = 0; i < nEnmuEntry; i++)
		{
			if ((pEnumDescription[i].nValue & GX_PIXEL_8BIT) == GX_PIXEL_8BIT)
			{
				emStatus = GXSetEnum(hDevice, GX_ENUM_PIXEL_FORMAT, pEnumDescription[i].nValue);
				break;
			}
		}

		// �ͷ���Դ
		if (pEnumDescription != NULL)
		{
			delete[]pEnumDescription;
			pEnumDescription = NULL;
		}
	}

	return emStatus;
}
bool CGxCamManager::GetCameraMaxWidth(int CameraIndex,int & nMaxWdith)
{
	if (m_Cameras == NULL)
	{
		nMaxWdith = 5496;
		return false;
	}
		
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	int64_t nValue = 0;
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_WIDTH_MAX, &nValue);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_INT_WIDTH_MAX() err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	nMaxWdith=nValue;
	return true;
}
bool CGxCamManager::GetCameraMaxHeight(int CameraIndex,int & nMaxHeight)
{
	if (m_Cameras == NULL)
	{
		nMaxHeight = 3672;
		return false;
	}
		
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	int64_t nValue = 0;
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_HEIGHT_MAX, &nValue);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_INT_HEIGHT_MAX() err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	nMaxHeight = nValue;
	return true;
}
bool CGxCamManager::SetCameraOffsetX(int CameraIndex, int offsetx)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_OFFSET_X, offsetx);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_INT_OFFSET_X() err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	return true;
}
bool CGxCamManager::SetCameraOffsetY(int CameraIndex, int offsety)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_OFFSET_Y, offsety);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_INT_OFFSET_Y() err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	return true;
}
bool CGxCamManager::SetCameraWidth(int CameraIndex, int width)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_WIDTH, width);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_INT_WIDTH() err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	return true;
}
bool CGxCamManager::SetCameraHeight(int CameraIndex, int height)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_HEIGHT, height);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_INT_WIDTH() err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	return true;
}
bool CGxCamManager::SetCameraReverseX(int CameraIndex,bool reverse)
{
	mlogger.TraceKeyInfo("xxxx--SetCameraReverseX begin,reverse=%d!", reverse);
	if (m_Cameras == NULL)
	{
		mlogger.TraceKeyInfo("xxxx--SetCameraReverseX Cameras==NULL!");
		return false;
	}
		
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	emStatus = GXSetBool(m_Cameras[CameraIndex], GX_BOOL_REVERSE_X, reverse);//����ͼ����,
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_BOOL_REVERSE_X() err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	return true;
}
bool CGxCamManager::SetCameraReverseY(int CameraIndex, bool reverse)
{
	mlogger.TraceKeyInfo("xxxx--SetCameraReverseY begin,reverse=%d!", reverse);
	if (m_Cameras == NULL)
	{
		mlogger.TraceKeyInfo("xxxx--SetCameraReverseY m_Cameras==NULL!");
		return false;
	}
		
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	emStatus = GXSetBool(m_Cameras[CameraIndex], GX_BOOL_REVERSE_Y, reverse);//����ͼ����,
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_BOOL_REVERSE_Y() err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	return true;
}
bool CGxCamManager::SetExposetime(int CameraIndex,double dShutter)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	GX_FLOAT_RANGE stFloatRange;
	emStatus = GXGetFloatRange(m_Cameras[CameraIndex], GX_FLOAT_EXPOSURE_TIME, &stFloatRange);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetFloatRange GX_FLOAT_EXPOSURE_TIME  err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	//���������ֵ���ع�ֵ��Ϊ���ֵ
	if (dShutter > stFloatRange.dMax)
	{
		dShutter = stFloatRange.dMax;
	}
	//��С����Сֵ���ع�ֵ��Ϊ��Сֵ
	if (dShutter < stFloatRange.dMin)
	{
		dShutter = stFloatRange.dMin;
	}

	emStatus = GXSetFloat(m_Cameras[CameraIndex], GX_FLOAT_EXPOSURE_TIME, dShutter);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXSetFloat GX_FLOAT_EXPOSURE_TIME err dShutter=%d ��\n", dShutter);
		ShowErrorString(emStatus);
		return false;
	}
	
	return true;
}
bool CGxCamManager::GetExposetime(int CameraIndex, double& dShutter)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	double dValue = 0;
	emStatus = GXGetFloat(m_Cameras[CameraIndex], GX_FLOAT_EXPOSURE_TIME, &dValue);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetFloat GX_FLOAT_EXPOSURE_TIME  err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	dShutter = dValue;
	return true;

}
bool CGxCamManager::GetExposetimeRange(int CameraIndex, int& nMin, int& nMax)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	GX_FLOAT_RANGE stFloatRange;
	emStatus = GXGetFloatRange(m_Cameras[CameraIndex], GX_FLOAT_EXPOSURE_TIME, &stFloatRange);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetFloatRange GX_FLOAT_EXPOSURE_TIME  err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	nMin = stFloatRange.dMin;
	nMax = stFloatRange.dMax;
	return true;
}
bool CGxCamManager::GetGainRange(int CameraIndex, int &nMin,int &nMax)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	GX_FLOAT_RANGE stFloatRange;
	emStatus = GXGetFloatRange(m_Cameras[CameraIndex], GX_FLOAT_GAIN, &stFloatRange);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetFloatRange GX_FLOAT_GAIN  err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	nMin = stFloatRange.dMin;
	nMax = stFloatRange.dMax;
	return true;
}
bool CGxCamManager::SetGain(int CameraIndex,double dGain)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	GX_FLOAT_RANGE stFloatRange;
	emStatus = GXGetFloatRange(m_Cameras[CameraIndex], GX_FLOAT_GAIN, &stFloatRange);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetFloatRange GX_FLOAT_GAIN  err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	//���������ֵ���ع�ֵ��Ϊ���ֵ
	if (dGain > stFloatRange.dMax)
	{
		dGain = stFloatRange.dMax;
	}
	//��С����Сֵ���ع�ֵ��Ϊ��Сֵ
	if (dGain < stFloatRange.dMin)
	{
		dGain = stFloatRange.dMin;
	}

	emStatus = GXSetFloat(m_Cameras[CameraIndex], GX_FLOAT_GAIN, dGain);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXSetFloat GX_FLOAT_GAIN err,dGain=%d ��\n", dGain);
		ShowErrorString(emStatus);
		return false;
	}
	return true;
}
bool CGxCamManager::GetGain(int CameraIndex, double& dGain)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	double dValue = 0;
	emStatus = GXGetFloat(m_Cameras[CameraIndex], GX_FLOAT_GAIN, &dValue);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetFloat GX_FLOAT_GAIN  err ��\n");
		ShowErrorString(emStatus);
		return false;
	}
	dGain = dValue;
	return true;

}
bool  CGxCamManager::ImportConfigFile(int CameraIndex)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	CString strConfigFile = NULL;

	char                      chModulePath[100];
	GetModuleFileName(NULL, chModulePath, sizeof(chModulePath));
	(_tcsrchr(chModulePath, _T('\\')))[1] = 0;
	string m_strPath = chModulePath;

	strConfigFile.Format("%s\\CamConfigFile\\%d.txt", m_strPath.c_str(), CameraIndex + 1);

	emStatus = GXImportConfigFile(m_Cameras[CameraIndex], strConfigFile);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXImportConfigFile() %s   err ��\n", strConfigFile);
		ShowErrorString(emStatus);
		return false;
	}
	return true;
}
bool  CGxCamManager::ExportConfigFile(int CameraIndex)
{
	if (m_Cameras == NULL)
		return false;
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	CString strConfigFile = NULL;

	char                      chModulePath[100];
	GetModuleFileName(NULL, chModulePath, sizeof(chModulePath));
	(_tcsrchr(chModulePath, _T('\\')))[1] = 0;
	string m_strPath = chModulePath;

	strConfigFile.Format("%s\\CamConfigFile\\%d.txt", m_strPath.c_str(), CameraIndex + 1);

	emStatus = GXExportConfigFile(m_Cameras[CameraIndex], strConfigFile);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXExportConfigFile() %s   err ��\n", strConfigFile);
		ShowErrorString(emStatus);
		return false;
	}
	return true;
}
void CGxCamManager::RegisterCallback(int nCamID)
{
	GX_STATUS emStatus = GX_STATUS_ERROR;
	mlogger.TraceKeyInfo("xxxx--RegisterCallback() %d begin \n", nCamID);
	//ע��ص�
	switch (nCamID)
	{
	case 0:
		emStatus = GXRegisterCaptureCallback(m_Cameras[nCamID], this, OnFrameCallbackFun1);
		if (emStatus!= GX_STATUS_SUCCESS)
		{
			ShowErrorString(emStatus);
		}
		break;

	case 1:
		emStatus = GXRegisterCaptureCallback(m_Cameras[nCamID], this, OnFrameCallbackFun2);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			ShowErrorString(emStatus);
		}
		break;

	case 2:
		emStatus = GXRegisterCaptureCallback(m_Cameras[nCamID], this, OnFrameCallbackFun3);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			ShowErrorString(emStatus);
		}
		break;

	case 3:
		emStatus = GXRegisterCaptureCallback(m_Cameras[nCamID], this, OnFrameCallbackFun4);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			ShowErrorString(emStatus);
		}
		break;

	default:
		break;
	}

}
void CGxCamManager::RegisterOffLineCallback(int nCamID)
{
	GX_STATUS emStatus = GX_STATUS_ERROR;

	//ע��ص�
	switch (nCamID)
	{
	case 0:
		emStatus = GXRegisterDeviceOfflineCallback(m_Cameras[nCamID], this, OnDeviceOfflineCallbackFun1, &m_hEventCB[nCamID]);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			ShowErrorString(emStatus);
		
		}
		break;

	case 1:
		emStatus = GXRegisterDeviceOfflineCallback(m_Cameras[nCamID], this, OnDeviceOfflineCallbackFun2, &m_hEventCB[nCamID]);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			ShowErrorString(emStatus);
		
		}
		break;

	case 2:
		emStatus = GXRegisterDeviceOfflineCallback(m_Cameras[nCamID], this, OnDeviceOfflineCallbackFun3, &m_hEventCB[nCamID]);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			ShowErrorString(emStatus);
			
		}
		break;

	case 3:
		emStatus = GXRegisterDeviceOfflineCallback(m_Cameras[nCamID], this, OnDeviceOfflineCallbackFun4, &m_hEventCB[nCamID]);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			ShowErrorString(emStatus);
		
		}
		break;

	default:
		break;
	}

}
uint32_t CGxCamManager::GetCameraIndex(uint32_t iCam)
{
	uint32_t Cam = iCam;

	/*if (0 == m_strCamSn[0].compare(m_pBaseinfo[Cam].szSN)) {
		Cam=m_CameraIndex1;
	}
	else if (0 == m_strCamSn[1].compare(m_pBaseinfo[Cam].szSN)) {

		Cam=m_CameraIndex2 ;
	}
	else if (0 == m_strCamSn[2].compare(m_pBaseinfo[Cam].szSN)) {

		Cam=m_CameraIndex3;
	}
	else if (0 == m_strCamSn[3].compare(m_pBaseinfo[Cam].szSN)) {
	
		Cam=m_CameraIndex4;
	}*/
	
	/*switch (iCam)
	{
	case 0:
		Cam = m_CameraIndex1;
		break;
	case 1:
		Cam = m_CameraIndex2;
		break;
	case 2:
		Cam = m_CameraIndex3;
		break;
	case 3:
		Cam = m_CameraIndex4;
		break;
	}*/
	return Cam;
}

void CGxCamManager::GetCameraSN(string strSN,int iCam)
{
	m_strCamSn[iCam]=strSN;
}
void CGxCamManager::SetCameraSN(string strSN, int iCam)
{
	m_strCamSn[iCam] = strSN;
}
int CGxCamManager::GetImageWidth(int CameraIndex)
{
	if (m_pstCam == nullptr)
		return 1;
	else
		return m_pstCam[CameraIndex].nImageWidth;
}
int CGxCamManager::GetImageHeight(int CameraIndex)
{
	if (m_pstCam == nullptr)
		return 1;
	else
		return m_pstCam[CameraIndex].nImageHeight;
}
#if 1
void __stdcall CGxCamManager::OnFrameCallbackFun1(GX_FRAME_CALLBACK_PARAM* pFrame)
{
	if (pFrame->status != 0)
	{
		return;
	}

	CGxCamManager* pf = (CGxCamManager*)(pFrame->pUserParam);
	int		    nID = 0;             //ID��

	BYTE* pImageBuffer = NULL;          //ת�����ͼ��buffer
	BYTE* pRawBuf = NULL;          //ת��ǰͼ��
//	BITMAPINFO* pBmpInfo = NULL;          //ͼ����Ϣ
	int64_t	    nBayerLayout = 0;             //Bayer��ʽ
	int		    nImgWidth = 0;             //ͼ��Ŀ�
	int         nImgHeight = 0;             //ͼ��ĸ�

	int         i = 0;             //ѭ������
	

	//��ʼ������
	//pf->m_pCamsFps[nID].IncreaseFrameNum();

	nImgWidth = (int)(pf->m_pstCam[nID].nImageWidth);
	nImgHeight = (int)(pf->m_pstCam[nID].nImageHeight);
	pImageBuffer = pf->m_pstCam[nID].pImageBuffer;
	pRawBuf = pf->m_pstCam[nID].pRawBuffer;
	nBayerLayout = pf->m_pstCam[nID].nBayerLayout;
	//pBmpInfo = pf->m_pstCam[nID].pBmpInfo;
	//mlogger.TraceKeyInfo("xxxx--���1�ص�--nImgWidth=%d��nImgHeight=%d��nBayerLayout=%d��bIsColorFilter=%d,pf->m_pstCam[nID].nPayLoadSise=%d ��\n", nImgWidth, nImgHeight, nBayerLayout, pf->m_pstCam[nID].bIsColorFilter, pf->m_pstCam[nID].nPayLoadSise);
	//����ͼ��pRawBuffer��
	memcpy(pf->m_pstCam[nID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[nID].nPayLoadSise));
	// ͼ��ֱ����ת��������������Ϊͬһ��buffer��

	BYTE* pOutRawBuf = NULL;          //����ת����ͼ��
	pOutRawBuf = new BYTE[(size_t)(pf->m_pstCam[nID].nPayLoadSise)];
	VxInt32 DxStatus = DxImageMirror((BYTE*)pRawBuf, (BYTE*)pOutRawBuf, nImgWidth, nImgHeight, VERTICAL_MIRROR);

	if (DxStatus != DX_OK)
	{
		return;
	}


	//ͼ��ת��
	if (pf->m_pstCam[nID].bIsColorFilter)
	{
		//��ɫ�����Ҫ����RGBת��
		DxRaw8toRGB24(pOutRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);
		//pf->ImageLeftToRight(pImageBuffer, nImgWidth, nImgHeight, 3);
	}
	else
	{
		//�ڰ�ͼ����Ҫͼ��ת
		for (i = 0; i < nImgHeight; i++)
		{
			memcpy((pImageBuffer + i * nImgWidth), (pOutRawBuf + (nImgHeight - i - 1) * nImgWidth), nImgWidth);
		}
		//pf->ImageLeftToRight(pImageBuffer, nImgWidth, nImgHeight, 1);
	}
	if(pOutRawBuf!=NULL)
	{
		delete[]pOutRawBuf;
		pOutRawBuf = NULL;
	}
	//cv::Mat img = cv::Mat::zeros(cv::Size(nImgWidth, nImgHeight), CV_8U);
	//std::memcpy(img.data, pImageBuffer, nImgWidth * nImgHeight);
	cv::Mat img = cv::Mat(nImgHeight, nImgWidth, pf->m_pstCam[nID].bIsColorFilter==true?CV_8UC3:CV_8UC1, pImageBuffer);
	//flip(img, img, 0);
	g_dlg->srcMat_src.push(img);
}
void __stdcall CGxCamManager::OnFrameCallbackFun2(GX_FRAME_CALLBACK_PARAM* pFrame)
{
	if (pFrame->status != 0)
	{
		return;
	}

	CGxCamManager* pf = (CGxCamManager*)(pFrame->pUserParam);
	int		    nID = 1;             //ID��

	BYTE* pImageBuffer = NULL;          //ת�����ͼ��buffer
	BYTE* pRawBuf = NULL;          //ת��ǰͼ��
	//BITMAPINFO* pBmpInfo = NULL;          //ͼ����Ϣ
	int64_t	    nBayerLayout = 0;             //Bayer��ʽ
	int		    nImgWidth = 0;             //ͼ��Ŀ�
	int         nImgHeight = 0;             //ͼ��ĸ�

	int         i = 0;             //ѭ������

	//��ʼ������
	//pf->m_pCamsFps[nID].IncreaseFrameNum();

	nImgWidth = (int)(pf->m_pstCam[nID].nImageWidth);
	nImgHeight = (int)(pf->m_pstCam[nID].nImageHeight);
	pImageBuffer = pf->m_pstCam[nID].pImageBuffer;
	pRawBuf = pf->m_pstCam[nID].pRawBuffer;
	nBayerLayout = pf->m_pstCam[nID].nBayerLayout;
	//pBmpInfo = pf->m_pstCam[nID].pBmpInfo;

	//����ͼ��pRawBuffer��
	memcpy(pf->m_pstCam[nID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[nID].nPayLoadSise));
	BYTE* pOutRawBuf = NULL;          //����ת����ͼ��
	pOutRawBuf = new BYTE[(size_t)(pf->m_pstCam[nID].nPayLoadSise)];
	VxInt32 DxStatus = DxImageMirror((BYTE*)pRawBuf, (BYTE*)pOutRawBuf, nImgWidth, nImgHeight, VERTICAL_MIRROR);

	if (DxStatus != DX_OK)
	{
		return;
	}

	//ͼ��ת��
	if (pf->m_pstCam[nID].bIsColorFilter)
	{
		//��ɫ�����Ҫ����RGBת��
		DxRaw8toRGB24(pOutRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);
	}
	else
	{
		//�ڰ�ͼ����Ҫͼ��ת
		for (i = 0; i < nImgHeight; i++)
		{
			memcpy((pImageBuffer + i * nImgWidth), (pOutRawBuf + (nImgHeight - i - 1) * nImgWidth), nImgWidth);
		}
	}
	if (pOutRawBuf != NULL)
	{
		delete[]pOutRawBuf;
		pOutRawBuf = NULL;
	}
	//cv::Mat img = cv::Mat::zeros(cv::Size(nImgWidth, nImgHeight), CV_8U);
	//std::memcpy(img.data, pImageBuffer, nImgWidth * nImgHeight);
	cv::Mat img = cv::Mat(nImgHeight, nImgWidth, pf->m_pstCam[nID].bIsColorFilter == true ? CV_8UC3 : CV_8UC1, pImageBuffer);
	//flip(img, img, 0);
	g_dlg->srcMat_src2.push(img);
}
void __stdcall CGxCamManager::OnFrameCallbackFun3(GX_FRAME_CALLBACK_PARAM* pFrame)
{
	if (pFrame->status != 0)
	{
		return;
	}

	CGxCamManager* pf = (CGxCamManager*)(pFrame->pUserParam);
	int		    nID = 2;             //ID��

	BYTE* pImageBuffer = NULL;          //ת�����ͼ��buffer
	BYTE* pRawBuf = NULL;          //ת��ǰͼ��
	//BITMAPINFO* pBmpInfo = NULL;          //ͼ����Ϣ
	int64_t	    nBayerLayout = 0;             //Bayer��ʽ
	int		    nImgWidth = 0;             //ͼ��Ŀ�
	int         nImgHeight = 0;             //ͼ��ĸ�

	int         i = 0;             //ѭ������

	//��ʼ������
	//pf->m_pCamsFps[nID].IncreaseFrameNum();

	nImgWidth = (int)(pf->m_pstCam[nID].nImageWidth);
	nImgHeight = (int)(pf->m_pstCam[nID].nImageHeight);
	pImageBuffer = pf->m_pstCam[nID].pImageBuffer;
	pRawBuf = pf->m_pstCam[nID].pRawBuffer;
	nBayerLayout = pf->m_pstCam[nID].nBayerLayout;
	//pBmpInfo = pf->m_pstCam[nID].pBmpInfo;

	//����ͼ��pRawBuffer��
	memcpy(pf->m_pstCam[nID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[nID].nPayLoadSise));
	BYTE* pOutRawBuf = NULL;          //����ת����ͼ��
	pOutRawBuf = new BYTE[(size_t)(pf->m_pstCam[nID].nPayLoadSise)];
	VxInt32 DxStatus = DxImageMirror((BYTE*)pRawBuf, (BYTE*)pOutRawBuf, nImgWidth, nImgHeight, VERTICAL_MIRROR);

	if (DxStatus != DX_OK)
	{
		return;
	}

	//ͼ��ת��
	if (pf->m_pstCam[nID].bIsColorFilter)
	{
		//��ɫ�����Ҫ����RGBת��
		DxRaw8toRGB24(pOutRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);
	}
	else
	{
		//�ڰ�ͼ����Ҫͼ��ת
		for (i = 0; i < nImgHeight; i++)
		{
			memcpy((pImageBuffer + i * nImgWidth), (pOutRawBuf + (nImgHeight - i - 1) * nImgWidth), nImgWidth);
		}
	}
	if (pOutRawBuf != NULL)
	{
		delete[]pOutRawBuf;
		pOutRawBuf = NULL;
	}
	//cv::Mat img = cv::Mat::zeros(cv::Size(nImgWidth, nImgHeight), CV_8U);
		//std::memcpy(img.data, pImageBuffer, nImgWidth * nImgHeight);
	cv::Mat img = cv::Mat(nImgHeight, nImgWidth, pf->m_pstCam[nID].bIsColorFilter == true ? CV_8UC3 : CV_8UC1, pImageBuffer);
	//flip(img, img, 0);
	g_dlg->srcMat_src3.push(img);
}
void __stdcall CGxCamManager::OnFrameCallbackFun4(GX_FRAME_CALLBACK_PARAM* pFrame)
{
	if (pFrame->status != 0)
	{
		return;
	}

	CGxCamManager* pf = (CGxCamManager*)(pFrame->pUserParam);
	int		    nID = 3;             //ID��

	BYTE* pImageBuffer = NULL;          //ת�����ͼ��buffer
	BYTE* pRawBuf = NULL;          //ת��ǰͼ��
	//BITMAPINFO* pBmpInfo = NULL;          //ͼ����Ϣ
	int64_t	    nBayerLayout = 0;             //Bayer��ʽ
	int		    nImgWidth = 0;             //ͼ��Ŀ�
	int         nImgHeight = 0;             //ͼ��ĸ�

	int         i = 0;             //ѭ������

	//��ʼ������
	//pf->m_pCamsFps[nID].IncreaseFrameNum();

	nImgWidth = (int)(pf->m_pstCam[nID].nImageWidth);
	nImgHeight = (int)(pf->m_pstCam[nID].nImageHeight);
	pImageBuffer = pf->m_pstCam[nID].pImageBuffer;
	pRawBuf = pf->m_pstCam[nID].pRawBuffer;
	nBayerLayout = pf->m_pstCam[nID].nBayerLayout;
	//pBmpInfo = pf->m_pstCam[nID].pBmpInfo;

	//����ͼ��pRawBuffer��
	memcpy(pf->m_pstCam[nID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[nID].nPayLoadSise));
	BYTE* pOutRawBuf = NULL;          //����ת����ͼ��
	pOutRawBuf = new BYTE[(size_t)(pf->m_pstCam[nID].nPayLoadSise)];
	VxInt32 DxStatus = DxImageMirror((BYTE*)pRawBuf, (BYTE*)pOutRawBuf, nImgWidth, nImgHeight, VERTICAL_MIRROR);

	if (DxStatus != DX_OK)
	{
		return;
	}

	//ͼ��ת��
	if (pf->m_pstCam[nID].bIsColorFilter)
	{
		//��ɫ�����Ҫ����RGBת��
		DxRaw8toRGB24(pOutRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);
	}
	else
	{
		//�ڰ�ͼ����Ҫͼ��ת
		for (i = 0; i < nImgHeight; i++)
		{
			memcpy((pImageBuffer + i * nImgWidth), (pOutRawBuf + (nImgHeight - i - 1) * nImgWidth), nImgWidth);
		}
	}
	if (pOutRawBuf != NULL)
	{
		delete[]pOutRawBuf;
		pOutRawBuf = NULL;
	}
	//cv::Mat img = cv::Mat::zeros(cv::Size(nImgWidth, nImgHeight), CV_8U);
	//std::memcpy(img.data, pImageBuffer, nImgWidth * nImgHeight);
	cv::Mat img = cv::Mat(nImgHeight, nImgWidth, pf->m_pstCam[nID].bIsColorFilter == true ? CV_8UC3 : CV_8UC1,pImageBuffer);
	//flip(img, img, 0);
	g_dlg->srcMat_src4.push(img);

}
#endif

void __stdcall CGxCamManager::OnDeviceOfflineCallbackFun1(void* pUserParam)
{
	CGxCamManager* pCtrDevice = (CGxCamManager*)pUserParam;
	int nCamId = 0;
	// �������
	pCtrDevice->m_pstCam[nCamId].bIsOffLine = true;
	mlogger.TraceError("**********************Device1 offline**********************\n");
}
void __stdcall CGxCamManager::OnDeviceOfflineCallbackFun2(void* pUserParam)
{
	CGxCamManager* pCtrDevice = (CGxCamManager*)pUserParam;
	int nCamId = 1;
	// �������
	pCtrDevice->m_pstCam[nCamId].bIsOffLine = true;
	mlogger.TraceError("**********************Device2 offline**********************\n");
}
void __stdcall CGxCamManager::OnDeviceOfflineCallbackFun3(void* pUserParam)
{
	CGxCamManager* pCtrDevice = (CGxCamManager*)pUserParam;
	int nCamId = 2;
	// �������
	pCtrDevice->m_pstCam[nCamId].bIsOffLine = true;
	mlogger.TraceError("**********************Device3 offline**********************\n");
}
void __stdcall CGxCamManager::OnDeviceOfflineCallbackFun4(void* pUserParam)
{
	CGxCamManager* pCtrDevice = (CGxCamManager*)pUserParam;
	int nCamId = 3;
	// �������
	pCtrDevice->m_pstCam[nCamId].bIsOffLine = true;
	mlogger.TraceError("**********************Device4 offline**********************\n");
}

int CGxCamManager::GrabImage(int CameraIndex, Mat& outImage)
{

	GX_STATUS emStatus = GX_STATUS_ERROR;

	//��ʱ��ʼ
	clock_t start_time = clock();
	//ÿ�η��ʹ�������֮ǰ��ղɼ��������
	//��ֹ���ڲ�����֡����ɱ���GXGetImage�õ���ͼ�����ϴη��ʹ����õ���ͼ
	emStatus = GXFlushQueue(m_Cameras[CameraIndex]);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		ShowErrorString(emStatus);
		return emStatus;
	}

	//������������
	emStatus = GXSendCommand(m_Cameras[CameraIndex], GX_COMMAND_TRIGGER_SOFTWARE);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		ShowErrorString(emStatus);
		return emStatus;
	}

	int64_t        nImageWidth;       ///< ԭʼͼ���
	int64_t        nImageHeight;      ///< ԭʼͼ���
	int64_t        nPixelColorFilter; ///< Bayer��ʽ
	int64_t        nPayLoadSize;      ///< ԭʼRawͼ���С
	//��ȡͼ��buffer��С�����涯̬�����ڴ�
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_PAYLOAD_SIZE, &nPayLoadSize);
	GX_FRAME_DATA  stFrameData;       ///< GetImage��ȡ��ͼ���ַ	
	//���ݻ�ȡ��ͼ��buffer��Сm_nPayLoadSize����buffer
	stFrameData.pImgBuf = malloc((size_t)nPayLoadSize);
	bool      bIsImplemented = false;;
	emStatus = GXIsImplemented(m_Cameras[CameraIndex], GX_ENUM_PIXEL_COLOR_FILTER, &bIsImplemented);
	
	// ֧�ֱ�ʾ�����ɫͼ��
	if (bIsImplemented)
	{
		emStatus = GXGetEnum(m_Cameras[CameraIndex], GX_ENUM_PIXEL_COLOR_FILTER, &nPixelColorFilter);
	}

	// ��ȡ���
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_WIDTH, &nImageWidth);
	
	// ��ȡ�߶�
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_HEIGHT, &nImageHeight);

	// ��ȡͼ���С
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_PAYLOAD_SIZE, &nPayLoadSize);

	//��ȡͼ��
	emStatus = GXGetImage(m_Cameras[CameraIndex], &stFrameData, 100);
	// ��ȡ����ͼ���Ƿ�Ϊ������ͼ��
	if (stFrameData.nStatus != 0)
	{//�ͷ�ͼ�񻺳���buffer
		free(stFrameData.pImgBuf);
		return emStatus;
	}
	BYTE* m_pImgBuffer=new byte[nImageWidth*nImageHeight*(bIsImplemented==false?1:3)];
	//��֧�ֲ�ɫ,ת��ΪRGBͼ������
	if (bIsImplemented)
	{
		//��Raw8ͼ��ת��ΪRGBͼ���Թ���ʾ
		DxRaw8toRGB24((BYTE*)stFrameData.pImgBuf, m_pImgBuffer, (VxUint32)nImageWidth, (VxUint32)nImageHeight, RAW2RGB_NEIGHBOUR,
			DX_PIXEL_COLOR_FILTER(nPixelColorFilter), TRUE);
	}
	else
	{
		// �ڰ������Ҫ��ת���ݺ���ʾ
		for (int i = 0; i < stFrameData.nHeight; i++)
		{
			memcpy(m_pImgBuffer + i * nImageWidth, (BYTE*)(stFrameData.pImgBuf) + (nImageHeight - i - 1) * nImageWidth, (size_t)nImageWidth);
		}
	}
	Mat srcImage(cv::Size(nImageWidth, nImageHeight), CV_8UC1);//����mat�ṹ
	srcImage = cv::Mat(nImageHeight, nImageWidth, CV_8UC1, (uchar*)m_pImgBuffer);//��ת������ʱ3ms
	//����һ��ͼ����������ڴ��ַ��������
	srcImage.copyTo(outImage);
	delete[]m_pImgBuffer;
	//������ʱ
	clock_t end_time = clock();
	float elaptime = static_cast<float>(end_time - start_time) / CLOCKS_PER_SEC * 1000;

	CString strtime;
	strtime.Format("The elapsed time of Camera Image to Mat in basler: %.2f ms.\n", elaptime);
	OutputDebugString(strtime);

	//�ͷ�ͼ�񻺳���buffer
	free(stFrameData.pImgBuf);

	return emStatus;
	
}

void CGxCamManager::ImageUpToDown(UCHAR* src, int width, int height, int iColor)
{
	int i, j;
	int W = width * iColor, H = height > 0 ? height : -height;
	UCHAR tmp;
	UCHAR* p = src;
	for (i = 0; i < H / 2; i++)
	{
		for (j = 0; j < W; j++)
		{
			tmp = p[i * W + j];
			p[i * W + j] = p[(H - i - 1) * W + j];
			p[(H - i - 1) * W + j] = tmp;
		}
	}

}

void CGxCamManager::ImageLeftToRight(BYTE* Buffer, int width, int height, int iColor)
{
	int i, j;
	int W = width * iColor, H = height > 0 ? height : -height;
	UCHAR tmp;
	UCHAR* p = Buffer;
	for (i = 0; i < H; i++)
	{
		for (j = 0; j < W / 2; j++)
		{
			tmp = p[i * W + j];
			p[i * W + j] = p[i * W + (W - j - 1)];
			p[i * W + (W - j - 1)] = tmp;
		}
		for (j = 0; j < W; j += 3)
		{
			tmp = p[i * W + j];
			p[i * W + j] = p[i * W + j + 2];
			p[i * W + j + 2] = tmp;
		}
	}
}