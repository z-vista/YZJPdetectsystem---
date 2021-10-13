
#pragma once
#include <stdio.h>
#include<windows.h>
#pragma comment(lib, "Ws2_32.lib")

class M_Client
{
public:
	//Constructor
	M_Client();

	void Initial(const char* Addr, int Port, int Id);
	//连接
	int Connect();
	//发送TCP包
	int SendMsg(const char* msg, int len);
	//关闭
	void Close();
	//发送modbus包-单寄存器
	void Modbus_sender_single(int Ref, int addr, int value);
	//发送modbus包-多寄存器
	int Modbus_sender_multi(int Ref, int addr, int value, int value1, int value2, int value3, int value4, int IsPaper1, int IsPaper2, int IsPaper3, int IsPaper4, int value5, int value6);
	int Modbus_sender_multi2(int Ref, int addr, int value, int value1, int value2, int value3, int value4, int IsPaper1, int IsPaper2, int value5, int value6);

	//接收TCP包
	int RecvMsg(char* msg, int len);
	//接收modbus包-单寄存器
	void Modbus_recv_single(int Ref, int addr, int value);

private:
	SOCKET m_sock;
	int port;
	const char* address;
	int id;
};