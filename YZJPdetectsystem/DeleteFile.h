#pragma once

#ifndef DeleteFile_H_
#define DeleteFile_H_


#include <afx.h>  
#include <iostream>  
#include <string>  
#include <io.h>  
#include<vector>
using namespace std;

class myDeleteFile
{
public:
	myDeleteFile();
	~myDeleteFile();

	int deleteFile(string &path, int retainDays);
	int deleteFileFolder(string path, int retainDays);

private:

	bool RemoveDir(const char* szFileDir);

	int  getFiles(const char *to_search, vector<string> & files);

	void split_number(string str, string delimit, vector<int>&result);

	bool isLeapYear(int year);
	int getDays(int year, int month, int day);
	int dayDis(int year1, int month1, int day1,
		int year2, int month2, int day2);
};

SYSTEMTIME GetFolderCreateTime(CString sFolder);
bool GetFileModifyDate(CString filePathName, SYSTEMTIME& modDate);
time_t TimeConvertToSec(int year, int month, int day);
int DaysOffset(int fYear, int fMonth, int fDay, int tYear, int tMonth, int tDay);
bool DeleteDirectory(CString directory_path);
void DeleteAllFile(CString strDir, int days);
#endif

