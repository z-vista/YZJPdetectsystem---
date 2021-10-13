#include "stdafx.h"
#include "CGxCamManager.h"
#include "YZJPdetectsystemDlg.h"

extern YZJPdetectsystemDlg* g_dlg;
extern NetDataLog mlogger;
CGxCamManager::CGxCamManager()
{

	InitializeGx();
	mlogger.TraceKeyInfo("相机管理类构造\n");
}
CGxCamManager::~CGxCamManager()
{
	mlogger.TraceKeyInfo("相机管理类析构\n");
}
// 拷贝构造函数
CGxCamManager::CGxCamManager(const CGxCamManager&)
{
	
}
// 赋值运算符
CGxCamManager& CGxCamManager::operator = (const CGxCamManager&)
{
	return *this;
}
// 获取相机实例
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
	//释放内存
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
//相机设备初始化、设备排序
bool CGxCamManager::InitCamera()
{
	mlogger.TraceKeyInfo("xxxx--InitCamera() begin \n");
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	m_Cameras = NULL;									//初始化相机句柄
	m_pstCam = NULL;									//初始化相机参数结构体
	m_pBaseinfo = NULL;									//初始化设备信息结构体
	m_nCamCount = 0;

	uint32_t          nDeviceNum = 0;     // 设备个数 
	emStatus = GXUpdateDeviceList(&nDeviceNum, 1000);

	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXUpdateDeviceList() err ！\n");
		ShowErrorString(emStatus);
	}

	if (nDeviceNum <= 0)//未检索到相机
		return false;

	if (nDeviceNum != MaxCameraNumber)//相机数量不正确
		return false;
	mlogger.TraceKeyInfo("xxxx--初始化相机个数：%d ！\n",nDeviceNum);
	m_pBaseinfo = new GX_DEVICE_BASE_INFO[nDeviceNum];
	
	//获取所有枚举相机的信息
	size_t nSize = nDeviceNum * sizeof(GX_DEVICE_BASE_INFO);  //计算结构体大小
	emStatus = GXGetAllDeviceBaseInfo(m_pBaseinfo, &nSize);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetAllDeviceBaseInfo() err ！\n");
		ShowErrorString(emStatus);
		delete[]m_pBaseinfo;
		m_pBaseinfo = NULL;

		// 将设备个数置为0
		nDeviceNum = 0;

		return false;
	}
	mlogger.TraceInfo("xxxx--匹配相机对应SN和编号 ！\n");

	//初始化资源
	m_Cameras = new GX_DEV_HANDLE[nDeviceNum];
	m_pstCam = new CAMER_INFO[nDeviceNum];
	for (uint32_t i = 0; i < nDeviceNum; i++)
	{
		
		//mlogger.TraceKeyInfo("xxxx--相机%dSN: %s ！\n",i, m_pBaseinfo[i].szSN);
		//if (0 == m_strCamSn[0].compare(m_pBaseinfo[i].szSN)) {
		//	m_CameraIndex1 = i;
		//	mlogger.TraceKeyInfo("xxxx--m_CameraIndex1=%d ！\n", i);
		//	
		//}
		//else if (0 == m_strCamSn[1].compare(m_pBaseinfo[i].szSN)) {
		//	mlogger.TraceKeyInfo("xxxx--m_CameraIndex2=%d ！\n", i);
		//	m_CameraIndex2 = i;
		//}
		//else if (0 == m_strCamSn[2].compare(m_pBaseinfo[i].szSN)) {
		//	mlogger.TraceKeyInfo("xxxx--m_CameraIndex3=%d ！\n", i);
		//	m_CameraIndex3 = i;
		//}
		//else if (0 == m_strCamSn[3].compare(m_pBaseinfo[i].szSN)) {
		//	mlogger.TraceKeyInfo("xxxx--m_CameraIndex4=%d ！\n", i);
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
	GX_OPEN_PARAM     stOpenParam;          // 打开相机参数
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
	// 通过已知的SN打开设备
	stOpenParam.pszContent = (char*)m_strCamSn[CameraIndex].c_str();

	mlogger.TraceKeyInfo("xxxx--Open Device %d by SN %s \n", CameraIndex, stOpenParam.pszContent);
	emStatus = GXOpenDevice(&stOpenParam, &m_Cameras[CameraIndex]);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXOpenDevice() err ！\n");
		ShowErrorString(emStatus);
		return false;
	}

	//导入配置文件前要将设备图像大小设置为与配置文件中图像大小一致
	int64_t nMaxWidth = 0, nMaxHeight = 0;
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_WIDTH_MAX, &nMaxWidth);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_INT_WIDTH_MAX() err ！\n");
		ShowErrorString(emStatus);
		return false;
	}
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_HEIGHT_MAX, &nMaxHeight);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_INT_HEIGHT_MAX() err ！\n");
		ShowErrorString(emStatus);
		return false;
	}
	char* cp = new char[64];
	std::string tp_NodeStr;
	int nMaxPic = GetPrivateProfileInt(_T("Set_Camera"), _T("useMaxPic"), 1, g_dlg->lpPath);;
	if (nMaxPic)
	{
		//设置
		emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_OFFSET_X, 0);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_INT_OFFSET_X() err ！\n");
			ShowErrorString(emStatus);
			return false;
		}
		emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_OFFSET_Y, 0);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_INT_OFFSET_Y() err ！\n");
			ShowErrorString(emStatus);
			return false;
		}

	
		emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_WIDTH, nMaxWidth);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_INT_WIDTH() err ！\n");
			ShowErrorString(emStatus);
			return false;
		}
		emStatus = GXSetInt(m_Cameras[CameraIndex], GX_INT_HEIGHT, nMaxHeight);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_INT_HEIGHT() err ！\n");
			ShowErrorString(emStatus);
			return false;
		}

		// 导入配置文件
		if (!ImportConfigFile(CameraIndex))
		{
			mlogger.TraceError("xxxx--ImportConfigFile() err ！\n");
			return false;
		}
	}
	else
	{
		
		// 导入配置文件
		//if (!ImportConfigFile(CameraIndex))
		//{
		//	mlogger.TraceError("xxxx--ImportConfigFile() err ！\n");
		//	return false;
		//}
		int nValue = 0;
		sprintf(cp, "ReverseX%d", CameraIndex + 1);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
		emStatus = GXSetBool(m_Cameras[CameraIndex], GX_BOOL_REVERSE_X, nValue==1?true:false);//设置图像镜像
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_BOOL_REVERSE_X() err ！\n");
			ShowErrorString(emStatus);
			return false;
		}
		sprintf(cp, "ReverseY%d", CameraIndex + 1);
		tp_NodeStr = cp;
		nValue = GetPrivateProfileInt("Set ImageFormat", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
		emStatus = GXSetBool(m_Cameras[CameraIndex], GX_BOOL_REVERSE_Y, nValue == 1 ? true : false);//设置图像镜像
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_BOOL_REVERSE_Y() err ！\n");
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
				mlogger.TraceError("xxxx--GX_INT_WIDTH() err ！\n");
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
				mlogger.TraceError("xxxx--GX_INT_HEIGHT() err ！\n");
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
				mlogger.TraceError("xxxx--GX_INT_OFFSET_X() err ！\n");
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
				mlogger.TraceError("xxxx--GX_INT_OFFSET_Y() err ！\n");
				ShowErrorString(emStatus);
				return false;
			}
		}
		
	}

	//设置曝光与增益
	int nShutter = 0, nGain = 0;
	double  dOldShutter =0,dOldGain=0;
	
	GetExposetime(CameraIndex,dOldShutter);
	sprintf(cp, "Exposetime%d", CameraIndex+1);
	tp_NodeStr = cp;
	nShutter = GetPrivateProfileInt("Set_Camera", tp_NodeStr.c_str(), 1000, g_dlg->lpPath);//
	if (nShutter != dOldShutter) {
		SetExposetime(CameraIndex, nShutter);
		mlogger.TraceKeyInfo("xxxx--设置曝光:%d   ！\n", nShutter);
	}
		
	GetGain(CameraIndex,dOldGain);
	sprintf(cp, "Gain%d", CameraIndex + 1);
	tp_NodeStr = cp;
	nGain = GetPrivateProfileInt("Set_Camera", tp_NodeStr.c_str(), 0, g_dlg->lpPath);//
	if(nGain !=dOldGain){
		SetGain(CameraIndex, nGain);
	mlogger.TraceKeyInfo("xxxx--设置增益:%d   ！\n", nGain);
	}

	// 已知当前相机支持哪个8位图像数据格式可以直接设置
	// 例如设备支持数据格式GX_PIXEL_FORMAT_BAYER_GR8,则可按照以下代码实现
	// emStatus = GXSetEnum(m_hDevice, GX_ENUM_PIXEL_FORMAT, GX_PIXEL_FORMAT_BAYER_GR8);
	// VERIFY_STATUS_RET(emStatus);

	// 不清楚当前相机的数据格式时，可以调用以下函数将图像数据格式设置为8Bit
	emStatus = SetPixelFormat8bit(m_Cameras[CameraIndex]);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--SetPixelFormat8bit err ！\n");
		ShowErrorString(emStatus);
		return false;
	}


	//设置采集模式连续采集
	emStatus = GXSetEnum(m_Cameras[CameraIndex], GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXSetEnum  GX_ENUM_ACQUISITION_MODEerr ！\n");
		ShowErrorString(emStatus);
		return false;
	}

	//设置触发模式为关
	emStatus = GXSetEnum(m_Cameras[CameraIndex], GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_OFF);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXSetEnum GX_ENUM_TRIGGER_MODE err ！\n");
		ShowErrorString(emStatus);
		return false;
	}
	m_pstCam[CameraIndex].bIsOpen = TRUE;
	mlogger.TraceKeyInfo("xxxx--bIsOpen    sucessed！\n");
	return true;
}
bool CGxCamManager::CloseCamera(int CameraIndex)
{
	
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	/*CameraIndex = GetCameraIndex(CameraIndex);
	mlogger.TraceKeyInfo("xxxx--CloseCamera() %d begin \n", CameraIndex);*/
	if (m_pstCam[CameraIndex].bIsSnap)
	{
		//停止采集
		emStatus = GXSendCommand(m_Cameras[CameraIndex], GX_COMMAND_ACQUISITION_STOP);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GX_COMMAND_ACQUISITION_STOP() err ！\n");
			// 错误处理
			ShowErrorString(emStatus);
			return false;
		}

		//注销回调
		emStatus = GXUnregisterCaptureCallback(m_Cameras[CameraIndex]);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			// 错误处理
			mlogger.TraceError("xxxx--GXUnregisterCaptureCallback() err ！\n");
			ShowErrorString(emStatus);
			return false;
		}
		//释放资源
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
		mlogger.TraceKeyInfo("xxxx--GXUnregisterDeviceOfflineCallback() err ！\n");
		ShowErrorString(emStatus); 
		return false;
	}*/
	if (m_pstCam[CameraIndex].bIsOpen)
	{
		//关闭相机
		emStatus = GXCloseDevice(m_Cameras[CameraIndex]);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GXCloseDevice() err ！\n");
			// 错误处理
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
	//获取相机Bayer转换类型及是否支持输出彩色图像
	emStatus = GXIsImplemented(m_Cameras[CameraIndex], GX_ENUM_PIXEL_COLOR_FILTER, &bIsImplemented);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXIsImplemented  err ！\n");
		ShowErrorString(emStatus);
		return false;
	}
	mlogger.TraceKeyInfo("xxxx--bIsImplemented:%d   ！\n", bIsImplemented);
	m_pstCam[CameraIndex].bIsColorFilter = bIsImplemented;
	if (bIsImplemented)
	{
		emStatus = GXGetEnum(m_Cameras[CameraIndex], GX_ENUM_PIXEL_COLOR_FILTER, &nValue);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			mlogger.TraceError("xxxx--GXGetEnum GX_ENUM_PIXEL_COLOR_FILTER  err ！\n");
			ShowErrorString(emStatus);
			return false;
		}
		m_pstCam[CameraIndex].nBayerLayout = nValue;
	}

	// 获取图像宽
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_WIDTH, &nValue);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetInt GX_INT_WIDTH err ！\n");
		ShowErrorString(emStatus);
		return false;
	}
	m_pstCam[CameraIndex].nImageWidth = nValue;
	mlogger.TraceKeyInfo("xxxx--nImageWidth:%d   ！\n", nValue);
	// 获取图像高
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_HEIGHT, &nValue);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetInt GX_INT_HEIGHT err ！\n");
		ShowErrorString(emStatus);
		return false;
	}
	m_pstCam[CameraIndex].nImageHeight = nValue;
	mlogger.TraceKeyInfo("xxxx--nImageHeight:%d   ！\n", nValue);
	// 获取原始图像大小
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_PAYLOAD_SIZE, &nValue);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXGetInt GX_INT_PAYLOAD_SIZE err ！\n");
		ShowErrorString(emStatus);
		return false;
	}
	m_pstCam[CameraIndex].nPayLoadSise = nValue;
	mlogger.TraceKeyInfo("xxxx--nPayLoadSise:%d   ！\n", nValue);

	/*CameraIndex = GetCameraIndex(CameraIndex);
	mlogger.TraceKeyInfo("xxxx--StartGrabCamera() %d begin \n", CameraIndex);*/
	bool      bIsColorFilter = m_pstCam[CameraIndex].bIsColorFilter;

	//// 初始化BMP头信息
	//m_pstCam[CameraIndex].pBmpInfo = (BITMAPINFO*)(m_pstCam[CameraIndex].chBmpBuf);
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biWidth = (LONG)m_pstCam[CameraIndex].nImageWidth;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biHeight = (LONG)m_pstCam[CameraIndex].nImageHeight;

	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biPlanes = 1;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biBitCount = bIsColorFilter ? 24 : 8; // 彩色图像为24,黑白图像为8;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biCompression = BI_RGB;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biSizeImage = 0;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biXPelsPerMeter = 0;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biYPelsPerMeter = 0;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biClrUsed = 0;
	//m_pstCam[CameraIndex].pBmpInfo->bmiHeader.biClrImportant = 0;

	//// 黑白相机需要初始化调色板信息
	//if (!bIsColorFilter)
	//{
	//	// 黑白相机需要进行初始化调色板操作
	//	for (int i = 0; i < 256; i++)
	//	{
	//		m_pstCam[CameraIndex].pBmpInfo->bmiColors[i].rgbBlue = i;
	//		m_pstCam[CameraIndex].pBmpInfo->bmiColors[i].rgbGreen = i;
	//		m_pstCam[CameraIndex].pBmpInfo->bmiColors[i].rgbRed = i;
	//		m_pstCam[CameraIndex].pBmpInfo->bmiColors[i].rgbReserved = i;
	//	}
	//}

	// 清空Buffer
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

	// 申请资源
	m_pstCam[CameraIndex].pRawBuffer = new BYTE[(size_t)(m_pstCam[CameraIndex].nPayLoadSise)];
	if (m_pstCam[CameraIndex].pRawBuffer == NULL)
	{

		mlogger.TraceKeyInfo("xxxx--申请资源失败 \n");
		//MessageBox("申请资源失败!");
		return false;
	}

	// 如果是彩色相机，为彩色相机申请资源
	if (bIsColorFilter)
	{
		m_pstCam[CameraIndex].pImageBuffer = new BYTE[(size_t)(m_pstCam[CameraIndex].nImageWidth * m_pstCam[CameraIndex].nImageHeight * 3)];
		if (m_pstCam[CameraIndex].pImageBuffer == NULL)
		{
			mlogger.TraceKeyInfo("xxxx--申请资源失败 \n");
			delete[]m_pstCam[CameraIndex].pRawBuffer;
			m_pstCam[CameraIndex].pRawBuffer = NULL;
			return false;
		}
	}

	// 如果是黑白相机， 为黑白相机申请资源
	else
	{
		m_pstCam[CameraIndex].pImageBuffer = new BYTE[(size_t)(m_pstCam[CameraIndex].nImageWidth * m_pstCam[CameraIndex].nImageHeight)];
		if (m_pstCam[CameraIndex].pImageBuffer == NULL)
		{
			mlogger.TraceKeyInfo("xxxx--申请资源失败 \n");
			delete[]m_pstCam[CameraIndex].pRawBuffer;
			m_pstCam[CameraIndex].pRawBuffer = NULL;
			return false;
		}
	}

	//注册回调
	RegisterCallback(CameraIndex);
	//开始采集
	emStatus = GXSendCommand(m_Cameras[CameraIndex], GX_COMMAND_ACQUISITION_START);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		// 错误处理
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
	//停止采集
	if (m_Cameras== NULL)
		return false;
	emStatus = GXSendCommand(m_Cameras[CameraIndex], GX_COMMAND_ACQUISITION_STOP);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		// 错误处理
		ShowErrorString(emStatus);
		return false;
	}
	//注销回调
	emStatus = GXUnregisterCaptureCallback(m_Cameras[CameraIndex]);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		// 错误处理
		ShowErrorString(emStatus);
		return false;
	}
	//释放资源
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

	// 获取错误描述信息长度
	emStatus = GXGetLastError(&emErrorStatus, NULL, &nSize);

	pchErrorInfo = new char[nSize];
	if (pchErrorInfo == NULL)
	{
		OutputDebugString("xxxx--GXGetLastError接口调用 size <1！\n");
		mlogger.TraceError("xxxx--GXGetLastError接口调用 size <1！\n");
		return;
	}

	// 获取错误信息描述
	emStatus = GXGetLastError(&emErrorStatus, pchErrorInfo, &nSize);

	// 显示错误描述信息
	if (emStatus != GX_STATUS_SUCCESS)
	{
		OutputDebugString("xxxx--GXGetLastError接口调用失败！\n");
		mlogger.TraceError("xxxx--GXGetLastError接口调用失败！\n");
	}
	else
	{
		CString strErrorInfo = NULL;
		strErrorInfo.Format("xxxx--%s\n", (LPCTSTR)pchErrorInfo);
		OutputDebugString(strErrorInfo);
		mlogger.TraceError(strErrorInfo);

	}

	// 释放资源
	if (pchErrorInfo != NULL)
	{
		delete[]pchErrorInfo;
		pchErrorInfo = NULL;
	}
}	/// 获取错误信息描述
GX_STATUS CGxCamManager::SetPixelFormat8bit(GX_DEV_HANDLE hDevice)
{
	GX_STATUS emStatus = GX_STATUS_SUCCESS;
	int64_t   nPixelSize = 0;
	uint32_t  nEnmuEntry = 0;
	size_t    nBufferSize = 0;
	BOOL      bIs8bit = TRUE;

	GX_ENUM_DESCRIPTION* pEnumDescription = NULL;
	GX_ENUM_DESCRIPTION* pEnumTemp = NULL;

	// 获取像素点大小
	emStatus = GXGetEnum(hDevice, GX_ENUM_PIXEL_SIZE, &nPixelSize);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		return emStatus;
	}

	// 判断为8bit时直接返回,否则设置为8bit
	if (nPixelSize == GX_PIXEL_SIZE_BPP8)
	{
		return GX_STATUS_SUCCESS;
	}
	else
	{
		// 获取设备支持的像素格式枚举数
		emStatus = GXGetEnumEntryNums(hDevice, GX_ENUM_PIXEL_FORMAT, &nEnmuEntry);
		if (emStatus != GX_STATUS_SUCCESS)
		{
			return emStatus;
		}

		// 为获取设备支持的像素格式枚举值准备资源
		nBufferSize = nEnmuEntry * sizeof(GX_ENUM_DESCRIPTION);
		pEnumDescription = new GX_ENUM_DESCRIPTION[nEnmuEntry];

		// 获取支持的枚举值
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

		// 遍历设备支持的像素格式,设置像素格式为8bit,
		// 如设备支持的像素格式为Mono10和Mono8则设置其为Mono8
		for (uint32_t i = 0; i < nEnmuEntry; i++)
		{
			if ((pEnumDescription[i].nValue & GX_PIXEL_8BIT) == GX_PIXEL_8BIT)
			{
				emStatus = GXSetEnum(hDevice, GX_ENUM_PIXEL_FORMAT, pEnumDescription[i].nValue);
				break;
			}
		}

		// 释放资源
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
		mlogger.TraceError("xxxx--GX_INT_WIDTH_MAX() err ！\n");
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
		mlogger.TraceError("xxxx--GX_INT_HEIGHT_MAX() err ！\n");
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
		mlogger.TraceError("xxxx--GX_INT_OFFSET_X() err ！\n");
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
		mlogger.TraceError("xxxx--GX_INT_OFFSET_Y() err ！\n");
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
		mlogger.TraceError("xxxx--GX_INT_WIDTH() err ！\n");
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
		mlogger.TraceError("xxxx--GX_INT_WIDTH() err ！\n");
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
	emStatus = GXSetBool(m_Cameras[CameraIndex], GX_BOOL_REVERSE_X, reverse);//设置图像镜像,
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_BOOL_REVERSE_X() err ！\n");
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
	emStatus = GXSetBool(m_Cameras[CameraIndex], GX_BOOL_REVERSE_Y, reverse);//设置图像镜像,
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GX_BOOL_REVERSE_Y() err ！\n");
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
		mlogger.TraceError("xxxx--GXGetFloatRange GX_FLOAT_EXPOSURE_TIME  err ！\n");
		ShowErrorString(emStatus);
		return false;
	}
	//若大于最大值则将曝光值设为最大值
	if (dShutter > stFloatRange.dMax)
	{
		dShutter = stFloatRange.dMax;
	}
	//若小于最小值将曝光值设为最小值
	if (dShutter < stFloatRange.dMin)
	{
		dShutter = stFloatRange.dMin;
	}

	emStatus = GXSetFloat(m_Cameras[CameraIndex], GX_FLOAT_EXPOSURE_TIME, dShutter);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXSetFloat GX_FLOAT_EXPOSURE_TIME err dShutter=%d ！\n", dShutter);
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
		mlogger.TraceError("xxxx--GXGetFloat GX_FLOAT_EXPOSURE_TIME  err ！\n");
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
		mlogger.TraceError("xxxx--GXGetFloatRange GX_FLOAT_EXPOSURE_TIME  err ！\n");
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
		mlogger.TraceError("xxxx--GXGetFloatRange GX_FLOAT_GAIN  err ！\n");
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
		mlogger.TraceError("xxxx--GXGetFloatRange GX_FLOAT_GAIN  err ！\n");
		ShowErrorString(emStatus);
		return false;
	}
	//若大于最大值则将曝光值设为最大值
	if (dGain > stFloatRange.dMax)
	{
		dGain = stFloatRange.dMax;
	}
	//若小于最小值将曝光值设为最小值
	if (dGain < stFloatRange.dMin)
	{
		dGain = stFloatRange.dMin;
	}

	emStatus = GXSetFloat(m_Cameras[CameraIndex], GX_FLOAT_GAIN, dGain);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		mlogger.TraceError("xxxx--GXSetFloat GX_FLOAT_GAIN err,dGain=%d ！\n", dGain);
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
		mlogger.TraceError("xxxx--GXGetFloat GX_FLOAT_GAIN  err ！\n");
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
		mlogger.TraceError("xxxx--GXImportConfigFile() %s   err ！\n", strConfigFile);
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
		mlogger.TraceError("xxxx--GXExportConfigFile() %s   err ！\n", strConfigFile);
		ShowErrorString(emStatus);
		return false;
	}
	return true;
}
void CGxCamManager::RegisterCallback(int nCamID)
{
	GX_STATUS emStatus = GX_STATUS_ERROR;
	mlogger.TraceKeyInfo("xxxx--RegisterCallback() %d begin \n", nCamID);
	//注册回调
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

	//注册回调
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
	int		    nID = 0;             //ID号

	BYTE* pImageBuffer = NULL;          //转换后的图像buffer
	BYTE* pRawBuf = NULL;          //转换前图像
//	BITMAPINFO* pBmpInfo = NULL;          //图像信息
	int64_t	    nBayerLayout = 0;             //Bayer格式
	int		    nImgWidth = 0;             //图像的宽
	int         nImgHeight = 0;             //图像的高

	int         i = 0;             //循环变量
	

	//初始化参数
	//pf->m_pCamsFps[nID].IncreaseFrameNum();

	nImgWidth = (int)(pf->m_pstCam[nID].nImageWidth);
	nImgHeight = (int)(pf->m_pstCam[nID].nImageHeight);
	pImageBuffer = pf->m_pstCam[nID].pImageBuffer;
	pRawBuf = pf->m_pstCam[nID].pRawBuffer;
	nBayerLayout = pf->m_pstCam[nID].nBayerLayout;
	//pBmpInfo = pf->m_pstCam[nID].pBmpInfo;
	//mlogger.TraceKeyInfo("xxxx--相机1回调--nImgWidth=%d，nImgHeight=%d，nBayerLayout=%d，bIsColorFilter=%d,pf->m_pstCam[nID].nPayLoadSise=%d ！\n", nImgWidth, nImgHeight, nBayerLayout, pf->m_pstCam[nID].bIsColorFilter, pf->m_pstCam[nID].nPayLoadSise);
	//拷贝图像到pRawBuffer中
	memcpy(pf->m_pstCam[nID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[nID].nPayLoadSise));
	// 图像垂直镜像翻转（输入和输出不能为同一个buffer）

	BYTE* pOutRawBuf = NULL;          //镜像转换后图像
	pOutRawBuf = new BYTE[(size_t)(pf->m_pstCam[nID].nPayLoadSise)];
	VxInt32 DxStatus = DxImageMirror((BYTE*)pRawBuf, (BYTE*)pOutRawBuf, nImgWidth, nImgHeight, VERTICAL_MIRROR);

	if (DxStatus != DX_OK)
	{
		return;
	}


	//图像转化
	if (pf->m_pstCam[nID].bIsColorFilter)
	{
		//彩色相机需要经过RGB转换
		DxRaw8toRGB24(pOutRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);
		//pf->ImageLeftToRight(pImageBuffer, nImgWidth, nImgHeight, 3);
	}
	else
	{
		//黑白图象需要图象翻转
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
	int		    nID = 1;             //ID号

	BYTE* pImageBuffer = NULL;          //转换后的图像buffer
	BYTE* pRawBuf = NULL;          //转换前图像
	//BITMAPINFO* pBmpInfo = NULL;          //图像信息
	int64_t	    nBayerLayout = 0;             //Bayer格式
	int		    nImgWidth = 0;             //图像的宽
	int         nImgHeight = 0;             //图像的高

	int         i = 0;             //循环变量

	//初始化参数
	//pf->m_pCamsFps[nID].IncreaseFrameNum();

	nImgWidth = (int)(pf->m_pstCam[nID].nImageWidth);
	nImgHeight = (int)(pf->m_pstCam[nID].nImageHeight);
	pImageBuffer = pf->m_pstCam[nID].pImageBuffer;
	pRawBuf = pf->m_pstCam[nID].pRawBuffer;
	nBayerLayout = pf->m_pstCam[nID].nBayerLayout;
	//pBmpInfo = pf->m_pstCam[nID].pBmpInfo;

	//拷贝图像到pRawBuffer中
	memcpy(pf->m_pstCam[nID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[nID].nPayLoadSise));
	BYTE* pOutRawBuf = NULL;          //镜像转换后图像
	pOutRawBuf = new BYTE[(size_t)(pf->m_pstCam[nID].nPayLoadSise)];
	VxInt32 DxStatus = DxImageMirror((BYTE*)pRawBuf, (BYTE*)pOutRawBuf, nImgWidth, nImgHeight, VERTICAL_MIRROR);

	if (DxStatus != DX_OK)
	{
		return;
	}

	//图像转化
	if (pf->m_pstCam[nID].bIsColorFilter)
	{
		//彩色相机需要经过RGB转换
		DxRaw8toRGB24(pOutRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);
	}
	else
	{
		//黑白图象需要图象翻转
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
	int		    nID = 2;             //ID号

	BYTE* pImageBuffer = NULL;          //转换后的图像buffer
	BYTE* pRawBuf = NULL;          //转换前图像
	//BITMAPINFO* pBmpInfo = NULL;          //图像信息
	int64_t	    nBayerLayout = 0;             //Bayer格式
	int		    nImgWidth = 0;             //图像的宽
	int         nImgHeight = 0;             //图像的高

	int         i = 0;             //循环变量

	//初始化参数
	//pf->m_pCamsFps[nID].IncreaseFrameNum();

	nImgWidth = (int)(pf->m_pstCam[nID].nImageWidth);
	nImgHeight = (int)(pf->m_pstCam[nID].nImageHeight);
	pImageBuffer = pf->m_pstCam[nID].pImageBuffer;
	pRawBuf = pf->m_pstCam[nID].pRawBuffer;
	nBayerLayout = pf->m_pstCam[nID].nBayerLayout;
	//pBmpInfo = pf->m_pstCam[nID].pBmpInfo;

	//拷贝图像到pRawBuffer中
	memcpy(pf->m_pstCam[nID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[nID].nPayLoadSise));
	BYTE* pOutRawBuf = NULL;          //镜像转换后图像
	pOutRawBuf = new BYTE[(size_t)(pf->m_pstCam[nID].nPayLoadSise)];
	VxInt32 DxStatus = DxImageMirror((BYTE*)pRawBuf, (BYTE*)pOutRawBuf, nImgWidth, nImgHeight, VERTICAL_MIRROR);

	if (DxStatus != DX_OK)
	{
		return;
	}

	//图像转化
	if (pf->m_pstCam[nID].bIsColorFilter)
	{
		//彩色相机需要经过RGB转换
		DxRaw8toRGB24(pOutRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);
	}
	else
	{
		//黑白图象需要图象翻转
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
	int		    nID = 3;             //ID号

	BYTE* pImageBuffer = NULL;          //转换后的图像buffer
	BYTE* pRawBuf = NULL;          //转换前图像
	//BITMAPINFO* pBmpInfo = NULL;          //图像信息
	int64_t	    nBayerLayout = 0;             //Bayer格式
	int		    nImgWidth = 0;             //图像的宽
	int         nImgHeight = 0;             //图像的高

	int         i = 0;             //循环变量

	//初始化参数
	//pf->m_pCamsFps[nID].IncreaseFrameNum();

	nImgWidth = (int)(pf->m_pstCam[nID].nImageWidth);
	nImgHeight = (int)(pf->m_pstCam[nID].nImageHeight);
	pImageBuffer = pf->m_pstCam[nID].pImageBuffer;
	pRawBuf = pf->m_pstCam[nID].pRawBuffer;
	nBayerLayout = pf->m_pstCam[nID].nBayerLayout;
	//pBmpInfo = pf->m_pstCam[nID].pBmpInfo;

	//拷贝图像到pRawBuffer中
	memcpy(pf->m_pstCam[nID].pRawBuffer, pFrame->pImgBuf, (size_t)(pf->m_pstCam[nID].nPayLoadSise));
	BYTE* pOutRawBuf = NULL;          //镜像转换后图像
	pOutRawBuf = new BYTE[(size_t)(pf->m_pstCam[nID].nPayLoadSise)];
	VxInt32 DxStatus = DxImageMirror((BYTE*)pRawBuf, (BYTE*)pOutRawBuf, nImgWidth, nImgHeight, VERTICAL_MIRROR);

	if (DxStatus != DX_OK)
	{
		return;
	}

	//图像转化
	if (pf->m_pstCam[nID].bIsColorFilter)
	{
		//彩色相机需要经过RGB转换
		DxRaw8toRGB24(pOutRawBuf, pImageBuffer, nImgWidth, nImgHeight, RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(nBayerLayout), TRUE);
	}
	else
	{
		//黑白图象需要图象翻转
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
	// 相机掉线
	pCtrDevice->m_pstCam[nCamId].bIsOffLine = true;
	mlogger.TraceError("**********************Device1 offline**********************\n");
}
void __stdcall CGxCamManager::OnDeviceOfflineCallbackFun2(void* pUserParam)
{
	CGxCamManager* pCtrDevice = (CGxCamManager*)pUserParam;
	int nCamId = 1;
	// 相机掉线
	pCtrDevice->m_pstCam[nCamId].bIsOffLine = true;
	mlogger.TraceError("**********************Device2 offline**********************\n");
}
void __stdcall CGxCamManager::OnDeviceOfflineCallbackFun3(void* pUserParam)
{
	CGxCamManager* pCtrDevice = (CGxCamManager*)pUserParam;
	int nCamId = 2;
	// 相机掉线
	pCtrDevice->m_pstCam[nCamId].bIsOffLine = true;
	mlogger.TraceError("**********************Device3 offline**********************\n");
}
void __stdcall CGxCamManager::OnDeviceOfflineCallbackFun4(void* pUserParam)
{
	CGxCamManager* pCtrDevice = (CGxCamManager*)pUserParam;
	int nCamId = 3;
	// 相机掉线
	pCtrDevice->m_pstCam[nCamId].bIsOffLine = true;
	mlogger.TraceError("**********************Device4 offline**********************\n");
}

int CGxCamManager::GrabImage(int CameraIndex, Mat& outImage)
{

	GX_STATUS emStatus = GX_STATUS_ERROR;

	//计时开始
	clock_t start_time = clock();
	//每次发送触发命令之前清空采集输出队列
	//防止库内部缓存帧，造成本次GXGetImage得到的图像是上次发送触发得到的图
	emStatus = GXFlushQueue(m_Cameras[CameraIndex]);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		ShowErrorString(emStatus);
		return emStatus;
	}

	//发送软触发命令
	emStatus = GXSendCommand(m_Cameras[CameraIndex], GX_COMMAND_TRIGGER_SOFTWARE);
	if (emStatus != GX_STATUS_SUCCESS)
	{
		ShowErrorString(emStatus);
		return emStatus;
	}

	int64_t        nImageWidth;       ///< 原始图像宽
	int64_t        nImageHeight;      ///< 原始图像高
	int64_t        nPixelColorFilter; ///< Bayer格式
	int64_t        nPayLoadSize;      ///< 原始Raw图像大小
	//获取图像buffer大小，下面动态申请内存
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_PAYLOAD_SIZE, &nPayLoadSize);
	GX_FRAME_DATA  stFrameData;       ///< GetImage获取的图像地址	
	//根据获取的图像buffer大小m_nPayLoadSize申请buffer
	stFrameData.pImgBuf = malloc((size_t)nPayLoadSize);
	bool      bIsImplemented = false;;
	emStatus = GXIsImplemented(m_Cameras[CameraIndex], GX_ENUM_PIXEL_COLOR_FILTER, &bIsImplemented);
	
	// 支持表示输出彩色图像
	if (bIsImplemented)
	{
		emStatus = GXGetEnum(m_Cameras[CameraIndex], GX_ENUM_PIXEL_COLOR_FILTER, &nPixelColorFilter);
	}

	// 获取宽度
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_WIDTH, &nImageWidth);
	
	// 获取高度
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_HEIGHT, &nImageHeight);

	// 获取图像大小
	emStatus = GXGetInt(m_Cameras[CameraIndex], GX_INT_PAYLOAD_SIZE, &nPayLoadSize);

	//获取图像
	emStatus = GXGetImage(m_Cameras[CameraIndex], &stFrameData, 100);
	// 获取到的图像是否为完整的图像
	if (stFrameData.nStatus != 0)
	{//释放图像缓冲区buffer
		free(stFrameData.pImgBuf);
		return emStatus;
	}
	BYTE* m_pImgBuffer=new byte[nImageWidth*nImageHeight*(bIsImplemented==false?1:3)];
	//若支持彩色,转换为RGB图像后输出
	if (bIsImplemented)
	{
		//将Raw8图像转换为RGB图像以供显示
		DxRaw8toRGB24((BYTE*)stFrameData.pImgBuf, m_pImgBuffer, (VxUint32)nImageWidth, (VxUint32)nImageHeight, RAW2RGB_NEIGHBOUR,
			DX_PIXEL_COLOR_FILTER(nPixelColorFilter), TRUE);
	}
	else
	{
		// 黑白相机需要翻转数据后显示
		for (int i = 0; i < stFrameData.nHeight; i++)
		{
			memcpy(m_pImgBuffer + i * nImageWidth, (BYTE*)(stFrameData.pImgBuf) + (nImageHeight - i - 1) * nImageWidth, (size_t)nImageWidth);
		}
	}
	Mat srcImage(cv::Size(nImageWidth, nImageHeight), CV_8UC1);//创建mat结构
	srcImage = cv::Mat(nImageHeight, nImageWidth, CV_8UC1, (uchar*)m_pImgBuffer);//该转换仅费时3ms
	//复制一份图像，以免出现内存地址错误问题
	srcImage.copyTo(outImage);
	delete[]m_pImgBuffer;
	//结束计时
	clock_t end_time = clock();
	float elaptime = static_cast<float>(end_time - start_time) / CLOCKS_PER_SEC * 1000;

	CString strtime;
	strtime.Format("The elapsed time of Camera Image to Mat in basler: %.2f ms.\n", elaptime);
	OutputDebugString(strtime);

	//释放图像缓冲区buffer
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