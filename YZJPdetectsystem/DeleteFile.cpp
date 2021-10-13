
#include "stdafx.h"
#include"DeleteFile.h"

myDeleteFile::myDeleteFile()
{
}

myDeleteFile::~myDeleteFile()
{
}

//删除文件夹函数
bool myDeleteFile::RemoveDir(const char* szFileDir)

{
	std::string strDir = szFileDir;
	if (strDir.at(strDir.length() - 1) != '\\')
		strDir += '\\';
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile((strDir + "*.*").c_str(), &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	do
	{
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (_stricmp(wfd.cFileName, ".") != 0 &&
				_stricmp(wfd.cFileName, "..") != 0)
				RemoveDir((strDir + wfd.cFileName).c_str());
		}
		else
		{
			DeleteFile((strDir + wfd.cFileName).c_str());
		}
	} while (FindNextFile(hFind, &wfd));
	FindClose(hFind);
	RemoveDirectory(szFileDir);
	return true;
}
//计算闰年
bool myDeleteFile::isLeapYear(int year)
{
	return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}
// 以公元 1 年 1 月 1 日为基准，计算经过的日期 
int myDeleteFile::getDays(int year, int month, int day)
{
	int m[] = { 0,31,28,31,30,31,30,31,31,30,31,30,31 };
	if (isLeapYear(year))
		m[2]++;
	int result = 0;
	for (int i = 1; i < year; i++)
	{
		result += 365;
		if (isLeapYear(i))
			result++;
	}
	for (int i = 1; i < month; i++)
	{
		result += m[i];
	}
	result += day;

	return result;
}
int myDeleteFile::dayDis(int year1, int month1, int day1,
	int year2, int month2, int day2)
{
	return abs(getDays(year2, month2, day2) - getDays(year1, month1, day1));
}

//获取文件夹下的所有文件名
int  myDeleteFile::getFiles(const char *to_search, vector<string> & files)
{
	long handle = -1;                                                //用于查找的句柄
	struct _finddata_t fileinfo;  //文件信息的结构体
	//const char *to_search = "E:\\Pictures\\*.jpg";
	handle = _findfirst(to_search, &fileinfo);         //第一次查找
	if (-1 == handle)
		return -1;
	files.push_back(fileinfo.name);
	while (!_findnext(handle, &fileinfo))               //循环查找其他符合的文件，知道找不到其他的为止
	{
		files.push_back(fileinfo.name);
	}
	_findclose(handle);
}

//将字符串按照特殊字符分割并转换成数字
void myDeleteFile::split_number(string str, string delimit, vector<int>&result) {
	size_t pos = str.find(delimit);
	str += delimit;
	while (pos != string::npos) {
		string temp = str.substr(0, pos);
		int n = atoi(temp.c_str());//atoi可以将字符串"1234"转换成int类型1234
		result.push_back(n);
		str = str.substr(pos + 1);
		pos = str.find(delimit);
	}
}
//按条件删除文件
int myDeleteFile::deleteFile(string &path, int retainDays)
{
	//删除单个文件
	vector<string> filenames;
	//string path = "E:\\03-myProgram\\ConsoleApp_DeleteFile\\ConsoleApp_DeleteFile\\log\\";
	//const char *to_search = "E:\\03-myProgram\\ConsoleApp_DeleteFile\\ConsoleApp_DeleteFile\\log\\*.log";//获取某类型的文件，本例程中是获取txt文件
	string pathname = path + "*.log";//获取某类型的文件，本例程中是获取log文件
	const char *to_search = pathname.c_str();//获取某类型的文件，本例程中是获取log文件
	int vback=getFiles(to_search, filenames);
	if (-1== vback)
	{
		return 0;
	}
	int  size = filenames.size();
	//cout << "测试样本数：" << size << endl;

	//获取当前系统时间
	time_t curTime;
	struct tm * pTimeInfo = NULL;
	time(&curTime);
	pTimeInfo = localtime(&curTime);

	int icout = 0;
	for (int i = 0; i < size; i++)
	{
		//cout << path + filenames[i] << endl;
		string filename = filenames[i].substr(0, filenames[i].rfind("."));//获取不带文件路径和后缀的文件名
		//cout << "filename =" << filename << endl;

		string fullpath = path + filenames[i];

		vector<int> data_split_num;
		split_number(filename, "-", data_split_num);
		int dufday = dayDis(data_split_num[0], data_split_num[1], data_split_num[2], pTimeInfo->tm_year + 1900, pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday);
		//std::cout << "第" << i + 1 << "行,相差天数：" << dufday << endl;
		if (dufday > retainDays)
		{
			remove(fullpath.c_str());//删除单个文件
			icout++;
		}
	}
	return icout;
}
//按条件删除文件夹
int myDeleteFile::deleteFileFolder(string path,int retainDays)
{
	//删除文件夹
	long hFile = 0;
	struct _finddata_t fileInfo;
	string pathName;
	// \\* 代表要遍历所有的类型,如改成\\*.jpg表示遍历jpg类型文件
	if ((hFile = _findfirst(pathName.assign(path).append("*.*").c_str(), &fileInfo)) == -1) {
		return 0;
	}
	//获取当前系统时间
	time_t curTime;
	struct tm * pTimeInfo = NULL;
	time(&curTime);
	pTimeInfo = localtime(&curTime);

	int i = 0;
	while (_findnext(hFile, &fileInfo) == 0)
	{
		vector<int> data_split_num;
		split_number(fileInfo.name, "-", data_split_num);
		if (data_split_num.empty())
		{
			continue;
		}

		int dufday = dayDis(data_split_num[0], data_split_num[1], data_split_num[2], pTimeInfo->tm_year + 1900, pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday);
	
		string strFilePath = path  + fileInfo.name;
		if (dufday > retainDays)
		{
			RemoveDir(strFilePath.c_str());//删除文件夹
			i++;
		}
	
	}
	_findclose(hFile);
	return i;
}




