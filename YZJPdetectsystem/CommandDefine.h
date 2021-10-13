//"CommandDefine.h"
#ifndef COMMAND_DEFINE_H
#define COMMAND_DEFINE_H
//日志级别的提示信息
static const char * KEYINFOPREFIX = " Key: ";
static const char * ERRORPREFIX = " Error: ";
static const char * WARNINGPREFIX = " Warning: ";
static const char * INFOPREFIX = " Info: ";

static const int MAX_STR_LEN = 1024;
//日志级别枚举
typedef enum EnumLogLevel
{
	LogLevelAll = 0,    //所有信息都写日志
	LogLevelMid,        //写错误、警告信息
	LogLevelNormal,     //只写错误信息
	LogLevelStop        //不写日志
};

#endif