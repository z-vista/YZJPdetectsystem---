#include "stdafx.h"
#include "LoggerEx.h"
NetDataLog::NetDataLog(string strDir, string filename, int maxsize, int filecount, int timeFormat, EnumLogLevel nLogLevel)
{

	time_t curTime;
	struct tm* pTimeInfo = NULL;
	time(&curTime);
	pTimeInfo = localtime(&curTime);
	char temp[1024] = { 0 };
	//日志的名称如：2013-01-01.log
	sprintf(temp, "\\%04d-%02d-%02d", pTimeInfo->tm_year + 1900, pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday);

	m_strDir = strDir+ temp;
	m_LogFileName = m_strDir + string("\\") + filename;
	m_MaxFileSize = maxsize;
	m_FileCount = filecount;
	m_timeFormat = timeFormat;
	//设置默认的写日志级别
	m_nLogLevel = EnumLogLevel::LogLevelNormal;
	//初始化临界区变量
	InitializeCriticalSection(&m_cs);
	//判断日志目录是否存在
	if (!checkFolderExist(m_strDir.c_str()))
	{
		_mkdir(m_strDir.c_str());
	}
	m_outputFile = new fstream;
	string strname = m_LogFileName + ".txt";
	m_outputFile->open(strname, ofstream::out | ofstream::app);	//打开日志文件
	bool b = m_outputFile->is_open();


}
NetDataLog::~NetDataLog()
{
	//释放临界区
	DeleteCriticalSection(&m_cs);
	if (m_outputFile)
		delete m_outputFile;
}
//********************************
//函数名：NetDataLog::checkFolderExist
//描  述：测试目录是否存在
//参  数：strPath 目录名
//返回值：存在返回真
//*************************************
bool NetDataLog::checkFolderExist(const string& strPath)
{
	if (_access(strPath.data(), 0) == 0)
		return true;
	else
		return false;
}
//写关键信息接口
void NetDataLog::TraceKeyInfo(const char* strInfo, ...)
{
	if (!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = { 0 };
	//strcpy_s(pTemp, GetCurrentTime());
	strcat_s(pTemp, KEYINFOPREFIX);
	//获取可变形参
	va_list arg_ptr = NULL;
	va_start(arg_ptr, strInfo);
	vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
	//	vsprintf_s(pTemp, strlen(pTemp), strInfo, arg_ptr);
	va_end(arg_ptr);
	//写日志文件
	Trace(pTemp);
	arg_ptr = NULL;

}
//写错误信息
void NetDataLog::TraceError(const char* strInfo, ...)
{
	//判断当前的写日志级别，若设置为不写日志则函数返回
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

//写警告信息
void NetDataLog::TraceWarning(const char* strInfo, ...)
{
	//判断当前的写日志级别，若设置为只写错误信息则函数返回
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


//写一般信息
void NetDataLog::TraceInfo(const char* strInfo, ...)
{
	//判断当前的写日志级别，若设置只写错误和警告信息则函数返回
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
//函数名：NetDataLog::Trace
//描  述：向文件中添加日志信息
//参  数 log 为信息内容
//返回值：void
//*************************************
void NetDataLog::Trace(string log)
{
	string currentTime = getCurrentTime(); //获取本地时间
	try
	{
		
		//进入临界区
		EnterCriticalSection(&m_cs);
		//判断文件大小
		fileSizeLimit();
		if (m_timeFormat == NETLOG)
			*m_outputFile << "[" << currentTime << "] " << log << endl;
		else
			*m_outputFile << currentTime << " " << log << endl;
		//离开临界区
		LeaveCriticalSection(&m_cs);
	}//若发生异常，则先离开临界区，防止死锁
	catch (...)
	{
		LeaveCriticalSection(&m_cs);
	}
	
}
//********************************
//函数名：NetDataLog::fileSizeLimit
//描  述：判断文件大小是否达到最大值
//参  数：无
//返回值：void
//*************************************
void NetDataLog::fileSizeLimit()
{
	int filesize = getCurrentLogFileSize();
	if (filesize >= m_MaxFileSize * 1024)
		fileOffset();

}
//********************************
//函数名：NetDataLog::fileOffset
//描  述：实现文件名的偏移
//参  数：无
//返回值：void
//*************************************
void NetDataLog::fileOffset()
{
	m_outputFile->close();	//关闭当前文件
	char filename[100] = { 0 };
	char newfilename[100] = { 0 };
	for (int i = m_FileCount - 1; i > 0; i--)
	{
		memset(filename, 0, 100);
		sprintf(filename, "%s%d.txt", m_LogFileName.data(), i);
		if (checkFolderExist(filename))  //存在
		{
			if (i == m_FileCount - 1)
			{
				remove(filename);//删除文件
				continue;
			}
			//文件名序号向后偏移
			memset(newfilename, 0, 100);
			sprintf(newfilename, "%s%d.txt", m_LogFileName.data(), i + 1);
			rename(filename, newfilename);
		}
	}
	memset(filename, 0, 100);
	sprintf(filename, "%s.txt", m_LogFileName.data());
	sprintf(newfilename, "%s%d.txt", m_LogFileName.data(), 1);
	rename(filename, newfilename);
	m_outputFile->open(filename, ofstream::out | ofstream::app);	//打开日志文件
}

//********************************
//函数名：NetDataLog::getCurrentLogFileSize
//描  述：计算当前日记文件的大小
//参  数：无
//返回值：文件大小（KB）
//*************************************
int NetDataLog::getCurrentLogFileSize()
{
	long long filepos = m_outputFile->tellp(); //保存当前文件位置
	m_outputFile->seekp(0, ios_base::end);			//移动到文件尾
	long long filesize = m_outputFile->tellp();
	m_outputFile->seekp(filepos, ios_base::beg);		//恢复文件位置
	return filesize / 1024;

}


//设置写日志级别
void NetDataLog::SetLogLevel(EnumLogLevel nLevel)
{
	m_nLogLevel = nLevel;
}

//获取文件名
string NetDataLog::getLogFileName()
{
	return m_LogFileName + ".txt";
}
//设置文件个数
void NetDataLog::setFileCount(int count)
{
	m_FileCount = count;
}
//设置文件名
void NetDataLog::setFileName(string filename)
{
	m_LogFileName = m_strDir + string("\\") + filename;
}
//设置文件大小
void NetDataLog::setMaxFileSize(int maxsize)
{
	m_MaxFileSize = maxsize;
}

//********************************
//函数名：NetDataLog::getCurrentTime
//描  述：获取本地时间
//返回值：时间字符串
//*************************************
string NetDataLog::getCurrentTime()
{
	time_t seconds = time(NULL);	//获取时间
	struct tm* p;
	p = localtime(&seconds);//获取本地时间
	char strTime[100] = { 0 };
	if (m_timeFormat == NETLOG)
		sprintf(strTime, "%d\\%d\\%d %d.%d.%d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	else
		sprintf(strTime, "%02d-%02d %02d:%02d:%02d", 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	return string(strTime);
}

