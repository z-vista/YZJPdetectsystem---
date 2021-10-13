
#ifndef NETDATALOG_H
#define NETDATALOG_H
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <direct.h>
#include <string>
#include <io.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include "CommandDefine.h"
using namespace std;
enum TIMEFORMAT
{
	NETLOG = 0,	//	[yyyy\mm\dd hh.MM.ss]
	LOGINLOG = 1,	//	mm-dd hh:MM:ss
};

class NetDataLog
{
public:
	NetDataLog(string strDir = "log", string filename = "record", int maxfilesize = 0, int filecount = 0, int timeformat = 0, EnumLogLevel nLogLevel = EnumLogLevel::LogLevelNormal);
	~NetDataLog();
	
	void fileSizeLimit();		//判断文件大小是否达到限定值
	int getCurrentLogFileSize();//获取当前日志文件的大小
	string getLogFileName();	//获取日志文件名称
	void setMaxFileSize(int);//设置文件最大大小
	void setFileName(string); //设置日志文件名
	void setFileCount(int);	//设置日志文件的个数
	//void setLogDir(string strDir);		//设置日志文件目录
	void Trace(string log);	//添加日志记录到日志文件
	//写关键信息
	void TraceKeyInfo(const char* strInfo, ...);
	//写错误信息
	void TraceError(const char* strInfo, ...);//LogLevelStop
	//写警告信息
	void TraceWarning(const char* strInfo, ...);//LogLevelNormal
	//写一般信息
	void TraceInfo(const char* strInfo, ...);//LogLevelMid
	//设置写日志级别
	void SetLogLevel(EnumLogLevel nLevel);
private:
	void fileOffset();		//文件名称进行偏移
	bool checkFolderExist(const string& strPath);
	string getCurrentTime();


private:
	string m_LogFileName;		//文件名
	int m_MaxFileSize;			//文件大小
	int m_FileCount;			//文件个数
	fstream* m_outputFile;		//输出文件流
	string m_strDir;			//目录
	int m_timeFormat;
	EnumLogLevel m_nLogLevel;	//写日志级别
	//线程同步的临界区变量
	CRITICAL_SECTION m_cs;
};
#endif