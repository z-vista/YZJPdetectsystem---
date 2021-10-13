#include "stdafx.h"
#include "LoggerEx.h"
NetDataLog::NetDataLog(string strDir, string filename, int maxsize, int filecount, int timeFormat, EnumLogLevel nLogLevel)
{

	time_t curTime;
	struct tm* pTimeInfo = NULL;
	time(&curTime);
	pTimeInfo = localtime(&curTime);
	char temp[1024] = { 0 };
	//��־�������磺2013-01-01.log
	sprintf(temp, "\\%04d-%02d-%02d", pTimeInfo->tm_year + 1900, pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday);

	m_strDir = strDir+ temp;
	m_LogFileName = m_strDir + string("\\") + filename;
	m_MaxFileSize = maxsize;
	m_FileCount = filecount;
	m_timeFormat = timeFormat;
	//����Ĭ�ϵ�д��־����
	m_nLogLevel = EnumLogLevel::LogLevelNormal;
	//��ʼ���ٽ�������
	InitializeCriticalSection(&m_cs);
	//�ж���־Ŀ¼�Ƿ����
	if (!checkFolderExist(m_strDir.c_str()))
	{
		_mkdir(m_strDir.c_str());
	}
	m_outputFile = new fstream;
	string strname = m_LogFileName + ".txt";
	m_outputFile->open(strname, ofstream::out | ofstream::app);	//����־�ļ�
	bool b = m_outputFile->is_open();


}
NetDataLog::~NetDataLog()
{
	//�ͷ��ٽ���
	DeleteCriticalSection(&m_cs);
	if (m_outputFile)
		delete m_outputFile;
}
//********************************
//��������NetDataLog::checkFolderExist
//��  ��������Ŀ¼�Ƿ����
//��  ����strPath Ŀ¼��
//����ֵ�����ڷ�����
//*************************************
bool NetDataLog::checkFolderExist(const string& strPath)
{
	if (_access(strPath.data(), 0) == 0)
		return true;
	else
		return false;
}
//д�ؼ���Ϣ�ӿ�
void NetDataLog::TraceKeyInfo(const char* strInfo, ...)
{
	if (!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = { 0 };
	//strcpy_s(pTemp, GetCurrentTime());
	strcat_s(pTemp, KEYINFOPREFIX);
	//��ȡ�ɱ��β�
	va_list arg_ptr = NULL;
	va_start(arg_ptr, strInfo);
	vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
	//	vsprintf_s(pTemp, strlen(pTemp), strInfo, arg_ptr);
	va_end(arg_ptr);
	//д��־�ļ�
	Trace(pTemp);
	arg_ptr = NULL;

}
//д������Ϣ
void NetDataLog::TraceError(const char* strInfo, ...)
{
	//�жϵ�ǰ��д��־����������Ϊ��д��־��������
	if (m_nLogLevel >= EnumLogLevel::LogLevelStop)
		return;
	if (!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = { 0 };
	//strcpy(pTemp, GetCurrentTime());
	strcat(pTemp, ERRORPREFIX);
	va_list arg_ptr = NULL;
	va_start(arg_ptr, strInfo);
	vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
	//vsprintf_s(pTemp, strlen(pTemp), strInfo, arg_ptr);
	va_end(arg_ptr);
	Trace(pTemp);
	arg_ptr = NULL;
}

//д������Ϣ
void NetDataLog::TraceWarning(const char* strInfo, ...)
{
	//�жϵ�ǰ��д��־����������Ϊֻд������Ϣ��������
	if (m_nLogLevel >= EnumLogLevel::LogLevelNormal)
		return;
	if (!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = { 0 };
	//strcpy(pTemp, GetCurrentTime());
	strcat(pTemp, WARNINGPREFIX);
	va_list arg_ptr = NULL;
	va_start(arg_ptr, strInfo);
	vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
	//vsprintf_s(pTemp, strlen(pTemp), strInfo, arg_ptr);
	va_end(arg_ptr);
	Trace(pTemp);
	arg_ptr = NULL;
}


//дһ����Ϣ
void NetDataLog::TraceInfo(const char* strInfo, ...)
{
	//�жϵ�ǰ��д��־����������ֻд����;�����Ϣ��������
	if (m_nLogLevel >= EnumLogLevel::LogLevelMid)
		return;
	if (!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = { 0 };
	//strcpy(pTemp, GetCurrentTime());
	strcat(pTemp, INFOPREFIX);
	va_list arg_ptr = NULL;
	va_start(arg_ptr, strInfo);
	vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
	//vsprintf_s(pTemp, strlen(pTemp), strInfo, arg_ptr);
	va_end(arg_ptr);
	Trace(pTemp);
	arg_ptr = NULL;
}
//********************************
//��������NetDataLog::Trace
//��  �������ļ��������־��Ϣ
//��  �� log Ϊ��Ϣ����
//����ֵ��void
//*************************************
void NetDataLog::Trace(string log)
{
	string currentTime = getCurrentTime(); //��ȡ����ʱ��
	try
	{
		
		//�����ٽ���
		EnterCriticalSection(&m_cs);
		//�ж��ļ���С
		fileSizeLimit();
		if (m_timeFormat == NETLOG)
			*m_outputFile << "[" << currentTime << "] " << log << endl;
		else
			*m_outputFile << currentTime << " " << log << endl;
		//�뿪�ٽ���
		LeaveCriticalSection(&m_cs);
	}//�������쳣�������뿪�ٽ�������ֹ����
	catch (...)
	{
		LeaveCriticalSection(&m_cs);
	}
	
}
//********************************
//��������NetDataLog::fileSizeLimit
//��  �����ж��ļ���С�Ƿ�ﵽ���ֵ
//��  ������
//����ֵ��void
//*************************************
void NetDataLog::fileSizeLimit()
{
	int filesize = getCurrentLogFileSize();
	if (filesize >= m_MaxFileSize * 1024)
		fileOffset();

}
//********************************
//��������NetDataLog::fileOffset
//��  ����ʵ���ļ�����ƫ��
//��  ������
//����ֵ��void
//*************************************
void NetDataLog::fileOffset()
{
	m_outputFile->close();	//�رյ�ǰ�ļ�
	char filename[100] = { 0 };
	char newfilename[100] = { 0 };
	for (int i = m_FileCount - 1; i > 0; i--)
	{
		memset(filename, 0, 100);
		sprintf(filename, "%s%d.txt", m_LogFileName.data(), i);
		if (checkFolderExist(filename))  //����
		{
			if (i == m_FileCount - 1)
			{
				remove(filename);//ɾ���ļ�
				continue;
			}
			//�ļ���������ƫ��
			memset(newfilename, 0, 100);
			sprintf(newfilename, "%s%d.txt", m_LogFileName.data(), i + 1);
			rename(filename, newfilename);
		}
	}
	memset(filename, 0, 100);
	sprintf(filename, "%s.txt", m_LogFileName.data());
	sprintf(newfilename, "%s%d.txt", m_LogFileName.data(), 1);
	rename(filename, newfilename);
	m_outputFile->open(filename, ofstream::out | ofstream::app);	//����־�ļ�
}

//********************************
//��������NetDataLog::getCurrentLogFileSize
//��  �������㵱ǰ�ռ��ļ��Ĵ�С
//��  ������
//����ֵ���ļ���С��KB��
//*************************************
int NetDataLog::getCurrentLogFileSize()
{
	long long filepos = m_outputFile->tellp(); //���浱ǰ�ļ�λ��
	m_outputFile->seekp(0, ios_base::end);			//�ƶ����ļ�β
	long long filesize = m_outputFile->tellp();
	m_outputFile->seekp(filepos, ios_base::beg);		//�ָ��ļ�λ��
	return filesize / 1024;

}


//����д��־����
void NetDataLog::SetLogLevel(EnumLogLevel nLevel)
{
	m_nLogLevel = nLevel;
}

//��ȡ�ļ���
string NetDataLog::getLogFileName()
{
	return m_LogFileName + ".txt";
}
//�����ļ�����
void NetDataLog::setFileCount(int count)
{
	m_FileCount = count;
}
//�����ļ���
void NetDataLog::setFileName(string filename)
{
	m_LogFileName = m_strDir + string("\\") + filename;
}
//�����ļ���С
void NetDataLog::setMaxFileSize(int maxsize)
{
	m_MaxFileSize = maxsize;
}

//********************************
//��������NetDataLog::getCurrentTime
//��  ������ȡ����ʱ��
//����ֵ��ʱ���ַ���
//*************************************
string NetDataLog::getCurrentTime()
{
	time_t seconds = time(NULL);	//��ȡʱ��
	struct tm* p;
	p = localtime(&seconds);//��ȡ����ʱ��
	char strTime[100] = { 0 };
	if (m_timeFormat == NETLOG)
		sprintf(strTime, "%d\\%d\\%d %d.%d.%d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	else
		sprintf(strTime, "%02d-%02d %02d:%02d:%02d", 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	return string(strTime);
}