//获取文件夹创建时间
SYSTEMTIME GetFolderCreateTime(CString sFolder)
{
	SYSTEMTIME screatetime;
	HANDLE hDir;
	hDir = CreateFile(sFolder, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
		OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);//打开现存目录 只读方式打开即可

	FILETIME lpCreateTime, lpAccessTime, lpWriteTime;
	if (GetFileTime(hDir, &lpCreateTime, &lpAccessTime, &lpWriteTime))
	{
		FILETIME fcreatetime;
		FileTimeToLocalFileTime(&lpCreateTime, &fcreatetime);//转换为本地时间
		FileTimeToSystemTime(&fcreatetime, &screatetime);//转换为系统时间
	}

	CloseHandle(hDir);//关闭文件句柄

	return screatetime;
}
//获取文件最后修改日期
bool GetFileModifyDate(CString filePathName, SYSTEMTIME& modDate)
{
	HANDLE hFile;
	WIN32_FIND_DATA wfd;
	SYSTEMTIME systime;
	FILETIME localtime;

	memset(&wfd, 0, sizeof(wfd));
	if ((hFile = FindFirstFile(filePathName, &wfd)) == INVALID_HANDLE_VALUE)
		return false;
	//转换时间  
	FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &localtime);
	FileTimeToSystemTime(&localtime, &systime);

	modDate = systime;
	return true;
}
//转换时间秒数
time_t TimeConvertToSec(int year, int month, int day)
{
	tm info = { 0 };
	info.tm_year = year - 1900;
	info.tm_mon = month - 1;
	info.tm_mday = day;
	return mktime(&info);
}
//计算两个日期相差天数
int DaysOffset(int fYear, int fMonth, int fDay, int tYear, int tMonth, int tDay)
{
	int fromSecond = (int)TimeConvertToSec(fYear, fMonth, fDay);
	int toSecond = (int)TimeConvertToSec(tYear, tMonth, tDay);
	return (toSecond - fromSecond) / 24 / 3600;
}

//删除目录
bool DeleteDirectory(CString directory_path)
{
	CFileFind finder;
	CString path;
	path.Format(_T("%s\\*.*"), directory_path);
	BOOL bWorking = finder.FindFile(path);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDirectory() && !finder.IsDots())//处理文件夹
		{
			//递归删除文件夹
			DeleteDirectory(finder.GetFilePath());
			RemoveDirectory(finder.GetFilePath());

		}
		else//删除文件
		{
			DeleteFile(finder.GetFilePath());
		}
	}
	RemoveDirectory(directory_path);

	return true;
}

//删除指定文件夹目录中全部文件(包含文件夹) 
//目录如：D:\TEST
void DeleteAllFile(CString strDir, int days)
{
	if (days < 0)
		days = 30;
	SYSTEMTIME curDate;
	GetLocalTime(&curDate); //获取当前时间
	CFileFind finder;
	BOOL bFound = finder.FindFile(strDir + L"\\*", 0);
	while (bFound)
	{
		bFound = finder.FindNextFile();
		if (finder.GetFileName() == "." || finder.GetFileName() == "..")
			continue;
		//   去掉文件(夹)只读等属性
		SetFileAttributes(finder.GetFilePath(), FILE_ATTRIBUTE_NORMAL);
		if (finder.IsDirectory())
		{
			SYSTEMTIME fDate = GetFolderCreateTime(finder.GetFilePath());//获取文件创建时间
			int dayOffset = DaysOffset(fDate.wYear, fDate.wMonth, fDate.wDay,
				curDate.wYear, curDate.wMonth, curDate.wDay);
			if (dayOffset > days)
			{
				DeleteDirectory(finder.GetFilePath());
			}
		}
		else
		{
			SYSTEMTIME fDate;
			if (GetFileModifyDate(finder.GetFilePath(), fDate))//获取文件最后修改时间
			{
				int dayOffset = DaysOffset(fDate.wYear, fDate.wMonth, fDate.wDay,
					curDate.wYear, curDate.wMonth, curDate.wDay);
				if (dayOffset > days)
					DeleteFile(finder.GetFilePath());
			}
		}
	}
	finder.Close();
	//  然后删除该文件夹
   // RemoveDirectory(strDir);
}